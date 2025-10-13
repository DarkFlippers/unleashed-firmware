#include "nfc_cli_command_raw.h"
#include "../helpers/nfc_cli_format.h"
#include "../helpers/nfc_cli_protocol_parser.h"

#include "protocol_handlers/nfc_cli_raw_common_types.h"
#include "protocol_handlers/iso14443_3a/nfc_cli_raw_iso14443_3a.h"
#include "protocol_handlers/iso14443_3b/nfc_cli_raw_iso14443_3b.h"
#include "protocol_handlers/iso15693_3/nfc_cli_raw_iso15693_3.h"
#include "protocol_handlers/felica/nfc_cli_raw_felica.h"

#include <toolbox/args.h>

#define NFC_CLI_PROTOCOL_SUPPORT_MAX_BUFFER_SIZE (256)

#define TAG "RAW"

typedef enum {
    NfcCliProtocolRequestTypeNormalExecute,
    NfcCliProtocolRequestTypeAbort,
} NfcCliProtocolRequestType;

typedef enum {
    NfcPollerStateStopped,
    NfcPollerStateStarted,
} NfcPollerState;

typedef NfcCommand (*NfcCliRawProtocolSpecificHandler)(
    NfcGenericInstance* poller,
    const NfcCliRawRequest* request,
    NfcCliRawResponse* const response);

typedef struct {
    Nfc* nfc;
    NfcCliRawRequest request;
    NfcCliRawResponse response;

    NfcPoller* poller;
    NfcPollerState poller_state;

    NfcCliProtocolRequestType request_type;
    FuriMessageQueue* input_queue;
    FuriSemaphore* sem_done;

} NfcCliRawCmdContext;

static const char* raw_error_names[] = {
    [NfcCliRawErrorNone] = "None",
    [NfcCliRawErrorTimeout] = "Timeout",
    [NfcCliRawErrorProtocol] = "Internal protocol",
    [NfcCliRawErrorWrongCrc] = "Wrong CRC",
    [NfcCliRawErrorNotPresent] = "No card",
};

static NfcCliActionContext* nfc_cli_raw_alloc_ctx(Nfc* nfc) {
    furi_assert(nfc);
    NfcCliRawCmdContext* instance = malloc(sizeof(NfcCliRawCmdContext));
    instance->nfc = nfc;

    instance->request.protocol = NfcProtocolInvalid;

    instance->request.tx_buffer = bit_buffer_alloc(NFC_CLI_PROTOCOL_SUPPORT_MAX_BUFFER_SIZE);
    instance->response.rx_buffer = bit_buffer_alloc(NFC_CLI_PROTOCOL_SUPPORT_MAX_BUFFER_SIZE);

    instance->input_queue = furi_message_queue_alloc(5, sizeof(NfcCliProtocolRequestType));
    instance->sem_done = furi_semaphore_alloc(1, 0);
    instance->response.activation_string = furi_string_alloc();
    instance->request.timeout = 0;
    return instance;
}

static void nfc_cli_raw_abort_nfc_thread(NfcCliRawCmdContext* instance) {
    if(instance->poller_state == NfcPollerStateStarted) {
        instance->request_type = NfcCliProtocolRequestTypeAbort;
        furi_message_queue_put(instance->input_queue, &instance->request_type, FuriWaitForever);
        furi_semaphore_acquire(instance->sem_done, FuriWaitForever);
        instance->poller_state = NfcPollerStateStopped;
    }
    if(instance->poller) nfc_poller_stop(instance->poller);
}

static void nfc_cli_raw_free_ctx(NfcCliActionContext* ctx) {
    furi_assert(ctx);
    NfcCliRawCmdContext* instance = ctx;

    nfc_cli_raw_abort_nfc_thread(instance);
    if(instance->poller) nfc_poller_free(instance->poller);

    furi_message_queue_free(instance->input_queue);
    furi_semaphore_free(instance->sem_done);

    furi_string_free(instance->response.activation_string);
    bit_buffer_free(instance->response.rx_buffer);
    bit_buffer_free(instance->request.tx_buffer);
    instance->nfc = NULL;
    free(instance);
}

static bool nfc_cli_raw_can_reuse_ctx(NfcCliActionContext* ctx) {
    furi_assert(ctx);
    NfcCliRawCmdContext* instance = ctx;
    NfcCliRawRequest* request = &instance->request;

    bool result = request->keep_field;
    request->keep_field = false;
    request->append_crc = false;
    request->select = false;
    instance->request.timeout = 0;
    return result;
}

const NfcCliRawProtocolSpecificHandler nfc_cli_raw_protocol_handlers[] = {
    [NfcProtocolIso14443_3a] = nfc_cli_raw_iso14443_3a_handler,
    [NfcProtocolIso14443_3b] = nfc_cli_raw_iso14443_3b_handler,
    [NfcProtocolIso14443_4a] = NULL,
    [NfcProtocolIso14443_4b] = NULL,
    [NfcProtocolIso15693_3] = nfc_cli_raw_iso15693_3_handler,
    [NfcProtocolFelica] = nfc_cli_raw_felica_handler,
    [NfcProtocolMfUltralight] = NULL,
    [NfcProtocolMfClassic] = NULL,
    [NfcProtocolMfDesfire] = NULL,
    [NfcProtocolSlix] = NULL,
    [NfcProtocolSt25tb] = NULL,
};

static NfcCommand nfc_cli_raw_poller_callback(NfcGenericEventEx event, void* context) {
    NfcEvent* nfc_event = event.parent_event_data;
    NfcCliRawCmdContext* instance = context;

    NfcCommand command = NfcCommandContinue;

    if(nfc_event->type == NfcEventTypePollerReady) {
        FURI_LOG_D(TAG, "Poller callback");
        NfcCliProtocolRequestType request_type = NfcCliProtocolRequestTypeAbort;
        furi_message_queue_get(instance->input_queue, &request_type, FuriWaitForever);

        if(request_type == NfcCliProtocolRequestTypeAbort) {
            command = NfcCommandStop;
        } else {
            const NfcCliRawProtocolSpecificHandler handler =
                nfc_cli_raw_protocol_handlers[instance->request.protocol];
            if(handler) handler(event.poller, &instance->request, &instance->response);
        }
    }
    furi_semaphore_release(instance->sem_done);
    if(command == NfcCommandStop) {
        FURI_LOG_D(TAG, "Aborting poller callback");
        instance->poller_state = NfcPollerStateStopped;
    }
    return command;
}

static inline void nfc_cli_raw_print_result(const NfcCliRawCmdContext* instance) {
    if(!furi_string_empty(instance->response.activation_string))
        printf("%s\r\n", furi_string_get_cstr(instance->response.activation_string));

    nfc_cli_printf_array(
        bit_buffer_get_data(instance->request.tx_buffer),
        bit_buffer_get_size_bytes(instance->request.tx_buffer),
        "Tx: ");

    if(instance->response.result != NfcCliRawErrorNone)
        printf("\r\nError: \"%s\"\r\n", raw_error_names[instance->response.result]);

    size_t rx_size = bit_buffer_get_size_bytes(instance->response.rx_buffer);
    if(rx_size > 0) {
        nfc_cli_printf_array(
            bit_buffer_get_data(instance->response.rx_buffer),
            bit_buffer_get_size_bytes(instance->response.rx_buffer),
            "\r\nRx: ");
    }
}

static void nfc_cli_raw_execute(PipeSide* pipe, void* context) {
    UNUSED(pipe);
    furi_assert(context);
    NfcCliRawCmdContext* instance = context;

    furi_string_reset(instance->response.activation_string);

    if(instance->poller_state == NfcPollerStateStopped) {
        if(instance->poller == NULL)
            instance->poller = nfc_poller_alloc(instance->nfc, instance->request.protocol);

        nfc_poller_start_ex(instance->poller, nfc_cli_raw_poller_callback, instance);
        instance->poller_state = NfcPollerStateStarted;
    }

    instance->request_type = NfcCliProtocolRequestTypeNormalExecute;
    furi_message_queue_put(instance->input_queue, &instance->request_type, FuriWaitForever);
    furi_semaphore_acquire(instance->sem_done, FuriWaitForever);

    nfc_cli_raw_print_result(instance);
}

static const NfcProtocolNameValuePair supported_protocols[] = {
    {.name = "14a", .value = NfcProtocolIso14443_3a},
    {.name = "iso14a", .value = NfcProtocolIso14443_3a},

    {.name = "14b", .value = NfcProtocolIso14443_3b},
    {.name = "iso14b", .value = NfcProtocolIso14443_3b},

    {.name = "15", .value = NfcProtocolIso15693_3},
    {.name = "felica", .value = NfcProtocolFelica},
};

static bool nfc_cli_raw_parse_protocol(FuriString* value, void* output) {
    NfcCliRawCmdContext* ctx = output;
    NfcProtocol new_protocol = NfcProtocolInvalid;

    NfcCliProtocolParser* parser =
        nfc_cli_protocol_parser_alloc(supported_protocols, COUNT_OF(supported_protocols));

    bool result = nfc_cli_protocol_parser_get(parser, value, &new_protocol);

    nfc_cli_protocol_parser_free(parser);

    if(result && ctx->request.protocol != NfcProtocolInvalid &&
       ctx->request.protocol != new_protocol) {
        printf(
            ANSI_FG_RED "Error: previous %s != new %s. Unable to continue." ANSI_RESET,
            nfc_cli_get_protocol_name(ctx->request.protocol),
            nfc_cli_get_protocol_name(new_protocol));
        result = false;
    }

    if(result) {
        ctx->request.protocol = new_protocol;
    }
    return result;
}

static bool nfc_cli_raw_parse_data(FuriString* value, void* output) {
    NfcCliRawCmdContext* ctx = output;

    bool result = false;
    do {
        size_t len = furi_string_size(value);
        if(len % 2 != 0) break;

        size_t data_length = len / 2;
        uint8_t* data = malloc(data_length);

        if(args_read_hex_bytes(value, data, data_length)) {
            bit_buffer_reset(ctx->request.tx_buffer);
            bit_buffer_copy_bytes(ctx->request.tx_buffer, data, data_length);
            result = true;
        }

        free(data);
    } while(false);

    return result;
}

static bool nfc_cli_raw_parse_timeout(FuriString* value, void* output) {
    furi_assert(value);
    furi_assert(output);
    NfcCliRawCmdContext* ctx = output;

    bool result = false;

    int timeout = 0;
    if(args_read_int_and_trim(value, &timeout)) {
        ctx->request.timeout = timeout;
        result = true;
    }
    return result;
}

static bool nfc_cli_raw_parse_select(FuriString* value, void* output) {
    UNUSED(value);
    NfcCliRawCmdContext* ctx = output;
    ctx->request.select = true;
    return true;
}

static bool nfc_cli_raw_parse_crc(FuriString* value, void* output) {
    UNUSED(value);
    NfcCliRawCmdContext* ctx = output;
    ctx->request.append_crc = true;
    return true;
}

static bool nfc_cli_raw_parse_keep(FuriString* value, void* output) {
    UNUSED(value);
    NfcCliRawCmdContext* ctx = output;
    ctx->request.keep_field = true;
    return true;
}

const NfcCliKeyDescriptor raw_action_keys[] = {
    {
        .long_name = NULL,
        .short_name = "t",
        .features = {.parameter = true, .required = false},
        .description = "timeout in fc",
        .parse = nfc_cli_raw_parse_timeout,
    },
    {
        .long_name = NULL,
        .short_name = "k",
        .description = "keep signal field ON after receive",
        .parse = nfc_cli_raw_parse_keep,
    },
    {
        .long_name = NULL,
        .short_name = "c",
        .description = "calculate and append CRC",
        .parse = nfc_cli_raw_parse_crc,
    },
    {
        .long_name = NULL,
        .short_name = "s",
        .description = "Select on FieldOn",
        .parse = nfc_cli_raw_parse_select,
    },
    {
        .long_name = "protocol",
        .short_name = "p",
        .description = "desired protocol. Possible values: 14a, iso14a, 14b, iso14b, 15, felica",
        .features = {.parameter = true, .required = true},
        .parse = nfc_cli_raw_parse_protocol,
    },
    {
        .long_name = "data",
        .short_name = "d",
        .description = "Raw bytes to send in HEX format",
        .features = {.parameter = true, .required = true},
        .parse = nfc_cli_raw_parse_data,
    },
};

const NfcCliActionDescriptor raw_action = {
    .name = "raw",
    .description = "Sends raw bytes using different protocols",
    .key_count = COUNT_OF(raw_action_keys),
    .keys = raw_action_keys,
    .execute = nfc_cli_raw_execute,
    .alloc = nfc_cli_raw_alloc_ctx,
    .free = nfc_cli_raw_free_ctx,
    .can_reuse = nfc_cli_raw_can_reuse_ctx,
};

const NfcCliActionDescriptor* raw_actions_collection[] = {&raw_action};

ADD_NFC_CLI_COMMAND(raw, "", raw_actions_collection);

//Command usage: raw <protocol> [keys] <data>
//Command examples:
//raw iso14a -sc 3000
//raw iso14a 3000
//raw iso14a 3000 -sc
