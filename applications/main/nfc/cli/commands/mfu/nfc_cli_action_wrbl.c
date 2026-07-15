#include "nfc_cli_action_rdbl.h"

#include "../helpers/nfc_cli_format.h"

#include <furi.h>
#include <toolbox/args.h>
#include <toolbox/strint.h>
#include <nfc/protocols/mf_ultralight/mf_ultralight_poller_sync.h>

#define MF_ULTRALIGHT_POLLER_COMPLETE_EVENT (1UL << 0)

typedef struct {
    Nfc* nfc;
    uint16_t block;
    MfUltralightPage page;
} NfcCliMfuWrblContext;

NfcCliActionContext* nfc_cli_mfu_wrbl_alloc_ctx(Nfc* nfc) {
    furi_assert(nfc);
    NfcCliMfuWrblContext* instance = malloc(sizeof(NfcCliMfuWrblContext));
    instance->nfc = nfc;
    return instance;
}

void nfc_cli_mfu_wrbl_free_ctx(NfcCliActionContext* ctx) {
    furi_assert(ctx);
    NfcCliMfuWrblContext* instance = ctx;
    free(instance);
}

void nfc_cli_mfu_wrbl_execute(PipeSide* pipe, NfcCliActionContext* ctx) {
    furi_assert(pipe);

    NfcCliMfuWrblContext* instance = ctx;

    MfUltralightError error =
        mf_ultralight_poller_sync_write_page(instance->nfc, instance->block, &instance->page);

    if(error == MfUltralightErrorNone) {
        printf(ANSI_FG_BR_GREEN "\r\nSuccess\r\n" ANSI_RESET);
        printf("Block: %d ", instance->block);
        nfc_cli_printf_array(instance->page.data, sizeof(MfUltralightPage), "Data: ");
        printf("\r\n");
    } else {
        printf(ANSI_FG_RED "Error: %s" ANSI_RESET, nfc_cli_mf_ultralight_get_error(error));
    }
}

bool nfc_cli_mfu_wrbl_parse_block(FuriString* value, NfcCliActionContext* output) {
    NfcCliMfuWrblContext* ctx = output;

    StrintParseError err = strint_to_uint16(furi_string_get_cstr(value), NULL, &ctx->block, 10);
    return err == StrintParseNoError;
}

bool nfc_cli_mfu_wrbl_parse_data(FuriString* value, void* output) {
    NfcCliMfuWrblContext* ctx = output;

    bool result = false;
    do {
        size_t len = furi_string_size(value);
        if(len % 2 != 0) break;

        size_t data_length = len / 2;
        if(data_length != MF_ULTRALIGHT_PAGE_SIZE) break;

        result = args_read_hex_bytes(value, ctx->page.data, data_length);
    } while(false);

    return result;
}
