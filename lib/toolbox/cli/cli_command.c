#include "cli_command.h"
#include "cli_ansi.h"

bool cli_is_pipe_broken_or_is_etx_next_char(PipeSide* side) {
    if(pipe_state(side) == PipeStateBroken) return true;
    if(!pipe_bytes_available(side)) return false;
    char c = getchar();
    return c == CliKeyETX;
}

void cli_print_usage(const char* cmd, const char* usage, const char* arg) {
    furi_check(cmd);
    furi_check(arg);
    furi_check(usage);

    printf("%s: illegal option -- %s\r\nusage: %s %s", cmd, arg, cmd, usage);
}

bool cli_sleep(PipeSide* side, uint32_t duration_in_ms) {
    uint32_t passed_time = 0;
    bool is_interrupted = false;

    do {
        uint32_t left_time = duration_in_ms - passed_time;
        uint32_t check_interval = left_time >= 100 ? 100 : left_time;
        furi_delay_ms(check_interval);
        passed_time += check_interval;
        is_interrupted = cli_is_pipe_broken_or_is_etx_next_char(side);
    } while(!is_interrupted && passed_time < duration_in_ms);

    return !is_interrupted;
}
