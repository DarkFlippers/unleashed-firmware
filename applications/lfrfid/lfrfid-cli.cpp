#include <furi.h>
#include <api-hal.h>
#include <stdarg.h>
#include <cli/cli.h>
#include <args.h>

#include "helpers/rfid-reader.h"
#include "helpers/rfid-timer-emulator.h"

void lfrfid_cli(Cli* cli, string_t args, void* context);

// app cli function
extern "C" void lfrfid_cli_init() {
    Cli* cli = static_cast<Cli*>(furi_record_open("cli"));
    cli_add_command(cli, "rfid", CliCommandFlagDefault, lfrfid_cli, NULL);
    furi_record_close("cli");
}

void lfrfid_cli_print_usage() {
    printf("Usage:\r\n");
    printf("rfid read\r\n");
    printf("rfid <write | emulate> <key_type> <key_data>\r\n");
    printf("\t<key_type> choose from:\r\n");
    printf("\tEM4100, EM-Marin (5 bytes key_data)\r\n");
    printf("\tH10301, HID26 (3 bytes key_data)\r\n");
    printf("\tI40134, Indala (3 bytes key_data)\r\n");
    printf("\t<key_data> are hex-formatted\r\n");
};

bool lfrfid_cli_get_key_type(string_t data, LfrfidKeyType* type) {
    bool result = false;

    if(string_cmp_str(data, "EM4100") == 0 || string_cmp_str(data, "EM-Marin") == 0) {
        result = true;
        *type = LfrfidKeyType::KeyEM4100;
    } else if(string_cmp_str(data, "H10301") == 0 || string_cmp_str(data, "HID26") == 0) {
        result = true;
        *type = LfrfidKeyType::KeyH10301;
    } else if(string_cmp_str(data, "I40134") == 0 || string_cmp_str(data, "Indala") == 0) {
        result = true;
        *type = LfrfidKeyType::KeyI40134;
    }

    return result;
}

void lfrfid_cli_read(Cli* cli) {
    RfidReader reader;
    reader.start(RfidReader::Type::Normal);

    static const uint8_t data_size = LFRFID_KEY_SIZE;
    uint8_t data[data_size] = {0};
    LfrfidKeyType type;

    printf("Reading RFID...\r\nPress Ctrl+C to abort\r\n");
    while(!cli_cmd_interrupt_received(cli)) {
        if(reader.read(&type, data, data_size)) {
            printf(lfrfid_key_get_type_string(type));
            printf(" ");

            for(uint8_t i = 0; i < lfrfid_key_get_type_data_count(type); i++) {
                printf("%02X", data[i]);
            }
            printf("\r\n");
            break;
        }
        delay(100);
    }

    printf("Reading stopped\r\n");
    reader.stop();
}

void lfrfid_cli_write(Cli* cli, string_t args) {
    // TODO implement rfid write
    printf("Not implemented :(\r\n");
}

void lfrfid_cli_emulate(Cli* cli, string_t args) {
    string_t data;
    string_init(data);
    RfidTimerEmulator emulator;

    static const uint8_t data_size = LFRFID_KEY_SIZE;
    uint8_t key_data[data_size] = {0};
    uint8_t key_data_size = 0;
    LfrfidKeyType type;

    if(!args_read_string_and_trim(args, data)) {
        lfrfid_cli_print_usage();
        string_clear(data);
        return;
    }

    if(!lfrfid_cli_get_key_type(data, &type)) {
        lfrfid_cli_print_usage();
        string_clear(data);
        return;
    }

    key_data_size = lfrfid_key_get_type_data_count(type);

    if(!args_read_hex_bytes(args, key_data, key_data_size)) {
        lfrfid_cli_print_usage();
        string_clear(data);
        return;
    }

    emulator.start(type, key_data, key_data_size);

    printf("Emulating RFID...\r\nPress Ctrl+C to abort\r\n");
    while(!cli_cmd_interrupt_received(cli)) {
        delay(100);
    }
    printf("Emulation stopped\r\n");
    emulator.stop();

    string_clear(data);
}

void lfrfid_cli(Cli* cli, string_t args, void* context) {
    string_t cmd;
    string_init(cmd);

    if(!args_read_string_and_trim(args, cmd)) {
        string_clear(cmd);
        lfrfid_cli_print_usage();
        return;
    }

    if(string_cmp_str(cmd, "read") == 0) {
        lfrfid_cli_read(cli);
    } else if(string_cmp_str(cmd, "write") == 0) {
        lfrfid_cli_write(cli, args);
    } else if(string_cmp_str(cmd, "emulate") == 0) {
        lfrfid_cli_emulate(cli, args);
    } else {
        lfrfid_cli_print_usage();
    }

    string_clear(cmd);
}
