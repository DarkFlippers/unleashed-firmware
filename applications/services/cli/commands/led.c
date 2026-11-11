#include "../cli_main_commands.h"
#include <notification/notification_app.h>
#include <notification/notification_messages.h>
#include <toolbox/cli/cli_command.h>
#include <toolbox/pipe.h>
#include <toolbox/strint.h>

static void execute(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);
    // Get first word as light name
    NotificationMessage notification_led_message;
    FuriString* light_name;
    light_name = furi_string_alloc();
    size_t ws = furi_string_search_char(args, ' ');
    if(ws == FURI_STRING_FAILURE) {
        cli_print_usage("led", "<r|g|b|bl> <0-255>", furi_string_get_cstr(args));
        furi_string_free(light_name);
        return;
    } else {
        furi_string_set_n(light_name, args, 0, ws);
        furi_string_right(args, ws);
        furi_string_trim(args);
    }
    // Check light name
    if(!furi_string_cmp(light_name, "r")) {
        notification_led_message.type = NotificationMessageTypeLedRed;
    } else if(!furi_string_cmp(light_name, "g")) {
        notification_led_message.type = NotificationMessageTypeLedGreen;
    } else if(!furi_string_cmp(light_name, "b")) {
        notification_led_message.type = NotificationMessageTypeLedBlue;
    } else if(!furi_string_cmp(light_name, "bl")) {
        notification_led_message.type = NotificationMessageTypeLedDisplayBacklight;
    } else {
        cli_print_usage("led", "<r|g|b|bl> <0-255>", furi_string_get_cstr(args));
        furi_string_free(light_name);
        return;
    }
    furi_string_free(light_name);
    // Read light value from the rest of the string
    uint32_t value;
    if(strint_to_uint32(furi_string_get_cstr(args), NULL, &value, 0) != StrintParseNoError ||
       value >= 256) {
        cli_print_usage("led", "<r|g|b|bl> <0-255>", furi_string_get_cstr(args));
        return;
    }

    // Set led value
    notification_led_message.data.led.value = value;

    // Form notification sequence
    const NotificationSequence notification_sequence = {
        &notification_led_message,
        NULL,
    };

    // Send notification
    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
    notification_internal_message_block(notification, &notification_sequence);
    furi_record_close(RECORD_NOTIFICATION);
}

CLI_COMMAND_INTERFACE(led, execute, CliCommandFlagDefault, 4 * 1024, CLI_APPID);
