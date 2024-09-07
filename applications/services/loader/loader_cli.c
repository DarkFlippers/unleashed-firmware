#include "loader.h"

#include <furi.h>
#include <cli/cli.h>
#include <applications.h>
#include <lib/toolbox/args.h>
#include <lib/toolbox/strint.h>
#include <notification/notification_messages.h>

static void loader_cli_print_usage(void) {
    printf("Usage:\r\n");
    printf("loader <cmd> <args>\r\n");
    printf("Cmd list:\r\n");
    printf("\tlist\t - List available applications\r\n");
    printf("\topen <Application Name:string>\t - Open application by name\r\n");
    printf("\tinfo\t - Show loader state\r\n");
    printf("\tclose\t - Close the current application\r\n");
    printf("\tsignal <signal:number> [arg:hex]\t - Send a signal with an optional argument\r\n");
}

static void loader_cli_list(void) {
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
    FuriString* app_name = furi_string_alloc();

    if(!loader_get_application_name(loader, app_name)) {
        printf("No application is running\r\n");
    } else {
        printf("Application \"%s\" is running\r\n", furi_string_get_cstr(app_name));
    }

    furi_string_free(app_name);
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
        } else {
#ifdef SRV_NOTIFICATION
            NotificationApp* notification_srv = furi_record_open(RECORD_NOTIFICATION);
            notification_message(notification_srv, &sequence_display_backlight_on);
            furi_record_close(RECORD_NOTIFICATION);
#endif
        }
        furi_string_free(error_message);
    } while(false);

    furi_string_free(app_name);
}

static void loader_cli_close(Loader* loader) {
    FuriString* app_name = furi_string_alloc();

    if(!loader_get_application_name(loader, app_name)) {
        printf("No application is running\r\n");
    } else if(!loader_signal(loader, FuriSignalExit, NULL)) {
        printf("Application \"%s\" has to be closed manually\r\n", furi_string_get_cstr(app_name));
    } else {
        printf("Application \"%s\" was closed\r\n", furi_string_get_cstr(app_name));
    }

    furi_string_free(app_name);
}

static void loader_cli_signal(FuriString* args, Loader* loader) {
    uint32_t signal;
    uint32_t arg = 0;
    StrintParseError parse_err = 0;
    char* args_cstr = (char*)furi_string_get_cstr(args);
    parse_err |= strint_to_uint32(args_cstr, &args_cstr, &signal, 10);
    parse_err |= strint_to_uint32(args_cstr, &args_cstr, &arg, 16);

    if(parse_err) {
        printf("Signal must be a decimal number\r\n");
    } else if(!loader_is_locked(loader)) {
        printf("No application is running\r\n");
    } else {
        const bool is_handled = loader_signal(loader, signal, (void*)arg);
        printf(
            "Signal %lu with argument 0x%p was %s\r\n",
            signal,
            (void*)arg,
            is_handled ? "handled" : "ignored");
    }
}

static void loader_cli(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);
    Loader* loader = furi_record_open(RECORD_LOADER);

    FuriString* cmd;
    cmd = furi_string_alloc();

    if(!args_read_string_and_trim(args, cmd)) {
        loader_cli_print_usage();
    } else if(furi_string_equal(cmd, "list")) {
        loader_cli_list();
    } else if(furi_string_equal(cmd, "open")) {
        loader_cli_open(args, loader);
    } else if(furi_string_equal(cmd, "info")) {
        loader_cli_info(loader);
    } else if(furi_string_equal(cmd, "close")) {
        loader_cli_close(loader);
    } else if(furi_string_equal(cmd, "signal")) {
        loader_cli_signal(args, loader);
    } else {
        loader_cli_print_usage();
    }

    furi_string_free(cmd);
    furi_record_close(RECORD_LOADER);
}

void loader_on_system_start(void) {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, RECORD_LOADER, CliCommandFlagParallelSafe, loader_cli, NULL);
    furi_record_close(RECORD_CLI);
#else
    UNUSED(loader_cli);
#endif
}
