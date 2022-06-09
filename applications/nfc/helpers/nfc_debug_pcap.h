#pragma once

#include <furi_hal_nfc.h>
#include <storage/storage.h>

/** Prepare tx/rx context for debug pcap logging, if enabled.
 *
 * @param      tx_rx   TX/RX context to log
 * @param      storage Storage to log to
 * @param      is_picc if true, record Flipper as PICC, else PCD.
 */
void nfc_debug_pcap_prepare_tx_rx(FuriHalNfcTxRxContext* tx_rx, Storage* storage, bool is_picc);
