#include "cli_i.h"
#include "cli_commands.h"
#include "cli_vcp.h"
#include "cli_ansi.h"
#include <furi_hal_version.h>
#include <loader/loader.h>

#define TAG "CliSrv"

#define CLI_INPUT_LEN_LIMIT 256
#define CLI_PROMPT          ">: " // qFlipper does not recognize us if we use escape sequences :(
#define CLI_PROMPT_LENGTH   3 // printable characters

Cli* cli_alloc(void) {
    Cli* cli = malloc(sizeof(Cli));

    CliCommandTree_init(cli->commands);

    cli->last_line = furi_string_alloc();
    cli->line = furi_string_alloc();

    cli->session = NULL;

    cli->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    cli->idle_sem = furi_semaphore_alloc(1, 0);

    return cli;
}

void cli_putc(Cli* cli, char c) {
    furi_check(cli);
    if(cli->session != NULL) {
        cli->session->tx((uint8_t*)&c, 1);
    }
}

char cli_getc(Cli* cli) {
    furi_check(cli);
    char c = 0;
    if(cli->session != NULL) {
        if(cli->session->rx((uint8_t*)&c, 1, FuriWaitForever) == 0) {
            cli_reset(cli);
            furi_delay_tick(10);
        }
    } else {
        cli_reset(cli);
        furi_delay_tick(10);
    }
    return c;
}

void cli_write(Cli* cli, const uint8_t* buffer, size_t size) {
    furi_check(cli);
    if(cli->session != NULL) {
        cli->session->tx(buffer, size);
    }
}

size_t cli_read(Cli* cli, uint8_t* buffer, size_t size) {
    furi_check(cli);
    if(cli->session != NULL) {
        return cli->session->rx(buffer, size, FuriWaitForever);
    } else {
        return 0;
    }
}

size_t cli_read_timeout(Cli* cli, uint8_t* buffer, size_t size, uint32_t timeout) {
    furi_check(cli);
    if(cli->session != NULL) {
        return cli->session->rx(buffer, size, timeout);
    } else {
        return 0;
    }
}

bool cli_is_connected(Cli* cli) {
    furi_check(cli);
    if(cli->session != NULL) {
        return cli->session->is_connected();
    }
    return false;
}

bool cli_cmd_interrupt_received(Cli* cli) {
    furi_check(cli);
    char c = '\0';
    if(cli_is_connected(cli)) {
        if(cli->session->rx((uint8_t*)&c, 1, 0) == 1) {
            return c == CliKeyETX;
        }
    } else {
        return true;
    }
    return false;
}

void cli_print_usage(const char* cmd, const char* usage, const char* arg) {
    furi_check(cmd);
    furi_check(arg);
    furi_check(usage);

    printf("%s: illegal option -- %s\r\nusage: %s %s", cmd, arg, cmd, usage);
}

void cli_motd(void) {
    printf(ANSI_FLIPPER_BRAND_ORANGE
           "\r\n"
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
           "|_|  |____||___||_|  |_|  |___||_|_\\   \\___||____||___|\r\n" ANSI_RESET
           "\r\n" ANSI_FG_BR_WHITE "Welcome to " ANSI_FLIPPER_BRAND_ORANGE
           "Flipper Zero" ANSI_FG_BR_WHITE " Command Line Interface!\r\n"
           "Read the manual: https://docs.flipper.net/development/cli\r\n"
           "Run `help` or `?` to list available commands\r\n" ANSI_RESET "\r\n");

    const Version* firmware_version = furi_hal_version_get_firmware_version();
    if(firmware_version) {
        printf(
            "Firmware version: %s %s (%s%s built on %s)\r\n",
            version_get_gitbranch(firmware_version),
            version_get_version(firmware_version),
            version_get_githash(firmware_version),
            version_get_dirty_flag(firmware_version) ? "-dirty" : "",
            version_get_builddate(firmware_version));
    }
}

void cli_nl(Cli* cli) {
    UNUSED(cli);
    printf("\r\n");
}

void cli_prompt(Cli* cli) {
    UNUSED(cli);
    printf("\r\n" CLI_PROMPT "%s", furi_string_get_cstr(cli->line));
    fflush(stdout);
}

void cli_reset(Cli* cli) {
    // cli->last_line is cleared and cli->line's buffer moved to cli->last_line
    furi_string_move(cli->last_line, cli->line);
    // Reiniting cli->line
    cli->line = furi_string_alloc();
    cli->cursor_position = 0;
}

static void cli_handle_backspace(Cli* cli) {
    if(cli->cursor_position > 0) {
        furi_assert(furi_string_size(cli->line) > 0);
        // Other side
        printf("\e[D\e[1P");
        fflush(stdout);
        // Our side
        furi_string_replace_at(cli->line, cli->cursor_position - 1, 1, "");

        cli->cursor_position--;
    } else {
        cli_putc(cli, CliKeyBell);
    }
}

static void cli_normalize_line(Cli* cli) {
    furi_string_trim(cli->line);
    cli->cursor_position = furi_string_size(cli->line);
}

static void cli_execute_command(Cli* cli, CliCommand* command, FuriString* args) {
    if(!(command->flags & CliCommandFlagInsomniaSafe)) {
        furi_hal_power_insomnia_enter();
    }

    // Ensure that we running alone
    if(!(command->flags & CliCommandFlagParallelSafe)) {
        Loader* loader = furi_record_open(RECORD_LOADER);
        bool safety_lock = loader_lock(loader);
        if(safety_lock) {
            // Execute command
            command->callback(cli, args, command->context);
            loader_unlock(loader);
        } else {
            printf("Other application is running, close it first");
        }
        furi_record_close(RECORD_LOADER);
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

    if(furi_string_size(cli->line) == 0) {
        cli_prompt(cli);
        return;
    }

    // Command and args container
    FuriString* command;
    command = furi_string_alloc();
    FuriString* args;
    args = furi_string_alloc();

    // Split command and args
    size_t ws = furi_string_search_char(cli->line, ' ');
    if(ws == FURI_STRING_FAILURE) {
        furi_string_set(command, cli->line);
    } else {
        furi_string_set_n(command, cli->line, 0, ws);
        furi_string_set_n(args, cli->line, ws, furi_string_size(cli->line));
        furi_string_trim(args);
    }

    // Search for command
    furi_check(furi_mutex_acquire(cli->mutex, FuriWaitForever) == FuriStatusOk);
    CliCommand* cli_command_ptr = CliCommandTree_get(cli->commands, command);

    if(cli_command_ptr) { //-V547
        CliCommand cli_command;
        memcpy(&cli_command, cli_command_ptr, sizeof(CliCommand));
        furi_check(furi_mutex_release(cli->mutex) == FuriStatusOk);
        cli_nl(cli);
        cli_execute_command(cli, &cli_command, args);
    } else {
        furi_check(furi_mutex_release(cli->mutex) == FuriStatusOk);
        cli_nl(cli);
        printf(
            "`%s` command not found, use `help` or `?` to list all available commands",
            furi_string_get_cstr(command));
        cli_putc(cli, CliKeyBell);
    }

    cli_reset(cli);
    cli_prompt(cli);

    // Cleanup command and args
    furi_string_free(command);
    furi_string_free(args);
}

static void cli_handle_autocomplete(Cli* cli) {
    cli_normalize_line(cli);

    if(furi_string_size(cli->line) == 0) {
        return;
    }

    cli_nl(cli);

    // Prepare common base for autocomplete
    FuriString* common;
    common = furi_string_alloc();
    // Iterate throw commands
    for
        M_EACH(cli_command, cli->commands, CliCommandTree_t) {
            // Process only if starts with line buffer
            if(furi_string_start_with(*cli_command->key_ptr, cli->line)) {
                // Show autocomplete option
                printf("%s\r\n", furi_string_get_cstr(*cli_command->key_ptr));
                // Process common base for autocomplete
                if(furi_string_size(common) > 0) {
                    // Choose shortest string
                    const size_t key_size = furi_string_size(*cli_command->key_ptr);
                    const size_t common_size = furi_string_size(common);
                    const size_t min_size = key_size > common_size ? common_size : key_size;
                    size_t i = 0;
                    while(i < min_size) {
                        // Stop when do not match
                        if(furi_string_get_char(*cli_command->key_ptr, i) !=
                           furi_string_get_char(common, i)) {
                            break;
                        }
                        i++;
                    }
                    // Cut right part if any
                    furi_string_left(common, i);
                } else {
                    // Start with something
                    furi_string_set(common, *cli_command->key_ptr);
                }
            }
        }
    // Replace line buffer if autocomplete better
    if(furi_string_size(common) > furi_string_size(cli->line)) {
        furi_string_set(cli->line, common);
        cli->cursor_position = furi_string_size(cli->line);
    }
    // Cleanup
    furi_string_free(common);
    // Show prompt
    cli_prompt(cli);
}

typedef enum {
    CliCharClassWord,
    CliCharClassSpace,
    CliCharClassOther,
} CliCharClass;

/**
 * @brief Determines the class that a character belongs to
 * 
 * The return value of this function should not be used on its own; it should
 * only be used for comparing it with other values returned by this function.
 * This function is used internally in `cli_skip_run`.
 */
static CliCharClass cli_char_class(char c) {
    if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_') {
        return CliCharClassWord;
    } else if(c == ' ') {
        return CliCharClassSpace;
    } else {
        return CliCharClassOther;
    }
}

typedef enum {
    CliSkipDirectionLeft,
    CliSkipDirectionRight,
} CliSkipDirection;

/**
 * @brief Skips a run of a class of characters
 * 
 * @param string Input string
 * @param original_pos Position to start the search at
 * @param direction Direction in which to perform the search
 * @returns The position at which the run ends
 */
static size_t cli_skip_run(FuriString* string, size_t original_pos, CliSkipDirection direction) {
    if(furi_string_size(string) == 0) return original_pos;
    if(direction == CliSkipDirectionLeft && original_pos == 0) return original_pos;
    if(direction == CliSkipDirectionRight && original_pos == furi_string_size(string))
        return original_pos;

    int8_t look_offset = (direction == CliSkipDirectionLeft) ? -1 : 0;
    int8_t increment = (direction == CliSkipDirectionLeft) ? -1 : 1;
    int32_t position = original_pos;
    CliCharClass start_class =
        cli_char_class(furi_string_get_char(string, position + look_offset));

    while(true) {
        position += increment;
        if(position < 0) break;
        if(position >= (int32_t)furi_string_size(string)) break;
        if(cli_char_class(furi_string_get_char(string, position + look_offset)) != start_class)
            break;
    }

    return MAX(0, position);
}

void cli_process_input(Cli* cli) {
    CliKeyCombo combo = cli_read_ansi_key_combo(cli);
    FURI_LOG_T(TAG, "code=0x%02x, mod=0x%x\r\n", combo.key, combo.modifiers);

    if(combo.key == CliKeyTab) {
        cli_handle_autocomplete(cli);

    } else if(combo.key == CliKeySOH) {
        furi_delay_ms(33); // We are too fast, Minicom is not ready yet
        cli_motd();
        cli_prompt(cli);

    } else if(combo.key == CliKeyETX) {
        cli_reset(cli);
        cli_prompt(cli);

    } else if(combo.key == CliKeyEOT) {
        cli_reset(cli);

    } else if(combo.key == CliKeyUp && combo.modifiers == CliModKeyNo) {
        // Use previous command if line buffer is empty
        if(furi_string_size(cli->line) == 0 && furi_string_cmp(cli->line, cli->last_line) != 0) {
            // Set line buffer and cursor position
            furi_string_set(cli->line, cli->last_line);
            cli->cursor_position = furi_string_size(cli->line);
            // Show new line to user
            printf("%s", furi_string_get_cstr(cli->line));
        }

    } else if(combo.key == CliKeyDown && combo.modifiers == CliModKeyNo) {
        // Clear input buffer
        furi_string_reset(cli->line);
        cli->cursor_position = 0;
        printf("\r" CLI_PROMPT "\e[0K");

    } else if(combo.key == CliKeyRight && combo.modifiers == CliModKeyNo) {
        // Move right
        if(cli->cursor_position < furi_string_size(cli->line)) {
            cli->cursor_position++;
            printf("\e[C");
        }

    } else if(combo.key == CliKeyLeft && combo.modifiers == CliModKeyNo) {
        // Move left
        if(cli->cursor_position > 0) {
            cli->cursor_position--;
            printf("\e[D");
        }

    } else if(combo.key == CliKeyHome && combo.modifiers == CliModKeyNo) {
        // Move to beginning of line
        cli->cursor_position = 0;
        printf("\e[%uG", CLI_PROMPT_LENGTH + 1); // columns start at 1 \(-_-)/

    } else if(combo.key == CliKeyEnd && combo.modifiers == CliModKeyNo) {
        // Move to end of line
        cli->cursor_position = furi_string_size(cli->line);
        printf("\e[%zuG", CLI_PROMPT_LENGTH + cli->cursor_position + 1);

    } else if(
        combo.modifiers == CliModKeyCtrl &&
        (combo.key == CliKeyLeft || combo.key == CliKeyRight)) {
        // Skip run of similar chars to the left or right
        CliSkipDirection direction = (combo.key == CliKeyLeft) ? CliSkipDirectionLeft :
                                                                 CliSkipDirectionRight;
        cli->cursor_position = cli_skip_run(cli->line, cli->cursor_position, direction);
        printf("\e[%zuG", CLI_PROMPT_LENGTH + cli->cursor_position + 1);

    } else if(combo.key == CliKeyBackspace || combo.key == CliKeyDEL) {
        cli_handle_backspace(cli);

    } else if(combo.key == CliKeyETB) { // Ctrl + Backspace
        // Delete run of similar chars to the left
        size_t run_start = cli_skip_run(cli->line, cli->cursor_position, CliSkipDirectionLeft);
        furi_string_replace_at(cli->line, run_start, cli->cursor_position - run_start, "");
        cli->cursor_position = run_start;
        printf(
            "\e[%zuG%s\e[0K\e[%zuG", // move cursor, print second half of line, erase remains, move cursor again
            CLI_PROMPT_LENGTH + cli->cursor_position + 1,
            furi_string_get_cstr(cli->line) + run_start,
            CLI_PROMPT_LENGTH + run_start + 1);

    } else if(combo.key == CliKeyCR) {
        cli_handle_enter(cli);

    } else if(
        (combo.key >= 0x20 && combo.key < 0x7F) && //-V560
        (furi_string_size(cli->line) < CLI_INPUT_LEN_LIMIT)) {
        if(cli->cursor_position == furi_string_size(cli->line)) {
            furi_string_push_back(cli->line, combo.key);
            cli_putc(cli, combo.key);
        } else {
            // Insert character to line buffer
            const char in_str[2] = {combo.key, 0};
            furi_string_replace_at(cli->line, cli->cursor_position, 0, in_str);

            // Print character in replace mode
            printf("\e[4h%c\e[4l", combo.key);
            fflush(stdout);
        }
        cli->cursor_position++;

    } else {
        cli_putc(cli, CliKeyBell);
    }

    fflush(stdout);
}

void cli_add_command(
    Cli* cli,
    const char* name,
    CliCommandFlag flags,
    CliCallback callback,
    void* context) {
    furi_check(cli);
    FuriString* name_str;
    name_str = furi_string_alloc_set(name);
    furi_string_trim(name_str);

    size_t name_replace;
    do {
        name_replace = furi_string_replace(name_str, " ", "_");
    } while(name_replace != FURI_STRING_FAILURE);

    CliCommand c;
    c.callback = callback;
    c.context = context;
    c.flags = flags;

    furi_check(furi_mutex_acquire(cli->mutex, FuriWaitForever) == FuriStatusOk);
    CliCommandTree_set_at(cli->commands, name_str, c);
    furi_check(furi_mutex_release(cli->mutex) == FuriStatusOk);

    furi_string_free(name_str);
}

void cli_delete_command(Cli* cli, const char* name) {
    furi_check(cli);
    FuriString* name_str;
    name_str = furi_string_alloc_set(name);
    furi_string_trim(name_str);

    size_t name_replace;
    do {
        name_replace = furi_string_replace(name_str, " ", "_");
    } while(name_replace != FURI_STRING_FAILURE);

    furi_check(furi_mutex_acquire(cli->mutex, FuriWaitForever) == FuriStatusOk);
    CliCommandTree_erase(cli->commands, name_str);
    furi_check(furi_mutex_release(cli->mutex) == FuriStatusOk);

    furi_string_free(name_str);
}

void cli_session_open(Cli* cli, void* session) {
    furi_check(cli);

    furi_check(furi_mutex_acquire(cli->mutex, FuriWaitForever) == FuriStatusOk);
    cli->session = session;
    if(cli->session != NULL) {
        cli->session->init();
        furi_thread_set_stdout_callback(cli->session->tx_stdout);
    } else {
        furi_thread_set_stdout_callback(NULL);
    }
    furi_semaphore_release(cli->idle_sem);
    furi_check(furi_mutex_release(cli->mutex) == FuriStatusOk);
}

void cli_session_close(Cli* cli) {
    furi_check(cli);

    furi_check(furi_mutex_acquire(cli->mutex, FuriWaitForever) == FuriStatusOk);
    if(cli->session != NULL) {
        cli->session->deinit();
    }
    cli->session = NULL;
    furi_thread_set_stdout_callback(NULL);
    furi_check(furi_mutex_release(cli->mutex) == FuriStatusOk);
}

int32_t cli_srv(void* p) {
    UNUSED(p);
    Cli* cli = cli_alloc();

    // Init basic cli commands
    cli_commands_init(cli);

    furi_record_create(RECORD_CLI, cli);

    if(cli->session != NULL) {
        furi_thread_set_stdout_callback(cli->session->tx_stdout);
    } else {
        furi_thread_set_stdout_callback(NULL);
    }

    if(furi_hal_rtc_get_boot_mode() == FuriHalRtcBootModeNormal) {
        cli_session_open(cli, &cli_vcp);
    } else {
        FURI_LOG_W(TAG, "Skipping start in special boot mode");
    }

    while(1) {
        if(cli->session != NULL) {
            cli_process_input(cli);
        } else {
            furi_check(furi_semaphore_acquire(cli->idle_sem, FuriWaitForever) == FuriStatusOk);
        }
    }

    return 0;
}
