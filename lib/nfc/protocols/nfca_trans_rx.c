#include <limits.h>
#include <furi.h>

#include <furi_hal.h>
#include <furi_hal_nfc.h>
#include <furi_hal_spi.h>
#include <furi_hal_gpio.h>
#include <furi_hal_cortex.h>
#include <furi_hal_resources.h>
#include <st25r3916.h>
#include <st25r3916_irq.h>

#include "nfca_trans_rx.h"

#define TAG "NfcA-trans-rx"

void nfca_trans_rx_init(NfcaTransRxState* state) {
    FURI_LOG_D(TAG, "Starting NfcA transparent rx");

    st25r3916ExecuteCommand(ST25R3916_CMD_STOP);
    st25r3916WriteRegister(ST25R3916_REG_OP_CONTROL, 0xC3);
    st25r3916WriteRegister(ST25R3916_REG_MODE, 0x88);
    st25r3916ExecuteCommand(ST25R3916_CMD_TRANSPARENT_MODE);

    furi_hal_spi_bus_handle_deinit(&furi_hal_spi_bus_handle_nfc);

    /* allocate a 512 edge buffer, more than enough */
    state->reader_signal = pulse_reader_alloc(&gpio_spi_r_miso, 512);
    /* timebase shall be 1 ns */
    pulse_reader_set_timebase(state->reader_signal, PulseReaderUnitNanosecond);

    pulse_reader_start(state->reader_signal);

    /* set start values */
    state->bits_received = 0;
    state->have_sof = false;
    state->valid_frame = false;
}

void nfca_trans_rx_deinit(NfcaTransRxState* state) {
    furi_hal_spi_bus_handle_init(&furi_hal_spi_bus_handle_nfc);
    pulse_reader_free(state->reader_signal);
}

void nfca_trans_rx_pause(NfcaTransRxState* state) {
    pulse_reader_stop(state->reader_signal);
}

void nfca_trans_rx_continue(NfcaTransRxState* state) {
    pulse_reader_start(state->reader_signal);
}

static void nfca_bit_received(NfcaTransRxState* state, uint8_t bit) {
    /* According to ISO14443-3 short frames have 7 bits and standard 9 bits per byte,
       where the 9th bit is odd parity. Data is transmitted LSB first. */
    uint32_t byte_num = (state->bits_received / 9);
    uint32_t bit_num = (state->bits_received % 9);

    if(byte_num >= NFCA_FRAME_LENGTH) {
        return;
    }

    if(bit_num == 8) {
        uint32_t parity_value = 1 << (state->bits_received / 9);
        state->parity_bits &= ~parity_value;
        state->parity_bits |= bit ? parity_value : 0;
    } else {
        uint32_t bit_value = 1 << bit_num;
        state->frame_data[byte_num] &= ~bit_value;
        state->frame_data[byte_num] |= bit ? bit_value : 0;
    }

    state->bits_received++;
}

bool nfca_trans_rx_loop(NfcaTransRxState* state, uint32_t timeout_ms) {
    furi_assert(state);

    state->valid_frame = false;
    state->have_sof = false;
    state->bits_received = 0;

    bool done = false;

    uint32_t timeout_us = timeout_ms * 1000;

    while(!done) {
        uint32_t nsec = pulse_reader_receive(state->reader_signal, timeout_us);

        bool eof = state->have_sof && (nsec >= (2 * NFCA_TB));
        bool lost_pulse = false;

        if(state->have_sof && nsec == PULSE_READER_LOST_EDGE) {
            nsec = NFCA_T1;
            lost_pulse = true;
        } else if(nsec == PULSE_READER_NO_EDGE) {
            done = true;
        }

        if(IS_T1(nsec) || eof) {
            timeout_us = (3 * NFCA_TB) / 1000;
            if(!state->have_sof) {
                state->frame_time = -(NFCA_TB - nsec);
                state->have_sof = true;
                state->valid_frame = false;
                state->bits_received = 0;
                state->debug_pos = 0;
                if(lost_pulse) {
                    state->frame_time -= nsec;
                }
                continue;
            }

            if(state->frame_time > NFCA_TB_MIN) {
                state->frame_time -= NFCA_TB;
                nfca_bit_received(state, 0);
            }

            if(IS_ZERO(state->frame_time)) {
                state->frame_time = -(NFCA_TB - nsec);
                nfca_bit_received(state, 0);
            } else if(IS_TX(state->frame_time)) {
                state->frame_time = -(NFCA_TX - nsec);
                nfca_bit_received(state, 1);
            } else {
                if(eof) {
                    state->have_sof = false;
                    state->valid_frame = true;
                    done = true;
                } else {
                }
            }
        } else {
            if(!state->have_sof) {
                if(IS_TB(nsec)) {
                    state->frame_time = 0;
                    state->have_sof = true;
                    state->valid_frame = false;
                    state->bits_received = 0;
                    state->debug_pos = 0;
                    if(lost_pulse) {
                        state->frame_time -= nsec;
                    }
                    continue;
                } else {
                    state->frame_time = 0;
                }
            } else {
                state->frame_time += nsec;
            }
        }

        if(lost_pulse) {
            state->frame_time -= nsec;
        }
    }

    if(state->valid_frame) {
        if(state->bits_received > 7) {
            /* a last 0-bit will look like a missing bit */
            if((state->bits_received % 9) == 8) {
                nfca_bit_received(state, 0);
                state->bits_received++;
            }
        }
    }

    return state->valid_frame;
}
