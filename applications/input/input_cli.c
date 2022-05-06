#include "input_i.h"

#include <furi.h>
#include <cli/cli.h>
#include <toolbox/args.h>

static void input_cli_usage() {
    printf("Usage:\r\n");
    printf("input <cmd> <args>\r\n");
    printf("Cmd list:\r\n");
    printf("\tdump\t\t\t - dump input events\r\n");
    printf("\tsend <key> <type>\t - send input event\r\n");
}

static void input_cli_dump_events_callback(const void* value, void* ctx) {
    furi_assert(value);
    furi_assert(ctx);
    osMessageQueueId_t input_queue = ctx;
    osMessageQueuePut(input_queue, value, 0, osWaitForever);
}

static void input_cli_dump(Cli* cli, string_t args, Input* input) {
    UNUSED(args);
    osMessageQueueId_t input_queue = osMessageQueueNew(8, sizeof(InputEvent), NULL);
    FuriPubSubSubscription* input_subscription =
        furi_pubsub_subscribe(input->event_pubsub, input_cli_dump_events_callback, input_queue);

    InputEvent input_event;
    printf("Press CTRL+C to stop\r\n");
    while(!cli_cmd_interrupt_received(cli)) {
        if(osMessageQueueGet(input_queue, &input_event, NULL, 100) == osOK) {
            printf(
                "key: %s type: %s\r\n",
                input_get_key_name(input_event.key),
                input_get_type_name(input_event.type));
        }
    }

    furi_pubsub_unsubscribe(input->event_pubsub, input_subscription);
    osMessageQueueDelete(input_queue);
}

static void input_cli_send_print_usage() {
    printf("Invalid arguments. Usage:\r\n");
    printf("\tinput send <key> <type>\r\n");
    printf("\t\t <key>\t - one of 'up', 'down', 'left', 'right', 'back', 'ok'\r\n");
    printf("\t\t <type>\t - one of 'press', 'release', 'short', 'long'\r\n");
}

static void input_cli_send(Cli* cli, string_t args, Input* input) {
    UNUSED(cli);
    InputEvent event;
    string_t key_str;
    string_init(key_str);
    bool parsed = false;

    do {
        // Parse Key
        if(!args_read_string_and_trim(args, key_str)) {
            break;
        }
        if(!string_cmp(key_str, "up")) {
            event.key = InputKeyUp;
        } else if(!string_cmp(key_str, "down")) {
            event.key = InputKeyDown;
        } else if(!string_cmp(key_str, "left")) {
            event.key = InputKeyLeft;
        } else if(!string_cmp(key_str, "right")) {
            event.key = InputKeyRight;
        } else if(!string_cmp(key_str, "ok")) {
            event.key = InputKeyOk;
        } else if(!string_cmp(key_str, "back")) {
            event.key = InputKeyBack;
        } else {
            break;
        }
        // Parse Type
        if(!string_cmp(args, "press")) {
            event.type = InputTypePress;
        } else if(!string_cmp(args, "release")) {
            event.type = InputTypeRelease;
        } else if(!string_cmp(args, "short")) {
            event.type = InputTypeShort;
        } else if(!string_cmp(args, "long")) {
            event.type = InputTypeLong;
        } else {
            break;
        }
        parsed = true;
    } while(false);

    if(parsed) {
        furi_pubsub_publish(input->event_pubsub, &event);
    } else {
        input_cli_send_print_usage();
    }
    string_clear(key_str);
}

void input_cli(Cli* cli, string_t args, void* context) {
    furi_assert(cli);
    furi_assert(context);
    Input* input = context;
    string_t cmd;
    string_init(cmd);

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            input_cli_usage();
            break;
        }
        if(string_cmp_str(cmd, "dump") == 0) {
            input_cli_dump(cli, args, input);
            break;
        }
        if(string_cmp_str(cmd, "send") == 0) {
            input_cli_send(cli, args, input);
            break;
        }

        input_cli_usage();
    } while(false);

    string_clear(cmd);
}
