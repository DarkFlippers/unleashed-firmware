#include "nfc_cli_commands.h"
#include "nfc_cli_command_processor.h"

#include "applications/services/loader/loader.h"
#include "applications/services/cli/cli_main_commands.h"
#include <toolbox/cli/shell/cli_shell.h>
#include <toolbox/cli/cli_registry.h>

#define NFC_DESKTOP_APP_NAME "NFC"

#define TAG "NfcCli"

#define NFC_PROMPT "[" ANSI_FG_GREEN "nfc" ANSI_RESET "]"

typedef struct {
    Nfc* nfc;
    CliRegistry* registry;
    CliShell* shell;
    NfcCliProcessorContext* processor_context;
} NfcCliContext;

static void nfc_cli_shell_motd(void* context) {
    UNUSED(context);
    printf(ANSI_FG_BR_BLUE "\r\n"
                           "                                     0000      \r\n"
                           "                                     0000      \r\n"
                           "                             000      0000     \r\n"
                           "                             0000     00000    \r\n"
                           "                    000      00000     0000    \r\n"
                           "     0              0000      0000     00000   \r\n"
                           "   000000           0000      00000     0000   \r\n"
                           "   00000000          0000      0000     0000   \r\n"
                           "   0000000000        0000      00000    0000   \r\n"
                           "   0000 00000000     00000     00000    0000   \r\n"
                           "   0000    0000000   00000     00000    0000   \r\n"
                           "   0000      000000000000      0000     0000   \r\n"
                           "   00000        000000000     00000     0000   \r\n"
                           "     00           000000      0000     00000   \r\n"
                           "                     00      00000     0000    \r\n"
                           "                             0000     00000    \r\n"
                           "                             000      0000     \r\n"
                           "                                     0000      \r\n"
                           "                                     0005      \r\n"
                           "\r\n" ANSI_FG_BR_WHITE "Welcome to NFC Command Line Interface!\r\n"
                           "Run `help` or `?` to list available commands\r\n" ANSI_RESET);
}

static void nfc_cli_subscribe_commands(NfcCliContext* instance) {
    size_t cnt = nfc_cli_command_get_count();
    for(size_t i = 0; i < cnt; i++) {
        const NfcCliCommandDescriptor* cmd = nfc_cli_command_get_by_index(i);
        CliCommandExecuteCallback callback = nfc_cli_command_get_execute(cmd);
        if(callback == NULL) continue;
        const char* name = nfc_cli_command_get_name(cmd);
        cli_registry_add_command(
            instance->registry,
            name,
            CliCommandFlagParallelSafe,
            callback,
            instance->processor_context);
    }
}

static bool nfc_cli_desktop_app_is_running() {
    FuriString* app_name = furi_string_alloc();
    Loader* ldr = furi_record_open(RECORD_LOADER);
    bool result = false;

    if(loader_get_application_name(ldr, app_name)) {
        result = furi_string_equal_str(app_name, NFC_DESKTOP_APP_NAME);
    }

    furi_record_close(RECORD_LOADER);
    furi_string_free(app_name);
    return result;
}

static NfcCliContext* nfc_cli_alloc(PipeSide* pipe) {
    NfcCliContext* instance = malloc(sizeof(NfcCliContext));
    instance->nfc = nfc_alloc();
    instance->processor_context = nfc_cli_command_processor_alloc(instance->nfc);

    instance->registry = cli_registry_alloc();

    nfc_cli_subscribe_commands(instance);

    instance->shell =
        cli_shell_alloc(nfc_cli_shell_motd, instance, pipe, instance->registry, NULL);

    cli_shell_set_prompt(instance->shell, NFC_PROMPT);
    return instance;
}

void nfc_cli_free(NfcCliContext* instance) {
    furi_assert(instance);
    nfc_cli_command_processor_free(instance->processor_context);

    cli_shell_free(instance->shell);
    cli_registry_free(instance->registry);

    nfc_free(instance->nfc);
    free(instance);
}

void nfc_cli_execute(PipeSide* pipe, FuriString* args, void* context) {
    furi_assert(pipe);
    UNUSED(args);
    UNUSED(context);

    if(nfc_cli_desktop_app_is_running()) {
        printf(ANSI_FG_YELLOW
               "NFC app is running, unable to run NFC CLI at the same time!\r\n" ANSI_RESET);
        return;
    }

    NfcCliContext* instance = nfc_cli_alloc(pipe);

    cli_shell_start(instance->shell);
    cli_shell_join(instance->shell);

    nfc_cli_free(instance);
}

CLI_COMMAND_INTERFACE(nfc, nfc_cli_execute, CliCommandFlagParallelSafe, 1024, CLI_APPID);
