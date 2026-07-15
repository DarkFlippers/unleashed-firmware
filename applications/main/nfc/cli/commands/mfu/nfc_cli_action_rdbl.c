#include "nfc_cli_action_rdbl.h"

#include "../helpers/nfc_cli_format.h"

#include <furi.h>
#include <toolbox/strint.h>
#include <nfc/protocols/mf_ultralight/mf_ultralight_poller_sync.h>

#define MF_ULTRALIGHT_POLLER_COMPLETE_EVENT (1UL << 0)

typedef struct {
    Nfc* nfc;
    uint16_t block;
} NfcCliMfuRdblContext;

NfcCliActionContext* nfc_cli_mfu_rdbl_alloc_ctx(Nfc* nfc) {
    furi_assert(nfc);
    NfcCliMfuRdblContext* instance = malloc(sizeof(NfcCliMfuRdblContext));
    instance->nfc = nfc;
    return instance;
}

void nfc_cli_mfu_rdbl_free_ctx(NfcCliActionContext* ctx) {
    furi_assert(ctx);
    NfcCliMfuRdblContext* instance = ctx;
    free(instance);
}

void nfc_cli_mfu_rdbl_execute(PipeSide* pipe, NfcCliActionContext* ctx) {
    furi_assert(pipe);

    NfcCliMfuRdblContext* instance = ctx;
    MfUltralightPage page = {0};

    MfUltralightError error =
        mf_ultralight_poller_sync_read_page(instance->nfc, instance->block, &page);

    if(error == MfUltralightErrorNone) {
        printf("\r\nBlock: %d ", instance->block);
        nfc_cli_printf_array(page.data, sizeof(MfUltralightPage), "Data: ");
        printf("\r\n");
    } else {
        printf(ANSI_FG_RED "Error: %s" ANSI_RESET, nfc_cli_mf_ultralight_get_error(error));
    }
}

bool nfc_cli_mfu_rdbl_parse_block(FuriString* value, NfcCliActionContext* output) {
    NfcCliMfuRdblContext* ctx = output;

    StrintParseError err = strint_to_uint16(furi_string_get_cstr(value), NULL, &ctx->block, 10);
    return err == StrintParseNoError;
}
