#include "mf_ultralight_poller_i.h"

#include <furi.h>
#include <furi_hal_random.h>
#include <mbedtls/aes.h>

#define TAG "MfUltralightPoller"

// Rotate a 16-byte block left by one byte (RndX -> RndX'), as required by the AES 3-pass scheme.
static void mf_ultralight_aes_rol16(uint8_t* data) {
    uint8_t first = data[0];
    for(uint8_t i = 1; i < MF_ULTRALIGHT_AES_BLOCK_SIZE; i++) {
        data[i - 1] = data[i];
    }
    data[MF_ULTRALIGHT_AES_BLOCK_SIZE - 1] = first;
}

// Rotate a 16-byte block right by one byte (undo one rol16).
static void mf_ultralight_aes_ror16(uint8_t* data) {
    uint8_t last = data[MF_ULTRALIGHT_AES_BLOCK_SIZE - 1];
    for(uint8_t i = MF_ULTRALIGHT_AES_BLOCK_SIZE - 1; i > 0; i--) {
        data[i] = data[i - 1];
    }
    data[0] = last;
}

// AES-CMAC (NIST SP 800-38B / RFC 4493). Used for UL-AES secure messaging (session-key derivation
// and per-command MACs). mac = full 16 bytes.
static void
    mf_ultralight_aes_cmac(const uint8_t* key, const uint8_t* data, size_t len, uint8_t* mac) {
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_enc(&ctx, key, 128);

    // Subkeys K1/K2 from L = AES(0).
    uint8_t l[16] = {0}, k1[16], k2[16];
    mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_ENCRYPT, l, l);
    uint8_t overflow = 0;
    for(int i = 15; i >= 0; i--) {
        k1[i] = (l[i] << 1) | overflow;
        overflow = (l[i] & 0x80) ? 1 : 0;
    }
    if(l[0] & 0x80) k1[15] ^= 0x87;
    overflow = 0;
    for(int i = 15; i >= 0; i--) {
        k2[i] = (k1[i] << 1) | overflow;
        overflow = (k1[i] & 0x80) ? 1 : 0;
    }
    if(k1[0] & 0x80) k2[15] ^= 0x87;

    size_t n = (len + 15) / 16;
    bool complete = (len != 0) && (len % 16 == 0);
    if(n == 0) n = 1;

    uint8_t last[16] = {0};
    if(complete) {
        for(int i = 0; i < 16; i++)
            last[i] = data[16 * (n - 1) + i] ^ k1[i];
    } else {
        uint8_t pad[16] = {0};
        size_t rem = len % 16;
        if(len != 0) memcpy(pad, data + 16 * (n - 1), rem);
        pad[rem] = 0x80;
        for(int i = 0; i < 16; i++)
            last[i] = pad[i] ^ k2[i];
    }

    uint8_t x[16] = {0}, y[16];
    for(size_t i = 0; i < n - 1; i++) {
        for(int j = 0; j < 16; j++)
            y[j] = x[j] ^ data[16 * i + j];
        mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_ENCRYPT, y, x);
    }
    for(int j = 0; j < 16; j++)
        y[j] = x[j] ^ last[j];
    mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_ENCRYPT, y, mac);
    mbedtls_aes_free(&ctx);
}

// The UL-AES message MAC is the 8 odd-indexed bytes of the full CMAC.
static void mf_ultralight_aes_cmac8(const uint8_t* mac16, uint8_t* out8) {
    for(int i = 0; i < 8; i++)
        out8[i] = mac16[2 * i + 1];
}

// Derive the secure-messaging session key from the auth randoms (SP 800-108 counter mode, MF0AES20
// §8.8.1). rnd_a/rnd_b are passed still rotated (as left after the 3-pass), so undo one rotation.
static void mf_ultralight_aes_derive_session_key(
    const uint8_t* key,
    const uint8_t* rnd_a_rot,
    const uint8_t* rnd_b_rot,
    uint8_t* session_key) {
    uint8_t ra[16], rb[16];
    memcpy(ra, rnd_a_rot, 16);
    memcpy(rb, rnd_b_rot, 16);
    mf_ultralight_aes_ror16(ra); // -> original RndA
    mf_ultralight_aes_ror16(rb); // -> original RndB

    const uint8_t sv2[32] = {
        0x5a,
        0xa5,
        0x00,
        0x01,
        0x00,
        0x80,
        ra[0],
        ra[1],
        (uint8_t)(ra[2] ^ rb[0]),
        (uint8_t)(ra[3] ^ rb[1]),
        (uint8_t)(ra[4] ^ rb[2]),
        (uint8_t)(ra[5] ^ rb[3]),
        (uint8_t)(ra[6] ^ rb[4]),
        (uint8_t)(ra[7] ^ rb[5]),
        rb[6],
        rb[7],
        rb[8],
        rb[9],
        rb[10],
        rb[11],
        rb[12],
        rb[13],
        rb[14],
        rb[15],
        ra[8],
        ra[9],
        ra[10],
        ra[11],
        ra[12],
        ra[13],
        ra[14],
        ra[15],
    };
    mf_ultralight_aes_cmac(key, sv2, sizeof(sv2), session_key);
}

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

        // Auth OK: derive the secure-messaging session key and reset the command counter. It is only
        // used if the card is later found to require CMAC (see the poller's read handler); a plain
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

MfUltralightError mf_ultralight_poller_read_page_aes_cmac(
    MfUltralightPoller* instance,
    uint8_t start_page,
    MfUltralightPageReadCommandData* data) {
    furi_check(instance);
    furi_check(data);

    // The command uses CmdCtr and the response CmdCtr+1 (§8.8.3). The stored counter is advanced by
    // 2 only after the whole exchange verifies, so a failed frame can't desync a continued session.
    const uint16_t ctr = instance->aes_cmac.counter;

    // Command frame: [READ, page] + 8-byte MAC over (CmdCtr_LE || READ || page).
    uint8_t cmd[2 + MF_ULTRALIGHT_AES_CMAC_SIZE];
    cmd[0] = MF_ULTRALIGHT_CMD_READ_PAGE;
    cmd[1] = start_page;
    uint8_t mac_in[2 + 2] = {ctr & 0xFF, (ctr >> 8) & 0xFF, cmd[0], cmd[1]};
    uint8_t mac16[MF_ULTRALIGHT_AES_BLOCK_SIZE];
    mf_ultralight_aes_cmac(instance->aes_cmac.session_key, mac_in, sizeof(mac_in), mac16);
    mf_ultralight_aes_cmac8(mac16, cmd + 2);

    bit_buffer_copy_bytes(instance->tx_buffer, cmd, sizeof(cmd));

    MfUltralightError ret = MfUltralightErrorNone;
    do {
        Iso14443_3aError error = iso14443_3a_poller_send_standard_frame(
            instance->iso14443_3a_poller,
            instance->tx_buffer,
            instance->rx_buffer,
            MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC);
        if(error != Iso14443_3aErrorNone) {
            ret = mf_ultralight_process_error(error);
            break;
        }

        // Response: 16 data bytes + 8-byte MAC (CRC already stripped by the ISO14443-3A layer).
        if(bit_buffer_get_size_bytes(instance->rx_buffer) !=
           sizeof(MfUltralightPageReadCommandData) + MF_ULTRALIGHT_AES_CMAC_SIZE) {
            ret = MfUltralightErrorProtocol;
            break;
        }
        const uint8_t* resp = bit_buffer_get_data(instance->rx_buffer);

        // Verify the response MAC over ((CmdCtr+1)_LE || 16 data).
        uint8_t rmac_in[2 + sizeof(MfUltralightPageReadCommandData)];
        rmac_in[0] = (ctr + 1) & 0xFF;
        rmac_in[1] = ((ctr + 1) >> 8) & 0xFF;
        memcpy(rmac_in + 2, resp, sizeof(MfUltralightPageReadCommandData));
        uint8_t rmac16[MF_ULTRALIGHT_AES_BLOCK_SIZE], rmac8[MF_ULTRALIGHT_AES_CMAC_SIZE];
        mf_ultralight_aes_cmac(instance->aes_cmac.session_key, rmac_in, sizeof(rmac_in), rmac16);
        mf_ultralight_aes_cmac8(rmac16, rmac8);
        if(memcmp(
               rmac8,
               resp + sizeof(MfUltralightPageReadCommandData),
               MF_ULTRALIGHT_AES_CMAC_SIZE) != 0) {
            ret = MfUltralightErrorProtocol;
            break;
        }

        memcpy(data, resp, sizeof(MfUltralightPageReadCommandData));
        instance->aes_cmac.counter = ctr + 2;
    } while(false);

    return ret;
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
