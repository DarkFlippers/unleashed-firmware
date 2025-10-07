
#include "nfc_cli_command_scanner.h"
#include "helpers/nfc_cli_scanner.h"
#include "helpers/nfc_cli_format.h"

typedef struct {
    NfcCliScanner* scanner;
    bool display_tree;
} NfcCliCmdScannerContext;

static NfcCliActionContext* nfc_cli_command_scanner_alloc_ctx(Nfc* nfc) {
    furi_assert(nfc);
    NfcCliCmdScannerContext* instance = malloc(sizeof(NfcCliCmdScannerContext));
    instance->scanner = nfc_cli_scanner_alloc(nfc);
    instance->display_tree = false;

    return instance;
}

static void nfc_cli_command_scanner_free_ctx(NfcCliActionContext* ctx) {
    furi_assert(ctx);
    NfcCliCmdScannerContext* instance = ctx;
    nfc_cli_scanner_free(instance->scanner);

    free(instance);
}

static void
    nfc_cli_command_scanner_format_protocol_tree(NfcProtocol protocol, FuriString* output) {
    const char* names[10] = {0};
    uint8_t cnt = 0;
    while(protocol != NfcProtocolInvalid) {
        names[cnt++] = nfc_cli_get_protocol_name(protocol);
        protocol = nfc_protocol_get_parent(protocol);
    }

    for(int8_t i = cnt - 1; i >= 0; i--) {
        furi_string_cat_printf(output, (i == 0) ? "%s" : "%s -> ", names[i]);
    }
}

static void nfc_cli_command_scanner_format_detected_protocols(NfcCliScanner* instance) {
    FuriString* str = furi_string_alloc();
    printf("Protocols detected: \r\n");
    for(size_t i = 0; i < nfc_cli_scanner_detected_protocol_num(instance); i++) {
        furi_string_reset(str);
        NfcProtocol protocol = nfc_cli_scanner_get_protocol(instance, i);
        nfc_cli_command_scanner_format_protocol_tree(protocol, str);
        printf("Protocol [%zu]: %s\r\n", i + 1, furi_string_get_cstr(str));
    }
    furi_string_free(str);
}

static void nfc_cli_command_scanner_execute(PipeSide* pipe, void* context) {
    NfcCliCmdScannerContext* instance = context;

    printf("Press Ctrl+C to abort\r\n\n");
    nfc_cli_scanner_begin_scan(instance->scanner);
    while(!cli_is_pipe_broken_or_is_etx_next_char(pipe) &&
          !nfc_cli_scanner_wait_scan(instance->scanner, 50))
        ;
    nfc_cli_scanner_end_scan(instance->scanner);

    if(!instance->display_tree)
        nfc_cli_scanner_list_detected_protocols(instance->scanner);
    else
        nfc_cli_command_scanner_format_detected_protocols(instance->scanner);
}

static bool nfc_cli_command_scanner_parse_tree(FuriString* value, void* output) {
    UNUSED(value);
    NfcCliCmdScannerContext* ctx = output;
    ctx->display_tree = true;
    return true;
}

const NfcCliKeyDescriptor tree_key = {
    .short_name = "t",
    .long_name = "tree",
    .features = {.parameter = false, .required = false, .multivalue = false},
    .description = "displays protocol hierarchy for each detected protocol",
    .parse = nfc_cli_command_scanner_parse_tree,
};

const NfcCliActionDescriptor scanner_action = {
    .name = "scanner",
    .description = "Detect tag type",
    .key_count = 1,
    .keys = &tree_key,
    .execute = nfc_cli_command_scanner_execute,
    .alloc = nfc_cli_command_scanner_alloc_ctx,
    .free = nfc_cli_command_scanner_free_ctx,
};

const NfcCliActionDescriptor* scanner_actions_collection[] = {&scanner_action};

ADD_NFC_CLI_COMMAND(scanner, "", scanner_actions_collection);
