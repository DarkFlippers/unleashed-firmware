#include "common.h"

#include <furi_hal_nfc.h>

#define REQA (0x26)
#define CL1_PREFIX (0x93)
#define SELECT (0x70)

#define MAGIC_BUFFER_SIZE (32)

bool magic_activate() {
    FuriHalNfcReturn ret = 0;

    // Setup nfc poller
    furi_hal_nfc_exit_sleep();
    furi_hal_nfc_ll_txrx_on();
    furi_hal_nfc_ll_poll();
    ret = furi_hal_nfc_ll_set_mode(
        FuriHalNfcModePollNfca, FuriHalNfcBitrate106, FuriHalNfcBitrate106);
    if(ret != FuriHalNfcReturnOk) return false;

    furi_hal_nfc_ll_set_fdt_listen(FURI_HAL_NFC_LL_FDT_LISTEN_NFCA_POLLER);
    furi_hal_nfc_ll_set_fdt_poll(FURI_HAL_NFC_LL_FDT_POLL_NFCA_POLLER);
    furi_hal_nfc_ll_set_error_handling(FuriHalNfcErrorHandlingNfc);
    furi_hal_nfc_ll_set_guard_time(FURI_HAL_NFC_LL_GT_NFCA);

    return true;
}

void magic_deactivate() {
    furi_hal_nfc_ll_txrx_off();
    furi_hal_nfc_sleep();
}