#pragma once

#include <furi_hal_nfc.h>
#include <storage/storage.h>

typedef struct NfcDebugPcapWorker NfcDebugPcapWorker;

NfcDebugPcapWorker* nfc_debug_pcap_alloc(Storage* storage);

void nfc_debug_pcap_free(NfcDebugPcapWorker* instance);

/** Prepare tx/rx context for debug pcap logging, if enabled.
 *
 * @param      instance NfcDebugPcapWorker* instance, can be NULL
 * @param      tx_rx   TX/RX context to log
 * @param      is_picc if true, record Flipper as PICC, else PCD.
 */
void nfc_debug_pcap_prepare_tx_rx(
    NfcDebugPcapWorker* instance,
    FuriHalNfcTxRxContext* tx_rx,
    bool is_picc);
