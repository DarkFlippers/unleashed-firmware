#include "sim800_settings.h"
#include "sim800_types.h"

#include <furi.h>
#include <storage/storage.h>
#include <flipper_format/flipper_format.h>

#define TAG "Sim800Settings"

#define SIM800_SETTINGS_FILE_TYPE    "Flipper SIM800 Settings File"
#define SIM800_SETTINGS_FILE_VERSION 1

#define SIM800_SETTINGS_FIELD_PORT   "UartPort"
#define SIM800_SETTINGS_FIELD_BAUD   "Baudrate"
#define SIM800_SETTINGS_FIELD_NOTIFY "Notify"
#define SIM800_SETTINGS_FIELD_ECHO   "Echo"
#define SIM800_SETTINGS_FIELD_PWRKEY "PwrkeyPin"

const uint32_t sim800_settings_baudrates[SIM800_BAUDRATE_COUNT] =
    {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400};

const char* const sim800_settings_baudrate_names[SIM800_BAUDRATE_COUNT] =
    {"1200", "2400", "4800", "9600", "19200", "38400", "57600", "115200", "230400"};

const char* const sim800_settings_port_names[Sim800UartCount] = {"USART", "LPUART"};

const char* const sim800_settings_port_pins[Sim800UartCount] = {
    "TX 13 / RX 14",
    "TX 15 / RX 16",
};

// Flipper header numbers, which is what the case is marked with. 2-6 are the external CC1101's
// SPI and G0, so with one of those attached the only free pins are the UART pair this app is
// not using - the app refuses the pair it is using.
const char* const sim800_settings_pwrkey_names[SIM800_PWRKEY_COUNT] = {
    "Off",
    "2",
    "3",
    "4",
    "5",
    "6",
    "13",
    "14",
    "15",
    "16",
};

void sim800_settings_load(Sim800Settings* instance) {
    furi_assert(instance);

    instance->uart_port = Sim800UartUsart;
    instance->baudrate = SIM800_DEFAULT_BAUDRATE;
    instance->notify = true;
    instance->echo = false;
    instance->pwrkey_pin = 0;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);
    FuriString* temp_str = furi_string_alloc();
    uint32_t version = 0;

    if(flipper_format_file_open_existing(file, SIM800_SETTINGS_PATH)) {
        do {
            if(!flipper_format_read_header(file, temp_str, &version)) break;
            if(strcmp(furi_string_get_cstr(temp_str), SIM800_SETTINGS_FILE_TYPE) != 0 ||
               version != SIM800_SETTINGS_FILE_VERSION) {
                FURI_LOG_W(TAG, "Type or version mismatch, using defaults");
                break;
            }
            // Every field is optional; rewind so a missing one does not eat the rest
            flipper_format_rewind(file);
            flipper_format_read_uint32(file, SIM800_SETTINGS_FIELD_PORT, &instance->uart_port, 1);
            flipper_format_rewind(file);
            flipper_format_read_uint32(file, SIM800_SETTINGS_FIELD_BAUD, &instance->baudrate, 1);
            flipper_format_rewind(file);
            flipper_format_read_bool(file, SIM800_SETTINGS_FIELD_NOTIFY, &instance->notify, 1);
            flipper_format_rewind(file);
            flipper_format_read_bool(file, SIM800_SETTINGS_FIELD_ECHO, &instance->echo, 1);
            flipper_format_rewind(file);
            flipper_format_read_uint32(
                file, SIM800_SETTINGS_FIELD_PWRKEY, &instance->pwrkey_pin, 1);
            if(instance->pwrkey_pin >= SIM800_PWRKEY_COUNT) instance->pwrkey_pin = 0;
        } while(false);
    }

    flipper_format_free(file);
    furi_string_free(temp_str);
    furi_record_close(RECORD_STORAGE);

    if(instance->uart_port >= Sim800UartCount) {
        instance->uart_port = Sim800UartUsart;
    }
    bool baud_known = false;
    for(size_t i = 0; i < SIM800_BAUDRATE_COUNT; i++) {
        if(sim800_settings_baudrates[i] == instance->baudrate) {
            baud_known = true;
            break;
        }
    }
    if(!baud_known) {
        instance->baudrate = SIM800_DEFAULT_BAUDRATE;
    }
}

bool sim800_settings_save(const Sim800Settings* instance) {
    furi_assert(instance);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);
    bool saved = false;

    do {
        if(!flipper_format_file_open_always(file, SIM800_SETTINGS_PATH)) break;
        if(!flipper_format_write_header_cstr(
               file, SIM800_SETTINGS_FILE_TYPE, SIM800_SETTINGS_FILE_VERSION))
            break;
        if(!flipper_format_write_uint32(file, SIM800_SETTINGS_FIELD_PORT, &instance->uart_port, 1))
            break;
        if(!flipper_format_write_uint32(file, SIM800_SETTINGS_FIELD_BAUD, &instance->baudrate, 1))
            break;
        if(!flipper_format_write_bool(file, SIM800_SETTINGS_FIELD_NOTIFY, &instance->notify, 1))
            break;
        if(!flipper_format_write_bool(file, SIM800_SETTINGS_FIELD_ECHO, &instance->echo, 1)) break;
        if(!flipper_format_write_uint32(
               file, SIM800_SETTINGS_FIELD_PWRKEY, &instance->pwrkey_pin, 1))
            break;
        saved = true;
    } while(false);

    if(!saved) FURI_LOG_E(TAG, "Failed to save settings");

    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);
    return saved;
}
