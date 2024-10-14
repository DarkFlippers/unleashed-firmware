#include "cli_ansi.h"

/**
 * @brief Converts a single character representing a special key into the enum
 * representation
 */
static CliKey cli_ansi_key_from_mnemonic(char c) {
    switch(c) {
    case 'A':
        return CliKeyUp;
    case 'B':
        return CliKeyDown;
    case 'C':
        return CliKeyRight;
    case 'D':
        return CliKeyLeft;
    case 'F':
        return CliKeyEnd;
    case 'H':
        return CliKeyHome;
    default:
        return CliKeyUnrecognized;
    }
}

CliKeyCombo cli_read_ansi_key_combo(Cli* cli) {
    char ch = cli_getc(cli);

    if(ch != CliKeyEsc)
        return (CliKeyCombo){
            .modifiers = CliModKeyNo,
            .key = ch,
        };

    ch = cli_getc(cli);

    // ESC ESC -> ESC
    if(ch == '\e')
        return (CliKeyCombo){
            .modifiers = CliModKeyNo,
            .key = '\e',
        };

    // ESC <char> -> Alt + <char>
    if(ch != '[')
        return (CliKeyCombo){
            .modifiers = CliModKeyAlt,
            .key = cli_getc(cli),
        };

    ch = cli_getc(cli);

    // ESC [ 1
    if(ch == '1') {
        // ESC [ 1 ; <modifier bitfield> <key mnemonic>
        if(cli_getc(cli) == ';') {
            CliModKey modifiers = (cli_getc(cli) - '0'); // convert following digit to a number
            modifiers &= ~1;
            return (CliKeyCombo){
                .modifiers = modifiers,
                .key = cli_ansi_key_from_mnemonic(cli_getc(cli)),
            };
        }

        return (CliKeyCombo){
            .modifiers = CliModKeyNo,
            .key = CliKeyUnrecognized,
        };
    }

    // ESC [ <key mnemonic>
    return (CliKeyCombo){
        .modifiers = CliModKeyNo,
        .key = cli_ansi_key_from_mnemonic(ch),
    };
}
