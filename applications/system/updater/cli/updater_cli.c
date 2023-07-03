
#include <furi.h>
#include <furi_hal.h>
#include <cli/cli.h>
#include <storage/storage.h>
#include <loader/loader.h>
#include <toolbox/path.h>
#include <toolbox/tar/tar_archive.h>
#include <toolbox/args.h>
#include <update_util/update_manifest.h>
#include <update_util/lfs_backup.h>
#include <update_util/update_operation.h>

typedef void (*cmd_handler)(FuriString* args);
typedef struct {
    const char* command;
    const cmd_handler handler;
} CliSubcommand;

static void updater_cli_install(FuriString* manifest_path) {
    printf("Verifying update package at '%s'\r\n", furi_string_get_cstr(manifest_path));

    UpdatePrepareResult result = update_operation_prepare(furi_string_get_cstr(manifest_path));
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

static void updater_cli_backup(FuriString* args) {
    printf("Backup /int to '%s'\r\n", furi_string_get_cstr(args));
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool success = lfs_backup_create(storage, furi_string_get_cstr(args));
    furi_record_close(RECORD_STORAGE);
    printf("Result: %s\r\n", success ? "OK" : "FAIL");
}

static void updater_cli_restore(FuriString* args) {
    printf("Restore /int from '%s'\r\n", furi_string_get_cstr(args));
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool success = lfs_backup_unpack(storage, furi_string_get_cstr(args));
    furi_record_close(RECORD_STORAGE);
    printf("Result: %s\r\n", success ? "OK" : "FAIL");
}

static void updater_cli_help(FuriString* args) {
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

static void updater_cli_ep(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);
    FuriString* subcommand;
    subcommand = furi_string_alloc();
    if(!args_read_string_and_trim(args, subcommand) || furi_string_empty(args)) {
        updater_cli_help(args);
        furi_string_free(subcommand);
        return;
    }
    for(size_t idx = 0; idx < COUNT_OF(update_cli_subcommands); ++idx) {
        const CliSubcommand* subcmd_def = &update_cli_subcommands[idx];
        if(furi_string_cmp_str(subcommand, subcmd_def->command) == 0) {
            subcmd_def->handler(args);
            furi_string_free(subcommand);
            return;
        }
    }
    furi_string_free(subcommand);
    updater_cli_help(args);
}

static void updater_start_app(void* context, uint32_t arg) {
    UNUSED(context);
    UNUSED(arg);

    FuriHalRtcBootMode mode = furi_hal_rtc_get_boot_mode();
    if((mode != FuriHalRtcBootModePreUpdate) && (mode != FuriHalRtcBootModePostUpdate)) {
        return;
    }

    /* We need to spawn a separate thread, because these callbacks are executed 
     * inside loader process, at startup. 
     * So, accessing its record would cause a deadlock 
     */
    Loader* loader = furi_record_open(RECORD_LOADER);
    loader_start(loader, "UpdaterApp", NULL, NULL);
    furi_record_close(RECORD_LOADER);
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
    furi_timer_pending_callback(updater_start_app, NULL, 0);
#else
    UNUSED(updater_start_app);
#endif
}
