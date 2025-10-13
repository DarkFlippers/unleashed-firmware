#include "nfc_cli_command_dump.h"
#include "protocols/nfc_cli_dump_common_types.h"
#include "../helpers/nfc_cli_format.h"
#include "../helpers/nfc_cli_protocol_parser.h"
#include "../helpers/nfc_cli_scanner.h"

#include "protocols/iso14443_3a/nfc_cli_dump_iso14443_3a.h"
#include "protocols/iso14443_3b/nfc_cli_dump_iso14443_3b.h"
#include "protocols/iso14443_4a/nfc_cli_dump_iso14443_4a.h"
#include "protocols/iso14443_4b/nfc_cli_dump_iso14443_4b.h"
#include "protocols/iso15693_3/nfc_cli_dump_iso15693_3.h"
#include "protocols/mf_classic/nfc_cli_dump_mf_classic.h"
#include "protocols/mf_desfire/nfc_cli_dump_mf_desfire.h"
#include "protocols/mf_plus/nfc_cli_dump_mf_plus.h"
#include "protocols/mf_ultralight/nfc_cli_dump_mf_ultralight.h"
#include "protocols/slix/nfc_cli_dump_slix.h"
#include "protocols/st25tb/nfc_cli_dump_st25tb.h"
#include "protocols/felica/nfc_cli_dump_felica.h"

#include <datetime.h>
#include <furi_hal_rtc.h>
#include <toolbox/strint.h>
#include <toolbox/path.h>
#include <toolbox/args.h>

#define NFC_DEFAULT_FOLDER              EXT_PATH("nfc")
#define NFC_FILE_EXTENSION              ".nfc"
#define NFC_CLI_DEFAULT_FILENAME_PREFIX "dump"

#define NFC_CLI_DUMP_DEFAULT_TIMEOUT (5000)

#define TAG "DUMP"

static const char* nfc_cli_dump_error_names[NfcCliDumpErrorNum] = {
    [NfcCliDumpErrorNone] = "",
    [NfcCliDumpErrorNotPresent] = "card not present",
    [NfcCliDumpErrorAuthFailed] = "authentication failed",
    [NfcCliDumpErrorTimeout] = "timeout",
    [NfcCliDumpErrorFailedToRead] = "failed to read",
};

static NfcCliActionContext* nfc_cli_dump_alloc_ctx(Nfc* nfc) {
    furi_assert(nfc);
    NfcCliDumpContext* instance = malloc(sizeof(NfcCliDumpContext));
    instance->nfc = nfc;
    instance->file_path = furi_string_alloc();
    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->sem_done = furi_semaphore_alloc(1, 0);
    instance->nfc_device = nfc_device_alloc();
    instance->desired_protocol = NfcProtocolInvalid;
    instance->auth_ctx.skip_auth = true;
    instance->auth_ctx.key_size = 0;
    instance->timeout = NFC_CLI_DUMP_DEFAULT_TIMEOUT;

    instance->mfc_key_cache = mf_classic_key_cache_alloc();
    instance->scanner = nfc_cli_scanner_alloc(nfc);
    return instance;
}

static void nfc_cli_dump_free_ctx(NfcCliActionContext* ctx) {
    furi_assert(ctx);
    NfcCliDumpContext* instance = ctx;
    instance->desired_protocol = NfcProtocolInvalid;
    furi_string_free(instance->file_path);
    instance->nfc = NULL;
    furi_record_close(RECORD_STORAGE);
    furi_semaphore_free(instance->sem_done);
    nfc_device_free(instance->nfc_device);

    mf_classic_key_cache_free(instance->mfc_key_cache);
    nfc_cli_scanner_free(instance->scanner);
    free(instance);
}

static bool nfc_cli_dump_parse_filename_key(FuriString* value, void* output) {
    furi_assert(value);
    furi_assert(output);
    NfcCliDumpContext* ctx = output;
    furi_string_set(ctx->file_path, value);
    return true;
}

NfcGenericCallback protocol_poller_callbacks[NfcProtocolNum] = {
    [NfcProtocolMfUltralight] = nfc_cli_dump_poller_callback_mf_ultralight,
    [NfcProtocolMfClassic] = nfc_cli_dump_poller_callback_mf_classic,
    [NfcProtocolFelica] = nfc_cli_dump_poller_callback_felica,
    [NfcProtocolIso14443_3a] = nfc_cli_dump_poller_callback_iso14443_3a,
    [NfcProtocolIso14443_3b] = nfc_cli_dump_poller_callback_iso14443_3b,
    [NfcProtocolIso14443_4a] = nfc_cli_dump_poller_callback_iso14443_4a,
    [NfcProtocolIso14443_4b] = nfc_cli_dump_poller_callback_iso14443_4b,
    [NfcProtocolIso15693_3] = nfc_cli_dump_poller_callback_iso15693_3,
    [NfcProtocolSlix] = nfc_cli_dump_poller_callback_slix,
    [NfcProtocolMfDesfire] = nfc_cli_dump_poller_callback_mf_desfire,
    [NfcProtocolMfPlus] = nfc_cli_dump_poller_callback_mf_plus,
    [NfcProtocolSt25tb] = nfc_cli_dump_poller_callback_st25tb,
};

static void nfc_cli_dump_generate_filename(FuriString* file_path) {
    furi_string_set_str(file_path, NFC_DEFAULT_FOLDER);
    DateTime dt;
    furi_hal_rtc_get_datetime(&dt);
    furi_string_cat_printf(
        file_path,
        "/%s-%.4d%.2d%.2d-%.2d%.2d%.2d%s",
        NFC_CLI_DEFAULT_FILENAME_PREFIX,
        dt.year,
        dt.month,
        dt.day,
        dt.hour,
        dt.minute,
        dt.second,
        NFC_FILE_EXTENSION);
}

static bool nfc_cli_dump_check_filepath_valid(FuriString* file_path, Storage* storage) {
    bool file_exists = false;
    bool dir_exists = false;

    FuriString* buf = furi_string_alloc();

    path_extract_dirname(furi_string_get_cstr(file_path), buf);
    dir_exists = storage_dir_exists(storage, furi_string_get_cstr(buf));
    file_exists = storage_file_exists(storage, furi_string_get_cstr(file_path));

    bool result = true;
    if(!dir_exists) {
        printf(ANSI_FG_RED "Path \'%s\' doesn't exist\r\n" ANSI_RESET, furi_string_get_cstr(buf));
        result = false;
    } else if(file_exists) {
        printf(
            ANSI_FG_RED "File \'%s\' already exists\r\n" ANSI_RESET,
            furi_string_get_cstr(file_path));
        result = false;
    }
    furi_string_free(buf);

    return result;
}

static bool nfc_cli_dump_process_filename(NfcCliDumpContext* instance) {
    bool result = false;
    if(furi_string_empty(instance->file_path)) {
        nfc_cli_dump_generate_filename(instance->file_path);
        result = true;
    } else {
        result = nfc_cli_dump_check_filepath_valid(instance->file_path, instance->storage);
    }
    return result;
}

static size_t nfc_cli_dump_set_protocol(NfcCliDumpContext* instance) {
    size_t protocol_count = 0;
    if(instance->desired_protocol != NfcProtocolInvalid) {
        protocol_count = 1;
    } else {
        if(!nfc_cli_scanner_detect_protocol(instance->scanner, instance->timeout)) {
            NfcCliDumpError error = NfcCliDumpErrorTimeout;
            printf(ANSI_FG_RED "Error: %s\r\n" ANSI_RESET, nfc_cli_dump_error_names[error]);
        } else {
            nfc_cli_scanner_list_detected_protocols(instance->scanner);
            protocol_count = nfc_cli_scanner_detected_protocol_num(instance->scanner);
            instance->desired_protocol = nfc_cli_scanner_get_protocol(instance->scanner, 0);
        }
    }
    return protocol_count;
}

static bool nfc_cli_dump_card(NfcCliDumpContext* instance) {
    instance->poller = nfc_poller_alloc(instance->nfc, instance->desired_protocol);
    NfcGenericCallback callback = protocol_poller_callbacks[instance->desired_protocol];
    if(callback) {
        nfc_poller_start(instance->poller, callback, instance);
        FuriStatus status = furi_semaphore_acquire(instance->sem_done, instance->timeout);

        if(status == FuriStatusErrorTimeout) instance->result = NfcCliDumpErrorTimeout;
        nfc_poller_stop(instance->poller);
    }
    nfc_poller_free(instance->poller);

    return instance->result == NfcCliDumpErrorNone;
}

static void nfc_cli_dump_execute(PipeSide* pipe, NfcCliActionContext* context) {
    UNUSED(pipe);
    furi_assert(context);
    NfcCliDumpContext* instance = context;
    do {
        if(!nfc_cli_dump_process_filename(instance)) break;

        size_t protocol_count = nfc_cli_dump_set_protocol(instance);
        if(instance->desired_protocol == NfcProtocolInvalid) break;

        printf("Dumping as \"%s\"\r\n", nfc_cli_get_protocol_name(instance->desired_protocol));
        if(protocol_count > 1) printf("Use \'-p\' key to specify another protocol\r\n");

        if(nfc_cli_dump_card(instance)) {
            const char* path = furi_string_get_cstr(instance->file_path);
            if(nfc_device_save(instance->nfc_device, path)) {
                printf("Dump saved to \'%s\'\r\n", path);
            }
        } else {
            printf(
                ANSI_FG_RED "Error: %s\r\n" ANSI_RESET,
                nfc_cli_dump_error_names[instance->result]);
        }
    } while(false);
}

static const NfcProtocolNameValuePair supported_protocols[] = {
    {.name = "14_3a", .value = NfcProtocolIso14443_3a},
    {.name = "14_3b", .value = NfcProtocolIso14443_3b},
    {.name = "14_4a", .value = NfcProtocolIso14443_4a},
    {.name = "14_4b", .value = NfcProtocolIso14443_4b},
    {.name = "15", .value = NfcProtocolIso15693_3},
    {.name = "felica", .value = NfcProtocolFelica},
    {.name = "mfu", .value = NfcProtocolMfUltralight},
    {.name = "mfc", .value = NfcProtocolMfClassic},
    {.name = "mfp", .value = NfcProtocolMfPlus},
    {.name = "des", .value = NfcProtocolMfDesfire},
    {.name = "slix", .value = NfcProtocolSlix},
    {.name = "st25", .value = NfcProtocolSt25tb},
};

static bool nfc_cli_dump_parse_protocol(FuriString* value, void* output) {
    furi_assert(value);
    furi_assert(output);
    NfcCliDumpContext* ctx = output;

    NfcCliProtocolParser* parser =
        nfc_cli_protocol_parser_alloc(supported_protocols, COUNT_OF(supported_protocols));

    bool result = nfc_cli_protocol_parser_get(parser, value, &ctx->desired_protocol);

    nfc_cli_protocol_parser_free(parser);
    return result;
}

static bool nfc_cli_dump_parse_key(FuriString* value, void* output) {
    furi_assert(value);
    furi_assert(output);
    NfcCliDumpContext* ctx = output;
    NfcCliDumpAuthContext* auth_ctx = &ctx->auth_ctx;

    bool result = false;
    do {
        size_t len = furi_string_size(value);
        if(len % 2 != 0) break;
        size_t data_length = len / 2;

        if(data_length != MF_ULTRALIGHT_AUTH_PASSWORD_SIZE &&
           data_length != MF_ULTRALIGHT_C_AUTH_DES_KEY_SIZE) {
            printf(ANSI_FG_RED "Error: Wrong key size" ANSI_RESET);
            break;
        }

        if(!args_read_hex_bytes(value, auth_ctx->key.key, data_length)) break;
        auth_ctx->key_size = data_length;
        auth_ctx->skip_auth = false;
        result = true;
    } while(false);

    return result;
}

bool nfc_cli_dump_parse_timeout(FuriString* value, NfcCliActionContext* output) {
    NfcCliDumpContext* ctx = output;

    StrintParseError err = strint_to_uint32(furi_string_get_cstr(value), NULL, &ctx->timeout, 10);
    return err == StrintParseNoError;
}

const NfcCliKeyDescriptor dump_keys[] = {
    {
        .long_name = "key",
        .short_name = "k",
        .description = "key to path auth in protocols which requires it",
        .features = {.required = false, .parameter = true},
        .parse = nfc_cli_dump_parse_key,
    },
    {
        .long_name = "protocol",
        .short_name = "p",
        .description = "desired protocol",
        .features = {.required = false, .parameter = true},
        .parse = nfc_cli_dump_parse_protocol,
    },
    {
        .features = {.required = false, .parameter = true},
        .long_name = "file",
        .short_name = "f",
        .description = "path to new file",
        .parse = nfc_cli_dump_parse_filename_key,
    },
    {
        .features = {.required = false, .parameter = true},
        .long_name = "timeout",
        .short_name = "t",
        .description = "timeout value in milliseconds",
        .parse = nfc_cli_dump_parse_timeout,
    },
};

const NfcCliActionDescriptor dump_action = {
    .name = "dump",
    .description = "Dump tag to .nfc file",
    .alloc = nfc_cli_dump_alloc_ctx,
    .free = nfc_cli_dump_free_ctx,
    .execute = nfc_cli_dump_execute,
    .key_count = COUNT_OF(dump_keys),
    .keys = dump_keys,
};

const NfcCliActionDescriptor* dump_actions_collection[] = {&dump_action};

//Command descriptor
ADD_NFC_CLI_COMMAND(dump, "", dump_actions_collection);

//Command examples:
//dump -f ext/nfc/test.nfc
