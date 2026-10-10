#include "furi_hal_nfc_i.h"
#include "furi_hal_nfc_tech_i.h"

#include <digital_signal/presets/nfc/texkom_signal.h>
#include <signal_reader/signal_reader.h>

#include <furi_hal_resources.h>

/**
 * Texkom tags neither answer commands nor take part in anticollision - they just load-modulate a
 * repeating impulse train for as long as they are in a field, and the payload is in the spacing
 * between impulses. The chip has no framing engine for that, so both directions run in transparent
 * mode, where the digital path is bypassed and the analog front end is wired straight to two pins.
 *
 * Reading configures the receive chain for a 212 kHz subcarrier and samples the digitized receive
 * output, which transparent mode puts on MISO. Gaps are handed up in sample units rather than the
 * protocol's own numbers; the decoder compares gaps against each other and does not care (see
 * texkom_i.h). Emulating drives the modulator from a precomputed waveform on MOSI.
 *
 * An earlier attempt sampled the IRQ pin instead, the way the ISO15693 listener does. In poller
 * mode that line carries nothing: with the field confirmed on, it produced not one edge.
 */

/**
 * How long one read attempt listens, in milliseconds.
 *
 * A frame is only cut once a whole repetition has been seen between two pauses, so a capture that
 * starts mid-frame has to wait for the next one and this window does not guarantee a hit. The
 * poller is called again as soon as it returns, and a short window is what stops Texkom from
 * slowing down every other card's detection.
 */
#define FURI_HAL_NFC_TEXKOM_RX_TIMEOUT_MS (20U)

/** Reload value of a 64 MHz timer. ARR is counted inclusively, so 127 gives 500 ksps. */
#define FURI_HAL_NFC_TEXKOM_SAMPLE_RATE_F64MHZ (127U)
#define FURI_HAL_NFC_TEXKOM_SAMPLE_PERIOD_US   ((FURI_HAL_NFC_TEXKOM_SAMPLE_RATE_F64MHZ + 1U) / 64U)

#define FURI_HAL_NFC_TEXKOM_SAMPLES(us) ((us) / FURI_HAL_NFC_TEXKOM_SAMPLE_PERIOD_US)

/** Bitstream bytes the signal reader hands over in two halves. Reading was validated at this. */
#define FURI_HAL_NFC_TEXKOM_BUFF_SIZE (32U)

/**
 * Samples closer together than this belong to the same impulse.
 *
 * Roughly two and a half impulse widths, to absorb jitter and ringing around the one edge the
 * digitizer reports, while staying clear of the shortest real gap - TK17's, at about 28 us.
 */
#define FURI_HAL_NFC_TEXKOM_REFRACTORY FURI_HAL_NFC_TEXKOM_SAMPLES(12U)

/**
 * A gap this long is the pause between frame repetitions, not part of a frame.
 *
 * The longest in-frame gap is the first of TK17's 00 symbol, about 123 us, and on hardware the
 * pause between repetitions is far longer than this. It also caps a stored gap at a byte.
 */
#define FURI_HAL_NFC_TEXKOM_FRAME_GAP FURI_HAL_NFC_TEXKOM_SAMPLES(200U)

/** One fewer than each frame's impulse count: the last gap falls in the pause, and is never timed. */
#define FURI_HAL_NFC_TEXKOM_TK17_GAPS (64U)
#define FURI_HAL_NFC_TEXKOM_TK13_GAPS (127U)
#define FURI_HAL_NFC_TEXKOM_MAX_GAPS  (FURI_HAL_NFC_TEXKOM_TK13_GAPS)

_Static_assert(
    FURI_HAL_NFC_TEXKOM_FRAME_GAP <= UINT8_MAX,
    "A gap is stored in a byte, so the saturation point has to fit in one");

/**
 * Pause between emitted frames, in milliseconds.
 *
 * Doubles as the quiet period a reader needs to tell one repetition from the next, and as the
 * window in which an abort gets noticed. PM3's figure.
 *
 * Note the cost: digital_sequence_transmit() masks interrupts for the whole waveform, so ticks are
 * lost while a frame goes out. Measured on hardware by comparing uptime against wall clock, the
 * system clock runs at about 70% of real time while emulating, against 1:1 when idle. Raising this
 * would reduce that, at the risk of a reader missing a repetition.
 */
#define FURI_HAL_NFC_TEXKOM_TX_GAP_MS (3U)

/**
 * Frame byte naming the variant, and the one value of it that selects the TK17 coding.
 *
 * Hand-kept copies of TEXKOM_TYPE_BYTE_TK17 and of the index texkom.c writes, because furi_hal
 * sits below lib/nfc and must not include it - and unlike the same duplication in texkom_signal.c
 * there is no _Static_assert available here, so this pair has no safety net.
 */
#define FURI_HAL_NFC_TEXKOM_TYPE_BYTE (2U)
#define FURI_HAL_NFC_TEXKOM_TYPE_TK17 (0xCAU)

#define BITS_IN_BYTE (8U)

#define TAG "FuriHalTexkom"

/** Running state of one capture, carried across callbacks. */
typedef struct {
    /** Samples since the last impulse, saturating so a long pause cannot wrap. */
    uint32_t since_impulse;
    bool impulse_seen;
    bool last_level;

    uint8_t gaps[FURI_HAL_NFC_TEXKOM_MAX_GAPS];
    size_t gap_count;
    /** Set when a train grew past the array; the rest of it is skipped. */
    bool overflow;

    /** Non-zero once gaps[] holds a whole frame, which is also what freezes gaps[]. */
    size_t frame_count;

    FuriThreadId thread_id;

    /** Longest train seen, usable length or not: the one number worth having when a read fails. */
    size_t longest_train;
} FuriHalNfcTexkomCapture;

static TexkomSignal* furi_hal_nfc_texkom_signal = NULL;
static SignalReader* furi_hal_nfc_texkom_reader = NULL;

/**
 * @brief Set up the receive chain for a 212 kHz subcarrier.
 *
 * The filter corners are the ones the ISO15693 poller uses - they pass the whole subcarrier band
 * - but the correlator is left at 212 kHz instead of being switched up to 424.
 */
static void furi_hal_nfc_texkom_configure_rx(const FuriHalSpiBusHandle* handle) {
    // Subcarrier stream mode, OOK.
    st25r3916_change_reg_bits(
        handle,
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_om_mask | ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_om_subcarrier_stream | ST25R3916_REG_MODE_tr_am_ook);

    // Subcarrier 212 kHz, which is what the digitizer feeding MISO is tuned to. The pulses-per-bit
    // field only governs FIFO reporting, which transparent mode bypasses.
    st25r3916_write_reg(
        handle,
        ST25R3916_REG_STREAM_MODE,
        ST25R3916_REG_STREAM_MODE_scf_sc212 | ST25R3916_REG_STREAM_MODE_stx_106 |
            ST25R3916_REG_STREAM_MODE_scp_1pulse);

    // 1st stage zero = 12 kHz, 3rd stage zero = 80 kHz, low-pass = 600 kHz
    st25r3916_write_reg(
        handle,
        ST25R3916_REG_RX_CONF1,
        ST25R3916_REG_RX_CONF1_z12k | ST25R3916_REG_RX_CONF1_h80 |
            ST25R3916_REG_RX_CONF1_lp_600khz);

    // AGC with reset, ratio 6, running over the whole receive period, squelch on TX end
    st25r3916_write_reg(
        handle,
        ST25R3916_REG_RX_CONF2,
        ST25R3916_REG_RX_CONF2_agc6_3 | ST25R3916_REG_RX_CONF2_agc_m |
            ST25R3916_REG_RX_CONF2_agc_en | ST25R3916_REG_RX_CONF2_sqm_dyn);

    // HF operation, AM channel attenuated by 3 of 7 steps. The ISO15693 poller runs this channel
    // wide open; the attenuation was added during bring-up while the digitizer sat pinned to one
    // rail, and is kept because reading was validated on hardware with it in place.
    st25r3916_write_reg(
        handle,
        ST25R3916_REG_RX_CONF3,
        ST25R3916_REG_RX_CONF3_rg1_am1 | ST25R3916_REG_RX_CONF3_rg1_am0);
    st25r3916_write_reg(handle, ST25R3916_REG_RX_CONF4, 0x00);

    // Collision level 53%, AM and PM summed before the digitizer
    st25r3916_write_reg(
        handle,
        ST25R3916_REG_CORR_CONF1,
        ST25R3916_REG_CORR_CONF1_corr_s0 | ST25R3916_REG_CORR_CONF1_corr_s1 |
            ST25R3916_REG_CORR_CONF1_corr_s4);
    // corr_s8 clear - 212 kHz subcarrier stream mode, matching scf_sc212 above
    st25r3916_write_reg(handle, ST25R3916_REG_CORR_CONF2, 0x00);

    // Regulator AM, resistive AM off - parity with the ISO15693 poller. Nothing is transmitted
    // here, so it has no effect either way.
    st25r3916_clear_reg_bits(
        handle,
        ST25R3916_REG_AUX_MOD,
        ST25R3916_REG_AUX_MOD_dis_reg_am | ST25R3916_REG_AUX_MOD_res_am);
}

/**
 * @brief Wait for any of a set of internal event flags.
 *
 * furi_thread_flags_wait() reports a timeout by returning FuriFlagErrorTimeout, whose bits overlap
 * any mask, so the status has to be ruled out before the flags are read.
 *
 * @returns the flags that were set, or 0 on timeout.
 */
static uint32_t furi_hal_nfc_texkom_wait_flags(uint32_t flags, uint32_t timeout_ms) {
    const uint32_t result = furi_thread_flags_wait(flags, FuriFlagWaitAny, timeout_ms);

    if(result == (unsigned)FuriFlagErrorTimeout) return 0;

    return result & flags;
}

/********************************** Poller **********************************/

/** Keep what has accumulated if it is one of the two frame lengths, otherwise start over. */
static void furi_hal_nfc_texkom_cut_frame(FuriHalNfcTexkomCapture* capture) {
    if(!capture->overflow && (capture->gap_count == FURI_HAL_NFC_TEXKOM_TK13_GAPS ||
                              capture->gap_count == FURI_HAL_NFC_TEXKOM_TK17_GAPS)) {
        // gaps[] is the frame. Setting this is what stops the sampling callback writing over it.
        capture->frame_count = capture->gap_count;
        furi_thread_flags_set(
            capture->thread_id, FuriHalNfcEventInternalTypeTransparentDataReceived);
        return;
    }

    capture->gap_count = 0;
    capture->overflow = false;
    capture->impulse_seen = false;
}

static void furi_hal_nfc_texkom_handle_impulse(FuriHalNfcTexkomCapture* capture) {
    if(capture->since_impulse < FURI_HAL_NFC_TEXKOM_REFRACTORY) {
        // Still inside the impulse we already counted.
        return;
    }

    if(capture->impulse_seen) {
        if(capture->gap_count < FURI_HAL_NFC_TEXKOM_MAX_GAPS) {
            capture->gaps[capture->gap_count++] = capture->since_impulse;
            if(capture->gap_count > capture->longest_train) {
                capture->longest_train = capture->gap_count;
            }
        } else {
            capture->overflow = true;
        }
    }

    capture->impulse_seen = true;
    capture->since_impulse = 0;
}

/**
 * @brief Time the gaps in one block of sampled line levels.
 *
 * Runs in the signal reader's interrupt, at the highest priority, and carries its state in
 * @p capture because a frame routinely spans several blocks.
 */
static void
    furi_hal_nfc_texkom_feed(FuriHalNfcTexkomCapture* capture, const uint8_t* data, size_t bytes) {
    for(size_t i = 0; i < bytes; i++) {
        const uint8_t byte = data[i];

        // An impulse is a handful of samples in every twenty, so most bytes are silence and only
        // the gap counter moves across them. Skipping the bit loop for those keeps this interrupt
        // off the back of the CPU: nothing below can change state on a silent byte that cannot
        // reach the frame-gap threshold, and before the first impulse nothing can happen at all.
        if(byte == 0 && !capture->last_level) {
            if(!capture->impulse_seen) continue;
            if(capture->since_impulse + BITS_IN_BYTE < FURI_HAL_NFC_TEXKOM_FRAME_GAP) {
                capture->since_impulse += BITS_IN_BYTE;
                continue;
            }
        }

        for(size_t bit = 0; bit < BITS_IN_BYTE; bit++) {
            const bool level = FURI_BIT(byte, bit);

            if(capture->since_impulse < FURI_HAL_NFC_TEXKOM_FRAME_GAP) {
                capture->since_impulse++;
            } else if(capture->impulse_seen) {
                // The pause between repetitions: whatever came before it was one whole frame.
                furi_hal_nfc_texkom_cut_frame(capture);
                // gaps[] now belongs to the waiting thread, which is about to stop us.
                if(capture->frame_count) return;
            }

            if(level && !capture->last_level) {
                furi_hal_nfc_texkom_handle_impulse(capture);
            }
            capture->last_level = level;
        }
    }
}

static void furi_hal_nfc_texkom_reader_callback(SignalReaderEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.data->data);

    FuriHalNfcTexkomCapture* capture = context;
    if(capture->frame_count) return;

    furi_hal_nfc_texkom_feed(capture, event.data->data, event.data->len);
}

static FuriHalNfcError furi_hal_nfc_texkom_poller_init(const FuriHalSpiBusHandle* handle) {
    furi_check(furi_hal_nfc_texkom_reader == NULL);

    furi_hal_nfc_texkom_configure_rx(handle);

    // The chip puts the digitized subcarrier on MISO in transparent mode.
    furi_hal_nfc_texkom_reader =
        signal_reader_alloc(&gpio_spi_r_miso, FURI_HAL_NFC_TEXKOM_BUFF_SIZE);
    signal_reader_set_sample_rate(
        furi_hal_nfc_texkom_reader,
        SignalReaderTimeUnit64Mhz,
        FURI_HAL_NFC_TEXKOM_SAMPLE_RATE_F64MHZ);
    // The SPI handle leaves MISO pulled up when it hands the pin over, and furi_hal_nfc_init()
    // disables the chip's own pull-down on it. Idle has to read as no impulse, so pull it down.
    signal_reader_set_pull(furi_hal_nfc_texkom_reader, GpioPullDown);
    signal_reader_set_polarity(furi_hal_nfc_texkom_reader, SignalReaderPolarityNormal);
    // The tag talks unprompted, so there is no frame edge to trigger on - sample free-running.
    signal_reader_set_trigger(furi_hal_nfc_texkom_reader, SignalReaderTriggerNone);

    return FuriHalNfcErrorNone;
}

static FuriHalNfcError furi_hal_nfc_texkom_poller_deinit(const FuriHalSpiBusHandle* handle) {
    UNUSED(handle);
    furi_check(furi_hal_nfc_texkom_reader);

    signal_reader_free(furi_hal_nfc_texkom_reader);
    furi_hal_nfc_texkom_reader = NULL;

    return FuriHalNfcErrorNone;
}

static FuriHalNfcError furi_hal_nfc_texkom_poller_tx(
    const FuriHalSpiBusHandle* handle,
    const uint8_t* tx_data,
    size_t tx_bits) {
    UNUSED(handle);
    UNUSED(tx_data);
    UNUSED(tx_bits);

    // There is nothing a Texkom tag would listen to. Reaching here is a caller mistake rather than
    // anything the chip did, but furi_hal_nfc_poller_tx() is exported, so report it instead of
    // taking the device down.
    return FuriHalNfcErrorDataFormat;
}

/**
 * @brief Capture one repetition of the impulse train.
 *
 * Fills @p rx_data with the gaps between impulses, one byte each, in capture samples.
 */
static FuriHalNfcError furi_hal_nfc_texkom_poller_rx(
    const FuriHalSpiBusHandle* handle,
    uint8_t* rx_data,
    size_t rx_data_size,
    size_t* rx_bits) {
    furi_check(furi_hal_nfc_texkom_reader);
    furi_check(rx_bits);

    *rx_bits = 0;

    FuriHalNfcTexkomCapture capture = {0};
    // Start saturated, so the first impulse is neither swallowed by the refractory window nor
    // taken for the end of a frame.
    capture.since_impulse = FURI_HAL_NFC_TEXKOM_FRAME_GAP;
    capture.thread_id = furi_thread_get_current_id();

    // Transparent mode hands MISO and MOSI to the analog front end; the SPI handle's deinit leaves
    // MOSI driven low, which is the idle the modulator needs.
    furi_hal_nfc_transparent_mode_enter(handle);

    furi_thread_flags_clear(FuriHalNfcEventInternalTypeTransparentDataReceived);
    signal_reader_start(furi_hal_nfc_texkom_reader, furi_hal_nfc_texkom_reader_callback, &capture);

    // Abort is deliberately not waited on here: the window is short, and leaving the flag alone
    // lets furi_hal_nfc_wait_event_common() handle it the way it does for every other technology.
    furi_hal_nfc_texkom_wait_flags(
        FuriHalNfcEventInternalTypeTransparentDataReceived, FURI_HAL_NFC_TEXKOM_RX_TIMEOUT_MS);

    // Disarms the interrupt before it returns, so nothing can write through `capture` afterwards.
    // That is what lets the capture live on this stack frame without being volatile.
    signal_reader_stop(furi_hal_nfc_texkom_reader);

    furi_hal_nfc_transparent_mode_exit(handle);

    if(capture.frame_count == 0) {
        FURI_LOG_D(TAG, "No frame, longest train %zu gaps", capture.longest_train);
        return FuriHalNfcErrorCommunicationTimeout;
    }

    if(capture.frame_count > rx_data_size) {
        return FuriHalNfcErrorBufferOverflow;
    }

    memcpy(rx_data, capture.gaps, capture.frame_count);
    *rx_bits = capture.frame_count * BITS_IN_BYTE;

    return FuriHalNfcErrorNone;
}

/********************************* Listener *********************************/

/**
 * @brief Coding a frame is to be sent in.
 *
 * Each variant uses one coding, and a decoded or loaded frame is guaranteed to agree with the
 * coding it arrived in, so this cannot disagree with the stored type.
 */
static TexkomSignalCoding furi_hal_nfc_texkom_coding(const uint8_t* frame) {
    return (frame[FURI_HAL_NFC_TEXKOM_TYPE_BYTE] == FURI_HAL_NFC_TEXKOM_TYPE_TK17) ?
               TexkomSignalCodingTk17 :
               TexkomSignalCodingTk13;
}

static FuriHalNfcError furi_hal_nfc_texkom_listener_init(const FuriHalSpiBusHandle* handle) {
    furi_check(furi_hal_nfc_texkom_signal == NULL);

    furi_hal_nfc_texkom_signal = texkom_signal_alloc(&gpio_spi_r_mosi);

    // Touch only what this mode needs. A SET_DEFAULT here would also wipe the IO drive strength,
    // supply, thermal and MISO pull-down settings that furi_hal_nfc_init() works out once per boot
    // and that nothing puts back.
    st25r3916_change_reg_bits(
        handle,
        ST25R3916_REG_OP_CONTROL,
        ST25R3916_REG_OP_CONTROL_rx_en,
        ST25R3916_REG_OP_CONTROL_rx_en);

    // Passive target, OOK. Transparent mode drives the modulator directly, so the operating mode
    // only has to put the chip into target state.
    st25r3916_change_reg_bits(
        handle,
        ST25R3916_REG_MODE,
        ST25R3916_REG_MODE_targ | ST25R3916_REG_MODE_om_mask | ST25R3916_REG_MODE_tr_am,
        ST25R3916_REG_MODE_targ_targ | ST25R3916_REG_MODE_om_targ_nfca |
            ST25R3916_REG_MODE_tr_am_ook);

    furi_hal_nfc_transparent_mode_enter(handle);

    return FuriHalNfcErrorNone;
}

static FuriHalNfcError furi_hal_nfc_texkom_listener_deinit(const FuriHalSpiBusHandle* handle) {
    furi_check(furi_hal_nfc_texkom_signal);

    furi_hal_nfc_transparent_mode_exit(handle);

    texkom_signal_free(furi_hal_nfc_texkom_signal);
    furi_hal_nfc_texkom_signal = NULL;

    return FuriHalNfcErrorNone;
}

/**
 * @brief Pace the emulation loop.
 *
 * In transparent mode the chip cannot report anything, not even whether a field is present, so
 * there is no hardware event to wait for. Hand the caller a tick instead, which makes it send the
 * frame again, and use the wait itself as the pause between repetitions.
 */
static FuriHalNfcEvent furi_hal_nfc_texkom_listener_wait_event(uint32_t timeout_ms) {
    const uint32_t wait_ms = MIN(timeout_ms, FURI_HAL_NFC_TEXKOM_TX_GAP_MS);

    if(furi_hal_nfc_texkom_wait_flags(FuriHalNfcEventInternalTypeAbort, wait_ms)) {
        return FuriHalNfcEventAbortRequest;
    }

    return FuriHalNfcEventListenerTick;
}

static FuriHalNfcError furi_hal_nfc_texkom_listener_tx(
    const FuriHalSpiBusHandle* handle,
    const uint8_t* tx_data,
    size_t tx_bits) {
    UNUSED(handle);
    furi_check(furi_hal_nfc_texkom_signal);
    furi_check(tx_data);

    if(tx_bits != TEXKOM_SIGNAL_FRAME_SIZE * BITS_IN_BYTE) {
        return FuriHalNfcErrorDataFormat;
    }

    texkom_signal_tx(furi_hal_nfc_texkom_signal, furi_hal_nfc_texkom_coding(tx_data), tx_data);

    return FuriHalNfcErrorNone;
}

static FuriHalNfcError furi_hal_nfc_texkom_listener_rx(
    const FuriHalSpiBusHandle* handle,
    uint8_t* rx_data,
    size_t rx_data_size,
    size_t* rx_bits) {
    UNUSED(handle);
    UNUSED(rx_data);
    UNUSED(rx_data_size);
    furi_check(rx_bits);

    // Only ever reached from an RxEnd event, which this technology never raises - a reader has
    // nothing to say to a Texkom tag, and transparent mode could not report it if it did.
    *rx_bits = 0;

    return FuriHalNfcErrorDataFormat;
}

static FuriHalNfcError furi_hal_nfc_texkom_listener_sleep(const FuriHalSpiBusHandle* handle) {
    UNUSED(handle);

    // Nothing to drop into or come back from: the tag has no halt command to obey, and in
    // transparent mode the chip would not hear one anyway.
    return FuriHalNfcErrorNone;
}

const FuriHalNfcTechBase furi_hal_nfc_texkom = {
    .poller =
        {
            .compensation =
                {
                    .fdt = FURI_HAL_NFC_POLLER_FDT_COMP_FC,
                    .fwt = FURI_HAL_NFC_POLLER_FWT_COMP_FC,
                },
            .init = furi_hal_nfc_texkom_poller_init,
            .deinit = furi_hal_nfc_texkom_poller_deinit,
            .wait_event = furi_hal_nfc_wait_event_common,
            .tx = furi_hal_nfc_texkom_poller_tx,
            .rx = furi_hal_nfc_texkom_poller_rx,
        },

    .listener =
        {
            .compensation = {.fdt = 0},
            .init = furi_hal_nfc_texkom_listener_init,
            .deinit = furi_hal_nfc_texkom_listener_deinit,
            .wait_event = furi_hal_nfc_texkom_listener_wait_event,
            .tx = furi_hal_nfc_texkom_listener_tx,
            .rx = furi_hal_nfc_texkom_listener_rx,
            .sleep = furi_hal_nfc_texkom_listener_sleep,
            .idle = furi_hal_nfc_texkom_listener_sleep,
        },
};
