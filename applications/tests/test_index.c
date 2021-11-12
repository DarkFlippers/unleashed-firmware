#include "m-string.h"

#include <stdio.h>
#include <furi.h>
#include <furi-hal.h>
#include "minunit_vars.h"
#include <notification/notification-messages.h>
#include <cli/cli.h>
#include <loader/loader.h>

#define TAG "UnitTests"

int run_minunit();
int run_minunit_test_irda_decoder_encoder();
int run_minunit_test_rpc();
int run_minunit_test_flipper_file();

void minunit_print_progress(void) {
    static char progress[] = {'\\', '|', '/', '-'};
    static uint8_t progress_counter = 0;
    static TickType_t last_tick = 0;
    TickType_t current_tick = xTaskGetTickCount();
    if(current_tick - last_tick > 20) {
        last_tick = current_tick;
        printf("[%c]\033[3D", progress[++progress_counter % COUNT_OF(progress)]);
    }
}

void minunit_print_fail(const char* str) {
    printf("%s\n", str);
}

void unit_tests_cli(Cli* cli, string_t args, void* context) {
    uint32_t test_result = 0;
    minunit_run = 0;
    minunit_assert = 0;
    minunit_fail = 0;
    minunit_status = 0;

    Loader* loader = furi_record_open("loader");
    NotificationApp* notification = furi_record_open("notification");

    // TODO: lock device while test running
    if(loader_is_locked(loader)) {
        FURI_LOG_E(TAG, "RPC: stop all applications to run tests");
        notification_message(notification, &sequence_blink_magenta_100);
    } else {
        notification_message_block(notification, &sequence_set_only_blue_255);

        uint32_t heap_before = memmgr_get_free_heap();
        uint32_t cycle_counter = DWT->CYCCNT;

        test_result |= run_minunit();
        test_result |= run_minunit_test_irda_decoder_encoder();
        test_result |= run_minunit_test_rpc();
        test_result |= run_minunit_test_flipper_file();
        cycle_counter = (DWT->CYCCNT - cycle_counter);

        FURI_LOG_I(TAG, "Consumed: %0.2fs", (float)cycle_counter / (SystemCoreClock));

        if(test_result == 0) {
            delay(200); /* wait for tested services and apps to deallocate */
            uint32_t heap_after = memmgr_get_free_heap();
            notification_message(notification, &sequence_success);
            if(heap_after != heap_before) {
                FURI_LOG_E(TAG, "Leaked: %d", heap_before - heap_after);
            } else {
                FURI_LOG_I(TAG, "No leaks");
            }
            FURI_LOG_I(TAG, "PASSED");
        } else {
            notification_message(notification, &sequence_error);
            FURI_LOG_E(TAG, "FAILED");
        }
    }

    furi_record_close("notification");
    furi_record_close("loader");
}

void unit_tests_cli_init() {
    Cli* cli = furi_record_open("cli");

    // We need to launch apps from tests, so we cannot lock loader
    cli_add_command(cli, "unit_tests", CliCommandFlagParallelSafe, unit_tests_cli, NULL);
    furi_record_close("cli");
}
