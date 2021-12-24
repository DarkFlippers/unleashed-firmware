#include <furi.h>
#include <furi-hal.h>
#include <stdarg.h>
#include <cli/cli.h>
#include <lib/toolbox/args.h>

#include "helpers/key-info.h"
#include "helpers/key-worker.h"

#include <memory>

void ibutton_cli(Cli* cli, string_t args, void* context);
void onewire_cli(Cli* cli, string_t args, void* context);

// app cli function
extern "C" void ibutton_on_system_start() {
#ifdef SRV_CLI
    Cli* cli = static_cast<Cli*>(furi_record_open("cli"));
    cli_add_command(cli, "ikey", CliCommandFlagDefault, ibutton_cli, cli);
    cli_add_command(cli, "onewire", CliCommandFlagDefault, onewire_cli, cli);
    furi_record_close("cli");
#endif
}

void ibutton_cli_print_usage() {
    printf("Usage:\r\n");
    printf("ikey read\r\n");
    printf("ikey <write | emulate> <key_type> <key_data>\r\n");
    printf("\t<key_type> choose from:\r\n");
    printf("\tDallas (8 bytes key_data)\r\n");
    printf("\tCyfral (2 bytes key_data)\r\n");
    printf("\tMetakom (4 bytes key_data)\r\n");
    printf("\t<key_data> are hex-formatted\r\n");
};

bool ibutton_cli_get_key_type(string_t data, iButtonKeyType* type) {
    bool result = false;

    if(string_cmp_str(data, "Dallas") == 0 || string_cmp_str(data, "dallas") == 0) {
        result = true;
        *type = iButtonKeyType::KeyDallas;
    } else if(string_cmp_str(data, "Cyfral") == 0 || string_cmp_str(data, "cyfral") == 0) {
        result = true;
        *type = iButtonKeyType::KeyCyfral;
    } else if(string_cmp_str(data, "Metakom") == 0 || string_cmp_str(data, "metakom") == 0) {
        result = true;
        *type = iButtonKeyType::KeyMetakom;
    }

    return result;
}

void ibutton_cli_print_key_data(iButtonKey* key) {
    uint8_t* key_data = key->get_data();
    switch(key->get_key_type()) {
    case iButtonKeyType::KeyDallas:
        printf(
            "Dallas %02X%02X%02X%02X%02X%02X%02X%02X\r\n",
            key_data[0],
            key_data[1],
            key_data[2],
            key_data[3],
            key_data[4],
            key_data[5],
            key_data[6],
            key_data[7]);
        break;
    case iButtonKeyType::KeyCyfral:
        printf("Cyfral %02X%02X\r\n", key_data[0], key_data[1]);
        break;
    case iButtonKeyType::KeyMetakom:
        printf("Metakom %02X%02X%02X%02X\r\n", key_data[0], key_data[1], key_data[2], key_data[3]);
        break;
    }
}

void ibutton_cli_read(Cli* cli) {
    iButtonKey key;
    std::unique_ptr<KeyWorker> worker(new KeyWorker(&ibutton_gpio));

    bool exit = false;

    worker->start_read();
    printf("Reading iButton...\r\nPress Ctrl+C to abort\r\n");

    while(!exit) {
        exit = cli_cmd_interrupt_received(cli);

        switch(worker->read(&key)) {
        case KeyReader::Error::EMPTY:
            break;
        case KeyReader::Error::CRC_ERROR:
            ibutton_cli_print_key_data(&key);
            printf("Warning: invalid CRC\r\n");
            exit = true;
            break;
        case KeyReader::Error::OK:
            ibutton_cli_print_key_data(&key);
            exit = true;
            break;
        case KeyReader::Error::NOT_ARE_KEY:
            ibutton_cli_print_key_data(&key);
            printf("Warning: not a key\r\n");
            exit = true;
            break;
        }

        delay(100);
    }

    worker->stop_read();
};

void ibutton_cli_write(Cli* cli, string_t args) {
    iButtonKey key;
    iButtonKeyType type;
    std::unique_ptr<KeyWorker> worker(new KeyWorker(&ibutton_gpio));

    bool exit = false;
    string_t data;
    string_init(data);

    if(!args_read_string_and_trim(args, data)) {
        ibutton_cli_print_usage();
        string_clear(data);
        return;
    }

    if(!ibutton_cli_get_key_type(data, &type)) {
        ibutton_cli_print_usage();
        string_clear(data);
        return;
    }

    key.set_type(type);

    if(!args_read_hex_bytes(args, key.get_data(), key.get_type_data_size())) {
        ibutton_cli_print_usage();
        string_clear(data);
        return;
    }

    printf("Writing key ");
    ibutton_cli_print_key_data(&key);
    printf("Press Ctrl+C to abort\r\n");

    worker->start_write();

    while(!exit) {
        exit = cli_cmd_interrupt_received(cli);

        KeyWriter::Error result = worker->write(&key);

        switch(result) {
        case KeyWriter::Error::SAME_KEY:
        case KeyWriter::Error::OK:
            printf("Write success\r\n");
            exit = true;
            break;
        case KeyWriter::Error::NO_DETECT:
            break;
        case KeyWriter::Error::CANNOT_WRITE:
            printf("Write fail\r\n");
            exit = true;
            break;
        }
    };

    worker->stop_write();

    string_clear(data);
};

void ibutton_cli_emulate(Cli* cli, string_t args) {
    iButtonKey key;
    iButtonKeyType type;
    std::unique_ptr<KeyWorker> worker(new KeyWorker(&ibutton_gpio));
    bool exit = false;
    string_t data;
    string_init(data);

    if(!args_read_string_and_trim(args, data)) {
        ibutton_cli_print_usage();
        string_clear(data);
        return;
    }

    if(!ibutton_cli_get_key_type(data, &type)) {
        ibutton_cli_print_usage();
        string_clear(data);
        return;
    }

    key.set_type(type);

    if(!args_read_hex_bytes(args, key.get_data(), key.get_type_data_size())) {
        ibutton_cli_print_usage();
        string_clear(data);
        return;
    }

    printf("Emulating key ");
    ibutton_cli_print_key_data(&key);
    printf("Press Ctrl+C to abort\r\n");

    worker->start_emulate(&key);

    while(!exit) {
        exit = cli_cmd_interrupt_received(cli);
    };

    worker->stop_emulate();

    string_clear(data);
};

void ibutton_cli(Cli* cli, string_t args, void* context) {
    string_t cmd;
    string_init(cmd);

    if(!args_read_string_and_trim(args, cmd)) {
        string_clear(cmd);
        ibutton_cli_print_usage();
        return;
    }

    if(string_cmp_str(cmd, "read") == 0) {
        ibutton_cli_read(cli);
    } else if(string_cmp_str(cmd, "write") == 0) {
        ibutton_cli_write(cli, args);
    } else if(string_cmp_str(cmd, "emulate") == 0) {
        ibutton_cli_emulate(cli, args);
    } else {
        ibutton_cli_print_usage();
    }

    string_clear(cmd);
}

void onewire_cli_print_usage() {
    printf("Usage:\r\n");
    printf("onewire search\r\n");
};

void onewire_cli_search(Cli* cli) {
    OneWireMaster onewire(&ibutton_gpio);
    uint8_t address[8];
    bool done = false;

    printf("Search started\r\n");

    onewire.start();
    furi_hal_power_enable_otg();

    while(!done) {
        if(onewire.search(address, true) != 1) {
            printf("Search finished\r\n");
            onewire.reset_search();
            done = true;
        } else {
            printf("Found: ");
            for(uint8_t i = 0; i < 8; i++) {
                printf("%02X", address[i]);
            }
            printf("\r\n");
        }
        delay(100);
    }

    furi_hal_power_disable_otg();
    onewire.stop();
}

void onewire_cli(Cli* cli, string_t args, void* context) {
    string_t cmd;
    string_init(cmd);

    if(!args_read_string_and_trim(args, cmd)) {
        string_clear(cmd);
        onewire_cli_print_usage();
        return;
    }

    if(string_cmp_str(cmd, "search") == 0) {
        onewire_cli_search(cli);
    }

    string_clear(cmd);
}
