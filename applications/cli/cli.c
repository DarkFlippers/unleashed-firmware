#include "cli_i.h"
#include "cli_commands.h"
#include <version.h>
#include <api-hal-version.h>

Cli* cli_alloc() {
    Cli* cli = furi_alloc(sizeof(Cli));
    CliCommandTree_init(cli->commands);

    cli->mutex = osMutexNew(NULL);
    furi_check(cli->mutex);

    cli_reset_state(cli);

    return cli;
}

void cli_free(Cli* cli) {
    free(cli);
}

void cli_reset_state(Cli* cli) {
    // Release allocated buffer, reset state
    string_clear(cli->line);
    string_init(cli->line);
}

void cli_putc(char c) {
    api_hal_vcp_tx((uint8_t*)&c, 1);
}

char cli_getc(Cli* cli) {
    furi_assert(cli);
    char c;
    if(api_hal_vcp_rx((uint8_t*)&c, 1) == 0) {
        cli_reset_state(cli);
    }
    return c;
}

void cli_stdout_callback(void* _cookie, const char* data, size_t size) {
    api_hal_vcp_tx((const uint8_t*)data, size);
}

void cli_write(Cli* cli, uint8_t* buffer, size_t size) {
    return api_hal_vcp_tx(buffer, size);
}

size_t cli_read(Cli* cli, uint8_t* buffer, size_t size) {
    return api_hal_vcp_rx(buffer, size);
}

bool cli_cmd_interrupt_received(Cli* cli) {
    char c = '\0';
    api_hal_vcp_rx_with_timeout((uint8_t*)&c, 1, 1);
    return c == CliSymbolAsciiETX;
}

void cli_print_version(const Version* version) {
    if(version) {
        printf("\tVersion:\t%s\r\n", version_get_version(version));
        printf("\tBuild date:\t%s\r\n", version_get_builddate(version));
        printf(
            "\tGit Commit:\t%s (%s)\r\n",
            version_get_githash(version),
            version_get_gitbranchnum(version));
        printf("\tGit Branch:\t%s\r\n", version_get_gitbranch(version));
    } else {
        printf("\tNo build info\r\n");
    }
}

void cli_print_usage(const char* cmd, const char* usage, const char* arg) {
    furi_assert(cmd);
    furi_assert(arg);
    furi_assert(usage);

    printf("%s: illegal option -- %s\r\nusage: %s %s", cmd, arg, cmd, usage);
}

void cli_motd() {
    printf("\r\n \
              _.-------.._                    -,\r\n \
          .-\"```\"--..,,_/ /`-,               -,  \\ \r\n \
       .:\"          /:/  /'\\  \\     ,_...,  `. |  |\r\n \
      /       ,----/:/  /`\\ _\\~`_-\"`     _;\r\n \
     '      / /`\"\"\"'\\ \\ \\.~`_-'      ,-\"'/ \r\n \
    |      | |  0    | | .-'      ,/`  /\r\n \
   |    ,..\\ \\     ,.-\"`       ,/`    /\r\n \
  ;    :    `/`\"\"\\`           ,/--==,/-----,\r\n \
  |    `-...|        -.___-Z:_______J...---;\r\n \
  :         `                           _-'\r\n \
 _L_  _     ___  ___  ___  ___  ____--\"`___  _     ___\r\n \
| __|| |   |_ _|| _ \\| _ \\| __|| _ \\   / __|| |   |_ _|\r\n \
| _| | |__  | | |  _/|  _/| _| |   /  | (__ | |__  | |\r\n \
|_|  |____||___||_|  |_|  |___||_|_\\   \\___||____||___|\r\n\r\n");

    printf("You are now connected to Flipper Command Line Interface.\r\n\r\n");

    printf("Bootloader\r\n");
    cli_print_version(api_hal_version_get_boot_version());

    printf("Firmware\r\n");
    cli_print_version(api_hal_version_get_firmware_version());
}

void cli_nl() {
    printf("\r\n");
}

void cli_prompt() {
    printf("\r\n>: ");
    fflush(stdout);
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
    CliCommand* cli_command = CliCommandTree_get(cli->commands, command);
    furi_check(osMutexRelease(cli->mutex) == osOK);
    if(cli_command) {
        cli_nl();
        cli_command->callback(cli, cli->line, cli_command->context);
        cli_prompt();
    } else {
        cli_nl();
        printf("Command not found: ");
        printf(string_get_cstr(command));
        cli_prompt();
        cli_putc(CliSymbolAsciiBell);
    }

    string_clear(command);
    // Always finish with clean state
    cli_reset_state(cli);
}

void cli_process_input(Cli* cli) {
    char c = cli_getc(cli);
    size_t r;

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
    string_strim(name_str);

    size_t name_replace;
    do {
        name_replace = string_replace_str(name_str, " ", "_");
    } while(name_replace != STRING_FAILURE);

    CliCommand c;
    c.callback = callback;
    c.context = context;

    furi_check(osMutexAcquire(cli->mutex, osWaitForever) == osOK);
    CliCommandTree_set_at(cli->commands, name_str, c);
    furi_check(osMutexRelease(cli->mutex) == osOK);

    string_clear(name_str);
}

void cli_delete_command(Cli* cli, const char* name) {
    string_t name_str;
    string_init_set_str(name_str, name);
    string_strim(name_str);

    size_t name_replace;
    do {
        name_replace = string_replace_str(name_str, " ", "_");
    } while(name_replace != STRING_FAILURE);

    furi_check(osMutexAcquire(cli->mutex, osWaitForever) == osOK);
    CliCommandTree_erase(cli->commands, name_str);
    furi_check(osMutexRelease(cli->mutex) == osOK);

    string_clear(name_str);
}

int32_t cli_task(void* p) {
    Cli* cli = cli_alloc();

    // Init basic cli commands
    cli_commands_init(cli);

    furi_record_create("cli", cli);

    furi_stdglue_set_thread_stdout_callback(cli_stdout_callback);
    while(1) {
        cli_process_input(cli);
    }

    return 0;
}
