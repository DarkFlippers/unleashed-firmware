#include "cli_i.h"
#include "cli_commands.h"

#include <api-hal-vcp.h>

Cli* cli_alloc() {
    Cli* cli = furi_alloc(sizeof(Cli));
    CliCommandDict_init(cli->commands);

    cli->mutex = osMutexNew(NULL);
    furi_check(cli->mutex);

    cli_reset_state(cli);

    return cli;
}

void cli_free(Cli* cli) {
    free(cli);
}

void cli_reset_state(Cli* cli) {
    string_clear(cli->line);
    string_init(cli->line);
}

void cli_putc(char c) {
    api_hal_vcp_tx((uint8_t*)&c, 1);
}

void cli_print(const char* str) {
    api_hal_vcp_tx((uint8_t*)str, strlen(str));
}

void cli_print_version() {
    cli_print("Build date:" BUILD_DATE ". "
              "Git Commit:" GIT_COMMIT ". "
              "Git Branch:" GIT_BRANCH ". "
              "Commit Number:" GIT_BRANCH_NUM ".");
}

void cli_motd() {
    cli_print("Flipper cli.\r\n");
    cli_print_version();
}

void cli_nl() {
    cli_print("\r\n");
}

void cli_prompt() {
    cli_print("\r\n>: ");
}

void cli_backspace(Cli* cli) {
    size_t s = string_size(cli->line);
    if(s > 0) {
        s--;
        string_left(cli->line, s);
        cli_putc(CliSymbolAsciiBackspace);
        cli_putc(CliSymbolAsciiSpace);
        cli_putc(CliSymbolAsciiBackspace);
    } else {
        cli_putc(CliSymbolAsciiBell);
    }
}

void cli_enter(Cli* cli) {
    // Normalize input
    string_strim(cli->line);
    if(string_size(cli->line) == 0) {
        cli_prompt();
        return;
    }

    // Get first word as command name
    string_t command;
    string_init(command);
    size_t ws = string_search_char(cli->line, ' ');
    if(ws == STRING_FAILURE) {
        string_set(command, cli->line);
        string_clear(cli->line);
        string_init(cli->line);
    } else {
        string_set_n(command, cli->line, 0, ws);
        string_right(cli->line, ws);
        string_strim(cli->line);
    }

    // Search for command
    furi_check(osMutexAcquire(cli->mutex, osWaitForever) == osOK);
    CliCommand* cli_command = CliCommandDict_get(cli->commands, command);
    furi_check(osMutexRelease(cli->mutex) == osOK);
    if(cli_command) {
        cli_nl();
        cli_command->callback(cli->line, cli_command->context);
        cli_prompt();
    } else {
        cli_nl();
        cli_print("Command not found: ");
        cli_print(string_get_cstr(command));
        cli_prompt();
        cli_putc(CliSymbolAsciiBell);
    }

    // Always finish with clean state
    cli_reset_state(cli);
}

void cli_process_input(Cli* cli) {
    char c;
    size_t r;

    r = api_hal_vcp_rx((uint8_t*)&c, 1);
    if(r == 0) {
        cli_reset_state(cli);
    }

    if(c == CliSymbolAsciiTab) {
        cli_putc(CliSymbolAsciiBell);
    } else if(c == CliSymbolAsciiSOH) {
        cli_motd();
        cli_prompt();
    } else if(c == CliSymbolAsciiEOT) {
        cli_reset_state(cli);
    } else if(c == CliSymbolAsciiEsc) {
        r = api_hal_vcp_rx((uint8_t*)&c, 1);
        if(r && c == '[') {
            api_hal_vcp_rx((uint8_t*)&c, 1);
        } else {
            cli_putc(CliSymbolAsciiBell);
        }
    } else if(c == CliSymbolAsciiBackspace || c == CliSymbolAsciiDel) {
        cli_backspace(cli);
    } else if(c == CliSymbolAsciiCR) {
        cli_enter(cli);
    } else if(c >= 0x20 && c < 0x7F) {
        string_push_back(cli->line, c);
        cli_putc(c);
    } else {
        cli_putc(CliSymbolAsciiBell);
    }
}

void cli_add_command(Cli* cli, const char* name, CliCallback callback, void* context) {
    string_t name_str;
    string_init_set_str(name_str, name);
    CliCommand c;
    c.callback = callback;
    c.context = context;

    furi_check(osMutexAcquire(cli->mutex, osWaitForever) == osOK);
    CliCommandDict_set_at(cli->commands, name_str, c);
    furi_check(osMutexRelease(cli->mutex) == osOK);
}

void cli_task(void* p) {
    Cli* cli = cli_alloc();

    // Init basic cli commands
    cli_commands_init(cli);

    if(!furi_create("cli", cli)) {
        printf("[cli_task] cannot create the cli record\n");
        furiac_exit(NULL);
    }

    furiac_ready();

    while(1) {
        cli_process_input(cli);
    }
}
