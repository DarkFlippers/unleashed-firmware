#include "../cli_main_commands.h"
#include <furi_hal.h>
#include <notification/notification_app.h>
#include <notification/notification_messages.h>
#include <toolbox/cli/cli_command.h>
#include <toolbox/pipe.h>

static void execute(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);

    if(!furi_string_cmp(args, "0")) {
        NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
        notification_message_block(notification, &sequence_reset_vibro);
        furi_record_close(RECORD_NOTIFICATION);
    } else if(!furi_string_cmp(args, "1")) {
        if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagStealthMode)) {
            printf("Flipper is in stealth mode. Unmute the device to control vibration.");
            return;
        }

        NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
        if(notification->settings.vibro_on) {
            notification_message_block(notification, &sequence_set_vibro_on);
        } else {
            printf("Vibro is disabled in settings. Enable it to control vibration.");
        }

        furi_record_close(RECORD_NOTIFICATION);
    } else {
        cli_print_usage("vibro", "<1|0>", furi_string_get_cstr(args));
    }
}

CLI_COMMAND_INTERFACE(vibro, execute, CliCommandFlagDefault, 4 * 1024, CLI_APPID);
