#include <stdio.h>
#include <furi.h>
#include <furi_hal.h>
#include "minunit_vars.h"
#include <notification/notification_messages.h>
#include <cli/cli.h>
#include <loader/loader.h>
#include <lib/toolbox/args.h>

#define TAG "UnitTests"

int run_minunit_test_furi();
int run_minunit_test_furi_hal();
int run_minunit_test_furi_string();
int run_minunit_test_infrared();
int run_minunit_test_rpc();
int run_minunit_test_manifest();
int run_minunit_test_flipper_format();
int run_minunit_test_flipper_format_string();
int run_minunit_test_stream();
int run_minunit_test_storage();
int run_minunit_test_subghz();
int run_minunit_test_subghz_app();
int run_minunit_test_dirwalk();
int run_minunit_test_power();
int run_minunit_test_protocol_dict();
int run_minunit_test_lfrfid_protocols();
int run_minunit_test_nfc();
int run_minunit_test_bit_lib();
int run_minunit_test_float_tools();
int run_minunit_test_bt();

typedef int (*UnitTestEntry)();

typedef struct {
    const char* name;
    const UnitTestEntry entry;
} UnitTest;

const UnitTest unit_tests[] = {
    {.name = "furi", .entry = run_minunit_test_furi},
    {.name = "furi_hal", .entry = run_minunit_test_furi_hal},
    {.name = "furi_string", .entry = run_minunit_test_furi_string},
    {.name = "storage", .entry = run_minunit_test_storage},
    {.name = "stream", .entry = run_minunit_test_stream},
    {.name = "dirwalk", .entry = run_minunit_test_dirwalk},
    {.name = "manifest", .entry = run_minunit_test_manifest},
    {.name = "flipper_format", .entry = run_minunit_test_flipper_format},
    {.name = "flipper_format_string", .entry = run_minunit_test_flipper_format_string},
    {.name = "rpc", .entry = run_minunit_test_rpc},
    {.name = "subghz", .entry = run_minunit_test_subghz},
    {.name = "subghz_app", .entry = run_minunit_test_subghz_app},
    {.name = "infrared", .entry = run_minunit_test_infrared},
    {.name = "nfc", .entry = run_minunit_test_nfc},
    {.name = "power", .entry = run_minunit_test_power},
    {.name = "protocol_dict", .entry = run_minunit_test_protocol_dict},
    {.name = "lfrfid", .entry = run_minunit_test_lfrfid_protocols},
    {.name = "bit_lib", .entry = run_minunit_test_bit_lib},
    {.name = "float_tools", .entry = run_minunit_test_float_tools},
    {.name = "bt", .entry = run_minunit_test_bt},
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
    printf(FURI_LOG_CLR_E "%s\r\n" FURI_LOG_CLR_RESET, str);
}

void unit_tests_cli_logs_puts(const char* str) {
    printf(str);
}

// by default there is no output from unit tests
// you have to enable them with this function
void unit_tests_cli_enable_logs(FuriLogLevel logLevel) {
    furi_log_init();
    furi_log_set_level(logLevel);
    furi_log_set_puts(unit_tests_cli_logs_puts);
}

void unit_tests_cli_print_help() {
    printf(
        "Usage:\r\n"
        "   unit_tests [log level] [test_suit_name]\r\n"
        "     Arguments:\r\n"
        "       [log level]: optional, enables printing output from FURI_LOG_* macro in unit_tests\r\n"
        "       [test_suit_name]: a specific test suit to run. If not specified runs all tests\r\n"
        "                         Warning: supports only 1 test suit to launch\r\n"
        "     Example:\r\n"
        "       unit_tests                  - run ALL tests wihtout log printing\r\n"
        "       unit_tests bt               - run bt tests wihtout log printing\r\n"
        "       unit_tests log debug subghz - run subghz tests with logs at debug level abd above\r\n\r\n"
        "   unit_tests help [?]\r\n"
        "           prints this help\r\n");
}

bool unit_tests_cli_parse_log_level(FuriString* args) {
    bool result = true;
    FuriString* level = furi_string_alloc();
    if(args_read_string_and_trim(args, level)) {
        if(furi_string_cmpi_str(level, "default") == 0) {
            unit_tests_cli_enable_logs(FuriLogLevelDefault);
        } else if(furi_string_cmpi_str(level, "none") == 0) {
            unit_tests_cli_enable_logs(FuriLogLevelNone);
        } else if(furi_string_cmpi_str(level, "error") == 0) {
            unit_tests_cli_enable_logs(FuriLogLevelError);
        } else if(furi_string_cmpi_str(level, "warn") == 0) {
            unit_tests_cli_enable_logs(FuriLogLevelWarn);
        } else if(furi_string_cmpi_str(level, "info") == 0) {
            unit_tests_cli_enable_logs(FuriLogLevelInfo);
        } else if(furi_string_cmpi_str(level, "debug") == 0) {
            unit_tests_cli_enable_logs(FuriLogLevelDebug);
        } else if(furi_string_cmpi_str(level, "trace") == 0) {
            unit_tests_cli_enable_logs(FuriLogLevelTrace);
        } else {
            printf("Error: Invalid log level: %s\r\n", furi_string_get_cstr(level));
            result = false;
        }
    }
    furi_string_free(level);
    return result;
}

FuriString* unit_tests_cli_parse_test_suit(FuriString* args) {
    FuriString* test_suit = furi_string_alloc();

    if(!args_read_string_and_trim(args, test_suit)) {
        furi_string_free(test_suit);
        return NULL;
    }

    return test_suit;
}

void unit_tests_cli(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(args);
    UNUSED(context);
    minunit_run = 0;
    minunit_assert = 0;
    minunit_fail = 0;
    minunit_status = 0;

    if(furi_string_cmp_str(args, "help") == 0 || furi_string_cmp_str(args, "?") == 0) {
        unit_tests_cli_print_help();
        return;
    }

    FuriString* test_suit_to_run = NULL;

    if(furi_string_size(args)) {
        FuriString* arg = furi_string_alloc();
        if(args_read_string_and_trim(args, arg)) {
            if(furi_string_cmp_str(arg, "log") == 0) {
                // read next argument - log level, if fail - show help and return
                if(!unit_tests_cli_parse_log_level(args)) {
                    unit_tests_cli_print_help();
                    furi_string_free(arg);
                    return;
                }

                // next argument might be test suit
                test_suit_to_run = unit_tests_cli_parse_test_suit(args);
            } else {
                // if first argument wasn't log - it was exact test suit to run
                test_suit_to_run = unit_tests_cli_parse_test_suit(arg);
            }
        }
        furi_string_free(arg);
    }

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

            if(test_suit_to_run) {
                if(furi_string_cmp_str(test_suit_to_run, unit_tests[i].name) == 0) {
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

    if(test_suit_to_run) {
        furi_string_free(test_suit_to_run);
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
