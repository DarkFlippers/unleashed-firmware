
#include <limits.h>
#include "nfcv.h"
#include "slix.h"
#include "nfc_util.h"
#include <furi.h>
#include "furi_hal_nfc.h"
#include <furi_hal_random.h>

#define TAG "SLIX"

static uint32_t slix_read_be(uint8_t* data, uint32_t length) {
    uint32_t value = 0;

    for(uint32_t pos = 0; pos < length; pos++) {
        value <<= 8;
        value |= data[pos];
    }

    return value;
}

uint8_t slix_get_ti(FuriHalNfcDevData* nfc_data) {
    return (nfc_data->uid[3] >> 3) & 3;
}

bool slix_check_card_type(FuriHalNfcDevData* nfc_data) {
    if((nfc_data->uid[0] == 0xE0) && (nfc_data->uid[1] == 0x04) && (nfc_data->uid[2] == 0x01) &&
       slix_get_ti(nfc_data) == 2) {
        return true;
    }
    return false;
}

bool slix2_check_card_type(FuriHalNfcDevData* nfc_data) {
    if((nfc_data->uid[0] == 0xE0) && (nfc_data->uid[1] == 0x04) && (nfc_data->uid[2] == 0x01) &&
       slix_get_ti(nfc_data) == 1) {
        return true;
    }
    return false;
}

bool slix_s_check_card_type(FuriHalNfcDevData* nfc_data) {
    if((nfc_data->uid[0] == 0xE0) && (nfc_data->uid[1] == 0x04) && (nfc_data->uid[2] == 0x02)) {
        return true;
    }
    return false;
}

bool slix_l_check_card_type(FuriHalNfcDevData* nfc_data) {
    if((nfc_data->uid[0] == 0xE0) && (nfc_data->uid[1] == 0x04) && (nfc_data->uid[2] == 0x03)) {
        return true;
    }
    return false;
}

ReturnCode slix_get_random(NfcVData* data) {
    uint16_t received = 0;
    uint8_t rxBuf[32];

    ReturnCode ret = rfalNfcvPollerTransceiveReq(
        ISO15693_CMD_NXP_GET_RANDOM_NUMBER,
        RFAL_NFCV_REQ_FLAG_DEFAULT,
        ISO15693_MANUFACTURER_NXP,
        NULL,
        NULL,
        0,
        rxBuf,
        sizeof(rxBuf),
        &received);

    if(ret == ERR_NONE) {
        if(received != 3) {
            return ERR_PROTO;
        }
        if(data != NULL) {
            data->sub_data.slix.rand[0] = rxBuf[2];
            data->sub_data.slix.rand[1] = rxBuf[1];
        }
    }

    return ret;
}

ReturnCode slix_unlock(NfcVData* data, uint32_t password_id) {
    furi_assert(rand);

    uint16_t received = 0;
    uint8_t rxBuf[32];
    uint8_t cmd_set_pass[] = {
        password_id,
        data->sub_data.slix.rand[1],
        data->sub_data.slix.rand[0],
        data->sub_data.slix.rand[1],
        data->sub_data.slix.rand[0]};
    uint8_t* password = NULL;

    switch(password_id) {
    case SLIX_PASS_READ:
        password = data->sub_data.slix.key_read;
        break;
    case SLIX_PASS_WRITE:
        password = data->sub_data.slix.key_write;
        break;
    case SLIX_PASS_PRIVACY:
        password = data->sub_data.slix.key_privacy;
        break;
    case SLIX_PASS_DESTROY:
        password = data->sub_data.slix.key_destroy;
        break;
    case SLIX_PASS_EASAFI:
        password = data->sub_data.slix.key_eas;
        break;
    default:
        break;
    }

    if(!password) {
        return ERR_NOTSUPP;
    }

    for(int pos = 0; pos < 4; pos++) {
        cmd_set_pass[1 + pos] ^= password[3 - pos];
    }

    ReturnCode ret = rfalNfcvPollerTransceiveReq(
        ISO15693_CMD_NXP_SET_PASSWORD,
        RFAL_NFCV_REQ_FLAG_DATA_RATE,
        ISO15693_MANUFACTURER_NXP,
        NULL,
        cmd_set_pass,
        sizeof(cmd_set_pass),
        rxBuf,
        sizeof(rxBuf),
        &received);

    return ret;
}

bool slix_generic_protocol_filter(
    FuriHalNfcTxRxContext* tx_rx,
    FuriHalNfcDevData* nfc_data,
    void* nfcv_data_in,
    uint32_t password_supported) {
    furi_assert(tx_rx);
    furi_assert(nfc_data);
    furi_assert(nfcv_data_in);

    NfcVData* nfcv_data = (NfcVData*)nfcv_data_in;
    NfcVEmuProtocolCtx* ctx = nfcv_data->emu_protocol_ctx;
    NfcVSlixData* slix = &nfcv_data->sub_data.slix;

    if(slix->privacy && ctx->command != ISO15693_CMD_NXP_GET_RANDOM_NUMBER &&
       ctx->command != ISO15693_CMD_NXP_SET_PASSWORD) {
        snprintf(
            nfcv_data->last_command,
            sizeof(nfcv_data->last_command),
            "command 0x%02X ignored, privacy mode",
            ctx->command);
        FURI_LOG_D(TAG, "%s", nfcv_data->last_command);
        return true;
    }

    bool handled = false;

    switch(ctx->command) {
    case ISO15693_CMD_NXP_GET_RANDOM_NUMBER: {
        slix->rand[0] = furi_hal_random_get();
        slix->rand[1] = furi_hal_random_get();

        ctx->response_buffer[0] = ISO15693_NOERROR;
        ctx->response_buffer[1] = slix->rand[1];
        ctx->response_buffer[2] = slix->rand[0];

        nfcv_emu_send(
            tx_rx, nfcv_data, ctx->response_buffer, 3, ctx->response_flags, ctx->send_time);
        snprintf(
            nfcv_data->last_command,
            sizeof(nfcv_data->last_command),
            "GET_RANDOM_NUMBER -> 0x%02X%02X",
            slix->rand[0],
            slix->rand[1]);

        handled = true;
        break;
    }

    case ISO15693_CMD_NXP_SET_PASSWORD: {
        uint8_t password_id = nfcv_data->frame[ctx->payload_offset];

        if(!(password_id & password_supported)) {
            break;
        }

        uint8_t* password_xored = &nfcv_data->frame[ctx->payload_offset + 1];
        uint8_t* rand = slix->rand;
        uint8_t* password = NULL;
        uint8_t password_rcv[4];

        switch(password_id) {
        case SLIX_PASS_READ:
            password = slix->key_read;
            break;
        case SLIX_PASS_WRITE:
            password = slix->key_write;
            break;
        case SLIX_PASS_PRIVACY:
            password = slix->key_privacy;
            break;
        case SLIX_PASS_DESTROY:
            password = slix->key_destroy;
            break;
        case SLIX_PASS_EASAFI:
            password = slix->key_eas;
            break;
        default:
            break;
        }

        for(int pos = 0; pos < 4; pos++) {
            password_rcv[pos] = password_xored[3 - pos] ^ rand[pos % 2];
        }
        uint32_t pass_expect = slix_read_be(password, 4);
        uint32_t pass_received = slix_read_be(password_rcv, 4);

        /* if the password is all-zeroes, just accept any password*/
        if(!pass_expect || pass_expect == pass_received) {
            switch(password_id) {
            case SLIX_PASS_READ:
                break;
            case SLIX_PASS_WRITE:
                break;
            case SLIX_PASS_PRIVACY:
                slix->privacy = false;
                break;
            case SLIX_PASS_DESTROY:
                FURI_LOG_D(TAG, "Pooof! Got destroyed");
                break;
            case SLIX_PASS_EASAFI:
                break;
            default:
                break;
            }
            ctx->response_buffer[0] = ISO15693_NOERROR;
            nfcv_emu_send(
                tx_rx, nfcv_data, ctx->response_buffer, 1, ctx->response_flags, ctx->send_time);
            snprintf(
                nfcv_data->last_command,
                sizeof(nfcv_data->last_command),
                "SET_PASSWORD #%02X 0x%08lX OK",
                password_id,
                pass_received);
        } else {
            snprintf(
                nfcv_data->last_command,
                sizeof(nfcv_data->last_command),
                "SET_PASSWORD #%02X 0x%08lX/%08lX FAIL",
                password_id,
                pass_received,
                pass_expect);
        }
        handled = true;
        break;
    }

    case ISO15693_CMD_NXP_ENABLE_PRIVACY: {
        ctx->response_buffer[0] = ISO15693_NOERROR;

        nfcv_emu_send(
            tx_rx, nfcv_data, ctx->response_buffer, 1, ctx->response_flags, ctx->send_time);
        snprintf(
            nfcv_data->last_command,
            sizeof(nfcv_data->last_command),
            "ISO15693_CMD_NXP_ENABLE_PRIVACY");

        slix->privacy = true;
        handled = true;
        break;
    }
    }

    return handled;
}

bool slix_l_protocol_filter(
    FuriHalNfcTxRxContext* tx_rx,
    FuriHalNfcDevData* nfc_data,
    void* nfcv_data_in) {
    furi_assert(tx_rx);
    furi_assert(nfc_data);
    furi_assert(nfcv_data_in);

    bool handled = false;

    /* many SLIX share some of the functions, place that in a generic handler */
    if(slix_generic_protocol_filter(
           tx_rx,
           nfc_data,
           nfcv_data_in,
           SLIX_PASS_PRIVACY | SLIX_PASS_DESTROY | SLIX_PASS_EASAFI)) {
        return true;
    }

    return handled;
}

void slix_l_prepare(NfcVData* nfcv_data) {
    FURI_LOG_D(
        TAG, "  Privacy pass: 0x%08lX", slix_read_be(nfcv_data->sub_data.slix.key_privacy, 4));
    FURI_LOG_D(
        TAG, "  Destroy pass: 0x%08lX", slix_read_be(nfcv_data->sub_data.slix.key_destroy, 4));
    FURI_LOG_D(TAG, "  EAS     pass: 0x%08lX", slix_read_be(nfcv_data->sub_data.slix.key_eas, 4));
    FURI_LOG_D(TAG, "  Privacy mode: %s", nfcv_data->sub_data.slix.privacy ? "ON" : "OFF");

    NfcVEmuProtocolCtx* ctx = nfcv_data->emu_protocol_ctx;
    ctx->emu_protocol_filter = &slix_l_protocol_filter;
}

bool slix_s_protocol_filter(
    FuriHalNfcTxRxContext* tx_rx,
    FuriHalNfcDevData* nfc_data,
    void* nfcv_data_in) {
    furi_assert(tx_rx);
    furi_assert(nfc_data);
    furi_assert(nfcv_data_in);

    bool handled = false;

    /* many SLIX share some of the functions, place that in a generic handler */
    if(slix_generic_protocol_filter(tx_rx, nfc_data, nfcv_data_in, SLIX_PASS_ALL)) {
        return true;
    }

    return handled;
}

void slix_s_prepare(NfcVData* nfcv_data) {
    FURI_LOG_D(
        TAG, "  Privacy pass: 0x%08lX", slix_read_be(nfcv_data->sub_data.slix.key_privacy, 4));
    FURI_LOG_D(
        TAG, "  Destroy pass: 0x%08lX", slix_read_be(nfcv_data->sub_data.slix.key_destroy, 4));
    FURI_LOG_D(TAG, "  EAS     pass: 0x%08lX", slix_read_be(nfcv_data->sub_data.slix.key_eas, 4));
    FURI_LOG_D(TAG, "  Privacy mode: %s", nfcv_data->sub_data.slix.privacy ? "ON" : "OFF");

    NfcVEmuProtocolCtx* ctx = nfcv_data->emu_protocol_ctx;
    ctx->emu_protocol_filter = &slix_s_protocol_filter;
}

bool slix_protocol_filter(
    FuriHalNfcTxRxContext* tx_rx,
    FuriHalNfcDevData* nfc_data,
    void* nfcv_data_in) {
    furi_assert(tx_rx);
    furi_assert(nfc_data);
    furi_assert(nfcv_data_in);

    bool handled = false;

    /* many SLIX share some of the functions, place that in a generic handler */
    if(slix_generic_protocol_filter(tx_rx, nfc_data, nfcv_data_in, SLIX_PASS_EASAFI)) {
        return true;
    }

    return handled;
}

void slix_prepare(NfcVData* nfcv_data) {
    FURI_LOG_D(
        TAG, "  Privacy pass: 0x%08lX", slix_read_be(nfcv_data->sub_data.slix.key_privacy, 4));
    FURI_LOG_D(
        TAG, "  Destroy pass: 0x%08lX", slix_read_be(nfcv_data->sub_data.slix.key_destroy, 4));
    FURI_LOG_D(TAG, "  EAS     pass: 0x%08lX", slix_read_be(nfcv_data->sub_data.slix.key_eas, 4));
    FURI_LOG_D(TAG, "  Privacy mode: %s", nfcv_data->sub_data.slix.privacy ? "ON" : "OFF");

    NfcVEmuProtocolCtx* ctx = nfcv_data->emu_protocol_ctx;
    ctx->emu_protocol_filter = &slix_protocol_filter;
}

bool slix2_protocol_filter(
    FuriHalNfcTxRxContext* tx_rx,
    FuriHalNfcDevData* nfc_data,
    void* nfcv_data_in) {
    furi_assert(tx_rx);
    furi_assert(nfc_data);
    furi_assert(nfcv_data_in);

    bool handled = false;

    /* many SLIX share some of the functions, place that in a generic handler */
    if(slix_generic_protocol_filter(tx_rx, nfc_data, nfcv_data_in, SLIX_PASS_ALL)) {
        return true;
    }

    return handled;
}

void slix2_prepare(NfcVData* nfcv_data) {
    FURI_LOG_D(
        TAG, "  Privacy pass: 0x%08lX", slix_read_be(nfcv_data->sub_data.slix.key_privacy, 4));
    FURI_LOG_D(
        TAG, "  Destroy pass: 0x%08lX", slix_read_be(nfcv_data->sub_data.slix.key_destroy, 4));
    FURI_LOG_D(TAG, "  EAS     pass: 0x%08lX", slix_read_be(nfcv_data->sub_data.slix.key_eas, 4));
    FURI_LOG_D(TAG, "  Privacy mode: %s", nfcv_data->sub_data.slix.privacy ? "ON" : "OFF");

    NfcVEmuProtocolCtx* ctx = nfcv_data->emu_protocol_ctx;
    ctx->emu_protocol_filter = &slix2_protocol_filter;
}
