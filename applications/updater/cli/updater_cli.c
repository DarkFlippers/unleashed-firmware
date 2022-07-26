
#include <furi.h>
#include <furi_hal.h>
#include <m-string.h>
#include <cli/cli.h>
#include <storage/storage.h>
#include <loader/loader.h>
#include <toolbox/path.h>
#include <toolbox/tar/tar_archive.h>
#include <toolbox/args.h>
#include <update_util/update_manifest.h>
#include <update_util/lfs_backup.h>
#include <update_util/update_operation.h>

typedef void (*cmd_handler)(string_t args);
typedef struct {
    const char* command;
    const cmd_handler handler;
} CliSubcommand;

static void updater_cli_install(string_t manifest_path) {
    printf("Verifying update package at '%s'\r\n", string_get_cstr(manifest_path));

    UpdatePrepareResult result = update_operation_prepare(string_get_cstr(manifest_path));
    if(result != UpdatePrepareResultOK) {
        printf(
            "Error: %s. Stopping update.\r\n",
            update_operation_describe_preparation_result(result));
        return;
    }
    printf("OK.\r\nRestarting to apply update. BRB\r\n");
    furi_delay_ms(100);
    furi_hal_power_reset();
}

static void updater_cli_backup(string_t args) {
    printf("Backup /int to '%s'\r\n", string_get_cstr(args));
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool success = lfs_backup_create(storage, string_get_cstr(args));
    furi_record_close(RECORD_STORAGE);
    printf("Result: %s\r\n", success ? "OK" : "FAIL");
}

static void updater_cli_restore(string_t args) {
    printf("Restore /int from '%s'\r\n", string_get_cstr(args));
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool success = lfs_backup_unpack(storage, string_get_cstr(args));
    furi_record_close(RECORD_STORAGE);
    printf("Result: %s\r\n", success ? "OK" : "FAIL");
}

static void updater_cli_help(string_t args) {
    UNUSED(args);
    printf("Commands:\r\n"
           "\tinstall /ext/path/to/update.fuf - verify & apply update package\r\n"
           "\tbackup /ext/path/to/backup.tar - create internal storage backup\r\n"
           "\trestore /ext/path/to/backup.tar - restore internal storage backup\r\n");
}

static const CliSubcommand update_cli_subcommands[] = {
    {.command = "install", .handler = updater_cli_install},
    {.command = "backup", .handler = updater_cli_backup},
    {.command = "restore", .handler = updater_cli_restore},
    {.command = "help", .handler = updater_cli_help},
};

static void updater_cli_ep(Cli* cli, string_t args, void* context) {
    UNUSED(cli);
    UNUSED(context);
    string_t subcommand;
    string_init(subcommand);
    if(!args_read_string_and_trim(args, subcommand) || string_empty_p(args)) {
        updater_cli_help(args);
        string_clear(subcommand);
        return;
    }
    for(size_t idx = 0; idx < COUNT_OF(update_cli_subcommands); ++idx) {
        const CliSubcommand* subcmd_def = &update_cli_subcommands[idx];
        if(string_cmp_str(subcommand, subcmd_def->command) == 0) {
            string_clear(subcommand);
            subcmd_def->handler(args);
            return;
        }
    }
    string_clear(subcommand);
    updater_cli_help(args);
}

static int32_t updater_spawner_thread_worker(void* arg) {
    UNUSED(arg);
    Loader* loader = furi_record_open(RECORD_LOADER);
    loader_start(loader, "UpdaterApp", NULL);
    furi_record_close(RECORD_LOADER);
    return 0;
}

static void updater_spawner_thread_cleanup(FuriThreadState state, void* context) {
    FuriThread* thread = context;
    if(state == FuriThreadStateStopped) {
        furi_thread_free(thread);
    }
}

static void updater_start_app() {
    FuriHalRtcBootMode mode = furi_hal_rtc_get_boot_mode();
    if((mode != FuriHalRtcBootModePreUpdate) && (mode != FuriHalRtcBootModePostUpdate)) {
        return;
    }

    /* We need to spawn a separate thread, because these callbacks are executed 
     * inside loader process, at startup. 
     * So, accessing its record would cause a deadlock 
     */
    FuriThread* thread = furi_thread_alloc();

    furi_thread_set_name(thread, "UpdateAppSpawner");
    furi_thread_set_stack_size(thread, 768);
    furi_thread_set_callback(thread, updater_spawner_thread_worker);
    furi_thread_set_state_callback(thread, updater_spawner_thread_cleanup);
    furi_thread_set_state_context(thread, thread);
    furi_thread_start(thread);
}

void updater_on_system_start() {
#ifdef SRV_CLI
    Cli* cli = (Cli*)furi_record_open(RECORD_CLI);
    cli_add_command(cli, "update", CliCommandFlagDefault, updater_cli_ep, NULL);
    furi_record_close(RECORD_CLI);
#else
    UNUSED(updater_cli_ep);
#endif
#ifndef FURI_RAM_EXEC
    updater_start_app();
#else
    UNUSED(updater_start_app);
#endif
}
