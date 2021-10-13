#include "m-string.h"
#include <stdio.h>
#include <furi.h>
#include <furi-hal.h>
#include "minunit_vars.h"
#include <notification/notification-messages.h>
#include <cli/cli.h>
#include <loader/loader.h>

int run_minunit();
int run_minunit_test_irda_decoder_encoder();
int run_minunit_test_rpc();

void unit_tests_cli(Cli* cli, string_t args, void* context) {
    uint32_t test_result = 0;
    minunit_run = 0;
    minunit_assert = 0;
    minunit_fail = 0;
    minunit_status = 0;

    Loader* loader = furi_record_open("loader");
    furi_record_close("loader");

    NotificationApp* notification = furi_record_open("notification");
    furi_record_close("notification");

    if(loader_is_locked(loader)) {
        FURI_LOG_E("UNIT_TESTS", "RPC: stop all applications to run tests");
        notification_message(notification, &sequence_blink_magenta_100);
    } else {
        notification_message_block(notification, &sequence_set_only_blue_255);

        test_result |= run_minunit();
        test_result |= run_minunit_test_irda_decoder_encoder();
        test_result |= run_minunit_test_rpc();

        if(test_result == 0) {
            notification_message(notification, &sequence_success);
            FURI_LOG_I("UNIT_TESTS", "PASSED");
        } else {
            notification_message(notification, &sequence_error);
            FURI_LOG_E("UNIT_TESTS", "FAILED");
        }
    }
}

void unit_tests_cli_init() {
    Cli* cli = furi_record_open("cli");
    cli_add_command(cli, "unit_tests", CliCommandFlagParallelSafe, unit_tests_cli, NULL);
    furi_record_close("cli");
}
