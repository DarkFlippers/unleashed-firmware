#include "mf_ultralight_poller_i.h"
#include "mf_ultralight_aes_crypto.h"

#include <furi.h>
#include <furi_hal_random.h>
#include <mbedtls/aes.h>

#define TAG "MfUltralightPoller"

MfUltralightError mf_ultralight_process_error(Iso14443_3aError error) {
    MfUltralightError ret = MfUltralightErrorNone;

    switch(error) {
    case Iso14443_3aErrorNone:
        ret = MfUltralightErrorNone;
        break;
    case Iso14443_3aErrorNotPresent:
        ret = MfUltralightErrorNotPresent;
        break;
    case Iso14443_3aErrorColResFailed:
    case Iso14443_3aErrorCommunication:
    case Iso14443_3aErrorWrongCrc:
        ret = MfUltralightErrorProtocol;
        break;
    case Iso14443_3aErrorTimeout:
        ret = MfUltralightErrorTimeout;
        break;
    default:
        ret = MfUltralightErrorProtocol;
        break;
    }

    return ret;
}

MfUltralightError mf_ultralight_poller_auth_pwd(
    MfUltralightPoller* instance,
    MfUltralightPollerAuthContext* data) {
    furi_check(instance);
    furi_check(data);

    uint8_t auth_cmd[5] = {MF_ULTRALIGHT_CMD_PWD_AUTH}; //-V1009
    memcpy(&auth_cmd[1], data->password.data, MF_ULTRALIGHT_AUTH_PASSWORD_SIZE);
    bit_buffer_copy_bytes(instance->tx_buffer, auth_cmd, sizeof(auth_cmd));

    MfUltralightError ret = MfUltralightErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;
    do {
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_ultralight_process_error(error);
            break;
        }
        if(bit_buffer_get_size_bytes(instance->rx_buffer) != MF_ULTRALIGHT_AUTH_PACK_SIZE) {
            ret = MfUltralightErrorAuth;
            break;
        }
        bit_buffer_write_bytes(instance->rx_buffer, data->pack.data, MF_ULTRALIGHT_AUTH_PACK_SIZE);
    } while(false);

    return ret;
}

static MfUltralightError mf_ultralight_poller_send_authenticate_cmd(
    MfUltralightPoller* instance,
    const uint8_t* cmd,
    const uint8_t length,
    const bool initial_cmd,
    uint8_t* response) {
    furi_check(instance);
    furi_check(cmd);
    furi_check(response);

    bit_buffer_copy_bytes(instance->tx_buffer, cmd, length);

    MfUltralightError ret = MfUltralightErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;
    do {
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_ultralight_process_error(error);
            break;
        }

        const uint8_t expected_response_code = initial_cmd ? 0xAF : 0x00;
        if((bit_buffer_get_byte(instance->rx_buffer, 0) != expected_response_code) ||
           (bit_buffer_get_size_bytes(instance->rx_buffer) !=
            MF_ULTRALIGHT_C_AUTH_RESPONSE_SIZE)) {
            ret = MfUltralightErrorAuth;
            break;
        }

        memcpy(
            response,
            bit_buffer_get_data(instance->rx_buffer) + 1,
            MF_ULTRALIGHT_C_AUTH_RND_BLOCK_SIZE);
    } while(false);

    return ret;
}

MfUltralightError mf_ultralight_poller_authentication_test(MfUltralightPoller* instance) {
    furi_check(instance);

    uint8_t auth_cmd[2] = {MF_ULTRALIGHT_CMD_AUTH, 0x00};
    uint8_t dummy[MF_ULTRALIGHT_C_AUTH_RND_BLOCK_SIZE];
    return mf_ultralight_poller_send_authenticate_cmd(
        instance, auth_cmd, sizeof(auth_cmd), true, dummy);
}

MfUltralightError mf_ultralight_poller_authenticate_start(
    MfUltralightPoller* instance,
    const uint8_t* RndA,
    uint8_t* output) {
    furi_check(instance);
    furi_check(RndA);
    furi_check(output);

    MfUltralightError ret = MfUltralightErrorNone;
    do {
        uint8_t encRndB[MF_ULTRALIGHT_C_AUTH_RND_BLOCK_SIZE] = {0};
        uint8_t auth_cmd[2] = {MF_ULTRALIGHT_CMD_AUTH, 0x00};
        ret = mf_ultralight_poller_send_authenticate_cmd(
            instance, auth_cmd, sizeof(auth_cmd), true, encRndB /* instance->encRndB */);

        if(ret != MfUltralightErrorNone) break;

        uint8_t iv[MF_ULTRALIGHT_C_AUTH_IV_BLOCK_SIZE] = {0};
        uint8_t* RndB = output + MF_ULTRALIGHT_C_AUTH_RND_B_BLOCK_OFFSET;
        mf_ultralight_3des_decrypt(
            &instance->des_context,
            instance->auth_context.tdes_key.data,
            iv,
            encRndB,
            sizeof(encRndB),
            RndB);
        mf_ultralight_3des_shift_data(RndB);

        memcpy(output, RndA, MF_ULTRALIGHT_C_AUTH_RND_BLOCK_SIZE);

        mf_ultralight_3des_encrypt(
            &instance->des_context,
            instance->auth_context.tdes_key.data,
            encRndB,
            output,
            MF_ULTRALIGHT_C_AUTH_DATA_SIZE,
            output);

    } while(false);

    return ret;
}

MfUltralightError mf_ultralight_poller_authenticate_end(
    MfUltralightPoller* instance,
    const uint8_t* RndB,
    const uint8_t* request,
    uint8_t* response) {
    furi_check(instance);
    furi_check(RndB);
    furi_check(request);
    furi_check(response);

    uint8_t auth_cmd[MF_ULTRALIGHT_C_ENCRYPTED_PACK_SIZE] = {0xAF}; //-V1009
    memcpy(&auth_cmd[1], request, MF_ULTRALIGHT_C_AUTH_DATA_SIZE);
    bit_buffer_copy_bytes(instance->tx_buffer, auth_cmd, sizeof(auth_cmd));

    MfUltralightError ret = MfUltralightErrorNone;
    do {
        ret = mf_ultralight_poller_send_authenticate_cmd(
            instance, auth_cmd, sizeof(auth_cmd), false, response);

        if(ret != MfUltralightErrorNone) break;

        mf_ultralight_3des_decrypt(
            &instance->des_context,
            instance->auth_context.tdes_key.data,
            RndB,
            bit_buffer_get_data(instance->rx_buffer) + 1,
            MF_ULTRALIGHT_C_AUTH_RND_BLOCK_SIZE,
            response);
    } while(false);

    return ret;
}

MfUltralightError mf_ultralight_poller_authenticate_aes(
    MfUltralightPoller* instance,
    const uint8_t* key,
    MfUltralightAesKeyType key_type) {
    furi_check(instance);
    furi_check(key);

    // AES-128 CBC, IV = 0, 3-pass mutual auth (MF0AES20 §8.6.1). Cross-checked against
    // Proxmark3 mifare_ultra_aes_auth(): decrypt ek(RndB), rotate left by one byte -> RndB', send
    // ek(RndA || RndB'), then verify the decrypted RndA' equals RndA rotated left by one byte.
    // The mbedtls setkey/crypt return codes are intentionally unchecked: for a fixed 128-bit key
    // and 16-byte-aligned lengths they cannot fail, and any anomaly would fail the RndA' verify
    // below (returning MfUltralightErrorAuth), never a false success.
    MfUltralightError ret = MfUltralightErrorNone;
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);

    do {
        // --- Part 1: [0x1A, key_type] -> 0xAF || ek(RndB) ---
        uint8_t cmd1[2] = {MF_ULTRALIGHT_CMD_AUTH, (uint8_t)key_type};
        bit_buffer_copy_bytes(instance->tx_buffer, cmd1, sizeof(cmd1));

        Iso14443_3aError error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_ultralight_process_error(error);
            break;
        }
        if((bit_buffer_get_size_bytes(instance->rx_buffer) !=
            MF_ULTRALIGHT_AES_AUTH_P1_RESP_SIZE) ||
           (bit_buffer_get_byte(instance->rx_buffer, 0) != 0xAF)) {
            ret = MfUltralightErrorAuth;
            break;
        }

        uint8_t iv[MF_ULTRALIGHT_AES_BLOCK_SIZE] = {0};
        uint8_t rnd_b[MF_ULTRALIGHT_AES_BLOCK_SIZE] = {0};
        mbedtls_aes_setkey_dec(&ctx, key, 128);
        mbedtls_aes_crypt_cbc(
            &ctx,
            MBEDTLS_AES_DECRYPT,
            MF_ULTRALIGHT_AES_BLOCK_SIZE,
            iv,
            bit_buffer_get_data(instance->rx_buffer) + 1,
            rnd_b);
        mf_ultralight_aes_rol16(rnd_b); // RndB -> RndB'

        uint8_t rnd_a[MF_ULTRALIGHT_AES_BLOCK_SIZE] = {0};
        furi_hal_random_fill_buf(rnd_a, sizeof(rnd_a));

        uint8_t rnd_ab[2 * MF_ULTRALIGHT_AES_BLOCK_SIZE] = {0};
        memcpy(rnd_ab, rnd_a, MF_ULTRALIGHT_AES_BLOCK_SIZE);
        memcpy(rnd_ab + MF_ULTRALIGHT_AES_BLOCK_SIZE, rnd_b, MF_ULTRALIGHT_AES_BLOCK_SIZE);

        uint8_t enc_ab[2 * MF_ULTRALIGHT_AES_BLOCK_SIZE] = {0};
        memset(iv, 0, sizeof(iv));
        mbedtls_aes_setkey_enc(&ctx, key, 128);
        mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, sizeof(rnd_ab), iv, rnd_ab, enc_ab);

        // --- Part 2: [0xAF, ek(RndA || RndB')] -> 0x00 || ek(RndA') ---
        uint8_t cmd2[MF_ULTRALIGHT_AES_AUTH_P2_CMD_SIZE] = {0xAF};
        memcpy(cmd2 + 1, enc_ab, sizeof(enc_ab));
        bit_buffer_copy_bytes(instance->tx_buffer, cmd2, sizeof(cmd2));

        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_ultralight_process_error(error);
            break;
        }
        if((bit_buffer_get_size_bytes(instance->rx_buffer) !=
            MF_ULTRALIGHT_AES_AUTH_P2_RESP_SIZE) ||
           (bit_buffer_get_byte(instance->rx_buffer, 0) != 0x00)) {
            ret = MfUltralightErrorAuth;
            break;
        }

        uint8_t rec_rnd_a[MF_ULTRALIGHT_AES_BLOCK_SIZE] = {0};
        memset(iv, 0, sizeof(iv));
        mbedtls_aes_setkey_dec(&ctx, key, 128);
        mbedtls_aes_crypt_cbc(
            &ctx,
            MBEDTLS_AES_DECRYPT,
            MF_ULTRALIGHT_AES_BLOCK_SIZE,
            iv,
            bit_buffer_get_data(instance->rx_buffer) + 1,
            rec_rnd_a);

        mf_ultralight_aes_rol16(rnd_a); // expected RndA'
        if(memcmp(rec_rnd_a, rnd_a, MF_ULTRALIGHT_AES_BLOCK_SIZE) != 0) {
            ret = MfUltralightErrorAuth;
            break;
        }

        // Auth OK: derive the secure-messaging session key and reset the command counter. It stays
        // dormant until aes_cmac.active is set (when a plain read or write NAKs post-auth), so a plain
        // card ignores it. rnd_a is now RndA', rnd_b is RndB' (both rotated once).
        mf_ultralight_aes_derive_session_key(key, rnd_a, rnd_b, instance->aes_cmac.session_key);
        instance->aes_cmac.counter = 0;
    } while(false);

    mbedtls_aes_free(&ctx);
    return ret;
}

MfUltralightError mf_ultralight_poller_read_page_from_sector(
    MfUltralightPoller* instance,
    uint8_t sector,
    uint8_t tag,
    MfUltralightPageReadCommandData* data) {
    furi_check(instance);
    furi_check(data);

    MfUltralightError ret = MfUltralightErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        const uint8_t select_sector_cmd[2] = {MF_ULTRALIGHT_CMD_SECTOR_SELECT, 0xff};
        bit_buffer_copy_bytes(instance->tx_buffer, select_sector_cmd, sizeof(select_sector_cmd));
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorWrongCrc) {
            FURI_LOG_D(TAG, "Failed to issue sector select command");
            ret = mf_ultralight_process_error(error);
            break;
        }

        const uint8_t read_sector_cmd[4] = {sector, 0x00, 0x00, 0x00};
        bit_buffer_copy_bytes(instance->tx_buffer, read_sector_cmd, sizeof(read_sector_cmd));
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorTimeout) {
            // This is NOT a typo! The tag ACKs by not sending a response within 1ms.
            FURI_LOG_D(TAG, "Sector %u select NAK'd", sector);
            ret = MfUltralightErrorProtocol;
            break;
        }

        ret = mf_ultralight_poller_read_page(instance, tag, data);
    } while(false);

    return ret;
}

MfUltralightError mf_ultralight_poller_read_page(
    MfUltralightPoller* instance,
    uint8_t start_page,
    MfUltralightPageReadCommandData* data) {
    furi_check(instance);
    furi_check(data);

    MfUltralightError ret = MfUltralightErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        uint8_t read_page_cmd[2] = {MF_ULTRALIGHT_CMD_READ_PAGE, start_page};
        bit_buffer_copy_bytes(instance->tx_buffer, read_page_cmd, sizeof(read_page_cmd));
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_ultralight_process_error(error);
            break;
        }
        if(bit_buffer_get_size_bytes(instance->rx_buffer) !=
           sizeof(MfUltralightPageReadCommandData)) {
            ret = MfUltralightErrorProtocol;
            break;
        }
        bit_buffer_write_bytes(instance->rx_buffer, data, sizeof(MfUltralightPageReadCommandData));
    } while(false);

    return ret;
}

// Transceive a UL-AES secure-messaging command and verify its MAC'd response. `cmd`/`cmd_len` is the
// plain command (code + args); the 8-byte command MAC over (CmdCtr || cmd) is appended before
// sending. The response is `resp_len` body bytes followed by an 8-byte MAC over ((CmdCtr+1) || body);
// `resp_len` is 0 for a WRITE, whose ACK is replaced by that standalone MAC over CmdCtr (§8.8.3). On
// success the body (if any) is copied to `resp` and the command counter advances by 2 - but only
// after the response MAC verifies, so a failed frame can't desync a continued session.
static MfUltralightError mf_ultralight_poller_txrx_aes_cmac(
    MfUltralightPoller* instance,
    const uint8_t* cmd,
    size_t cmd_len,
    uint8_t* resp,
    size_t resp_len) {
    const uint16_t ctr = instance->aes_cmac.counter;

    uint8_t mac8[MF_ULTRALIGHT_AES_CMAC_SIZE];
    mf_ultralight_aes_cmac8_ctr(instance->aes_cmac.session_key, ctr, cmd, cmd_len, mac8);
    bit_buffer_copy_bytes(instance->tx_buffer, cmd, cmd_len);
    bit_buffer_append_bytes(instance->tx_buffer, mac8, sizeof(mac8));

    MfUltralightError ret = MfUltralightErrorNone;
    do {
        Iso14443_3aError error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            // A NAK (bad CMAC, locked page, ...) is a bare 4-bit frame with no CRC (§8.8.3).
            ret = mf_ultralight_process_error(error);
            break;
        }

        // Response body + 8-byte MAC (CRC already stripped by the ISO14443-3A layer).
        const uint8_t* rx = bit_buffer_get_data(instance->rx_buffer);
        if(bit_buffer_get_size_bytes(instance->rx_buffer) !=
           resp_len + MF_ULTRALIGHT_AES_CMAC_SIZE) {
            ret = MfUltralightErrorProtocol;
            break;
        }

        uint8_t rmac8[MF_ULTRALIGHT_AES_CMAC_SIZE];
        mf_ultralight_aes_cmac8_ctr(instance->aes_cmac.session_key, ctr + 1, rx, resp_len, rmac8);
        if(memcmp(rmac8, rx + resp_len, MF_ULTRALIGHT_AES_CMAC_SIZE) != 0) {
            // Distinct from a plain protocol NAK: the card answered under the secure session with a
            // MAC that doesn't verify (tamper, relay, or a counter desync).
            FURI_LOG_W(TAG, "UL-AES response MAC mismatch (ctr %u)", ctr);
            ret = MfUltralightErrorProtocol;
            break;
        }

        if(resp_len) memcpy(resp, rx, resp_len);
        instance->aes_cmac.counter = ctr + 2;
    } while(false);

    return ret;
}

MfUltralightError mf_ultralight_poller_read_page_aes_cmac(
    MfUltralightPoller* instance,
    uint8_t start_page,
    MfUltralightPageReadCommandData* data) {
    furi_check(instance);
    furi_check(data);
    const uint8_t cmd[2] = {MF_ULTRALIGHT_CMD_READ_PAGE, start_page};
    return mf_ultralight_poller_txrx_aes_cmac(
        instance, cmd, sizeof(cmd), (uint8_t*)data, sizeof(MfUltralightPageReadCommandData));
}

MfUltralightError mf_ultralight_poller_read_counter_aes_cmac(
    MfUltralightPoller* instance,
    uint8_t counter_num,
    MfUltralightCounter* data) {
    furi_check(instance);
    furi_check(data);
    const uint8_t cmd[2] = {MF_ULTRALIGHT_CMD_READ_CNT, counter_num};
    return mf_ultralight_poller_txrx_aes_cmac(
        instance, cmd, sizeof(cmd), data->data, MF_ULTRALIGHT_COUNTER_SIZE);
}

MfUltralightError mf_ultralight_poller_write_page(
    MfUltralightPoller* instance,
    uint8_t page,
    const MfUltralightPage* data) {
    furi_check(instance);
    furi_check(data);

    MfUltralightError ret = MfUltralightErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        uint8_t write_page_cmd[MF_ULTRALIGHT_PAGE_SIZE + 2] = {MF_ULTRALIGHT_CMD_WRITE_PAGE, page};
        memcpy(&write_page_cmd[2], data->data, MF_ULTRALIGHT_PAGE_SIZE);
        bit_buffer_copy_bytes(instance->tx_buffer, write_page_cmd, sizeof(write_page_cmd));
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorWrongCrc) {
            ret = mf_ultralight_process_error(error);
            break;
        }
        if(bit_buffer_get_size(instance->rx_buffer) != 4) {
            ret = MfUltralightErrorProtocol;
            break;
        }
        if(!bit_buffer_starts_with_byte(instance->rx_buffer, MF_ULTRALIGHT_CMD_ACK)) {
            ret = MfUltralightErrorProtocol;
            break;
        }
    } while(false);

    return ret;
}

MfUltralightError mf_ultralight_poller_write_page_aes_cmac(
    MfUltralightPoller* instance,
    uint8_t page,
    const MfUltralightPage* data) {
    furi_check(instance);
    furi_check(data);
    // Command = [WRITE, page] + 4 data bytes; the response is a standalone MAC with no body.
    uint8_t cmd[2 + MF_ULTRALIGHT_PAGE_SIZE] = {MF_ULTRALIGHT_CMD_WRITE_PAGE, page};
    memcpy(cmd + 2, data->data, MF_ULTRALIGHT_PAGE_SIZE);
    return mf_ultralight_poller_txrx_aes_cmac(instance, cmd, sizeof(cmd), NULL, 0);
}

MfUltralightError
    mf_ultralight_poller_read_version(MfUltralightPoller* instance, MfUltralightVersion* data) {
    furi_check(instance);
    furi_check(data);

    MfUltralightError ret = MfUltralightErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        const uint8_t get_version_cmd = MF_ULTRALIGHT_CMD_GET_VERSION;
        bit_buffer_copy_bytes(instance->tx_buffer, &get_version_cmd, sizeof(get_version_cmd));
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_ultralight_process_error(error);
            break;
        }
        if(bit_buffer_get_size_bytes(instance->rx_buffer) != sizeof(MfUltralightVersion)) {
            FURI_LOG_I(
                TAG, "Read Version failed: %zu", bit_buffer_get_size_bytes(instance->rx_buffer));
            ret = MfUltralightErrorProtocol;
            break;
        }
        bit_buffer_write_bytes(instance->rx_buffer, data, sizeof(MfUltralightVersion));
    } while(false);

    return ret;
}

MfUltralightError
    mf_ultralight_poller_read_signature(MfUltralightPoller* instance, MfUltralightSignature* data) {
    furi_check(instance);
    furi_check(data);

    MfUltralightError ret = MfUltralightErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        const uint8_t read_signature_cmd[2] = {MF_ULTRALIGHT_CMD_READ_SIG, 0x00};
        bit_buffer_copy_bytes(instance->tx_buffer, read_signature_cmd, sizeof(read_signature_cmd));
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_ultralight_process_error(error);
            break;
        }
        if(bit_buffer_get_size_bytes(instance->rx_buffer) != sizeof(MfUltralightSignature)) {
            ret = MfUltralightErrorProtocol;
            break;
        }
        bit_buffer_write_bytes(instance->rx_buffer, data, sizeof(MfUltralightSignature));
    } while(false);

    return ret;
}

MfUltralightError mf_ultralight_poller_read_signature_aes(
    MfUltralightPoller* instance,
    uint8_t* signature,
    bool* present) {
    furi_check(instance);
    furi_check(signature);
    furi_check(present);

    *present = false;
    MfUltralightError ret = MfUltralightErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        // Same READ_SIG (0x3C 0x00) as the 32-byte variant; MF0AES20 answers 48 bytes (secp192r1).
        const uint8_t read_signature_cmd[2] = {MF_ULTRALIGHT_CMD_READ_SIG, 0x00};
        bit_buffer_copy_bytes(instance->tx_buffer, read_signature_cmd, sizeof(read_signature_cmd));
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_ultralight_process_error(error);
            break;
        }
        // A card that answers but not with a full 48-byte signature counts as "no signature"
        // (present stays false), not a transport error - matches mf_plus's best-effort read.
        if(bit_buffer_get_size_bytes(instance->rx_buffer) != MF_ULTRALIGHT_AES_SIGNATURE_SIZE) {
            break;
        }
        bit_buffer_write_bytes(instance->rx_buffer, signature, MF_ULTRALIGHT_AES_SIGNATURE_SIZE);
        *present = true;
    } while(false);

    return ret;
}

MfUltralightError mf_ultralight_poller_read_counter(
    MfUltralightPoller* instance,
    uint8_t counter_num,
    MfUltralightCounter* data) {
    furi_check(instance);
    furi_check(data);

    MfUltralightError ret = MfUltralightErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        uint8_t read_counter_cmd[2] = {MF_ULTRALIGHT_CMD_READ_CNT, counter_num};
        bit_buffer_copy_bytes(instance->tx_buffer, read_counter_cmd, sizeof(read_counter_cmd));
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_ultralight_process_error(error);
            break;
        }
        if(bit_buffer_get_size_bytes(instance->rx_buffer) != MF_ULTRALIGHT_COUNTER_SIZE) {
            ret = MfUltralightErrorProtocol;
            break;
        }
        bit_buffer_write_bytes(instance->rx_buffer, data->data, MF_ULTRALIGHT_COUNTER_SIZE);
    } while(false);

    return ret;
}

MfUltralightError mf_ultralight_poller_read_tearing_flag(
    MfUltralightPoller* instance,
    uint8_t tearing_falg_num,
    MfUltralightTearingFlag* data) {
    furi_check(instance);
    furi_check(data);

    MfUltralightError ret = MfUltralightErrorNone;
    Iso14443_3aError error = Iso14443_3aErrorNone;

    do {
        uint8_t check_tearing_cmd[2] = {MF_ULTRALIGHT_CMD_CHECK_TEARING, tearing_falg_num};
        bit_buffer_copy_bytes(instance->tx_buffer, check_tearing_cmd, sizeof(check_tearing_cmd));
        error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_ultralight_process_error(error);
            break;
        }
        if(bit_buffer_get_size_bytes(instance->rx_buffer) != sizeof(MfUltralightTearingFlag)) {
            ret = MfUltralightErrorProtocol;
            break;
        }
        bit_buffer_write_bytes(instance->rx_buffer, data, sizeof(MfUltralightTearingFlag));
    } while(false);

    return ret;
}
