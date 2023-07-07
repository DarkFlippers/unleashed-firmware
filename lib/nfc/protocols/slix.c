
#include <limits.h>
#include "nfcv.h"
#include "slix.h"
#include "nfc_util.h"
#include <furi.h>
#include "furi_hal_nfc.h"
#include <furi_hal_random.h>

#define TAG "SLIX"

ReturnCode slix2_read_nxp_sysinfo(FuriHalNfcDevData* nfc_data, NfcVData* nfcv_data) {
    furi_assert(nfc_data);
    furi_assert(nfcv_data);

    uint8_t rxBuf[32];
    uint16_t received = 0;
    ReturnCode ret = ERR_NONE;

    FURI_LOG_D(TAG, "Read NXP SYSTEM INFORMATION...");

    for(int tries = 0; tries < NFCV_COMMAND_RETRIES; tries++) {
        uint8_t cmd[] = {};
        uint8_t uid[NFCV_UID_LENGTH];

        /* UID is stored reversed in requests */
        for(int pos = 0; pos < nfc_data->uid_len; pos++) {
            uid[pos] = nfc_data->uid[nfc_data->uid_len - 1 - pos];
        }

        ReturnCode ret = rfalNfcvPollerTransceiveReq(
            NFCV_CMD_NXP_GET_NXP_SYSTEM_INFORMATION,
            RFAL_NFCV_REQ_FLAG_DEFAULT,
            NFCV_MANUFACTURER_NXP,
            uid,
            cmd,
            sizeof(cmd),
            rxBuf,
            sizeof(rxBuf),
            &received);

        if(ret == ERR_NONE) {
            break;
        }
    }

    if(ret != ERR_NONE || received != 8) { //-V560
        FURI_LOG_D(TAG, "Failed: %d, %d", ret, received);
        return ret;
    }
    FURI_LOG_D(TAG, "Success...");

    NfcVSlixData* slix = &nfcv_data->sub_data.slix;
    slix->pp_pointer = rxBuf[1];
    slix->pp_condition = rxBuf[2];

    /* convert NXP's to our internal lock bits format */
    nfcv_data->security_status[0] = 0;
    nfcv_data->security_status[0] |= (rxBuf[3] & SlixLockBitDsfid) ? NfcVLockBitDsfid : 0;
    nfcv_data->security_status[0] |= (rxBuf[3] & SlixLockBitAfi) ? NfcVLockBitAfi : 0;
    nfcv_data->security_status[0] |= (rxBuf[3] & SlixLockBitEas) ? NfcVLockBitEas : 0;
    nfcv_data->security_status[0] |= (rxBuf[3] & SlixLockBitPpl) ? NfcVLockBitPpl : 0;

    return ERR_NONE;
}

ReturnCode slix2_read_signature(FuriHalNfcDevData* nfc_data, NfcVData* nfcv_data) {
    furi_assert(nfc_data);
    furi_assert(nfcv_data);

    uint8_t rxBuf[64];
    uint16_t received = 0;
    ReturnCode ret = ERR_NONE;

    FURI_LOG_D(TAG, "Read SIGNATURE...");

    for(int tries = 0; tries < NFCV_COMMAND_RETRIES; tries++) {
        uint8_t cmd[] = {};
        uint8_t uid[NFCV_UID_LENGTH];

        /* UID is stored reversed in requests */
        for(int pos = 0; pos < nfc_data->uid_len; pos++) {
            uid[pos] = nfc_data->uid[nfc_data->uid_len - 1 - pos];
        }

        ReturnCode ret = rfalNfcvPollerTransceiveReq(
            NFCV_CMD_NXP_READ_SIGNATURE,
            RFAL_NFCV_REQ_FLAG_DEFAULT,
            NFCV_MANUFACTURER_NXP,
            uid,
            cmd,
            sizeof(cmd),
            rxBuf,
            sizeof(rxBuf),
            &received);

        if(ret == ERR_NONE) {
            break;
        }
    }

    if(ret != ERR_NONE || received != 33) { //-V560
        FURI_LOG_D(TAG, "Failed: %d, %d", ret, received);
        return ret;
    }
    FURI_LOG_D(TAG, "Success...");

    NfcVSlixData* slix = &nfcv_data->sub_data.slix;
    memcpy(slix->signature, &rxBuf[1], 32);

    return ERR_NONE;
}

ReturnCode slix2_read_custom(FuriHalNfcDevData* nfc_data, NfcVData* nfcv_data) {
    ReturnCode ret = ERR_NONE;

    ret = slix2_read_nxp_sysinfo(nfc_data, nfcv_data);
    if(ret != ERR_NONE) {
        return ret;
    }
    ret = slix2_read_signature(nfc_data, nfcv_data);

    return ret;
}

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
        NFCV_CMD_NXP_GET_RANDOM_NUMBER,
        RFAL_NFCV_REQ_FLAG_DEFAULT,
        NFCV_MANUFACTURER_NXP,
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
        NFCV_CMD_NXP_SET_PASSWORD,
        RFAL_NFCV_REQ_FLAG_DATA_RATE,
        NFCV_MANUFACTURER_NXP,
        NULL,
        cmd_set_pass,
        sizeof(cmd_set_pass),
        rxBuf,
        sizeof(rxBuf),
        &received);

    return ret;
}

static void slix_generic_pass_infos(
    uint8_t password_id,
    NfcVSlixData* slix,
    uint8_t** password,
    uint32_t* flag_valid,
    uint32_t* flag_set) {
    switch(password_id) {
    case SLIX_PASS_READ:
        *password = slix->key_read;
        *flag_valid = NfcVSlixDataFlagsValidKeyRead;
        *flag_set = NfcVSlixDataFlagsHasKeyRead;
        break;
    case SLIX_PASS_WRITE:
        *password = slix->key_write;
        *flag_valid = NfcVSlixDataFlagsValidKeyWrite;
        *flag_set = NfcVSlixDataFlagsHasKeyWrite;
        break;
    case SLIX_PASS_PRIVACY:
        *password = slix->key_privacy;
        *flag_valid = NfcVSlixDataFlagsValidKeyPrivacy;
        *flag_set = NfcVSlixDataFlagsHasKeyPrivacy;
        break;
    case SLIX_PASS_DESTROY:
        *password = slix->key_destroy;
        *flag_valid = NfcVSlixDataFlagsValidKeyDestroy;
        *flag_set = NfcVSlixDataFlagsHasKeyDestroy;
        break;
    case SLIX_PASS_EASAFI:
        *password = slix->key_eas;
        *flag_valid = NfcVSlixDataFlagsValidKeyEas;
        *flag_set = NfcVSlixDataFlagsHasKeyEas;
        break;
    default:
        break;
    }
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

    if((slix->flags & NfcVSlixDataFlagsPrivacy) &&
       ctx->command != NFCV_CMD_NXP_GET_RANDOM_NUMBER &&
       ctx->command != NFCV_CMD_NXP_SET_PASSWORD) {
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
    case NFCV_CMD_NXP_GET_RANDOM_NUMBER: {
        slix->rand[0] = furi_hal_random_get();
        slix->rand[1] = furi_hal_random_get();

        ctx->response_buffer[0] = NFCV_NOERROR;
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

    case NFCV_CMD_NXP_SET_PASSWORD: {
        /* the password to be set is the first parameter */
        uint8_t password_id = nfcv_data->frame[ctx->payload_offset];
        /* right after that is the XORed password */
        uint8_t* password_xored = &nfcv_data->frame[ctx->payload_offset + 1];

        /* only handle if the password type is supported */
        if(!(password_id & password_supported)) {
            break;
        }

        /* fetch the last RAND value */
        uint8_t* rand = slix->rand;

        /* first calc the password that has been sent */
        uint8_t password_rcv[4];
        for(int pos = 0; pos < 4; pos++) {
            password_rcv[pos] = password_xored[3 - pos] ^ rand[pos % 2];
        }
        uint32_t pass_received = slix_read_be(password_rcv, 4);

        /* then determine the password type (or even update if not set yet) */
        uint8_t* password = NULL;
        uint32_t flag_valid = 0;
        uint32_t flag_set = 0;

        slix_generic_pass_infos(password_id, slix, &password, &flag_valid, &flag_set);

        /* when the password is not supported, return silently */
        if(!password) {
            break;
        }

        /* check if the password is known */
        bool pass_valid = false;
        uint32_t pass_expect = 0;

        if(slix->flags & flag_set) {
            /* if so, fetch the stored password and compare */
            pass_expect = slix_read_be(password, 4);
            pass_valid = (pass_expect == pass_received);
        } else {
            /* if not known, just accept it and store that password */
            memcpy(password, password_rcv, 4);
            nfcv_data->modified = true;
            slix->flags |= flag_set;

            pass_valid = true;
        }

        /* if the pass was valid or accepted for other reasons, continue */
        if(pass_valid) {
            slix->flags |= flag_valid;

            /* handle actions when a correct password was given, aside of setting the flag */
            switch(password_id) {
            case SLIX_PASS_PRIVACY:
                slix->flags &= ~NfcVSlixDataFlagsPrivacy;
                nfcv_data->modified = true;
                break;
            case SLIX_PASS_DESTROY:
                slix->flags |= NfcVSlixDataFlagsDestroyed;
                FURI_LOG_D(TAG, "Pooof! Got destroyed");
                break;
            default:
                break;
            }

            ctx->response_buffer[0] = NFCV_NOERROR;
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

    case NFCV_CMD_NXP_WRITE_PASSWORD: {
        uint8_t password_id = nfcv_data->frame[ctx->payload_offset];

        if(!(password_id & password_supported)) {
            break;
        }

        uint8_t* new_password = &nfcv_data->frame[ctx->payload_offset + 1];
        uint8_t* password = NULL;
        uint32_t flag_valid = 0;
        uint32_t flag_set = 0;

        slix_generic_pass_infos(password_id, slix, &password, &flag_valid, &flag_set);

        /* when the password is not supported, return silently */
        if(!password) {
            break;
        }

        bool pass_valid = (slix->flags & flag_valid);
        if(!(slix->flags & flag_set)) {
            pass_valid = true;
        }

        if(pass_valid) {
            slix->flags |= flag_valid;
            slix->flags |= flag_set;

            memcpy(password, new_password, 4);

            ctx->response_buffer[0] = NFCV_NOERROR;
            nfcv_emu_send(
                tx_rx, nfcv_data, ctx->response_buffer, 1, ctx->response_flags, ctx->send_time);
            snprintf(
                nfcv_data->last_command, sizeof(nfcv_data->last_command), "WRITE_PASSWORD OK");
        } else {
            snprintf(
                nfcv_data->last_command, sizeof(nfcv_data->last_command), "WRITE_PASSWORD FAIL");
        }
        handled = true;
        break;
    }

    case NFCV_CMD_NXP_ENABLE_PRIVACY: {
        ctx->response_buffer[0] = NFCV_NOERROR;

        nfcv_emu_send(
            tx_rx, nfcv_data, ctx->response_buffer, 1, ctx->response_flags, ctx->send_time);
        snprintf(
            nfcv_data->last_command,
            sizeof(nfcv_data->last_command),
            "NFCV_CMD_NXP_ENABLE_PRIVACY");

        slix->flags |= NfcVSlixDataFlagsPrivacy;
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
    FURI_LOG_D(
        TAG,
        "  Privacy mode: %s",
        (nfcv_data->sub_data.slix.flags & NfcVSlixDataFlagsPrivacy) ? "ON" : "OFF");

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
    FURI_LOG_D(
        TAG,
        "  Privacy mode: %s",
        (nfcv_data->sub_data.slix.flags & NfcVSlixDataFlagsPrivacy) ? "ON" : "OFF");

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
    FURI_LOG_D(
        TAG,
        "  Privacy mode: %s",
        (nfcv_data->sub_data.slix.flags & NfcVSlixDataFlagsPrivacy) ? "ON" : "OFF");

    NfcVEmuProtocolCtx* ctx = nfcv_data->emu_protocol_ctx;
    ctx->emu_protocol_filter = &slix_protocol_filter;
}

bool slix2_protocol_filter( // -V524
    FuriHalNfcTxRxContext* tx_rx,
    FuriHalNfcDevData* nfc_data,
    void* nfcv_data_in) {
    furi_assert(tx_rx);
    furi_assert(nfc_data);
    furi_assert(nfcv_data_in);

    NfcVData* nfcv_data = (NfcVData*)nfcv_data_in;
    NfcVEmuProtocolCtx* ctx = nfcv_data->emu_protocol_ctx;
    NfcVSlixData* slix = &nfcv_data->sub_data.slix;

    bool handled = false;

    /* many SLIX share some of the functions, place that in a generic handler */
    if(slix_generic_protocol_filter(tx_rx, nfc_data, nfcv_data_in, SLIX_PASS_ALL)) {
        return true;
    }

    switch(ctx->command) {
    /* override WRITE BLOCK for block 79 (16 bit counter)  */
    case NFCV_CMD_WRITE_BLOCK:
    case NFCV_CMD_WRITE_MULTI_BLOCK: {
        uint8_t resp_len = 1;
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

            for(int block_num = block; block_num < block + blocks; block_num++) {
                /* special case, 16-bit counter */
                if(block_num == 79) {
                    uint32_t dest;
                    uint32_t ctr_old;

                    memcpy(&dest, &nfcv_data->frame[data_pos], 4);
                    memcpy(&ctr_old, &nfcv_data->data[nfcv_data->block_size * block_num], 4);

                    uint32_t ctr_new = ctr_old;
                    bool allowed = true;

                    /* increment counter */
                    if(dest == 1) {
                        ctr_new = (ctr_old & 0xFFFF0000) | ((ctr_old + 1) & 0xFFFF);

                        /* protection flag set? */
                        if(ctr_old & 0x01000000) { //-V1051
                            allowed = nfcv_data->sub_data.slix.flags &
                                      NfcVSlixDataFlagsValidKeyRead;
                        }
                    } else {
                        ctr_new = dest;
                        allowed = nfcv_data->sub_data.slix.flags & NfcVSlixDataFlagsValidKeyWrite;
                    }

                    if(allowed) {
                        memcpy( //-V1086
                            &nfcv_data->data[nfcv_data->block_size * block_num],
                            &ctr_new,
                            4);
                    } else {
                        /* incorrect read or write password */
                        ctx->response_buffer[0] = NFCV_RES_FLAG_ERROR;
                        ctx->response_buffer[1] = NFCV_ERROR_GENERIC;
                        resp_len = 2;
                    }
                } else {
                    memcpy(
                        &nfcv_data->data[nfcv_data->block_size * block_num],
                        &nfcv_data->frame[data_pos],
                        nfcv_data->block_size);
                }
                data_pos += nfcv_data->block_size;
            }
            nfcv_data->modified = true;

        } else {
            ctx->response_buffer[0] = NFCV_RES_FLAG_ERROR;
            ctx->response_buffer[1] = NFCV_ERROR_GENERIC;
            resp_len = 2;
        }

        bool respond = (ctx->response_buffer[0] == NFCV_NOERROR) ||
                       (ctx->addressed || ctx->selected);

        if(respond) {
            nfcv_emu_send(
                tx_rx,
                nfcv_data,
                ctx->response_buffer,
                resp_len,
                ctx->response_flags,
                ctx->send_time);
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
        handled = true;
        break;
    }

    case NFCV_CMD_NXP_READ_SIGNATURE: {
        uint32_t len = 0;
        ctx->response_buffer[len++] = NFCV_NOERROR;
        memcpy(&ctx->response_buffer[len], slix->signature, sizeof(slix->signature));
        len += sizeof(slix->signature);

        nfcv_emu_send(
            tx_rx, nfcv_data, ctx->response_buffer, len, ctx->response_flags, ctx->send_time);
        snprintf(nfcv_data->last_command, sizeof(nfcv_data->last_command), "READ_SIGNATURE");

        handled = true;
        break;
    }

    case NFCV_CMD_NXP_GET_NXP_SYSTEM_INFORMATION: {
        uint32_t len = 0;
        uint8_t lock_bits = 0;

        /* convert our internal lock bits format into NXP's */
        lock_bits |= (nfcv_data->security_status[0] & NfcVLockBitDsfid) ? SlixLockBitDsfid : 0;
        lock_bits |= (nfcv_data->security_status[0] & NfcVLockBitAfi) ? SlixLockBitAfi : 0;
        lock_bits |= (nfcv_data->security_status[0] & NfcVLockBitEas) ? SlixLockBitEas : 0;
        lock_bits |= (nfcv_data->security_status[0] & NfcVLockBitPpl) ? SlixLockBitPpl : 0;

        ctx->response_buffer[len++] = NFCV_NOERROR;
        ctx->response_buffer[len++] = nfcv_data->sub_data.slix.pp_pointer;
        ctx->response_buffer[len++] = nfcv_data->sub_data.slix.pp_condition;
        ctx->response_buffer[len++] = lock_bits;
        ctx->response_buffer[len++] = 0x7F; /* features LSB */
        ctx->response_buffer[len++] = 0x35; /* features */
        ctx->response_buffer[len++] = 0; /* features */
        ctx->response_buffer[len++] = 0; /* features MSB */

        nfcv_emu_send(
            tx_rx, nfcv_data, ctx->response_buffer, len, ctx->response_flags, ctx->send_time);
        snprintf(
            nfcv_data->last_command,
            sizeof(nfcv_data->last_command),
            "GET_NXP_SYSTEM_INFORMATION");

        handled = true;
        break;
    }
    }

    return handled;
}

void slix2_prepare(NfcVData* nfcv_data) {
    FURI_LOG_D(
        TAG, "  Privacy pass: 0x%08lX", slix_read_be(nfcv_data->sub_data.slix.key_privacy, 4));
    FURI_LOG_D(
        TAG, "  Destroy pass: 0x%08lX", slix_read_be(nfcv_data->sub_data.slix.key_destroy, 4));
    FURI_LOG_D(TAG, "  EAS     pass: 0x%08lX", slix_read_be(nfcv_data->sub_data.slix.key_eas, 4));
    FURI_LOG_D(
        TAG,
        "  Privacy mode: %s",
        (nfcv_data->sub_data.slix.flags & NfcVSlixDataFlagsPrivacy) ? "ON" : "OFF");

    NfcVEmuProtocolCtx* ctx = nfcv_data->emu_protocol_ctx;
    ctx->emu_protocol_filter = &slix2_protocol_filter;
}
