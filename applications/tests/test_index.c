#include <stdio.h>
#include "flipper.h"
#include "log.h"

// #include "flipper-core.h" TODO: Rust build disabled

bool test_furi_ac_create_kill(FuriRecordSubscriber* log);
bool test_furi_ac_switch_exit(FuriRecordSubscriber* log);

bool test_furi_pipe_record(FuriRecordSubscriber* log);
bool test_furi_holding_data(FuriRecordSubscriber* log);
bool test_furi_concurrent_access(FuriRecordSubscriber* log);
bool test_furi_nonexistent_data(FuriRecordSubscriber* log);
bool test_furi_mute_algorithm(FuriRecordSubscriber* log);

void flipper_test_app(void* p) {
    FuriRecordSubscriber* log = get_default_log();

    if(test_furi_ac_create_kill(log)) {
        fuprintf(log, "[TEST] test_furi_ac_create_kill PASSED\n");
    } else {
        fuprintf(log, "[TEST] test_furi_ac_create_kill FAILED\n");
    }

    if(test_furi_ac_switch_exit(log)) {
        fuprintf(log, "[TEST] test_furi_ac_switch_exit PASSED\n");
    } else {
        fuprintf(log, "[TEST] test_furi_ac_switch_exit FAILED\n");
    }

    if(test_furi_pipe_record(log)) {
        fuprintf(log, "[TEST] test_furi_pipe_record PASSED\n");
    } else {
        fuprintf(log, "[TEST] test_furi_pipe_record FAILED\n");
    }

    if(test_furi_holding_data(log)) {
        fuprintf(log, "[TEST] test_furi_holding_data PASSED\n");
    } else {
        fuprintf(log, "[TEST] test_furi_holding_data FAILED\n");
    }

    if(test_furi_concurrent_access(log)) {
        fuprintf(log, "[TEST] test_furi_concurrent_access PASSED\n");
    } else {
        fuprintf(log, "[TEST] test_furi_concurrent_access FAILED\n");
    }

    if(test_furi_nonexistent_data(log)) {
        fuprintf(log, "[TEST] test_furi_nonexistent_data PASSED\n");
    } else {
        fuprintf(log, "[TEST] test_furi_nonexistent_data FAILED\n");
    }

    if(test_furi_mute_algorithm(log)) {
        fuprintf(log, "[TEST] test_furi_mute_algorithm PASSED\n");
    } else {
        fuprintf(log, "[TEST] test_furi_mute_algorithm FAILED\n");
    }

    /*
    TODO: Rust build disabled
    if(add(1, 2) == 3) {
        fuprintf(log, "[TEST] Rust add PASSED\n");
    } else {
        fuprintf(log, "[TEST] Rust add FAILED\n");
    }

    rust_uart_write();
    */

    furiac_exit(NULL);
}