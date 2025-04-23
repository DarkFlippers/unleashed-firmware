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
