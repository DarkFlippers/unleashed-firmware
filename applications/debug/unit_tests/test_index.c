#include <stdio.h>
#include <furi.h>
#include <furi_hal.h>
#include "minunit_vars.h"
#include <notification/notification_messages.h>
#include <cli/cli.h>
#include <loader/loader.h>

#define TAG "UnitTests"

int run_minunit_test_furi();
int run_minunit_test_furi_hal();
int run_minunit_test_furi_hal_crypto();
int run_minunit_test_furi_string();
int run_minunit_test_infrared();
int run_minunit_test_rpc();
int run_minunit_test_manifest();
int run_minunit_test_flipper_format();
int run_minunit_test_flipper_format_string();
int run_minunit_test_stream();
int run_minunit_test_storage();
int run_minunit_test_subghz();
int run_minunit_test_dirwalk();
int run_minunit_test_power();
int run_minunit_test_protocol_dict();
int run_minunit_test_lfrfid_protocols();
int run_minunit_test_nfc();
int run_minunit_test_bit_lib();
int run_minunit_test_float_tools();
int run_minunit_test_bt();
int run_minunit_test_dialogs_file_browser_options();

typedef int (*UnitTestEntry)();

typedef struct {
    const char* name;
    const UnitTestEntry entry;
} UnitTest;

const UnitTest unit_tests[] = {
    {.name = "furi", .entry = run_minunit_test_furi},
    {.name = "furi_hal", .entry = run_minunit_test_furi_hal},
    {.name = "furi_hal_crypto", .entry = run_minunit_test_furi_hal_crypto},
    {.name = "furi_string", .entry = run_minunit_test_furi_string},
    {.name = "storage", .entry = run_minunit_test_storage},
    {.name = "stream", .entry = run_minunit_test_stream},
    {.name = "dirwalk", .entry = run_minunit_test_dirwalk},
    {.name = "manifest", .entry = run_minunit_test_manifest},
    {.name = "flipper_format", .entry = run_minunit_test_flipper_format},
    {.name = "flipper_format_string", .entry = run_minunit_test_flipper_format_string},
    {.name = "rpc", .entry = run_minunit_test_rpc},
    {.name = "subghz", .entry = run_minunit_test_subghz},
    {.name = "infrared", .entry = run_minunit_test_infrared},
    {.name = "nfc", .entry = run_minunit_test_nfc},
    {.name = "power", .entry = run_minunit_test_power},
    {.name = "protocol_dict", .entry = run_minunit_test_protocol_dict},
    {.name = "lfrfid", .entry = run_minunit_test_lfrfid_protocols},
    {.name = "bit_lib", .entry = run_minunit_test_bit_lib},
    {.name = "float_tools", .entry = run_minunit_test_float_tools},
    {.name = "bt", .entry = run_minunit_test_bt},
    {.name = "dialogs_file_browser_options",
     .entry = run_minunit_test_dialogs_file_browser_options},
};

void minunit_print_progress() {
    static const char progress[] = {'\\', '|', '/', '-'};
    static uint8_t progress_counter = 0;
    static TickType_t last_tick = 0;
    TickType_t current_tick = xTaskGetTickCount();
    if(current_tick - last_tick > 20) {
        last_tick = current_tick;
        printf("[%c]\033[3D", progress[++progress_counter % COUNT_OF(progress)]);
        fflush(stdout);
    }
}

void minunit_print_fail(const char* str) {
    printf(_FURI_LOG_CLR_E "%s\r\n" _FURI_LOG_CLR_RESET, str);
}

void unit_tests_cli(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(args);
    UNUSED(context);
    minunit_run = 0;
    minunit_assert = 0;
    minunit_fail = 0;
    minunit_status = 0;

    Loader* loader = furi_record_open(RECORD_LOADER);
    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);

    // TODO: lock device while test running
    if(loader_is_locked(loader)) {
        printf("RPC: stop all applications to run tests\r\n");
        notification_message(notification, &sequence_blink_magenta_100);
    } else {
        notification_message_block(notification, &sequence_set_only_blue_255);

        uint32_t heap_before = memmgr_get_free_heap();
        uint32_t cycle_counter = furi_get_tick();

        for(size_t i = 0; i < COUNT_OF(unit_tests); i++) {
            if(cli_cmd_interrupt_received(cli)) {
                break;
            }

            if(furi_string_size(args)) {
                if(furi_string_cmp_str(args, unit_tests[i].name) == 0) {
                    unit_tests[i].entry();
                } else {
                    printf("Skipping %s\r\n", unit_tests[i].name);
                }
            } else {
                unit_tests[i].entry();
            }
        }

        if(minunit_run != 0) {
            printf("\r\nFailed tests: %u\r\n", minunit_fail);

            // Time report
            cycle_counter = (furi_get_tick() - cycle_counter);
            printf("Consumed: %lu ms\r\n", cycle_counter);

            // Wait for tested services and apps to deallocate memory
            furi_delay_ms(200);
            uint32_t heap_after = memmgr_get_free_heap();
            printf("Leaked: %ld\r\n", heap_before - heap_after);

            // Final Report
            if(minunit_fail == 0) {
                notification_message(notification, &sequence_success);
                printf("Status: PASSED\r\n");
            } else {
                notification_message(notification, &sequence_error);
                printf("Status: FAILED\r\n");
            }
        }
    }

    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_LOADER);
}

void unit_tests_on_system_start() {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);

    // We need to launch apps from tests, so we cannot lock loader
    cli_add_command(cli, "unit_tests", CliCommandFlagParallelSafe, unit_tests_cli, NULL);
    furi_record_close(RECORD_CLI);
#endif
}
