#include "furi_hal_nfc_i.h"
#include "furi_hal_nfc_tech_i.h"

#include <digital_signal/presets/nfc/iso15693_signal.h>
#include <signal_reader/parsers/iso15693/iso15693_parser.h>

#include <furi_hal_resources.h>

#define FURI_HAL_NFC_ISO15693_MAX_FRAME_SIZE         (1024U)
#define FURI_HAL_NFC_ISO15693_POLLER_MAX_BUFFER_SIZE (64)

#define FURI_HAL_NFC_ISO15693_RESP_SOF_SIZE    (5)
#define FURI_HAL_NFC_ISO15693_RESP_EOF_SIZE    (5)
#define FURI_HAL_NFC_ISO15693_RESP_SOF_MASK    (0x1FU)
#define FURI_HAL_NFC_ISO15693_RESP_SOF_PATTERN (0x17U)
#define FURI_HAL_NFC_ISO15693_RESP_EOF_PATTERN (0x1DU)

#define FURI_HAL_NFC_ISO15693_RESP_PATTERN_MASK (0x03U)
#define FURI_HAL_NFC_ISO15693_RESP_PATTERN_0    (0x01U)
#define FURI_HAL_NFC_ISO15693_RESP_PATTERN_1    (0x02U)

// Derived experimentally
#define FURI_HAL_NFC_ISO15693_POLLER_FWT_COMP_FC   (-1300)
#define FURI_HAL_NFC_ISO15693_LISTENER_FDT_COMP_FC (2850)

#define BITS_IN_BYTE (8U)

#define TAG "FuriHalIso15693"

typedef struct {
    Iso15693Signal* signal;
    Iso15693Parser* parser;
} FuriHalNfcIso15693Listener;

typedef struct {
    // 4 bits per data bit on transmit
    uint8_t fifo_buf[FURI_HAL_NFC_ISO15693_POLLER_MAX_BUFFER_SIZE * 4];
    size_t fifo_buf_bits;
    uint8_t frame_buf[FURI_HAL_NFC_ISO15693_POLLER_MAX_BUFFER_SIZE * 2];
    size_t frame_buf_bits;
} FuriHalNfcIso15693Poller;

static FuriHalNfcIso15693Listener* furi_hal_nfc_iso15693_listener = NULL;
static FuriHalNfcIso15693Poller* furi_hal_nfc_iso15693_poller = NULL;

static FuriHalNfcIso15693Listener* furi_hal_nfc_iso15693_listener_alloc(void) {
    FuriHalNfcIso15693Listener* instance = malloc(sizeof(FuriHalNfcIso15693Listener));

    instance->signal = iso15693_signal_alloc(&gpio_spi_r_mosi);
    instance->parser =
        iso15693_parser_alloc(&gpio_nfc_irq_rfid_pull, FURI_HAL_NFC_ISO15693_MAX_FRAME_SIZE);

    return instance;
}

static void furi_hal_nfc_iso15693_listener_free(FuriHalNfcIso15693Listener* instance) {
    furi_check(instance);

    iso15693_signal_free(instance->signal);
    iso15693_parser_free(instance->parser);

    free(instance);
}

static FuriHalNfcIso15693Poller* furi_hal_nfc_iso15693_poller_alloc(void) {
    FuriHalNfcIso15693Poller* instance = malloc(sizeof(FuriHalNfcIso15693Poller));

    return instance;
}

static void furi_hal_nfc_iso15693_poller_free(FuriHalNfcIso15693Poller* instance) {
    furi_check(instance);

    free(instance);
}

static FuriHalNfcError furi_hal_nfc_iso15693_common_init(FuriHalSpiBusHandle* handle) {
    // Common NFC-V settings, 26.48 kbps

    // 1st stage zero = 12 kHz, 3rd stage zero = 80 kHz, low-pass = 600 kHz
    st25r3916_write_reg(
        handle,
        ST25R3916_REG_RX_CONF1,
        ST25R3916_REG_RX_CONF1_z12k | ST25R3916_REG_RX_CONF1_h80 |
            ST25R3916_REG_RX_CONF1_lp_600khz);

    // Enable AGC
    // AGC Ratio 6
    // AGC algorithm with RESET (recommended for ISO15693)
    // AGC operation during complete receive period
    // Squelch automatic activation on TX end
    st25r3916_write_reg(
        handle,
        ST25R3916_REG_RX_CONF2,
        ST25R3916_REG_RX_CONF2_agc6_3 | ST25R3916_REG_RX_CONF2_agc_m |
            ST25R3916_REG_RX_CONF2_agc_en | ST25R3916_REG_RX_CONF2_sqm_dyn);

    // HF operation, full gain on AM and PM channels
    st25r3916_write_reg(handle, ST25R3916_REG_RX_CONF3, 0x00);
    // No gain reduction on AM and PM channels
    st25r3916_write_reg(handle, ST25R3916_REG_RX_CONF4, 0x00);

    // Collision detection level 53%
    // AM & PM summation before digitizing on
    st25r3916_write_reg(
        handle,
        ST25R3916_REG_CORR_CONF1,
        ST25R3916_REG_CORR_CONF1_corr_s0 | ST25R3916_REG_CORR_CONF1_corr_s1 |
            ST25R3916_REG_CORR_CONF1_corr_s4);
    // 424 kHz subcarrier stream mode on
    st25r3916_write_reg(handle, ST25R3916_REG_CORR_CONF2, ST25R3916_REG_CORR_CONF2_corr_s8);
    return FuriHalNfcErrorNone;
}

static FuriHalNfcError furi_hal_nfc_iso15693_poller_init(FuriHalSpiBusHandle* handle) {
    furi_check(furi_hal_nfc_iso15693_poller == NULL);

    furi_hal_nfc_iso15693_poller = furi_hal_nfc_iso15693_poller_alloc();

    // Enable Subcarrier Stream mode, OOK modulation
    st25r3916_change_reg_bits(
        handle,
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_om_mask | ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_om_subcarrier_stream | ST25R3916_REG_MODE_tr_am_ook);

    // Subcarrier 424 kHz mode
    // 8 sub-carrier pulses in report period
    st25r3916_write_reg(
        handle,
        ST25R3916_REG_STREAM_MODE,
        ST25R3916_REG_STREAM_MODE_scf_sc424 | ST25R3916_REG_STREAM_MODE_stx_106 |
            ST25R3916_REG_STREAM_MODE_scp_8pulses);

    // Use regulator AM, resistive AM disabled
    st25r3916_clear_reg_bits(
        handle,
        ST25R3916_REG_AUX_MOD,
        ST25R3916_REG_AUX_MOD_dis_reg_am | ST25R3916_REG_AUX_MOD_res_am);

    return furi_hal_nfc_iso15693_common_init(handle);
}

static FuriHalNfcError furi_hal_nfc_iso15693_poller_deinit(FuriHalSpiBusHandle* handle) {
    UNUSED(handle);
    furi_check(furi_hal_nfc_iso15693_poller);

    furi_hal_nfc_iso15693_poller_free(furi_hal_nfc_iso15693_poller);
    furi_hal_nfc_iso15693_poller = NULL;

    return FuriHalNfcErrorNone;
}

static void iso15693_3_poller_encode_frame(
    const uint8_t* tx_data,
    size_t tx_bits,
    uint8_t* frame_buf,
    size_t frame_buf_size,
    size_t* frame_buf_bits) {
    static const uint8_t bit_patterns_1_out_of_4[] = {0x02, 0x08, 0x20, 0x80};
    size_t frame_buf_size_calc = (tx_bits / 2) + 2;
    furi_check(frame_buf_size >= frame_buf_size_calc);

    // Add SOF 1 out of 4
    frame_buf[0] = 0x21;

    size_t byte_pos = 1;
    for(size_t i = 0; i < tx_bits / BITS_IN_BYTE; ++i) {
        for(size_t j = 0; j < BITS_IN_BYTE; j += (BITS_IN_BYTE) / 4) {
            const uint8_t bit_pair = (tx_data[i] >> j) & 0x03;
            frame_buf[byte_pos++] = bit_patterns_1_out_of_4[bit_pair];
        }
    }
    // Add EOF
    frame_buf[byte_pos++] = 0x04;
    *frame_buf_bits = byte_pos * BITS_IN_BYTE;
}

static FuriHalNfcError iso15693_3_poller_decode_frame(
    const uint8_t* buf,
    size_t buf_bits,
    uint8_t* buf_decoded,
    size_t buf_decoded_size,
    size_t* buf_decoded_bits) {
    FuriHalNfcError ret = FuriHalNfcErrorDataFormat;
    size_t bit_pos = 0;
    memset(buf_decoded, 0, buf_decoded_size);

    do {
        if(buf_bits == 0) break;
        // Check SOF
        if((buf[0] & FURI_HAL_NFC_ISO15693_RESP_SOF_MASK) !=
           FURI_HAL_NFC_ISO15693_RESP_SOF_PATTERN)
            break;

        if(buf_bits == BITS_IN_BYTE) {
            ret = FuriHalNfcErrorIncompleteFrame;
            break;
        }

        // 2 response bits = 1 data bit
        for(uint32_t i = FURI_HAL_NFC_ISO15693_RESP_SOF_SIZE;
            i < buf_bits - FURI_HAL_NFC_ISO15693_RESP_SOF_SIZE;
            i += BITS_IN_BYTE / 4) {
            const size_t byte_index = i / BITS_IN_BYTE;
            const size_t bit_offset = i % BITS_IN_BYTE;
            const uint8_t resp_byte = (buf[byte_index] >> bit_offset) |
                                      (buf[byte_index + 1] << (BITS_IN_BYTE - bit_offset));

            // Check EOF
            if(resp_byte == FURI_HAL_NFC_ISO15693_RESP_EOF_PATTERN) {
                ret = FuriHalNfcErrorNone;
                break;
            }

            const uint8_t bit_pattern = resp_byte & FURI_HAL_NFC_ISO15693_RESP_PATTERN_MASK;

            if(bit_pattern == FURI_HAL_NFC_ISO15693_RESP_PATTERN_0) {
                bit_pos++;
            } else if(bit_pattern == FURI_HAL_NFC_ISO15693_RESP_PATTERN_1) {
                buf_decoded[bit_pos / BITS_IN_BYTE] |= 1 << (bit_pos % BITS_IN_BYTE);
                bit_pos++;
            } else {
                break;
            }
            if(bit_pos / BITS_IN_BYTE > buf_decoded_size) {
                break;
            }
        }

    } while(false);

    if(ret == FuriHalNfcErrorNone) {
        *buf_decoded_bits = bit_pos;
    }

    return ret;
}

static FuriHalNfcError furi_hal_nfc_iso15693_poller_tx(
    FuriHalSpiBusHandle* handle,
    const uint8_t* tx_data,
    size_t tx_bits) {
    FuriHalNfcIso15693Poller* instance = furi_hal_nfc_iso15693_poller;
    iso15693_3_poller_encode_frame(
        tx_data,
        tx_bits,
        instance->frame_buf,
        sizeof(instance->frame_buf),
        &instance->frame_buf_bits);
    return furi_hal_nfc_poller_tx_common(handle, instance->frame_buf, instance->frame_buf_bits);
}

static FuriHalNfcError furi_hal_nfc_iso15693_poller_rx(
    FuriHalSpiBusHandle* handle,
    uint8_t* rx_data,
    size_t rx_data_size,
    size_t* rx_bits) {
    FuriHalNfcError error = FuriHalNfcErrorNone;
    FuriHalNfcIso15693Poller* instance = furi_hal_nfc_iso15693_poller;

    do {
        error = furi_hal_nfc_common_fifo_rx(
            handle, instance->fifo_buf, sizeof(instance->fifo_buf), &instance->fifo_buf_bits);
        if(error != FuriHalNfcErrorNone) break;

        error = iso15693_3_poller_decode_frame(
            instance->fifo_buf,
            instance->fifo_buf_bits,
            instance->frame_buf,
            sizeof(instance->frame_buf),
            &instance->frame_buf_bits);
        if(error != FuriHalNfcErrorNone) break;

        if(rx_data_size < instance->frame_buf_bits / BITS_IN_BYTE) {
            error = FuriHalNfcErrorBufferOverflow;
            break;
        }

        memcpy(rx_data, instance->frame_buf, instance->frame_buf_bits / BITS_IN_BYTE);
        *rx_bits = instance->frame_buf_bits;
    } while(false);

    return error;
}

static void furi_hal_nfc_iso15693_listener_transparent_mode_enter(FuriHalSpiBusHandle* handle) {
    st25r3916_direct_cmd(handle, ST25R3916_CMD_TRANSPARENT_MODE);

    furi_hal_spi_bus_handle_deinit(handle);
    furi_hal_nfc_deinit_gpio_isr();
}

static void furi_hal_nfc_iso15693_listener_transparent_mode_exit(FuriHalSpiBusHandle* handle) {
    // Configure gpio back to SPI and exit transparent mode
    furi_hal_nfc_init_gpio_isr();
    furi_hal_spi_bus_handle_init(handle);

    st25r3916_direct_cmd(handle, ST25R3916_CMD_UNMASK_RECEIVE_DATA);
}

static FuriHalNfcError furi_hal_nfc_iso15693_listener_init(FuriHalSpiBusHandle* handle) {
    furi_check(furi_hal_nfc_iso15693_listener == NULL);

    furi_hal_nfc_iso15693_listener = furi_hal_nfc_iso15693_listener_alloc();

    // Set default operation mode
    st25r3916_change_reg_bits(
        handle,
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_om_mask | ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_om_targ_nfca | ST25R3916_REG_MODE_tr_am_ook);

    st25r3916_change_reg_bits(
        handle,
        ST25R3916_REG_OP_CONTROL,
        ST25R3916_REG_OP_CONTROL_rx_en,
        ST25R3916_REG_OP_CONTROL_rx_en);

    // Enable passive target mode
    st25r3916_change_reg_bits(
        handle, ST25R3916_REG_MODE, ST25R3916_REG_MODE_targ, ST25R3916_REG_MODE_targ_targ);

    FuriHalNfcError error = furi_hal_nfc_iso15693_common_init(handle);

    furi_hal_nfc_iso15693_listener_transparent_mode_enter(handle);

    return error;
}

static FuriHalNfcError furi_hal_nfc_iso15693_listener_deinit(FuriHalSpiBusHandle* handle) {
    furi_check(furi_hal_nfc_iso15693_listener);

    furi_hal_nfc_iso15693_listener_transparent_mode_exit(handle);

    furi_hal_nfc_iso15693_listener_free(furi_hal_nfc_iso15693_listener);
    furi_hal_nfc_iso15693_listener = NULL;

    return FuriHalNfcErrorNone;
}

static FuriHalNfcError
    furi_hal_nfc_iso15693_listener_tx_transparent(const uint8_t* data, size_t data_size) {
    iso15693_signal_tx(
        furi_hal_nfc_iso15693_listener->signal, Iso15693SignalDataRateHi, data, data_size);

    return FuriHalNfcErrorNone;
}

static void furi_hal_nfc_iso15693_parser_callback(Iso15693ParserEvent event, void* context) {
    furi_check(context);

    if(event == Iso15693ParserEventDataReceived) {
        FuriThreadId thread_id = context;
        furi_thread_flags_set(thread_id, FuriHalNfcEventInternalTypeTransparentDataReceived);
    }
}

static FuriHalNfcEvent furi_hal_nfc_iso15693_wait_event(uint32_t timeout_ms) {
    FuriHalNfcEvent event = 0;

    FuriThreadId thread_id = furi_thread_get_current_id();
    iso15693_parser_start(
        furi_hal_nfc_iso15693_listener->parser, furi_hal_nfc_iso15693_parser_callback, thread_id);

    while(true) {
        uint32_t flag = furi_thread_flags_wait(
            FuriHalNfcEventInternalTypeAbort | FuriHalNfcEventInternalTypeTransparentDataReceived,
            FuriFlagWaitAny,
            timeout_ms);
        furi_thread_flags_clear(flag);

        if(flag & FuriHalNfcEventInternalTypeAbort) {
            event = FuriHalNfcEventAbortRequest;
            break;
        }
        if(flag & FuriHalNfcEventInternalTypeTransparentDataReceived) {
            if(iso15693_parser_run(furi_hal_nfc_iso15693_listener->parser)) {
                event = FuriHalNfcEventRxEnd;
                break;
            }
        }
    }
    iso15693_parser_stop(furi_hal_nfc_iso15693_listener->parser);

    return event;
}

static FuriHalNfcError furi_hal_nfc_iso15693_listener_tx(
    FuriHalSpiBusHandle* handle,
    const uint8_t* tx_data,
    size_t tx_bits) {
    UNUSED(handle);
    furi_check(furi_hal_nfc_iso15693_listener);

    FuriHalNfcError error = FuriHalNfcErrorNone;

    error = furi_hal_nfc_iso15693_listener_tx_transparent(tx_data, tx_bits / BITS_IN_BYTE);

    return error;
}

FuriHalNfcError furi_hal_nfc_iso15693_listener_tx_sof(void) {
    iso15693_signal_tx_sof(furi_hal_nfc_iso15693_listener->signal, Iso15693SignalDataRateHi);

    return FuriHalNfcErrorNone;
}

static FuriHalNfcError furi_hal_nfc_iso15693_listener_rx(
    FuriHalSpiBusHandle* handle,
    uint8_t* rx_data,
    size_t rx_data_size,
    size_t* rx_bits) {
    furi_check(furi_hal_nfc_iso15693_listener);
    UNUSED(handle);

    if(rx_data_size <
       iso15693_parser_get_data_size_bytes(furi_hal_nfc_iso15693_listener->parser)) {
        return FuriHalNfcErrorBufferOverflow;
    }

    iso15693_parser_get_data(
        furi_hal_nfc_iso15693_listener->parser, rx_data, rx_data_size, rx_bits);

    return FuriHalNfcErrorNone;
}

FuriHalNfcError furi_hal_nfc_iso15693_listener_sleep(FuriHalSpiBusHandle* handle) {
    UNUSED(handle);

    return FuriHalNfcErrorNone;
}

const FuriHalNfcTechBase furi_hal_nfc_iso15693 = {
    .poller =
        {
            .compensation =
                {
                    .fdt = FURI_HAL_NFC_POLLER_FDT_COMP_FC,
                    .fwt = FURI_HAL_NFC_ISO15693_POLLER_FWT_COMP_FC,
                },
            .init = furi_hal_nfc_iso15693_poller_init,
            .deinit = furi_hal_nfc_iso15693_poller_deinit,
            .wait_event = furi_hal_nfc_wait_event_common,
            .tx = furi_hal_nfc_iso15693_poller_tx,
            .rx = furi_hal_nfc_iso15693_poller_rx,
        },

    .listener =
        {
            .compensation =
                {
                    .fdt = FURI_HAL_NFC_ISO15693_LISTENER_FDT_COMP_FC,
                },
            .init = furi_hal_nfc_iso15693_listener_init,
            .deinit = furi_hal_nfc_iso15693_listener_deinit,
            .wait_event = furi_hal_nfc_iso15693_wait_event,
            .tx = furi_hal_nfc_iso15693_listener_tx,
            .rx = furi_hal_nfc_iso15693_listener_rx,
            .sleep = furi_hal_nfc_iso15693_listener_sleep,
            .idle = furi_hal_nfc_iso15693_listener_sleep,
        },
};
