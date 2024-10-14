#pragma once

#include "cli.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ANSI_RESET "\e[0m"
#define ANSI_BOLD  "\e[1m"
#define ANSI_FAINT "\e[2m"

#define ANSI_FG_BLACK      "\e[30m"
#define ANSI_FG_RED        "\e[31m"
#define ANSI_FG_GREEN      "\e[32m"
#define ANSI_FG_YELLOW     "\e[33m"
#define ANSI_FG_BLUE       "\e[34m"
#define ANSI_FG_MAGENTA    "\e[35m"
#define ANSI_FG_CYAN       "\e[36m"
#define ANSI_FG_WHITE      "\e[37m"
#define ANSI_FG_BR_BLACK   "\e[90m"
#define ANSI_FG_BR_RED     "\e[91m"
#define ANSI_FG_BR_GREEN   "\e[92m"
#define ANSI_FG_BR_YELLOW  "\e[93m"
#define ANSI_FG_BR_BLUE    "\e[94m"
#define ANSI_FG_BR_MAGENTA "\e[95m"
#define ANSI_FG_BR_CYAN    "\e[96m"
#define ANSI_FG_BR_WHITE   "\e[97m"

#define ANSI_BG_BLACK      "\e[40m"
#define ANSI_BG_RED        "\e[41m"
#define ANSI_BG_GREEN      "\e[42m"
#define ANSI_BG_YELLOW     "\e[43m"
#define ANSI_BG_BLUE       "\e[44m"
#define ANSI_BG_MAGENTA    "\e[45m"
#define ANSI_BG_CYAN       "\e[46m"
#define ANSI_BG_WHITE      "\e[47m"
#define ANSI_BG_BR_BLACK   "\e[100m"
#define ANSI_BG_BR_RED     "\e[101m"
#define ANSI_BG_BR_GREEN   "\e[102m"
#define ANSI_BG_BR_YELLOW  "\e[103m"
#define ANSI_BG_BR_BLUE    "\e[104m"
#define ANSI_BG_BR_MAGENTA "\e[105m"
#define ANSI_BG_BR_CYAN    "\e[106m"
#define ANSI_BG_BR_WHITE   "\e[107m"

#define ANSI_FLIPPER_BRAND_ORANGE "\e[38;2;255;130;0m"

typedef enum {
    CliKeyUnrecognized = 0,

    CliKeySOH = 0x01,
    CliKeyETX = 0x03,
    CliKeyEOT = 0x04,
    CliKeyBell = 0x07,
    CliKeyBackspace = 0x08,
    CliKeyTab = 0x09,
    CliKeyLF = 0x0A,
    CliKeyCR = 0x0D,
    CliKeyETB = 0x17,
    CliKeyEsc = 0x1B,
    CliKeyUS = 0x1F,
    CliKeySpace = 0x20,
    CliKeyDEL = 0x7F,

    CliKeySpecial = 0x80,
    CliKeyLeft,
    CliKeyRight,
    CliKeyUp,
    CliKeyDown,
    CliKeyHome,
    CliKeyEnd,
} CliKey;

typedef enum {
    CliModKeyNo = 0,
    CliModKeyAlt = 2,
    CliModKeyCtrl = 4,
    CliModKeyMeta = 8,
} CliModKey;

typedef struct {
    CliModKey modifiers;
    CliKey key;
} CliKeyCombo;

/**
 * @brief Reads a key or key combination
 */
CliKeyCombo cli_read_ansi_key_combo(Cli* cli);

#ifdef __cplusplus
}
#endif
