#include "input.h"

#include <furi.h>
#include <cli/cli.h>
#include <toolbox/args.h>

static void input_cli_usage(void) {
    printf("Usage:\r\n");
    printf("input <cmd> <args>\r\n");
    printf("Cmd list:\r\n");
    printf("\tdump\t\t\t - dump input events\r\n");
    printf("\tsend <key> <type>\t - send input event\r\n");
}

static void input_cli_dump_events_callback(const void* value, void* ctx) {
    furi_assert(value);
    furi_assert(ctx);
    FuriMessageQueue* input_queue = ctx;
    furi_message_queue_put(input_queue, value, FuriWaitForever);
}

static void input_cli_dump(Cli* cli, FuriString* args, FuriPubSub* event_pubsub) {
    UNUSED(args);
    FuriMessageQueue* input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    FuriPubSubSubscription* input_subscription =
        furi_pubsub_subscribe(event_pubsub, input_cli_dump_events_callback, input_queue);

    InputEvent input_event;
    printf("Press CTRL+C to stop\r\n");
    while(!cli_cmd_interrupt_received(cli)) {
        if(furi_message_queue_get(input_queue, &input_event, 100) == FuriStatusOk) {
            printf(
                "key: %s type: %s\r\n",
                input_get_key_name(input_event.key),
                input_get_type_name(input_event.type));
        }
    }

    furi_pubsub_unsubscribe(event_pubsub, input_subscription);
    furi_message_queue_free(input_queue);
}

static void input_cli_send_print_usage(void) {
    printf("Invalid arguments. Usage:\r\n");
    printf("\tinput send <key> <type>\r\n");
    printf("\t\t <key>\t - one of 'up', 'down', 'left', 'right', 'back', 'ok'\r\n");
    printf("\t\t <type>\t - one of 'press', 'release', 'short', 'long'\r\n");
}

static void input_cli_send(Cli* cli, FuriString* args, FuriPubSub* event_pubsub) {
    UNUSED(cli);
    InputEvent event;
    FuriString* key_str;
    key_str = furi_string_alloc();
    bool parsed = false;

    do {
        // Parse Key
        if(!args_read_string_and_trim(args, key_str)) {
            break;
        }
        if(!furi_string_cmp(key_str, "up")) {
            event.key = InputKeyUp;
        } else if(!furi_string_cmp(key_str, "down")) {
            event.key = InputKeyDown;
        } else if(!furi_string_cmp(key_str, "left")) {
            event.key = InputKeyLeft;
        } else if(!furi_string_cmp(key_str, "right")) {
            event.key = InputKeyRight;
        } else if(!furi_string_cmp(key_str, "ok")) {
            event.key = InputKeyOk;
        } else if(!furi_string_cmp(key_str, "back")) {
            event.key = InputKeyBack;
        } else {
            break;
        }
        // Parse Type
        if(!furi_string_cmp(args, "press")) {
            event.type = InputTypePress;
        } else if(!furi_string_cmp(args, "release")) {
            event.type = InputTypeRelease;
        } else if(!furi_string_cmp(args, "short")) {
            event.type = InputTypeShort;
        } else if(!furi_string_cmp(args, "long")) {
            event.type = InputTypeLong;
        } else {
            break;
        }
        parsed = true;
    } while(false);

    if(parsed) { //-V547
        furi_pubsub_publish(event_pubsub, &event);
    } else {
        input_cli_send_print_usage();
    }
    furi_string_free(key_str);
}

void input_cli(Cli* cli, FuriString* args, void* context) {
    furi_assert(cli);
    furi_assert(context);
    FuriPubSub* event_pubsub = context;
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            input_cli_usage();
            break;
        }
        if(furi_string_cmp_str(cmd, "dump") == 0) {
            input_cli_dump(cli, args, event_pubsub);
            break;
        }
        if(furi_string_cmp_str(cmd, "send") == 0) {
            input_cli_send(cli, args, event_pubsub);
            break;
        }

        input_cli_usage();
    } while(false);

    furi_string_free(cmd);
}
