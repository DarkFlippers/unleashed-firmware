#include "cli_i.h"
#include "cli_commands.h"

#include <furi-hal-version.h>
#include <loader/loader.h>

Cli* cli_alloc() {
    Cli* cli = furi_alloc(sizeof(Cli));

    CliCommandTree_init(cli->commands);

    string_init(cli->last_line);
    string_init(cli->line);

    cli->mutex = osMutexNew(NULL);
    furi_check(cli->mutex);

    return cli;
}

void cli_free(Cli* cli) {
    furi_assert(cli);

    string_clear(cli->last_line);
    string_clear(cli->line);

    CliCommandTree_clear(cli->commands);

    free(cli);
}

void cli_putc(char c) {
    furi_hal_vcp_tx((uint8_t*)&c, 1);
}

char cli_getc(Cli* cli) {
    furi_assert(cli);
    char c;
    if(furi_hal_vcp_rx((uint8_t*)&c, 1) == 0) {
        cli_reset(cli);
    }
    return c;
}

void cli_stdout_callback(void* _cookie, const char* data, size_t size) {
    furi_hal_vcp_tx((const uint8_t*)data, size);
}

void cli_write(Cli* cli, const uint8_t* buffer, size_t size) {
    return furi_hal_vcp_tx(buffer, size);
}

size_t cli_read(Cli* cli, uint8_t* buffer, size_t size) {
    return furi_hal_vcp_rx(buffer, size);
}

bool cli_cmd_interrupt_received(Cli* cli) {
    char c = '\0';
    if(furi_hal_vcp_rx_with_timeout((uint8_t*)&c, 1, 0) == 1) {
        return c == CliSymbolAsciiETX;
    } else {
        return false;
    }
}

void cli_print_usage(const char* cmd, const char* usage, const char* arg) {
    furi_assert(cmd);
    furi_assert(arg);
    furi_assert(usage);

    printf("%s: illegal option -- %s\r\nusage: %s %s", cmd, arg, cmd, usage);
}

void cli_motd() {
    printf("\r\n"
           "              _.-------.._                    -,\r\n"
           "          .-\"```\"--..,,_/ /`-,               -,  \\ \r\n"
           "       .:\"          /:/  /'\\  \\     ,_...,  `. |  |\r\n"
           "      /       ,----/:/  /`\\ _\\~`_-\"`     _;\r\n"
           "     '      / /`\"\"\"'\\ \\ \\.~`_-'      ,-\"'/ \r\n"
           "    |      | |  0    | | .-'      ,/`  /\r\n"
           "   |    ,..\\ \\     ,.-\"`       ,/`    /\r\n"
           "  ;    :    `/`\"\"\\`           ,/--==,/-----,\r\n"
           "  |    `-...|        -.___-Z:_______J...---;\r\n"
           "  :         `                           _-'\r\n"
           " _L_  _     ___  ___  ___  ___  ____--\"`___  _     ___\r\n"
           "| __|| |   |_ _|| _ \\| _ \\| __|| _ \\   / __|| |   |_ _|\r\n"
           "| _| | |__  | | |  _/|  _/| _| |   /  | (__ | |__  | |\r\n"
           "|_|  |____||___||_|  |_|  |___||_|_\\   \\___||____||___|\r\n"
           "\r\n"
           "Welcome to Flipper Zero Command Line Interface!\r\n"
           "Read Manual https://docs.flipperzero.one\r\n"
           "\r\n");

    const Version* firmware_version = furi_hal_version_get_firmware_version();
    if(firmware_version) {
        printf(
            "Firmware version: %s %s (%s built on %s)\r\n",
            version_get_gitbranch(firmware_version),
            version_get_version(firmware_version),
            version_get_githash(firmware_version),
            version_get_builddate(firmware_version));
    }
}

void cli_nl(Cli* cli) {
    printf("\r\n");
}

void cli_prompt(Cli* cli) {
    printf("\r\n>: %s", string_get_cstr(cli->line));
    fflush(stdout);
}

void cli_reset(Cli* cli) {
    // cli->last_line is cleared and cli->line's buffer moved to cli->last_line
    string_move(cli->last_line, cli->line);
    // Reiniting cli->line
    string_init(cli->line);
    cli->cursor_position = 0;
}

static void cli_handle_backspace(Cli* cli) {
    if(string_size(cli->line) > 0) {
        // Other side
        printf("\e[D\e[1P");
        fflush(stdout);
        // Our side
        string_t temp;
        string_init(temp);
        string_reserve(temp, string_size(cli->line) - 1);
        string_set_strn(temp, string_get_cstr(cli->line), cli->cursor_position - 1);
        string_cat_str(temp, string_get_cstr(cli->line) + cli->cursor_position);

        // cli->line is cleared and temp's buffer moved to cli->line
        string_move(cli->line, temp);
        // NO MEMORY LEAK, STOP REPORTING IT

        cli->cursor_position--;
    } else {
        cli_putc(CliSymbolAsciiBell);
    }
}

static void cli_normalize_line(Cli* cli) {
    string_strim(cli->line);
    cli->cursor_position = string_size(cli->line);
}

static void cli_execute_command(Cli* cli, CliCommand* command, string_t args) {
    if(!(command->flags & CliCommandFlagInsomniaSafe)) {
        furi_hal_power_insomnia_enter();
    }

    // Ensure that we running alone
    if(!(command->flags & CliCommandFlagParallelSafe)) {
        Loader* loader = furi_record_open("loader");
        bool safety_lock = loader_lock(loader);
        if(safety_lock) {
            // Execute command
            command->callback(cli, args, command->context);
            loader_unlock(loader);
        } else {
            printf("Other application is running, close it first");
        }
        furi_record_close("loader");
    } else {
        // Execute command
        command->callback(cli, args, command->context);
    }

    if(!(command->flags & CliCommandFlagInsomniaSafe)) {
        furi_hal_power_insomnia_exit();
    }
}

static void cli_handle_enter(Cli* cli) {
    cli_normalize_line(cli);

    if(string_size(cli->line) == 0) {
        cli_prompt(cli);
        return;
    }

    // Command and args container
    string_t command;
    string_init(command);
    string_t args;
    string_init(args);

    // Split command and args
    size_t ws = string_search_char(cli->line, ' ');
    if(ws == STRING_FAILURE) {
        string_set(command, cli->line);
    } else {
        string_set_n(command, cli->line, 0, ws);
        string_set_n(args, cli->line, ws, string_size(cli->line));
        string_strim(args);
    }

    // Search for command
    furi_check(osMutexAcquire(cli->mutex, osWaitForever) == osOK);
    CliCommand* cli_command = CliCommandTree_get(cli->commands, command);
    if(cli_command) {
        cli_nl(cli);
        cli_execute_command(cli, cli_command, args);
    } else {
        cli_nl(cli);
        printf(
            "`%s` command not found, use `help` or `?` to list all available commands",
            string_get_cstr(command));
        cli_putc(CliSymbolAsciiBell);
    }
    furi_check(osMutexRelease(cli->mutex) == osOK);

    cli_reset(cli);
    cli_prompt(cli);

    // Cleanup command and args
    string_clear(command);
    string_clear(args);
}

static void cli_handle_autocomplete(Cli* cli) {
    cli_normalize_line(cli);

    if(string_size(cli->line) == 0) {
        return;
    }

    cli_nl(cli);

    // Prepare common base for autocomplete
    string_t common;
    string_init(common);
    // Iterate throw commands
    for
        M_EACH(cli_command, cli->commands, CliCommandTree_t) {
            // Process only if starts with line buffer
            if(string_start_with_string_p(*cli_command->key_ptr, cli->line)) {
                // Show autocomplete option
                printf("%s\r\n", string_get_cstr(*cli_command->key_ptr));
                // Process common base for autocomplete
                if(string_size(common) > 0) {
                    // Choose shortest string
                    const size_t key_size = string_size(*cli_command->key_ptr);
                    const size_t common_size = string_size(common);
                    const size_t min_size = key_size > common_size ? common_size : key_size;
                    size_t i = 0;
                    while(i < min_size) {
                        // Stop when do not match
                        if(string_get_char(*cli_command->key_ptr, i) !=
                           string_get_char(common, i)) {
                            break;
                        }
                        i++;
                    }
                    // Cut right part if any
                    string_left(common, i);
                } else {
                    // Start with something
                    string_set(common, *cli_command->key_ptr);
                }
            }
        }
    // Replace line buffer if autocomplete better
    if(string_size(common) > string_size(cli->line)) {
        string_set(cli->line, common);
        cli->cursor_position = string_size(cli->line);
    }
    // Cleanup
    string_clear(common);
    // Show prompt
    cli_prompt(cli);
}

static void cli_handle_escape(Cli* cli, char c) {
    if(c == 'A') {
        // Use previous command if line buffer is empty
        if(string_size(cli->line) == 0 && string_cmp(cli->line, cli->last_line) != 0) {
            // Set line buffer and cursor position
            string_set(cli->line, cli->last_line);
            cli->cursor_position = string_size(cli->line);
            // Show new line to user
            printf("%s", string_get_cstr(cli->line));
        }
    } else if(c == 'B') {
    } else if(c == 'C') {
        if(cli->cursor_position < string_size(cli->line)) {
            cli->cursor_position++;
            printf("\e[C");
        }
    } else if(c == 'D') {
        if(cli->cursor_position > 0) {
            cli->cursor_position--;
            printf("\e[D");
        }
    }
    fflush(stdout);
}

void cli_process_input(Cli* cli) {
    char c = cli_getc(cli);
    size_t r;

    if(c == CliSymbolAsciiTab) {
        cli_handle_autocomplete(cli);
    } else if(c == CliSymbolAsciiSOH) {
        osDelay(33); // We are too fast, Minicom is not ready yet
        cli_motd();
        cli_prompt(cli);
    } else if(c == CliSymbolAsciiETX) {
        cli_reset(cli);
        cli_prompt(cli);
    } else if(c == CliSymbolAsciiEOT) {
        cli_reset(cli);
    } else if(c == CliSymbolAsciiEsc) {
        r = furi_hal_vcp_rx((uint8_t*)&c, 1);
        if(r && c == '[') {
            furi_hal_vcp_rx((uint8_t*)&c, 1);
            cli_handle_escape(cli, c);
        } else {
            cli_putc(CliSymbolAsciiBell);
        }
    } else if(c == CliSymbolAsciiBackspace || c == CliSymbolAsciiDel) {
        cli_handle_backspace(cli);
    } else if(c == CliSymbolAsciiCR) {
        cli_handle_enter(cli);
    } else if(c >= 0x20 && c < 0x7F) {
        if(cli->cursor_position == string_size(cli->line)) {
            string_push_back(cli->line, c);
            cli_putc(c);
        } else {
            // ToDo: better way?
            string_t temp;
            string_init(temp);
            string_reserve(temp, string_size(cli->line) + 1);
            string_set_strn(temp, string_get_cstr(cli->line), cli->cursor_position);
            string_push_back(temp, c);
            string_cat_str(temp, string_get_cstr(cli->line) + cli->cursor_position);

            // cli->line is cleared and temp's buffer moved to cli->line
            string_move(cli->line, temp);
            // NO MEMORY LEAK, STOP REPORTING IT

            // Print character in replace mode
            printf("\e[4h%c\e[4l", c);
            fflush(stdout);
        }
        cli->cursor_position++;
    } else {
        cli_putc(CliSymbolAsciiBell);
    }
}

void cli_add_command(
    Cli* cli,
    const char* name,
    CliCommandFlag flags,
    CliCallback callback,
    void* context) {
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
    c.flags = flags;

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

int32_t cli_srv(void* p) {
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
