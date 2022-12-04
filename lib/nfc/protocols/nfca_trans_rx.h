#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <lib/digital_signal/digital_signal.h>
#include <lib/pulse_reader/pulse_reader.h>
#include <furi_hal_nfc.h>

#include "nfc_util.h"

/* assume fc/128 */
#define NFCA_FC (13560000.0f) /* MHz */
#define NFCA_FC_K ((uint32_t)(NFCA_FC / 1000)) /* kHz */
#define NFCA_T1 (28.0f / NFCA_FC * 1000000000)
#define NFCA_T1_MIN (24.0f / NFCA_FC * 1000000000)
#define NFCA_T1_MAX (41.0f / NFCA_FC * 1000000000)
#define NFCA_TX (64.0f / NFCA_FC * 1000000000) /* 4.7198 µs */
#define NFCA_TX_MIN (0.90f * NFCA_TX)
#define NFCA_TX_MAX (1.10f * NFCA_TX)
#define NFCA_TB (128.0f / NFCA_FC * 1000000000) /* 9.4395 µs */
#define NFCA_TB_MIN (0.80f * NFCA_TB)
#define NFCA_TB_MAX (1.20f * NFCA_TB)

#define IS_T1(x) ((x) >= NFCA_T1_MIN && (x) <= NFCA_T1_MAX)
#define IS_TX(x) ((x) >= NFCA_TX_MIN && (x) <= NFCA_TX_MAX)
#define IS_TB(x) ((x) >= NFCA_TB_MIN && (x) <= NFCA_TB_MAX)
#define IS_ZERO(x) ((x) >= -NFCA_T1_MIN / 2 && (x) <= NFCA_T1_MIN / 2)

#define DIGITAL_SIGNAL_UNIT_S (100000000000.0f)
#define DIGITAL_SIGNAL_UNIT_US (100000.0f)

#define NFCA_FRAME_LENGTH 32
#define NFCA_DEBUG_LENGTH 128

typedef struct {
    bool have_sof;
    bool valid_frame;
    int32_t frame_time;
    size_t bits_received;
    uint8_t frame_data[NFCA_FRAME_LENGTH];
    uint32_t debug_buffer[NFCA_DEBUG_LENGTH];
    size_t debug_pos;
    uint32_t parity_bits;
    PulseReader* reader_signal;
} NfcaTransRxState;

bool nfca_trans_rx_loop(NfcaTransRxState* state, uint32_t timeout_ms);
void nfca_trans_rx_deinit(NfcaTransRxState* state);
void nfca_trans_rx_init(NfcaTransRxState* state);

void nfca_trans_rx_pause(NfcaTransRxState* state);
void nfca_trans_rx_continue(NfcaTransRxState* state);