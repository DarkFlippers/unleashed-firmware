#include <furi.h>
#include <cli/cli.h>
#include <applications.h>
#include <lib/toolbox/args.h>
#include "loader.h"

static void loader_cli_print_usage() {
    printf("Usage:\r\n");
    printf("loader <cmd> <args>\r\n");
    printf("Cmd list:\r\n");
    printf("\tlist\t - List available applications\r\n");
    printf("\topen <Application Name:string>\t - Open application by name\r\n");
    printf("\tinfo\t - Show loader state\r\n");
}

static void loader_cli_list() {
    printf("Apps:\r\n");
    for(size_t i = 0; i < FLIPPER_APPS_COUNT; i++) {
        printf("\t%s\r\n", FLIPPER_APPS[i].name);
    }
    printf("Settings:\r\n");
    for(size_t i = 0; i < FLIPPER_SETTINGS_APPS_COUNT; i++) {
        printf("\t%s\r\n", FLIPPER_SETTINGS_APPS[i].name);
    }
}

static void loader_cli_info(Loader* loader) {
    if(!loader_is_locked(loader)) {
        printf("No application is running\r\n");
    } else {
        // TODO FL-3513: print application name ???
        printf("Application is running\r\n");
    }
}

static void loader_cli_open(FuriString* args, Loader* loader) {
    FuriString* app_name = furi_string_alloc();

    do {
        if(!args_read_probably_quoted_string_and_trim(args, app_name)) {
            printf("No application provided\r\n");
            break;
        }
        furi_string_trim(args);

        const char* args_str = furi_string_get_cstr(args);
        if(strlen(args_str) == 0) {
            args_str = NULL;
        }

        const char* app_name_str = furi_string_get_cstr(app_name);

        FuriString* error_message = furi_string_alloc();
        if(loader_start(loader, app_name_str, args_str, error_message) != LoaderStatusOk) {
            printf("%s\r\n", furi_string_get_cstr(error_message));
        }
        furi_string_free(error_message);
    } while(false);

    furi_string_free(app_name);
}

static void loader_cli(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);
    Loader* loader = furi_record_open(RECORD_LOADER);

    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            loader_cli_print_usage();
            break;
        }

        if(furi_string_cmp_str(cmd, "list") == 0) {
            loader_cli_list();
            break;
        }

        if(furi_string_cmp_str(cmd, "open") == 0) {
            loader_cli_open(args, loader);
            break;
        }

        if(furi_string_cmp_str(cmd, "info") == 0) {
            loader_cli_info(loader);
            break;
        }

        loader_cli_print_usage();
    } while(false);

    furi_string_free(cmd);
    furi_record_close(RECORD_LOADER);
}

void loader_on_system_start() {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, RECORD_LOADER, CliCommandFlagParallelSafe, loader_cli, NULL);
    furi_record_close(RECORD_CLI);
#else
    UNUSED(loader_cli);
#endif
}