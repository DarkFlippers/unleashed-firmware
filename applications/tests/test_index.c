#include <stdio.h>
#include "flipper.h"
#include "debug.h"

bool furi_ac_create_kill(FILE* debug_uart);
bool furi_ac_switch_exit(FILE* debug_uart);

bool furi_pipe_record(FILE* debug_uart);
bool furi_holding_data(FILE* debug_uart);
bool furi_concurrent_access(FILE* debug_uart);
bool furi_nonexistent_data(FILE* debug_uart);
bool furi_mute_algorithm(FILE* debug_uart);

void flipper_test_app(void* p) {
    FILE* debug_uart = get_debug();

    if(furi_ac_create_kill(debug_uart)) {
        fprintf(debug_uart, "[TEST] furi_ac_create_kill PASSED\n");
    } else {
        fprintf(debug_uart, "[TEST] furi_ac_create_kill FAILED\n");
    }

    if(furi_ac_switch_exit(debug_uart)) {
        fprintf(debug_uart, "[TEST] furi_ac_switch_exit PASSED\n");
    } else {
        fprintf(debug_uart, "[TEST] furi_ac_switch_exit FAILED\n");
    }

    if(furi_pipe_record(debug_uart)) {
        fprintf(debug_uart, "[TEST] furi_pipe_record PASSED\n");
    } else {
        fprintf(debug_uart, "[TEST] furi_pipe_record FAILED\n");
    }

    if(furi_holding_data(debug_uart)) {
        fprintf(debug_uart, "[TEST] furi_holding_data PASSED\n");
    } else {
        fprintf(debug_uart, "[TEST] furi_holding_data FAILED\n");
    }

    if(furi_concurrent_access(debug_uart)) {
        fprintf(debug_uart, "[TEST] furi_concurrent_access PASSED\n");
    } else {
        fprintf(debug_uart, "[TEST] furi_concurrent_access FAILED\n");
    }

    if(furi_nonexistent_data(debug_uart)) {
        fprintf(debug_uart, "[TEST] furi_nonexistent_data PASSED\n");
    } else {
        fprintf(debug_uart, "[TEST] furi_nonexistent_data FAILED\n");
    }

    if(furi_mute_algorithm(debug_uart)) {
        fprintf(debug_uart, "[TEST] furi_mute_algorithm PASSED\n");
    } else {
        fprintf(debug_uart, "[TEST] furi_mute_algorithm FAILED\n");
    }

    furiac_exit(NULL);
}