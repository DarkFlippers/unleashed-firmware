#include "cli_shell.h"
#include "cli_shell_i.h"
#include "../cli_ansi.h"
#include "../cli_registry_i.h"
#include "../cli_command.h"
#include "cli_shell_line.h"
#include "cli_shell_completions.h"
#include <stdio.h>
#include <furi_hal_version.h>
#include <m-array.h>
#include <loader/loader.h>
#include <toolbox/pipe.h>
#include <flipper_application/plugins/plugin_manager.h>
#include <loader/firmware_api/firmware_api.h>
#include <storage/storage.h>

#define TAG "CliShell"

#define ANSI_TIMEOUT_MS 10

typedef enum {
    CliShellComponentCompletions,
    CliShellComponentLine,
    CliShellComponentMAX, //<! do not use
} CliShellComponent;

CliShellKeyComboSet* component_key_combo_sets[] = {
    [CliShellComponentCompletions] = &cli_shell_completions_key_combo_set,
    [CliShellComponentLine] = &cli_shell_line_key_combo_set,
};
static_assert(CliShellComponentMAX == COUNT_OF(component_key_combo_sets));

typedef enum {
    CliShellStorageEventMount = (1 << 0),
    CliShellStorageEventUnmount = (1 << 1),
} CliShellStorageEvent;

#define CliShellStorageEventAll (CliShellStorageEventMount | CliShellStorageEventUnmount)

typedef struct {
    Storage* storage;
    FuriPubSubSubscription* subscription;
    FuriEventFlag* event_flag;
} CliShellStorage;

struct CliShell {
    // Set and freed by external thread
    CliShellMotd motd;
    void* callback_context;
    PipeSide* pipe;
    CliRegistry* registry;
    const CliCommandExternalConfig* ext_config;
    FuriThread* thread;
    const char* prompt;

    // Set and freed by shell thread
    FuriEventLoop* event_loop;
    CliAnsiParser* ansi_parser;
    FuriEventLoopTimer* ansi_parsing_timer;
    CliShellStorage storage;
    void* components[CliShellComponentMAX];
};

typedef struct {
    CliRegistryCommand* command;
    PipeSide* pipe;
    FuriString* args;
} CliCommandThreadData;

static void cli_shell_data_available(PipeSide* pipe, void* context);
static void cli_shell_pipe_broken(PipeSide* pipe, void* context);

static void cli_shell_install_pipe(CliShell* cli_shell) {
    pipe_install_as_stdio(cli_shell->pipe);
    pipe_attach_to_event_loop(cli_shell->pipe, cli_shell->event_loop);
    pipe_set_callback_context(cli_shell->pipe, cli_shell);
    pipe_set_data_arrived_callback(cli_shell->pipe, cli_shell_data_available, 0);
    pipe_set_broken_callback(cli_shell->pipe, cli_shell_pipe_broken, 0);
}

static void cli_shell_detach_pipe(CliShell* cli_shell) {
    pipe_detach_from_event_loop(cli_shell->pipe);
    furi_thread_set_stdin_callback(NULL, NULL);
    furi_thread_set_stdout_callback(NULL, NULL);
}

// =================
// Built-in commands
// =================

void cli_command_reload_external(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    CliShell* shell = context;
    furi_check(shell->ext_config);
    cli_registry_reload_external_commands(shell->registry, shell->ext_config);
    printf("OK!");
}

void cli_command_help(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    CliShell* shell = context;
    CliRegistry* registry = shell->registry;

    const size_t columns = 3;

    printf("Available commands:\r\n" ANSI_FG_GREEN);
    cli_registry_lock(registry);
    CliCommandDict_t* commands = cli_registry_get_commands(registry);
    size_t commands_count = CliCommandDict_size(*commands);

    CliCommandDict_it_t iterator;
    CliCommandDict_it(iterator, *commands);
    for(size_t i = 0; i < commands_count; i++) {
        const CliCommandDict_itref_t* item = CliCommandDict_cref(iterator);
        printf("%-30s", furi_string_get_cstr(item->key));
        CliCommandDict_next(iterator);

        if(i % columns == columns - 1) printf("\r\n");
    }

    if(shell->ext_config)
        printf(
            ANSI_RESET
            "\r\nIf you added a new external command and can't see it above, run `reload_ext_cmds`");
    printf(ANSI_RESET "\r\nFind out more: https://docs.flipper.net/development/cli");

    cli_registry_unlock(registry);
}

void cli_command_exit(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    CliShell* shell = context;
    cli_shell_line_set_about_to_exit(shell->components[CliShellComponentLine]);
    furi_event_loop_stop(shell->event_loop);
}

// ==================
// Internal functions
// ==================

static int32_t cli_command_thread(void* context) {
    CliCommandThreadData* thread_data = context;
    if(!(thread_data->command->flags & CliCommandFlagDontAttachStdio))
        pipe_install_as_stdio(thread_data->pipe);

    thread_data->command->execute_callback(
        thread_data->pipe, thread_data->args, thread_data->command->context);

    fflush(stdout);
    return 0;
}

void cli_shell_execute_command(CliShell* cli_shell, FuriString* command) {
    // split command into command and args
    size_t space = furi_string_search_char(command, ' ');
    if(space == FURI_STRING_FAILURE) space = furi_string_size(command);
    FuriString* command_name = furi_string_alloc_set(command);
    furi_string_left(command_name, space);
    FuriString* args = furi_string_alloc_set(command);
    furi_string_right(args, space + 1);

    PluginManager* plugin_manager = NULL;
    Loader* loader = furi_record_open(RECORD_LOADER);
    bool loader_locked = false;
    CliRegistryCommand command_data;

    do {
        // find handler
        if(!cli_registry_get_command(cli_shell->registry, command_name, &command_data)) {
            printf(
                ANSI_FG_RED "could not find command `%s`, try `help`" ANSI_RESET,
                furi_string_get_cstr(command_name));
            break;
        }

        // load external command
        if(command_data.flags & CliCommandFlagExternal) {
            const CliCommandExternalConfig* ext_config = cli_shell->ext_config;
            plugin_manager = plugin_manager_alloc(
                ext_config->appid, CLI_PLUGIN_API_VERSION, firmware_api_interface);
            FuriString* path = furi_string_alloc_printf(
                "%s/%s%s.fal",
                ext_config->search_directory,
                ext_config->fal_prefix,
                furi_string_get_cstr(command_name));
            uint32_t plugin_cnt_last = plugin_manager_get_count(plugin_manager);
            PluginManagerError error =
                plugin_manager_load_single(plugin_manager, furi_string_get_cstr(path));
            furi_string_free(path);

            if(error != PluginManagerErrorNone) {
                printf(ANSI_FG_RED "failed to load external command" ANSI_RESET);
                break;
            }

            const CliCommandDescriptor* plugin =
                plugin_manager_get_ep(plugin_manager, plugin_cnt_last);
            furi_assert(plugin);
            furi_check(furi_string_cmp_str(command_name, plugin->name) == 0);
            command_data.execute_callback = plugin->execute_callback;
            command_data.flags = plugin->flags | CliCommandFlagExternal;
            command_data.stack_depth = plugin->stack_depth;

            // external commands have to run in an external thread
            furi_check(!(command_data.flags & CliCommandFlagUseShellThread));
        }

        // lock loader
        if(!(command_data.flags & CliCommandFlagParallelSafe)) {
            loader_locked = loader_lock(loader);
            if(!loader_locked) {
                printf(ANSI_FG_RED
                       "this command cannot be run while an application is open" ANSI_RESET);
                break;
            }
        }

        if(command_data.flags & CliCommandFlagUseShellThread) {
            // run command in this thread
            command_data.execute_callback(cli_shell->pipe, args, command_data.context);
        } else {
            // run command in separate thread
            cli_shell_detach_pipe(cli_shell);
            CliCommandThreadData thread_data = {
                .command = &command_data,
                .pipe = cli_shell->pipe,
                .args = args,
            };
            FuriThread* thread = furi_thread_alloc_ex(
                furi_string_get_cstr(command_name),
                command_data.stack_depth,
                cli_command_thread,
                &thread_data);
            furi_thread_start(thread);
            furi_thread_join(thread);
            furi_thread_free(thread);
            cli_shell_install_pipe(cli_shell);
        }
    } while(0);

    furi_string_free(command_name);
    furi_string_free(args);

    // unlock loader
    if(loader_locked) loader_unlock(loader);
    furi_record_close(RECORD_LOADER);

    // unload external command
    if(plugin_manager) plugin_manager_free(plugin_manager);
}

const char* cli_shell_get_prompt(CliShell* cli_shell) {
    return cli_shell->prompt;
}

// ==============
// Event handlers
// ==============

static void cli_shell_signal_storage_event(CliShell* cli_shell, CliShellStorageEvent event) {
    furi_check(!(furi_event_flag_set(cli_shell->storage.event_flag, event) & FuriFlagError));
}

static void cli_shell_storage_event(const void* message, void* context) {
    CliShell* cli_shell = context;
    const StorageEvent* event = message;

    if(event->type == StorageEventTypeCardMount) {
        cli_shell_signal_storage_event(cli_shell, CliShellStorageEventMount);
    } else if(event->type == StorageEventTypeCardUnmount) {
        cli_shell_signal_storage_event(cli_shell, CliShellStorageEventUnmount);
    }
}

static void cli_shell_storage_internal_event(FuriEventLoopObject* object, void* context) {
    CliShell* cli_shell = context;
    FuriEventFlag* event_flag = object;
    CliShellStorageEvent event =
        furi_event_flag_wait(event_flag, FuriFlagWaitAll, FuriFlagWaitAny, 0);
    furi_check(!(event & FuriFlagError));

    if(event & CliShellStorageEventUnmount) {
        cli_registry_remove_external_commands(cli_shell->registry);
    } else if(event & CliShellStorageEventMount) {
        cli_registry_reload_external_commands(cli_shell->registry, cli_shell->ext_config);
    } else {
        furi_crash();
    }
}

static void
    cli_shell_process_parser_result(CliShell* cli_shell, CliAnsiParserResult parse_result) {
    if(!parse_result.is_done) return;
    CliKeyCombo key_combo = parse_result.result;
    if(key_combo.key == CliKeyUnrecognized) return;

    for(size_t i = 0; i < CliShellComponentMAX; i++) { // -V1008
        CliShellKeyComboSet* set = component_key_combo_sets[i];
        void* component_context = cli_shell->components[i];

        for(size_t j = 0; j < set->count; j++) {
            if(set->records[j].combo.modifiers == key_combo.modifiers &&
               set->records[j].combo.key == key_combo.key)
                if(set->records[j].action(key_combo, component_context)) return;
        }

        if(set->fallback)
            if(set->fallback(key_combo, component_context)) return;
    }
}

static void cli_shell_pipe_broken(PipeSide* pipe, void* context) {
    // allow commands to be processed before we stop the shell
    if(pipe_bytes_available(pipe)) return;

    CliShell* cli_shell = context;
    furi_event_loop_stop(cli_shell->event_loop);
}

static void cli_shell_data_available(PipeSide* pipe, void* context) {
    UNUSED(pipe);
    CliShell* cli_shell = context;

    furi_event_loop_timer_start(cli_shell->ansi_parsing_timer, furi_ms_to_ticks(ANSI_TIMEOUT_MS));

    // process ANSI escape sequences
    int c = getchar();
    furi_assert(c >= 0);
    cli_shell_process_parser_result(cli_shell, cli_ansi_parser_feed(cli_shell->ansi_parser, c));
}

static void cli_shell_timer_expired(void* context) {
    CliShell* cli_shell = context;
    cli_shell_process_parser_result(
        cli_shell, cli_ansi_parser_feed_timeout(cli_shell->ansi_parser));
}

// ===========
// Thread code
// ===========

static void cli_shell_init(CliShell* shell) {
    cli_registry_add_command(
        shell->registry,
        "help",
        CliCommandFlagUseShellThread | CliCommandFlagParallelSafe,
        cli_command_help,
        shell);
    cli_registry_add_command(
        shell->registry,
        "?",
        CliCommandFlagUseShellThread | CliCommandFlagParallelSafe,
        cli_command_help,
        shell);
    cli_registry_add_command(
        shell->registry,
        "exit",
        CliCommandFlagUseShellThread | CliCommandFlagParallelSafe,
        cli_command_exit,
        shell);

    if(shell->ext_config) {
        cli_registry_add_command(
            shell->registry,
            "reload_ext_cmds",
            CliCommandFlagUseShellThread,
            cli_command_reload_external,
            shell);
        cli_registry_reload_external_commands(shell->registry, shell->ext_config);
    }

    shell->components[CliShellComponentLine] = cli_shell_line_alloc(shell);
    shell->components[CliShellComponentCompletions] = cli_shell_completions_alloc(
        shell->registry, shell, shell->components[CliShellComponentLine]);

    shell->ansi_parser = cli_ansi_parser_alloc();

    shell->event_loop = furi_event_loop_alloc();
    shell->ansi_parsing_timer = furi_event_loop_timer_alloc(
        shell->event_loop, cli_shell_timer_expired, FuriEventLoopTimerTypeOnce, shell);

    shell->storage.event_flag = furi_event_flag_alloc();
    furi_event_loop_subscribe_event_flag(
        shell->event_loop,
        shell->storage.event_flag,
        FuriEventLoopEventIn,
        cli_shell_storage_internal_event,
        shell);
    shell->storage.storage = furi_record_open(RECORD_STORAGE);
    shell->storage.subscription = furi_pubsub_subscribe(
        storage_get_pubsub(shell->storage.storage), cli_shell_storage_event, shell);

    cli_shell_install_pipe(shell);
}

static void cli_shell_deinit(CliShell* shell) {
    furi_pubsub_unsubscribe(
        storage_get_pubsub(shell->storage.storage), shell->storage.subscription);
    furi_record_close(RECORD_STORAGE);
    furi_event_loop_unsubscribe(shell->event_loop, shell->storage.event_flag);
    furi_event_flag_free(shell->storage.event_flag);

    cli_shell_completions_free(shell->components[CliShellComponentCompletions]);
    cli_shell_line_free(shell->components[CliShellComponentLine]);

    cli_shell_detach_pipe(shell);
    furi_event_loop_timer_free(shell->ansi_parsing_timer);
    furi_event_loop_free(shell->event_loop);
    cli_ansi_parser_free(shell->ansi_parser);
}

static int32_t cli_shell_thread(void* context) {
    CliShell* shell = context;

    // Sometimes, the other side closes the pipe even before our thread is started. Although the
    // rest of the code will eventually find this out if this check is removed, there's no point in
    // wasting time.
    if(pipe_state(shell->pipe) == PipeStateBroken) return 0;

    cli_shell_init(shell);
    FURI_LOG_D(TAG, "Started");

    shell->motd(shell->callback_context);
    cli_shell_line_prompt(shell->components[CliShellComponentLine]);

    furi_event_loop_run(shell->event_loop);

    FURI_LOG_D(TAG, "Stopped");
    cli_shell_deinit(shell);
    return 0;
}

// ==========
// Public API
// ==========

CliShell* cli_shell_alloc(
    CliShellMotd motd,
    void* context,
    PipeSide* pipe,
    CliRegistry* registry,
    const CliCommandExternalConfig* ext_config) {
    furi_check(motd);
    furi_check(pipe);
    furi_check(registry);

    CliShell* shell = malloc(sizeof(CliShell));
    *shell = (CliShell){
        .motd = motd,
        .callback_context = context,
        .pipe = pipe,
        .registry = registry,
        .ext_config = ext_config,
    };

    shell->thread =
        furi_thread_alloc_ex("CliShell", CLI_SHELL_STACK_SIZE, cli_shell_thread, shell);

    return shell;
}

void cli_shell_free(CliShell* shell) {
    furi_check(shell);
    furi_thread_free(shell->thread);
    free(shell);
}

void cli_shell_start(CliShell* shell) {
    furi_check(shell);
    furi_thread_start(shell->thread);
}

void cli_shell_join(CliShell* shell) {
    furi_check(shell);
    furi_thread_join(shell->thread);
}

void cli_shell_set_prompt(CliShell* shell, const char* prompt) {
    furi_check(shell);
    furi_check(furi_thread_get_state(shell->thread) == FuriThreadStateStopped);
    shell->prompt = prompt;
}
