#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

// text styling

#define ANSI_RESET  "\e[0m"
#define ANSI_BOLD   "\e[1m"
#define ANSI_FAINT  "\e[2m"
#define ANSI_INVERT "\e[7m"

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

// cursor positioning

#define ANSI_CURSOR_UP_BY(rows)                    "\e[" rows "A"
#define ANSI_CURSOR_DOWN_BY(rows)                  "\e[" rows "B"
#define ANSI_CURSOR_RIGHT_BY(cols)                 "\e[" cols "C"
#define ANSI_CURSOR_LEFT_BY(cols)                  "\e[" cols "D"
#define ANSI_CURSOR_DOWN_BY_AND_FIRST_COLUMN(rows) "\e[" rows "E"
#define ANSI_CURSOR_UP_BY_AND_FIRST_COLUMN(rows)   "\e[" rows "F"
#define ANSI_CURSOR_HOR_POS(pos)                   "\e[" pos "G"
#define ANSI_CURSOR_POS(row, col)                  "\e[" row ";" col "H"

// erasing

#define ANSI_ERASE_FROM_CURSOR_TO_END   "0"
#define ANSI_ERASE_FROM_START_TO_CURSOR "1"
#define ANSI_ERASE_ENTIRE               "2"

#define ANSI_ERASE_DISPLAY(portion)  "\e[" portion "J"
#define ANSI_ERASE_LINE(portion)     "\e[" portion "K"
#define ANSI_ERASE_SCROLLBACK_BUFFER ANSI_ERASE_DISPLAY("3")

// misc

#define ANSI_INSERT_MODE_ENABLE  "\e[4h"
#define ANSI_INSERT_MODE_DISABLE "\e[4l"

typedef enum {
    CliKeyUnrecognized = 0,

    CliKeySOH = 0x01,
    CliKeyETX = 0x03,
    CliKeyEOT = 0x04,
    CliKeyBell = 0x07,
    CliKeyBackspace = 0x08,
    CliKeyTab = 0x09,
    CliKeyLF = 0x0A,
    CliKeyFF = 0x0C,
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

typedef struct CliAnsiParser CliAnsiParser;

typedef struct {
    bool is_done;
    CliKeyCombo result;
} CliAnsiParserResult;

/**
 * @brief Allocates an ANSI parser
 */
CliAnsiParser* cli_ansi_parser_alloc(void);

/**
 * @brief Frees an ANSI parser
 */
void cli_ansi_parser_free(CliAnsiParser* parser);

/**
 * @brief Feeds an ANSI parser a character
 */
CliAnsiParserResult cli_ansi_parser_feed(CliAnsiParser* parser, char c);

/**
 * @brief Feeds an ANSI parser a timeout event
 * 
 * As a user of the ANSI parser API, you are responsible for calling this
 * function some time after the last character was fed into the parser. The
 * recommended timeout is about 10 ms. The exact value does not matter as long
 * as it is small enough for the user not notice a delay, but big enough that
 * when a terminal is sending an escape sequence, this function does not get
 * called in between the characters of the sequence.
 */
CliAnsiParserResult cli_ansi_parser_feed_timeout(CliAnsiParser* parser);

#ifdef __cplusplus
}
#endif
