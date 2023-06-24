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

#include "nfcv.h"
#include "nfc_util.h"
#include "slix.h"

#define TAG "NfcV"

/* macros to map "modulate field" flag to GPIO level */
#define GPIO_LEVEL_MODULATED NFCV_LOAD_MODULATION_POLARITY
#define GPIO_LEVEL_UNMODULATED (!GPIO_LEVEL_MODULATED)

/* timing macros */
#define DIGITAL_SIGNAL_UNIT_S (100000000000.0f)
#define DIGITAL_SIGNAL_UNIT_US (100000.0f)

ReturnCode nfcv_inventory(uint8_t* uid) {
    uint16_t received = 0;
    rfalNfcvInventoryRes res;
    ReturnCode ret = ERR_NONE;

    for(int tries = 0; tries < NFCV_COMMAND_RETRIES; tries++) {
        /* TODO: needs proper abstraction via fury_hal(_ll)_* */
        ret = rfalNfcvPollerInventory(RFAL_NFCV_NUM_SLOTS_1, 0, NULL, &res, &received);

        if(ret == ERR_NONE) {
            break;
        }
    }

    if(ret == ERR_NONE) {
        if(uid != NULL) {
            memcpy(uid, res.UID, NFCV_UID_LENGTH);
        }
    }

    return ret;
}

ReturnCode nfcv_read_blocks(NfcVReader* reader, NfcVData* nfcv_data) {
    UNUSED(reader);

    uint16_t received = 0;
    for(size_t block = 0; block < nfcv_data->block_num; block++) {
        uint8_t rxBuf[32];
        FURI_LOG_D(TAG, "Reading block %d/%d", block, (nfcv_data->block_num - 1));

        ReturnCode ret = ERR_NONE;
        for(int tries = 0; tries < NFCV_COMMAND_RETRIES; tries++) {
            ret = rfalNfcvPollerReadSingleBlock(
                RFAL_NFCV_REQ_FLAG_DEFAULT, NULL, block, rxBuf, sizeof(rxBuf), &received);

            if(ret == ERR_NONE) {
                break;
            }
        }
        if(ret != ERR_NONE) {
            FURI_LOG_D(TAG, "failed to read: %d", ret);
            return ret;
        }
        memcpy(
            &(nfcv_data->data[block * nfcv_data->block_size]), &rxBuf[1], nfcv_data->block_size);
        FURI_LOG_D(
            TAG,
            "  %02X %02X %02X %02X",
            nfcv_data->data[block * nfcv_data->block_size + 0],
            nfcv_data->data[block * nfcv_data->block_size + 1],
            nfcv_data->data[block * nfcv_data->block_size + 2],
            nfcv_data->data[block * nfcv_data->block_size + 3]);
    }

    return ERR_NONE;
}

ReturnCode nfcv_read_sysinfo(FuriHalNfcDevData* nfc_data, NfcVData* nfcv_data) {
    uint8_t rxBuf[32];
    uint16_t received = 0;
    ReturnCode ret = ERR_NONE;

    FURI_LOG_D(TAG, "Read SYSTEM INFORMATION...");

    for(int tries = 0; tries < NFCV_COMMAND_RETRIES; tries++) {
        /* TODO: needs proper abstraction via fury_hal(_ll)_* */
        ret = rfalNfcvPollerGetSystemInformation(
            RFAL_NFCV_REQ_FLAG_DEFAULT, NULL, rxBuf, sizeof(rxBuf), &received);

        if(ret == ERR_NONE) {
            break;
        }
    }

    if(ret == ERR_NONE) {
        nfc_data->type = FuriHalNfcTypeV;
        nfc_data->uid_len = NFCV_UID_LENGTH;
        /* UID is stored reversed in this response */
        for(int pos = 0; pos < nfc_data->uid_len; pos++) {
            nfc_data->uid[pos] = rxBuf[2 + (NFCV_UID_LENGTH - 1 - pos)];
        }
        nfcv_data->dsfid = rxBuf[NFCV_UID_LENGTH + 2];
        nfcv_data->afi = rxBuf[NFCV_UID_LENGTH + 3];
        nfcv_data->block_num = rxBuf[NFCV_UID_LENGTH + 4] + 1;
        nfcv_data->block_size = rxBuf[NFCV_UID_LENGTH + 5] + 1;
        nfcv_data->ic_ref = rxBuf[NFCV_UID_LENGTH + 6];
        FURI_LOG_D(
            TAG,
            "  UID:          %02X %02X %02X %02X %02X %02X %02X %02X",
            nfc_data->uid[0],
            nfc_data->uid[1],
            nfc_data->uid[2],
            nfc_data->uid[3],
            nfc_data->uid[4],
            nfc_data->uid[5],
            nfc_data->uid[6],
            nfc_data->uid[7]);
        FURI_LOG_D(
            TAG,
            "  DSFID %d, AFI %d, Blocks %d, Size %d, IC Ref %d",
            nfcv_data->dsfid,
            nfcv_data->afi,
            nfcv_data->block_num,
            nfcv_data->block_size,
            nfcv_data->ic_ref);
        return ret;
    }
    FURI_LOG_D(TAG, "Failed: %d", ret);

    return ret;
}

bool nfcv_read_card(NfcVReader* reader, FuriHalNfcDevData* nfc_data, NfcVData* nfcv_data) {
    furi_assert(reader);
    furi_assert(nfc_data);
    furi_assert(nfcv_data);

    if(nfcv_read_sysinfo(nfc_data, nfcv_data) != ERR_NONE) {
        return false;
    }

    if(nfcv_read_blocks(reader, nfcv_data) != ERR_NONE) {
        return false;
    }

    /* clear all know sub type data before reading them */
    memset(&nfcv_data->sub_data, 0x00, sizeof(nfcv_data->sub_data));

    if(slix_check_card_type(nfc_data)) {
        FURI_LOG_I(TAG, "NXP SLIX detected");
        nfcv_data->sub_type = NfcVTypeSlix;
    } else if(slix2_check_card_type(nfc_data)) {
        FURI_LOG_I(TAG, "NXP SLIX2 detected");
        nfcv_data->sub_type = NfcVTypeSlix2;
        if(slix2_read_custom(nfc_data, nfcv_data) != ERR_NONE) {
            return false;
        }
    } else if(slix_s_check_card_type(nfc_data)) {
        FURI_LOG_I(TAG, "NXP SLIX-S detected");
        nfcv_data->sub_type = NfcVTypeSlixS;
    } else if(slix_l_check_card_type(nfc_data)) {
        FURI_LOG_I(TAG, "NXP SLIX-L detected");
        nfcv_data->sub_type = NfcVTypeSlixL;
    } else {
        nfcv_data->sub_type = NfcVTypePlain;
    }

    return true;
}

void nfcv_crc(uint8_t* data, uint32_t length) {
    uint32_t reg = 0xFFFF;

    for(size_t i = 0; i < length; i++) {
        reg = reg ^ ((uint32_t)data[i]);
        for(size_t j = 0; j < 8; j++) {
            if(reg & 0x0001) {
                reg = (reg >> 1) ^ 0x8408;
            } else {
                reg = (reg >> 1);
            }
        }
    }

    uint16_t crc = ~(uint16_t)(reg & 0xffff);

    data[length + 0] = crc & 0xFF;
    data[length + 1] = crc >> 8;
}

void nfcv_emu_free_signals(NfcVEmuAirSignals* signals) {
    furi_assert(signals);

    if(signals->nfcv_resp_one) {
        digital_signal_free(signals->nfcv_resp_one);
    }
    if(signals->nfcv_resp_zero) {
        digital_signal_free(signals->nfcv_resp_zero);
    }
    if(signals->nfcv_resp_sof) {
        digital_signal_free(signals->nfcv_resp_sof);
    }
    if(signals->nfcv_resp_eof) {
        digital_signal_free(signals->nfcv_resp_eof);
    }
    signals->nfcv_resp_one = NULL;
    signals->nfcv_resp_zero = NULL;
    signals->nfcv_resp_sof = NULL;
    signals->nfcv_resp_eof = NULL;
}

bool nfcv_emu_alloc_signals(NfcVEmuAir* air, NfcVEmuAirSignals* signals, uint32_t slowdown) {
    furi_assert(air);
    furi_assert(signals);

    bool success = true;

    if(!signals->nfcv_resp_one) {
        /* logical one: unmodulated then 8 pulses */
        signals->nfcv_resp_one = digital_signal_alloc(
            slowdown * (air->nfcv_resp_unmod->edge_cnt + 8 * air->nfcv_resp_pulse->edge_cnt));
        if(!signals->nfcv_resp_one) {
            return false;
        }
        for(size_t i = 0; i < slowdown; i++) {
            success &= digital_signal_append(signals->nfcv_resp_one, air->nfcv_resp_unmod);
        }
        for(size_t i = 0; i < slowdown * 8; i++) {
            success &= digital_signal_append(signals->nfcv_resp_one, air->nfcv_resp_pulse);
        }
        if(!success) {
            return false;
        }
    }
    if(!signals->nfcv_resp_zero) {
        /* logical zero: 8 pulses then unmodulated */
        signals->nfcv_resp_zero = digital_signal_alloc(
            slowdown * (8 * air->nfcv_resp_pulse->edge_cnt + air->nfcv_resp_unmod->edge_cnt));
        if(!signals->nfcv_resp_zero) {
            return false;
        }
        for(size_t i = 0; i < slowdown * 8; i++) {
            success &= digital_signal_append(signals->nfcv_resp_zero, air->nfcv_resp_pulse);
        }
        for(size_t i = 0; i < slowdown; i++) {
            success &= digital_signal_append(signals->nfcv_resp_zero, air->nfcv_resp_unmod);
        }
        if(!success) {
            return false;
        }
    }
    if(!signals->nfcv_resp_sof) {
        /* SOF: unmodulated, 24 pulses, logic 1 */
        signals->nfcv_resp_sof = digital_signal_alloc(
            slowdown * (3 * air->nfcv_resp_unmod->edge_cnt + 24 * air->nfcv_resp_pulse->edge_cnt) +
            signals->nfcv_resp_one->edge_cnt);
        if(!signals->nfcv_resp_sof) {
            return false;
        }
        for(size_t i = 0; i < slowdown * 3; i++) {
            success &= digital_signal_append(signals->nfcv_resp_sof, air->nfcv_resp_unmod);
        }
        for(size_t i = 0; i < slowdown * 24; i++) {
            success &= digital_signal_append(signals->nfcv_resp_sof, air->nfcv_resp_pulse);
        }
        success &= digital_signal_append(signals->nfcv_resp_sof, signals->nfcv_resp_one);
        if(!success) {
            return false;
        }
    }
    if(!signals->nfcv_resp_eof) {
        /* EOF: logic 0, 24 pulses, unmodulated */
        signals->nfcv_resp_eof = digital_signal_alloc(
            signals->nfcv_resp_zero->edge_cnt +
            slowdown * (24 * air->nfcv_resp_pulse->edge_cnt + 3 * air->nfcv_resp_unmod->edge_cnt) +
            air->nfcv_resp_unmod->edge_cnt);
        if(!signals->nfcv_resp_eof) {
            return false;
        }
        success &= digital_signal_append(signals->nfcv_resp_eof, signals->nfcv_resp_zero);
        for(size_t i = 0; i < slowdown * 23; i++) {
            success &= digital_signal_append(signals->nfcv_resp_eof, air->nfcv_resp_pulse);
        }
        /* we don't want to add the last level as we just want a transition to "unmodulated" again */
        for(size_t i = 0; i < slowdown; i++) {
            success &= digital_signal_append(signals->nfcv_resp_eof, air->nfcv_resp_half_pulse);
        }
    }
    return success;
}

bool nfcv_emu_alloc(NfcVData* nfcv_data) {
    furi_assert(nfcv_data);

    if(!nfcv_data->frame) {
        nfcv_data->frame = malloc(NFCV_FRAMESIZE_MAX);
        if(!nfcv_data->frame) {
            return false;
        }
    }

    if(!nfcv_data->emu_air.nfcv_signal) {
        /* assuming max frame length is 255 bytes */
        nfcv_data->emu_air.nfcv_signal = digital_sequence_alloc(8 * 255 + 2, &gpio_spi_r_mosi);
        if(!nfcv_data->emu_air.nfcv_signal) {
            return false;
        }
    }
    if(!nfcv_data->emu_air.nfcv_resp_unmod) {
        /* unmodulated 256/fc or 1024/fc signal as building block */
        nfcv_data->emu_air.nfcv_resp_unmod = digital_signal_alloc(4);
        if(!nfcv_data->emu_air.nfcv_resp_unmod) {
            return false;
        }
        nfcv_data->emu_air.nfcv_resp_unmod->start_level = GPIO_LEVEL_UNMODULATED;
        nfcv_data->emu_air.nfcv_resp_unmod->edge_timings[0] =
            (uint32_t)(NFCV_RESP_SUBC1_UNMOD_256 * DIGITAL_SIGNAL_UNIT_S);
        nfcv_data->emu_air.nfcv_resp_unmod->edge_cnt = 1;
    }
    if(!nfcv_data->emu_air.nfcv_resp_pulse) {
        /* modulated fc/32 or fc/8 pulse as building block */
        nfcv_data->emu_air.nfcv_resp_pulse = digital_signal_alloc(4);
        if(!nfcv_data->emu_air.nfcv_resp_pulse) {
            return false;
        }
        nfcv_data->emu_air.nfcv_resp_pulse->start_level = GPIO_LEVEL_MODULATED;
        nfcv_data->emu_air.nfcv_resp_pulse->edge_timings[0] =
            (uint32_t)(NFCV_RESP_SUBC1_PULSE_32 * DIGITAL_SIGNAL_UNIT_S);
        nfcv_data->emu_air.nfcv_resp_pulse->edge_timings[1] =
            (uint32_t)(NFCV_RESP_SUBC1_PULSE_32 * DIGITAL_SIGNAL_UNIT_S);
        nfcv_data->emu_air.nfcv_resp_pulse->edge_cnt = 2;
    }

    if(!nfcv_data->emu_air.nfcv_resp_half_pulse) {
        /* modulated fc/32 or fc/8 pulse as building block */
        nfcv_data->emu_air.nfcv_resp_half_pulse = digital_signal_alloc(4);
        if(!nfcv_data->emu_air.nfcv_resp_half_pulse) {
            return false;
        }
        nfcv_data->emu_air.nfcv_resp_half_pulse->start_level = GPIO_LEVEL_MODULATED;
        nfcv_data->emu_air.nfcv_resp_half_pulse->edge_timings[0] =
            (uint32_t)(NFCV_RESP_SUBC1_PULSE_32 * DIGITAL_SIGNAL_UNIT_S);
        nfcv_data->emu_air.nfcv_resp_half_pulse->edge_cnt = 1;
    }

    bool success = true;
    success &= nfcv_emu_alloc_signals(&nfcv_data->emu_air, &nfcv_data->emu_air.signals_high, 1);
    success &= nfcv_emu_alloc_signals(&nfcv_data->emu_air, &nfcv_data->emu_air.signals_low, 4);

    if(!success) {
        FURI_LOG_E(TAG, "Failed to allocate signals");
        return false;
    }

    digital_sequence_set_signal(
        nfcv_data->emu_air.nfcv_signal,
        NFCV_SIG_SOF,
        nfcv_data->emu_air.signals_high.nfcv_resp_sof);
    digital_sequence_set_signal(
        nfcv_data->emu_air.nfcv_signal,
        NFCV_SIG_BIT0,
        nfcv_data->emu_air.signals_high.nfcv_resp_zero);
    digital_sequence_set_signal(
        nfcv_data->emu_air.nfcv_signal,
        NFCV_SIG_BIT1,
        nfcv_data->emu_air.signals_high.nfcv_resp_one);
    digital_sequence_set_signal(
        nfcv_data->emu_air.nfcv_signal,
        NFCV_SIG_EOF,
        nfcv_data->emu_air.signals_high.nfcv_resp_eof);
    digital_sequence_set_signal(
        nfcv_data->emu_air.nfcv_signal,
        NFCV_SIG_LOW_SOF,
        nfcv_data->emu_air.signals_low.nfcv_resp_sof);
    digital_sequence_set_signal(
        nfcv_data->emu_air.nfcv_signal,
        NFCV_SIG_LOW_BIT0,
        nfcv_data->emu_air.signals_low.nfcv_resp_zero);
    digital_sequence_set_signal(
        nfcv_data->emu_air.nfcv_signal,
        NFCV_SIG_LOW_BIT1,
        nfcv_data->emu_air.signals_low.nfcv_resp_one);
    digital_sequence_set_signal(
        nfcv_data->emu_air.nfcv_signal,
        NFCV_SIG_LOW_EOF,
        nfcv_data->emu_air.signals_low.nfcv_resp_eof);

    return true;
}

void nfcv_emu_free(NfcVData* nfcv_data) {
    furi_assert(nfcv_data);

    if(nfcv_data->frame) {
        free(nfcv_data->frame);
    }
    if(nfcv_data->emu_protocol_ctx) {
        free(nfcv_data->emu_protocol_ctx);
    }
    if(nfcv_data->emu_air.nfcv_resp_unmod) {
        digital_signal_free(nfcv_data->emu_air.nfcv_resp_unmod);
    }
    if(nfcv_data->emu_air.nfcv_resp_pulse) {
        digital_signal_free(nfcv_data->emu_air.nfcv_resp_pulse);
    }
    if(nfcv_data->emu_air.nfcv_resp_half_pulse) {
        digital_signal_free(nfcv_data->emu_air.nfcv_resp_half_pulse);
    }
    if(nfcv_data->emu_air.nfcv_signal) {
        digital_sequence_free(nfcv_data->emu_air.nfcv_signal);
    }
    if(nfcv_data->emu_air.reader_signal) {
        // Stop pulse reader and disable bus before free
        pulse_reader_stop(nfcv_data->emu_air.reader_signal);
        // Free pulse reader
        pulse_reader_free(nfcv_data->emu_air.reader_signal);
    }

    nfcv_data->frame = NULL;
    nfcv_data->emu_air.nfcv_resp_unmod = NULL;
    nfcv_data->emu_air.nfcv_resp_pulse = NULL;
    nfcv_data->emu_air.nfcv_resp_half_pulse = NULL;
    nfcv_data->emu_air.nfcv_signal = NULL;
    nfcv_data->emu_air.reader_signal = NULL;

    nfcv_emu_free_signals(&nfcv_data->emu_air.signals_high);
    nfcv_emu_free_signals(&nfcv_data->emu_air.signals_low);
}

void nfcv_emu_send(
    FuriHalNfcTxRxContext* tx_rx,
    NfcVData* nfcv,
    uint8_t* data,
    uint8_t length,
    NfcVSendFlags flags,
    uint32_t send_time) {
    furi_assert(tx_rx);
    furi_assert(nfcv);

    /* picked default value (0) to match the most common format */
    if(flags == NfcVSendFlagsNormal) {
        flags = NfcVSendFlagsSof | NfcVSendFlagsCrc | NfcVSendFlagsEof |
                NfcVSendFlagsOneSubcarrier | NfcVSendFlagsHighRate;
    }

    if(flags & NfcVSendFlagsCrc) {
        nfcv_crc(data, length);
        length += 2;
    }

    /* depending on the request flags, send with high or low rate */
    uint32_t bit0 = (flags & NfcVSendFlagsHighRate) ? NFCV_SIG_BIT0 : NFCV_SIG_LOW_BIT0;
    uint32_t bit1 = (flags & NfcVSendFlagsHighRate) ? NFCV_SIG_BIT1 : NFCV_SIG_LOW_BIT1;
    uint32_t sof = (flags & NfcVSendFlagsHighRate) ? NFCV_SIG_SOF : NFCV_SIG_LOW_SOF;
    uint32_t eof = (flags & NfcVSendFlagsHighRate) ? NFCV_SIG_EOF : NFCV_SIG_LOW_EOF;

    digital_sequence_clear(nfcv->emu_air.nfcv_signal);

    if(flags & NfcVSendFlagsSof) {
        digital_sequence_add(nfcv->emu_air.nfcv_signal, sof);
    }

    for(int bit_total = 0; bit_total < length * 8; bit_total++) {
        uint32_t byte_pos = bit_total / 8;
        uint32_t bit_pos = bit_total % 8;
        uint8_t bit_val = 0x01 << bit_pos;

        digital_sequence_add(nfcv->emu_air.nfcv_signal, (data[byte_pos] & bit_val) ? bit1 : bit0);
    }

    if(flags & NfcVSendFlagsEof) {
        digital_sequence_add(nfcv->emu_air.nfcv_signal, eof);
    }

    furi_hal_gpio_write(&gpio_spi_r_mosi, GPIO_LEVEL_UNMODULATED);
    digital_sequence_set_sendtime(nfcv->emu_air.nfcv_signal, send_time);
    digital_sequence_send(nfcv->emu_air.nfcv_signal);
    furi_hal_gpio_write(&gpio_spi_r_mosi, GPIO_LEVEL_UNMODULATED);

    if(tx_rx->sniff_tx) {
        tx_rx->sniff_tx(data, length * 8, false, tx_rx->sniff_context);
    }
}

static void nfcv_revuidcpy(uint8_t* dst, uint8_t* src) {
    for(int pos = 0; pos < NFCV_UID_LENGTH; pos++) {
        dst[pos] = src[NFCV_UID_LENGTH - 1 - pos];
    }
}

static int nfcv_revuidcmp(uint8_t* dst, uint8_t* src) {
    for(int pos = 0; pos < NFCV_UID_LENGTH; pos++) {
        if(dst[pos] != src[NFCV_UID_LENGTH - 1 - pos]) {
            return 1;
        }
    }
    return 0;
}

void nfcv_emu_handle_packet(
    FuriHalNfcTxRxContext* tx_rx,
    FuriHalNfcDevData* nfc_data,
    void* nfcv_data_in) {
    furi_assert(tx_rx);
    furi_assert(nfc_data);
    furi_assert(nfcv_data_in);

    NfcVData* nfcv_data = (NfcVData*)nfcv_data_in;
    NfcVEmuProtocolCtx* ctx = nfcv_data->emu_protocol_ctx;

    if(nfcv_data->frame_length < 2) {
        return;
    }

    if(nfcv_data->echo_mode) {
        nfcv_emu_send(
            tx_rx,
            nfcv_data,
            nfcv_data->frame,
            nfcv_data->frame_length,
            NfcVSendFlagsSof | NfcVSendFlagsHighRate | NfcVSendFlagsEof,
            ctx->send_time);
        snprintf(nfcv_data->last_command, sizeof(nfcv_data->last_command), "ECHO data");
        return;
    }

    /* parse the frame data for the upcoming part 3 handling */
    ctx->flags = nfcv_data->frame[0];
    ctx->command = nfcv_data->frame[1];
    ctx->selected = !(ctx->flags & NFCV_REQ_FLAG_INVENTORY) && (ctx->flags & NFCV_REQ_FLAG_SELECT);
    ctx->addressed = !(ctx->flags & NFCV_REQ_FLAG_INVENTORY) &&
                     (ctx->flags & NFCV_REQ_FLAG_ADDRESS);
    ctx->advanced = (ctx->command >= NFCV_CMD_ADVANCED);
    ctx->address_offset = 2 + (ctx->advanced ? 1 : 0);
    ctx->payload_offset = ctx->address_offset + (ctx->addressed ? NFCV_UID_LENGTH : 0);
    ctx->response_flags = NfcVSendFlagsSof | NfcVSendFlagsCrc | NfcVSendFlagsEof;
    ctx->send_time = nfcv_data->eof_timestamp + NFCV_FDT_FC(4380);

    if(ctx->flags & NFCV_REQ_FLAG_DATA_RATE) {
        ctx->response_flags |= NfcVSendFlagsHighRate;
    }
    if(ctx->flags & NFCV_REQ_FLAG_SUB_CARRIER) {
        ctx->response_flags |= NfcVSendFlagsTwoSubcarrier;
    }

    if(ctx->payload_offset + 2 > nfcv_data->frame_length) {
#ifdef NFCV_VERBOSE
        FURI_LOG_D(TAG, "command 0x%02X, but packet is too short", ctx->command);
#endif
        return;
    }

    /* standard behavior is implemented */
    if(ctx->addressed) {
        uint8_t* address = &nfcv_data->frame[ctx->address_offset];
        if(nfcv_revuidcmp(address, nfc_data->uid)) {
#ifdef NFCV_VERBOSE
            FURI_LOG_D(TAG, "addressed command 0x%02X, but not for us:", ctx->command);
            FURI_LOG_D(
                TAG,
                "  dest:     %02X%02X%02X%02X%02X%02X%02X%02X",
                address[7],
                address[6],
                address[5],
                address[4],
                address[3],
                address[2],
                address[1],
                address[0]);
            FURI_LOG_D(
                TAG,
                "  our UID:  %02X%02X%02X%02X%02X%02X%02X%02X",
                nfc_data->uid[0],
                nfc_data->uid[1],
                nfc_data->uid[2],
                nfc_data->uid[3],
                nfc_data->uid[4],
                nfc_data->uid[5],
                nfc_data->uid[6],
                nfc_data->uid[7]);
#endif
            return;
        }
    }

    if(ctx->selected && !nfcv_data->selected) {
#ifdef NFCV_VERBOSE
        FURI_LOG_D(
            TAG,
            "selected card shall execute command 0x%02X, but we were not selected",
            ctx->command);
#endif
        return;
    }

    /* then give control to the card subtype specific protocol filter */
    if(ctx->emu_protocol_filter != NULL) {
        if(ctx->emu_protocol_filter(tx_rx, nfc_data, nfcv_data)) {
            if(strlen(nfcv_data->last_command) > 0) {
#ifdef NFCV_VERBOSE
                FURI_LOG_D(
                    TAG, "Received command %s (handled by filter)", nfcv_data->last_command);
#endif
            }
            return;
        }
    }

    switch(ctx->command) {
    case NFCV_CMD_INVENTORY: {
        bool respond = false;

        if(ctx->flags & NFCV_REQ_FLAG_AFI) {
            uint8_t afi = nfcv_data->frame[ctx->payload_offset];

            uint8_t family = (afi & 0xF0);
            uint8_t subfamily = (afi & 0x0F);

            if(family) {
                if(subfamily) {
                    /* selected family and subfamily only */
                    if(afi == nfcv_data->afi) {
                        respond = true;
                    }
                } else {
                    /* selected family, any subfamily */
                    if(family == (nfcv_data->afi & 0xf0)) {
                        respond = true;
                    }
                }
            } else {
                if(subfamily) {
                    /* proprietary subfamily only */
                    if(afi == nfcv_data->afi) {
                        respond = true;
                    }
                } else {
                    /* all families and subfamilies */
                    respond = true;
                }
            }

        } else {
            respond = true;
        }

        if(!nfcv_data->quiet && respond) {
            int buffer_pos = 0;
            ctx->response_buffer[buffer_pos++] = NFCV_NOERROR;
            ctx->response_buffer[buffer_pos++] = nfcv_data->dsfid;
            nfcv_revuidcpy(&ctx->response_buffer[buffer_pos], nfc_data->uid);
            buffer_pos += NFCV_UID_LENGTH;

            nfcv_emu_send(
                tx_rx,
                nfcv_data,
                ctx->response_buffer,
                buffer_pos,
                ctx->response_flags,
                ctx->send_time);
            snprintf(nfcv_data->last_command, sizeof(nfcv_data->last_command), "INVENTORY");
        } else {
            snprintf(
                nfcv_data->last_command, sizeof(nfcv_data->last_command), "INVENTORY (quiet)");
        }
        break;
    }

    case NFCV_CMD_STAY_QUIET: {
        snprintf(nfcv_data->last_command, sizeof(nfcv_data->last_command), "STAYQUIET");
        nfcv_data->quiet = true;
        break;
    }

    case NFCV_CMD_LOCK_BLOCK: {
        uint8_t block = nfcv_data->frame[ctx->payload_offset];
        nfcv_data->security_status[block] |= 0x01;
        nfcv_data->modified = true;

        ctx->response_buffer[0] = NFCV_NOERROR;
        nfcv_emu_send(
            tx_rx, nfcv_data, ctx->response_buffer, 1, ctx->response_flags, ctx->send_time);

        snprintf(nfcv_data->last_command, sizeof(nfcv_data->last_command), "LOCK BLOCK %d", block);
        break;
    }

    case NFCV_CMD_WRITE_DSFID: {
        uint8_t id = nfcv_data->frame[ctx->payload_offset];

        if(!(nfcv_data->security_status[0] & NfcVLockBitDsfid)) {
            nfcv_data->dsfid = id;
            nfcv_data->modified = true;
            ctx->response_buffer[0] = NFCV_NOERROR;
            nfcv_emu_send(
                tx_rx, nfcv_data, ctx->response_buffer, 1, ctx->response_flags, ctx->send_time);
        }

        snprintf(nfcv_data->last_command, sizeof(nfcv_data->last_command), "WRITE DSFID %02X", id);
        break;
    }

    case NFCV_CMD_WRITE_AFI: {
        uint8_t id = nfcv_data->frame[ctx->payload_offset];

        if(!(nfcv_data->security_status[0] & NfcVLockBitAfi)) {
            nfcv_data->afi = id;
            nfcv_data->modified = true;
            ctx->response_buffer[0] = NFCV_NOERROR;
            nfcv_emu_send(
                tx_rx, nfcv_data, ctx->response_buffer, 1, ctx->response_flags, ctx->send_time);
        }

        snprintf(nfcv_data->last_command, sizeof(nfcv_data->last_command), "WRITE AFI %02X", id);
        break;
    }

    case NFCV_CMD_LOCK_DSFID: {
        if(!(nfcv_data->security_status[0] & NfcVLockBitDsfid)) {
            nfcv_data->security_status[0] |= NfcVLockBitDsfid;
            nfcv_data->modified = true;

            ctx->response_buffer[0] = NFCV_NOERROR;
            nfcv_emu_send(
                tx_rx, nfcv_data, ctx->response_buffer, 1, ctx->response_flags, ctx->send_time);
        }

        snprintf(nfcv_data->last_command, sizeof(nfcv_data->last_command), "LOCK DSFID");
        break;
    }

    case NFCV_CMD_LOCK_AFI: {
        if(!(nfcv_data->security_status[0] & NfcVLockBitAfi)) {
            nfcv_data->security_status[0] |= NfcVLockBitAfi;
            nfcv_data->modified = true;

            ctx->response_buffer[0] = NFCV_NOERROR;
            nfcv_emu_send(
                tx_rx, nfcv_data, ctx->response_buffer, 1, ctx->response_flags, ctx->send_time);
        }

        snprintf(nfcv_data->last_command, sizeof(nfcv_data->last_command), "LOCK AFI");
        break;
    }

    case NFCV_CMD_SELECT: {
        ctx->response_buffer[0] = NFCV_NOERROR;
        nfcv_data->selected = true;
        nfcv_data->quiet = false;
        nfcv_emu_send(
            tx_rx, nfcv_data, ctx->response_buffer, 1, ctx->response_flags, ctx->send_time);
        snprintf(nfcv_data->last_command, sizeof(nfcv_data->last_command), "SELECT");
        break;
    }

    case NFCV_CMD_RESET_TO_READY: {
        ctx->response_buffer[0] = NFCV_NOERROR;
        nfcv_data->quiet = false;
        nfcv_emu_send(
            tx_rx, nfcv_data, ctx->response_buffer, 1, ctx->response_flags, ctx->send_time);
        snprintf(nfcv_data->last_command, sizeof(nfcv_data->last_command), "RESET_TO_READY");
        break;
    }

    case NFCV_CMD_READ_MULTI_BLOCK:
    case NFCV_CMD_READ_BLOCK: {
        uint8_t block = nfcv_data->frame[ctx->payload_offset];
        int blocks = 1;

        if(ctx->command == NFCV_CMD_READ_MULTI_BLOCK) {
            blocks = nfcv_data->frame[ctx->payload_offset + 1] + 1;
        }

        /* limit the maximum block count, underflow accepted */
        if(block + blocks > nfcv_data->block_num) {
            blocks = nfcv_data->block_num - block;
        }

        /* only respond with the valid blocks, if there are any */
        if(blocks > 0) {
            uint8_t buffer_pos = 0;

            ctx->response_buffer[buffer_pos++] = NFCV_NOERROR;

            for(int block_index = 0; block_index < blocks; block_index++) {
                int block_current = block + block_index;
                /* prepend security status */
                if(ctx->flags & NFCV_REQ_FLAG_OPTION) {
                    ctx->response_buffer[buffer_pos++] =
                        nfcv_data->security_status[1 + block_current];
                }
                /* then the data block */
                memcpy(
                    &ctx->response_buffer[buffer_pos],
                    &nfcv_data->data[nfcv_data->block_size * block_current],
                    nfcv_data->block_size);
                buffer_pos += nfcv_data->block_size;
            }
            nfcv_emu_send(
                tx_rx,
                nfcv_data,
                ctx->response_buffer,
                buffer_pos,
                ctx->response_flags,
                ctx->send_time);
        } else {
            /* reply with an error only in addressed or selected mode */
            if(ctx->addressed || ctx->selected) {
                ctx->response_buffer[0] = NFCV_RES_FLAG_ERROR;
                ctx->response_buffer[1] = NFCV_ERROR_GENERIC;
                nfcv_emu_send(
                    tx_rx, nfcv_data, ctx->response_buffer, 2, ctx->response_flags, ctx->send_time);
            }
        }
        snprintf(nfcv_data->last_command, sizeof(nfcv_data->last_command), "READ BLOCK %d", block);

        break;
    }

    case NFCV_CMD_WRITE_MULTI_BLOCK:
    case NFCV_CMD_WRITE_BLOCK: {
        uint8_t blocks = 1;
        uint8_t block = nfcv_data->frame[ctx->payload_offset];
        uint8_t data_pos = ctx->payload_offset + 1;

        if(ctx->command == NFCV_CMD_WRITE_MULTI_BLOCK) {
            blocks = nfcv_data->frame[data_pos] + 1;
            data_pos++;
        }

        uint8_t* data = &nfcv_data->frame[data_pos];
        uint32_t data_len = nfcv_data->block_size * blocks;

        if((block + blocks) <= nfcv_data->block_num &&
           (data_pos + data_len + 2) == nfcv_data->frame_length) {
            ctx->response_buffer[0] = NFCV_NOERROR;
            memcpy(
                &nfcv_data->data[nfcv_data->block_size * block],
                &nfcv_data->frame[data_pos],
                data_len);
            nfcv_data->modified = true;

            nfcv_emu_send(
                tx_rx, nfcv_data, ctx->response_buffer, 1, ctx->response_flags, ctx->send_time);
        } else {
            ctx->response_buffer[0] = NFCV_RES_FLAG_ERROR;
            ctx->response_buffer[1] = NFCV_ERROR_GENERIC;
            nfcv_emu_send(
                tx_rx, nfcv_data, ctx->response_buffer, 2, ctx->response_flags, ctx->send_time);
        }

        if(ctx->command == NFCV_CMD_WRITE_MULTI_BLOCK) {
            snprintf(
                nfcv_data->last_command,
                sizeof(nfcv_data->last_command),
                "WRITE MULTI BLOCK %d, %d blocks",
                block,
                blocks);
        } else {
            snprintf(
                nfcv_data->last_command,
                sizeof(nfcv_data->last_command),
                "WRITE BLOCK %d <- %02X %02X %02X %02X",
                block,
                data[0],
                data[1],
                data[2],
                data[3]);
        }
        break;
    }

    case NFCV_CMD_GET_SYSTEM_INFO: {
        int buffer_pos = 0;
        ctx->response_buffer[buffer_pos++] = NFCV_NOERROR;
        ctx->response_buffer[buffer_pos++] = NFCV_SYSINFO_FLAG_DSFID | NFCV_SYSINFO_FLAG_AFI |
                                             NFCV_SYSINFO_FLAG_MEMSIZE | NFCV_SYSINFO_FLAG_ICREF;
        nfcv_revuidcpy(&ctx->response_buffer[buffer_pos], nfc_data->uid);
        buffer_pos += NFCV_UID_LENGTH;
        ctx->response_buffer[buffer_pos++] = nfcv_data->dsfid; /* DSFID */
        ctx->response_buffer[buffer_pos++] = nfcv_data->afi; /* AFI */
        ctx->response_buffer[buffer_pos++] = nfcv_data->block_num - 1; /* number of blocks */
        ctx->response_buffer[buffer_pos++] = nfcv_data->block_size - 1; /* block size */
        ctx->response_buffer[buffer_pos++] = nfcv_data->ic_ref; /* IC reference */

        nfcv_emu_send(
            tx_rx,
            nfcv_data,
            ctx->response_buffer,
            buffer_pos,
            ctx->response_flags,
            ctx->send_time);
        snprintf(nfcv_data->last_command, sizeof(nfcv_data->last_command), "SYSTEMINFO");
        break;
    }

    case NFCV_CMD_CUST_ECHO_MODE: {
        ctx->response_buffer[0] = NFCV_NOERROR;
        nfcv_data->echo_mode = true;
        nfcv_emu_send(
            tx_rx, nfcv_data, ctx->response_buffer, 1, ctx->response_flags, ctx->send_time);
        snprintf(nfcv_data->last_command, sizeof(nfcv_data->last_command), "ECHO mode");
        break;
    }

    case NFCV_CMD_CUST_ECHO_DATA: {
        nfcv_emu_send(
            tx_rx,
            nfcv_data,
            &nfcv_data->frame[ctx->payload_offset],
            nfcv_data->frame_length - ctx->payload_offset - 2,
            NfcVSendFlagsSof | NfcVSendFlagsHighRate | NfcVSendFlagsEof,
            ctx->send_time);
        snprintf(nfcv_data->last_command, sizeof(nfcv_data->last_command), "ECHO data");
        break;
    }

    default:
        snprintf(
            nfcv_data->last_command,
            sizeof(nfcv_data->last_command),
            "unsupported: %02X",
            ctx->command);
        break;
    }

    if(strlen(nfcv_data->last_command) > 0) {
#ifdef NFCV_VERBOSE
        FURI_LOG_D(TAG, "Received command %s", nfcv_data->last_command);
#endif
    }
}

void nfcv_emu_sniff_packet(
    FuriHalNfcTxRxContext* tx_rx,
    FuriHalNfcDevData* nfc_data,
    void* nfcv_data_in) {
    furi_assert(tx_rx);
    furi_assert(nfc_data);
    furi_assert(nfcv_data_in);

    NfcVData* nfcv_data = (NfcVData*)nfcv_data_in;
    NfcVEmuProtocolCtx* ctx = nfcv_data->emu_protocol_ctx;

    if(nfcv_data->frame_length < 2) {
        return;
    }

    /* parse the frame data for the upcoming part 3 handling */
    ctx->flags = nfcv_data->frame[0];
    ctx->command = nfcv_data->frame[1];
    ctx->selected = (ctx->flags & NFCV_REQ_FLAG_SELECT);
    ctx->addressed = !(ctx->flags & NFCV_REQ_FLAG_INVENTORY) &&
                     (ctx->flags & NFCV_REQ_FLAG_ADDRESS);
    ctx->advanced = (ctx->command >= NFCV_CMD_ADVANCED);
    ctx->address_offset = 2 + (ctx->advanced ? 1 : 0);
    ctx->payload_offset = ctx->address_offset + (ctx->addressed ? NFCV_UID_LENGTH : 0);

    char flags_string[5];

    snprintf(
        flags_string,
        5,
        "%c%c%c%d",
        (ctx->flags & NFCV_REQ_FLAG_INVENTORY) ?
            'I' :
            (ctx->addressed ? 'A' : (ctx->selected ? 'S' : '*')),
        ctx->advanced ? 'X' : ' ',
        (ctx->flags & NFCV_REQ_FLAG_DATA_RATE) ? 'h' : 'l',
        (ctx->flags & NFCV_REQ_FLAG_SUB_CARRIER) ? 2 : 1);

    switch(ctx->command) {
    case NFCV_CMD_INVENTORY: {
        snprintf(
            nfcv_data->last_command, sizeof(nfcv_data->last_command), "%s INVENTORY", flags_string);
        break;
    }

    case NFCV_CMD_STAY_QUIET: {
        snprintf(
            nfcv_data->last_command, sizeof(nfcv_data->last_command), "%s STAYQUIET", flags_string);
        nfcv_data->quiet = true;
        break;
    }

    case NFCV_CMD_LOCK_BLOCK: {
        uint8_t block = nfcv_data->frame[ctx->payload_offset];
        snprintf(
            nfcv_data->last_command,
            sizeof(nfcv_data->last_command),
            "%s LOCK %d",
            flags_string,
            block);
        break;
    }

    case NFCV_CMD_WRITE_DSFID: {
        uint8_t id = nfcv_data->frame[ctx->payload_offset];
        snprintf(
            nfcv_data->last_command,
            sizeof(nfcv_data->last_command),
            "%s WR DSFID %d",
            flags_string,
            id);
        break;
    }

    case NFCV_CMD_WRITE_AFI: {
        uint8_t id = nfcv_data->frame[ctx->payload_offset];
        snprintf(
            nfcv_data->last_command,
            sizeof(nfcv_data->last_command),
            "%s WR AFI %d",
            flags_string,
            id);
        break;
    }

    case NFCV_CMD_LOCK_DSFID: {
        snprintf(
            nfcv_data->last_command,
            sizeof(nfcv_data->last_command),
            "%s LOCK DSFID",
            flags_string);
        break;
    }

    case NFCV_CMD_LOCK_AFI: {
        snprintf(
            nfcv_data->last_command, sizeof(nfcv_data->last_command), "%s LOCK AFI", flags_string);
        break;
    }

    case NFCV_CMD_SELECT: {
        snprintf(
            nfcv_data->last_command, sizeof(nfcv_data->last_command), "%s SELECT", flags_string);
        break;
    }

    case NFCV_CMD_RESET_TO_READY: {
        snprintf(
            nfcv_data->last_command, sizeof(nfcv_data->last_command), "%s RESET", flags_string);
        break;
    }

    case NFCV_CMD_READ_MULTI_BLOCK:
    case NFCV_CMD_READ_BLOCK: {
        uint8_t block = nfcv_data->frame[ctx->payload_offset];
        uint8_t blocks = 1;

        if(ctx->command == NFCV_CMD_READ_MULTI_BLOCK) {
            blocks = nfcv_data->frame[ctx->payload_offset + 1] + 1;
        }

        snprintf(
            nfcv_data->last_command,
            sizeof(nfcv_data->last_command),
            "%s READ %d cnt: %d",
            flags_string,
            block,
            blocks);

        break;
    }

    case NFCV_CMD_WRITE_MULTI_BLOCK:
    case NFCV_CMD_WRITE_BLOCK: {
        uint8_t block = nfcv_data->frame[ctx->payload_offset];
        uint8_t blocks = 1;
        uint8_t data_pos = 1;

        if(ctx->command == NFCV_CMD_WRITE_MULTI_BLOCK) {
            blocks = nfcv_data->frame[ctx->payload_offset + 1] + 1;
            data_pos++;
        }

        uint8_t* data = &nfcv_data->frame[ctx->payload_offset + data_pos];

        if(ctx->command == NFCV_CMD_WRITE_MULTI_BLOCK) {
            snprintf(
                nfcv_data->last_command,
                sizeof(nfcv_data->last_command),
                "%s WRITE %d, cnd %d",
                flags_string,
                block,
                blocks);
        } else {
            snprintf(
                nfcv_data->last_command,
                sizeof(nfcv_data->last_command),
                "%s WRITE %d %02X %02X %02X %02X",
                flags_string,
                block,
                data[0],
                data[1],
                data[2],
                data[3]);
        }
        break;
    }

    case NFCV_CMD_GET_SYSTEM_INFO: {
        snprintf(
            nfcv_data->last_command,
            sizeof(nfcv_data->last_command),
            "%s SYSTEMINFO",
            flags_string);
        break;
    }

    default:
        snprintf(
            nfcv_data->last_command,
            sizeof(nfcv_data->last_command),
            "%s unsupported: %02X",
            flags_string,
            ctx->command);
        break;
    }

    if(strlen(nfcv_data->last_command) > 0) {
        FURI_LOG_D(TAG, "Received command %s", nfcv_data->last_command);
    }
}

void nfcv_emu_init(FuriHalNfcDevData* nfc_data, NfcVData* nfcv_data) {
    furi_assert(nfc_data);
    furi_assert(nfcv_data);

    if(!nfcv_emu_alloc(nfcv_data)) {
        FURI_LOG_E(TAG, "Failed to allocate structures");
        nfcv_data->ready = false;
        return;
    }

    strcpy(nfcv_data->last_command, "");
    nfcv_data->quiet = false;
    nfcv_data->selected = false;
    nfcv_data->modified = false;

    /* everything is initialized */
    nfcv_data->ready = true;

    /* ensure the GPIO is already in unmodulated state */
    furi_hal_gpio_init(&gpio_spi_r_mosi, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_write(&gpio_spi_r_mosi, GPIO_LEVEL_UNMODULATED);

    rfal_platform_spi_acquire();
    /* stop operation to configure for transparent and passive mode */
    st25r3916ExecuteCommand(ST25R3916_CMD_STOP);
    /* set enable, rx_enable and field detector enable */
    st25r3916WriteRegister(
        ST25R3916_REG_OP_CONTROL,
        ST25R3916_REG_OP_CONTROL_en | ST25R3916_REG_OP_CONTROL_rx_en |
            ST25R3916_REG_OP_CONTROL_en_fd_auto_efd);
    /* explicitely set the modulation resistor in case system config changes for some reason */
    st25r3916WriteRegister(
        ST25R3916_REG_PT_MOD,
        (0 << ST25R3916_REG_PT_MOD_ptm_res_shift) | (15 << ST25R3916_REG_PT_MOD_pt_res_shift));
    /* target mode: target, other fields do not have any effect as we use transparent mode */
    st25r3916WriteRegister(ST25R3916_REG_MODE, ST25R3916_REG_MODE_targ);
    /* let us modulate the field using MOSI, read ASK modulation using IRQ */
    st25r3916ExecuteCommand(ST25R3916_CMD_TRANSPARENT_MODE);

    furi_hal_spi_bus_handle_deinit(&furi_hal_spi_bus_handle_nfc);

    /* if not set already, initialize the default protocol handler */
    if(!nfcv_data->emu_protocol_ctx) {
        nfcv_data->emu_protocol_ctx = malloc(sizeof(NfcVEmuProtocolCtx));
        if(nfcv_data->sub_type == NfcVTypeSniff) {
            nfcv_data->emu_protocol_handler = &nfcv_emu_sniff_packet;
        } else {
            nfcv_data->emu_protocol_handler = &nfcv_emu_handle_packet;
        }
    }

    FURI_LOG_D(TAG, "Starting NfcV emulation");
    FURI_LOG_D(
        TAG,
        "  UID:          %02X %02X %02X %02X %02X %02X %02X %02X",
        nfc_data->uid[0],
        nfc_data->uid[1],
        nfc_data->uid[2],
        nfc_data->uid[3],
        nfc_data->uid[4],
        nfc_data->uid[5],
        nfc_data->uid[6],
        nfc_data->uid[7]);

    switch(nfcv_data->sub_type) {
    case NfcVTypeSlixL:
        FURI_LOG_D(TAG, "  Card type:    SLIX-L");
        slix_l_prepare(nfcv_data);
        break;
    case NfcVTypeSlixS:
        FURI_LOG_D(TAG, "  Card type:    SLIX-S");
        slix_s_prepare(nfcv_data);
        break;
    case NfcVTypeSlix2:
        FURI_LOG_D(TAG, "  Card type:    SLIX2");
        slix2_prepare(nfcv_data);
        break;
    case NfcVTypeSlix:
        FURI_LOG_D(TAG, "  Card type:    SLIX");
        slix_prepare(nfcv_data);
        break;
    case NfcVTypePlain:
        FURI_LOG_D(TAG, "  Card type:    Plain");
        break;
    case NfcVTypeSniff:
        FURI_LOG_D(TAG, "  Card type:    Sniffing");
        break;
    }

    /* allocate a 512 edge buffer, more than enough */
    nfcv_data->emu_air.reader_signal =
        pulse_reader_alloc(&gpio_nfc_irq_rfid_pull, NFCV_PULSE_BUFFER);
    /* timebase shall be 1 ns */
    pulse_reader_set_timebase(nfcv_data->emu_air.reader_signal, PulseReaderUnitNanosecond);
    /* and configure to already calculate the number of bits */
    pulse_reader_set_bittime(nfcv_data->emu_air.reader_signal, NFCV_PULSE_DURATION_NS);
    /* this IO is fed into the µC via a diode, so we need a pulldown */
    pulse_reader_set_pull(nfcv_data->emu_air.reader_signal, GpioPullDown);

    /* start sampling */
    pulse_reader_start(nfcv_data->emu_air.reader_signal);
}

void nfcv_emu_deinit(NfcVData* nfcv_data) {
    furi_assert(nfcv_data);

    furi_hal_spi_bus_handle_init(&furi_hal_spi_bus_handle_nfc);
    nfcv_emu_free(nfcv_data);

    if(nfcv_data->emu_protocol_ctx) {
        free(nfcv_data->emu_protocol_ctx);
        nfcv_data->emu_protocol_ctx = NULL;
    }

    /* set registers back to how we found them */
    st25r3916WriteRegister(ST25R3916_REG_OP_CONTROL, 0x00);
    st25r3916WriteRegister(ST25R3916_REG_MODE, 0x08);
    rfal_platform_spi_release();
}

bool nfcv_emu_loop(
    FuriHalNfcTxRxContext* tx_rx,
    FuriHalNfcDevData* nfc_data,
    NfcVData* nfcv_data,
    uint32_t timeout_ms) {
    furi_assert(tx_rx);
    furi_assert(nfc_data);
    furi_assert(nfcv_data);

    bool ret = false;
    uint32_t frame_state = NFCV_FRAME_STATE_SOF1;
    uint32_t periods_previous = 0;
    uint32_t frame_pos = 0;
    uint32_t byte_value = 0;
    uint32_t bits_received = 0;
    uint32_t timeout = timeout_ms * 1000;
    bool wait_for_pulse = false;

    if(!nfcv_data->ready) {
        return false;
    }

#ifdef NFCV_DIAGNOSTIC_DUMPS
    uint8_t period_buffer[NFCV_DIAGNOSTIC_DUMP_SIZE];
    uint32_t period_buffer_pos = 0;
#endif

    while(true) {
        uint32_t periods = pulse_reader_receive(nfcv_data->emu_air.reader_signal, timeout);
        uint32_t timestamp = DWT->CYCCNT;

        /* when timed out, reset to SOF state */
        if(periods == PULSE_READER_NO_EDGE || periods == PULSE_READER_LOST_EDGE) {
            break;
        }

#ifdef NFCV_DIAGNOSTIC_DUMPS
        if(period_buffer_pos < sizeof(period_buffer)) {
            period_buffer[period_buffer_pos++] = periods;
        }
#endif

        /* short helper for detecting a pulse position */
        if(wait_for_pulse) {
            wait_for_pulse = false;
            if(periods != 1) {
                frame_state = NFCV_FRAME_STATE_RESET;
            }
            continue;
        }

        switch(frame_state) {
        case NFCV_FRAME_STATE_SOF1:
            if(periods == 1) {
                frame_state = NFCV_FRAME_STATE_SOF2;
            } else {
                frame_state = NFCV_FRAME_STATE_SOF1;
                break;
            }
            break;

        case NFCV_FRAME_STATE_SOF2:
            /* waiting for the second low period, telling us about coding */
            if(periods == 6) {
                frame_state = NFCV_FRAME_STATE_CODING_256;
                periods_previous = 0;
                wait_for_pulse = true;
            } else if(periods == 4) {
                frame_state = NFCV_FRAME_STATE_CODING_4;
                periods_previous = 2;
                wait_for_pulse = true;
            } else {
                frame_state = NFCV_FRAME_STATE_RESET;
            }
            break;

        case NFCV_FRAME_STATE_CODING_256:
            if(periods_previous > periods) {
                frame_state = NFCV_FRAME_STATE_RESET;
                break;
            }

            /* previous symbol left us with some pulse periods */
            periods -= periods_previous;

            if(periods > 512) {
                frame_state = NFCV_FRAME_STATE_RESET;
                break;
            } else if(periods == 2) {
                frame_state = NFCV_FRAME_STATE_EOF;
                break;
            }

            periods_previous = 512 - (periods + 1);
            byte_value = (periods - 1) / 2;
            if(frame_pos < NFCV_FRAMESIZE_MAX) {
                nfcv_data->frame[frame_pos++] = (uint8_t)byte_value;
            }

            wait_for_pulse = true;

            break;

        case NFCV_FRAME_STATE_CODING_4:
            if(periods_previous > periods) {
                frame_state = NFCV_FRAME_STATE_RESET;
                break;
            }

            /* previous symbol left us with some pulse periods */
            periods -= periods_previous;
            periods_previous = 0;

            byte_value >>= 2;
            bits_received += 2;

            if(periods == 1) {
                byte_value |= 0x00 << 6; // -V684
                periods_previous = 6;
            } else if(periods == 3) {
                byte_value |= 0x01 << 6;
                periods_previous = 4;
            } else if(periods == 5) {
                byte_value |= 0x02 << 6;
                periods_previous = 2;
            } else if(periods == 7) {
                byte_value |= 0x03 << 6;
                periods_previous = 0;
            } else if(periods == 2) {
                frame_state = NFCV_FRAME_STATE_EOF;
                break;
            } else {
                frame_state = NFCV_FRAME_STATE_RESET;
                break;
            }

            if(bits_received >= 8) {
                if(frame_pos < NFCV_FRAMESIZE_MAX) {
                    nfcv_data->frame[frame_pos++] = (uint8_t)byte_value;
                }
                bits_received = 0;
            }
            wait_for_pulse = true;
            break;
        }

        /* post-state-machine cleanup and reset */
        if(frame_state == NFCV_FRAME_STATE_RESET) {
            frame_state = NFCV_FRAME_STATE_SOF1;
        } else if(frame_state == NFCV_FRAME_STATE_EOF) {
            nfcv_data->frame_length = frame_pos;
            nfcv_data->eof_timestamp = timestamp;
            break;
        }
    }

    if(frame_state == NFCV_FRAME_STATE_EOF) {
        /* we know that this code uses TIM2, so stop pulse reader */
        pulse_reader_stop(nfcv_data->emu_air.reader_signal);
        if(tx_rx->sniff_rx) {
            tx_rx->sniff_rx(nfcv_data->frame, frame_pos * 8, false, tx_rx->sniff_context);
        }
        nfcv_data->emu_protocol_handler(tx_rx, nfc_data, nfcv_data);

        pulse_reader_start(nfcv_data->emu_air.reader_signal);
        ret = true;

    }
#ifdef NFCV_VERBOSE
    else {
        if(frame_state != NFCV_FRAME_STATE_SOF1) {
            FURI_LOG_T(TAG, "leaving while in state: %lu", frame_state);
        }
    }
#endif

#ifdef NFCV_DIAGNOSTIC_DUMPS
    if(period_buffer_pos) {
        FURI_LOG_T(TAG, "pulses:");
        for(uint32_t pos = 0; pos < period_buffer_pos; pos++) {
            FURI_LOG_T(TAG, "     #%lu: %u", pos, period_buffer[pos]);
        }
    }
#endif

    return ret;
}
