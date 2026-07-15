#include "nfc_cli_command_emulate.h"
#include "helpers/nfc_cli_format.h"

#include <nfc.h>
#include <nfc_listener.h>
#include <nfc_device.h>

#include <storage/storage.h>

typedef struct {
    Nfc* nfc;
    NfcDevice* nfc_device;
    FuriString* file_path;
    Storage* storage;
} NfcCliEmulateContext;

static NfcCliActionContext* nfc_cli_emulate_alloc_ctx(Nfc* nfc) {
    furi_assert(nfc);
    NfcCliEmulateContext* instance = malloc(sizeof(NfcCliEmulateContext));
    instance->nfc = nfc;
    instance->file_path = furi_string_alloc();
    instance->nfc_device = nfc_device_alloc();
    instance->storage = furi_record_open(RECORD_STORAGE);
    return instance;
}

static void nfc_cli_emulate_free_ctx(NfcCliActionContext* ctx) {
    furi_assert(ctx);
    NfcCliEmulateContext* instance = ctx;
    furi_record_close(RECORD_STORAGE);
    furi_string_free(instance->file_path);
    nfc_device_free(instance->nfc_device);
    free(instance);
}

static const NfcProtocol supported_protocols[] = {
    NfcProtocolIso14443_3a,
    NfcProtocolIso14443_4a,
    NfcProtocolIso15693_3,
    NfcProtocolMfUltralight,
    NfcProtocolMfClassic,
    NfcProtocolSlix,
    NfcProtocolFelica,
};

static bool nfc_cli_emulate_protocol_supports_emulation(NfcProtocol protocol) {
    for(size_t i = 0; i < COUNT_OF(supported_protocols); i++) {
        if(supported_protocols[i] == protocol) return true;
    }
    return false;
}

static void nfc_cli_emulate_execute(PipeSide* pipe, NfcCliActionContext* context) {
    UNUSED(pipe);
    furi_assert(context);
    NfcCliEmulateContext* instance = context;
    do {
        const char* path = furi_string_get_cstr(instance->file_path);
        if(!storage_common_exists(instance->storage, path)) {
            printf(ANSI_FG_RED "Wrong path \'%s\'.\r\n" ANSI_RESET, path);
            break;
        }

        if(!nfc_device_load(instance->nfc_device, path)) {
            printf(ANSI_FG_RED "Failed to load \'%s\'.\r\n" ANSI_RESET, path);
            break;
        }

        const NfcProtocol protocol = nfc_device_get_protocol(instance->nfc_device);

        if(!nfc_cli_emulate_protocol_supports_emulation(protocol)) {
            printf(
                ANSI_FG_RED "Error. Emulation for %s is not supported\r\n" ANSI_RESET,
                nfc_cli_get_protocol_name(protocol));
            break;
        }

        const NfcDeviceData* data = nfc_device_get_data(instance->nfc_device, protocol);
        NfcListener* listener = nfc_listener_alloc(instance->nfc, protocol, data);

        nfc_listener_start(listener, NULL, NULL);
        printf("\r\nEmulating. Press Ctrl+C to abort\r\n");
        while(!cli_is_pipe_broken_or_is_etx_next_char(pipe)) {
            furi_delay_ms(100);
        }
        nfc_listener_stop(listener);
        nfc_listener_free(listener);
    } while(false);
}

static bool nfc_cli_emulate_parse_filename_key(FuriString* value, void* output) {
    furi_assert(value);
    furi_assert(output);
    NfcCliEmulateContext* ctx = output;
    furi_string_set(ctx->file_path, value);
    return true;
}

const NfcCliKeyDescriptor emulate_keys[] = {
    {
        .features = {.required = true, .parameter = true},
        .long_name = "file",
        .short_name = "f",
        .description = "path to new file",
        .parse = nfc_cli_emulate_parse_filename_key,
    },
};

const NfcCliActionDescriptor emulate_action = {
    .name = "emulate",
    .description = "Emulate .nfc file content",
    .alloc = nfc_cli_emulate_alloc_ctx,
    .free = nfc_cli_emulate_free_ctx,
    .execute = nfc_cli_emulate_execute,
    .key_count = COUNT_OF(emulate_keys),
    .keys = emulate_keys,
};

const NfcCliActionDescriptor* emulate_actions_collection[] = {&emulate_action};

//Command descriptor
ADD_NFC_CLI_COMMAND(emulate, "", emulate_actions_collection);

//Command usage: emulate [-f <file>]
//Command examples:
//emulate -f ext/nfc/test.nfc
