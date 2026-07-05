#include "mf_plus_poller_i.h"

#include <furi.h>
#include <furi_hal_random.h>
#include <nfc/protocols/iso14443_4a/iso14443_4a_poller.h>

#include "mf_plus_i.h"
#include "mf_plus_crypto.h"

#define TAG "MfPlusPoller"

MfPlusError mf_plus_process_error(Iso14443_4aError error) {
    switch(error) {
    case Iso14443_4aErrorNone:
        return MfPlusErrorNone;
    case Iso14443_4aErrorNotPresent:
        return MfPlusErrorNotPresent;
    case Iso14443_4aErrorTimeout:
        return MfPlusErrorTimeout;
    default:
        return MfPlusErrorProtocol;
    }
}

MfPlusError mf_plus_process_status_code(uint8_t status_code) {
    switch(status_code) {
    case NXP_NATIVE_COMMAND_STATUS_OPERATION_OK:
        return MfPlusErrorNone;
    default:
        return MfPlusErrorProtocol;
    }
}

MfPlusError mf_plus_poller_send_chunks(
    MfPlusPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer) {
    furi_assert(instance);

    NxpNativeCommandStatus status_code = NXP_NATIVE_COMMAND_STATUS_OPERATION_OK;
    Iso14443_4aError iso14443_4a_error = nxp_native_command_iso14443_4a_poller(
        instance->iso14443_4a_poller,
        &status_code,
        tx_buffer,
        rx_buffer,
        NxpNativeCommandModePlain,
        instance->tx_buffer,
        instance->rx_buffer);

    if(iso14443_4a_error != Iso14443_4aErrorNone) {
        return mf_plus_process_error(iso14443_4a_error);
    }

    return mf_plus_process_status_code(status_code);
}

// WritePerso (0xA8) to the intentionally-invalid block 0x9090: the block is always rejected (no
// write happens), only the status byte varies by card state -- the discriminator PM3 uses in
// `hf mfp info`. Classified in mf_plus_poller_probe_security_level below.
#define MF_PLUS_CMD_WRITE_PERSO       (0xA8)
#define MF_PLUS_WRITE_PERSO_PROBE_LEN (1 + 2 + 16) // cmd + invalid block addr + 16 data bytes

MfPlusError
    mf_plus_poller_probe_security_level(MfPlusPoller* instance, MfPlusProbeResult* result) {
    furi_check(instance);
    furi_check(result);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_PLUS_CMD_WRITE_PERSO);
    bit_buffer_append_byte(instance->input_buffer, 0x90); // block 0x9090 (little-endian), invalid
    bit_buffer_append_byte(instance->input_buffer, 0x90);
    for(size_t i = 0; i < MF_PLUS_WRITE_PERSO_PROBE_LEN - 3; i++) {
        bit_buffer_append_byte(instance->input_buffer, 0x00);
    }

    NxpNativeCommandStatus status = NXP_NATIVE_COMMAND_STATUS_OPERATION_OK;
    Iso14443_4aError error = nxp_native_command_iso14443_4a_poller(
        instance->iso14443_4a_poller,
        &status,
        instance->input_buffer,
        instance->result_buffer,
        NxpNativeCommandModePlain,
        instance->tx_buffer,
        instance->rx_buffer);
    if(error != Iso14443_4aErrorNone) {
        return mf_plus_process_error(error);
    }

    // MIFARE Plus answers WritePerso with a single status byte: 0x09 = invalid block rejected (still
    // in SL0 personalization), 0x06/0x0B = the documented SL3 rejections. Treat any other reply
    // (DESFire status words, length error, ...) as not-a-Plus so the caller keeps its DESFire-safe
    // fallback -- no real DESFire status code is 0x06/0x09/0x0B. (PM3 hf mfp info.)
    FURI_LOG_D(TAG, "SL probe (SAK 20) WritePerso status 0x%02X", status);
    switch(status) {
    case 0x09:
        *result = MfPlusProbeResultSl0;
        break;
    case 0x06:
    case 0x0B:
        *result = MfPlusProbeResultSl3;
        break;
    default:
        *result = MfPlusProbeResultNotPlus;
        break;
    }
    return MfPlusErrorNone;
}

MfPlusError mf_plus_poller_read_version(MfPlusPoller* instance, MfPlusVersion* data) {
    furi_check(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_append_byte(instance->input_buffer, MF_PLUS_CMD_GET_VERSION);
    // Clear the result buffer first: nxp_native_command only clears it once its first frame
    // succeeds, so a first-frame failure could leave stale bytes the trust check below would accept.
    bit_buffer_reset(instance->result_buffer);

    MfPlusError error =
        mf_plus_poller_send_chunks(instance, instance->input_buffer, instance->result_buffer);

    // A genuine Plus EV1/EV2 returns the full 28-byte GetVersion block but ends the native frame with
    // a non-zero status (unlike DESFire), so send_chunks flags MfPlusErrorProtocol on a complete,
    // correct reply. Trust it when it parses as an NXP Plus (vendor 0x04, family nibble 0x02);
    // otherwise fail so the caller falls back to ATS (EV1/EV2 share the Plus-X ATS, so GetVersion is
    // their only tell).
    if(mf_plus_version_parse(data, instance->result_buffer) == MfPlusErrorNone &&
       data->hw_vendor == 0x04 && (data->hw_type & 0x0F) == 0x02) {
        return MfPlusErrorNone;
    }

    return (error != MfPlusErrorNone) ? error : MfPlusErrorProtocol;
}

/* ---- SL3 secure messaging ---- */

#define MF_PLUS_CMD_AUTH_FIRST    (0x70)
#define MF_PLUS_CMD_AUTH_CONTINUE (0x72)
#define MF_PLUS_CMD_READ_ENC      (0x31)
#define MF_PLUS_CMD_READ_SIG      (0x3C)
#define MF_PLUS_STATUS_OK         (0x90)

// Raw MFP command over ISO14443-4 I-blocks. SL3 auth/read/signature frames are not
// nxp-native/AF-wrapped (they carry their own status byte), so they go straight over
// iso14443_4a_poller_send_block, unlike GetVersion / the WritePerso probe above.
static MfPlusError mf_plus_poller_send_raw(
    MfPlusPoller* instance,
    const uint8_t* cmd,
    size_t cmd_len,
    uint8_t* resp,
    size_t* resp_len,
    size_t resp_capacity) {
    bit_buffer_reset(instance->input_buffer);
    bit_buffer_copy_bytes(instance->input_buffer, cmd, cmd_len);
    bit_buffer_reset(instance->result_buffer);

    Iso14443_4aError error = iso14443_4a_poller_send_block(
        instance->iso14443_4a_poller, instance->input_buffer, instance->result_buffer);
    if(error != Iso14443_4aErrorNone) {
        return mf_plus_process_error(error);
    }

    size_t len = bit_buffer_get_size_bytes(instance->result_buffer);
    if(len > resp_capacity) {
        FURI_LOG_W(TAG, "Truncating %zu-byte response to %zu", len, resp_capacity);
        len = resp_capacity;
    }
    memcpy(resp, bit_buffer_get_data(instance->result_buffer), len);
    *resp_len = len;
    return MfPlusErrorNone;
}

MfPlusError mf_plus_poller_authenticate_key_id(
    MfPlusPoller* instance,
    uint16_t key_id,
    const MfPlusKey* key,
    MfPlusPollerSession* session) {
    furi_check(instance);
    furi_check(key);
    furi_check(session);

    // Part 1: request E(key, RndB). The card answers regardless of whether the key is correct;
    // the key is only proven in Part 2.
    const uint8_t cmd1[4] = {
        MF_PLUS_CMD_AUTH_FIRST, (uint8_t)(key_id & 0xFF), (uint8_t)(key_id >> 8), 0x00};
    uint8_t resp[64];
    size_t resp_len = 0;
    MfPlusError error =
        mf_plus_poller_send_raw(instance, cmd1, sizeof(cmd1), resp, &resp_len, sizeof(resp));
    if(error != MfPlusErrorNone) return error;
    if(resp_len < 1 + MF_PLUS_AES_BLOCK_SIZE || resp[0] != MF_PLUS_STATUS_OK) {
        return MfPlusErrorProtocol;
    }

    uint8_t rnd_b[MF_PLUS_AES_BLOCK_SIZE];
    mf_plus_crypto_ecb_decrypt(key->data, &resp[1], rnd_b);

    // Our RndA, and rrot(RndA) (rotate right one byte) — the value the card echoes and keys off.
    uint8_t rnd_a[MF_PLUS_AES_BLOCK_SIZE];
    furi_hal_random_fill_buf(rnd_a, sizeof(rnd_a));
    uint8_t rnd_a_rrot[MF_PLUS_AES_BLOCK_SIZE];
    rnd_a_rrot[0] = rnd_a[MF_PLUS_AES_BLOCK_SIZE - 1];
    memcpy(&rnd_a_rrot[1], rnd_a, MF_PLUS_AES_BLOCK_SIZE - 1);

    uint8_t rnd_b_rot[MF_PLUS_AES_BLOCK_SIZE];
    mf_plus_crypto_rotate_left(rnd_b, rnd_b_rot);

    // Part 2: token = AES-CBC(key, iv=0, rrot(RndA) || rol(RndB)).
    uint8_t plain[2 * MF_PLUS_AES_BLOCK_SIZE];
    memcpy(&plain[0], rnd_a_rrot, MF_PLUS_AES_BLOCK_SIZE);
    memcpy(&plain[MF_PLUS_AES_BLOCK_SIZE], rnd_b_rot, MF_PLUS_AES_BLOCK_SIZE);

    const uint8_t zero_iv[MF_PLUS_AES_BLOCK_SIZE] = {0};
    uint8_t cmd2[1 + 2 * MF_PLUS_AES_BLOCK_SIZE];
    cmd2[0] = MF_PLUS_CMD_AUTH_CONTINUE;
    mf_plus_crypto_cbc_encrypt(key->data, zero_iv, plain, &cmd2[1], sizeof(plain));

    error = mf_plus_poller_send_raw(instance, cmd2, sizeof(cmd2), resp, &resp_len, sizeof(resp));
    if(error != MfPlusErrorNone) return error;
    if(resp_len < 1 + 2 * MF_PLUS_AES_BLOCK_SIZE || resp[0] != MF_PLUS_STATUS_OK) {
        return MfPlusErrorAuth;
    }

    // Response = E(TI(4) || RndA'(16) || caps). Decrypt and verify the RndA echo (the auth proof;
    // a wrong key fails to reproduce it). Accept the rotation variants the fork validated.
    uint8_t dec[2 * MF_PLUS_AES_BLOCK_SIZE];
    mf_plus_crypto_cbc_decrypt(key->data, zero_iv, &resp[1], dec, sizeof(dec));

    uint8_t rnd_a_rot[MF_PLUS_AES_BLOCK_SIZE];
    mf_plus_crypto_rotate_left(rnd_a, rnd_a_rot);
    if(memcmp(&dec[4], rnd_a_rrot, MF_PLUS_AES_BLOCK_SIZE) != 0 &&
       memcmp(&dec[4], rnd_a_rot, MF_PLUS_AES_BLOCK_SIZE) != 0 &&
       memcmp(&dec[4], rnd_a, MF_PLUS_AES_BLOCK_SIZE) != 0) {
        return MfPlusErrorAuth;
    }

    memcpy(session->ti, &dec[0], 4);
    session->r_ctr = 0;
    session->w_ctr = 0;
    // Derive the session keys from rrot(RndA) (what the card treats as RndA) and RndB.
    mf_plus_crypto_derive_session_keys(
        key->data, rnd_a_rrot, rnd_b, session->k_enc, session->k_mac);

    return MfPlusErrorNone;
}

MfPlusError mf_plus_poller_authenticate(
    MfPlusPoller* instance,
    uint8_t sector,
    MfPlusKeyType key_type,
    const MfPlusKey* key,
    MfPlusPollerSession* session) {
    // Sector keys live in the 0x40xx keyspace: 0x4000 + 2*sector + key_type.
    const uint16_t key_id = 0x4000U + ((uint16_t)sector << 1) + (uint8_t)key_type;
    return mf_plus_poller_authenticate_key_id(instance, key_id, key, session);
}

MfPlusError mf_plus_poller_read_encrypted_block(
    MfPlusPoller* instance,
    uint8_t block_low,
    uint8_t block_high,
    MfPlusPollerSession* session,
    MfPlusBlock* out) {
    furi_check(instance);
    furi_check(session);
    furi_check(out);

    // Command MAC over the 2-byte little-endian block address {low, high} + count, with the
    // pre-increment R_ctr. Data blocks use high = 0x00; config blocks (0xB0xx) use high = 0xB0.
    const uint8_t payload[3] = {block_low, block_high, 0x01};
    uint8_t cmd_mac[MF_PLUS_MAC_SIZE];
    mf_plus_crypto_calculate_mac(
        session->k_mac,
        MF_PLUS_CMD_READ_ENC,
        session->r_ctr,
        session->ti,
        payload,
        sizeof(payload),
        cmd_mac);

    uint8_t cmd[4 + MF_PLUS_MAC_SIZE];
    cmd[0] = MF_PLUS_CMD_READ_ENC;
    cmd[1] = block_low;
    cmd[2] = block_high;
    cmd[3] = 0x01;
    memcpy(&cmd[4], cmd_mac, MF_PLUS_MAC_SIZE);

    uint8_t resp[64];
    size_t resp_len = 0;
    MfPlusError error =
        mf_plus_poller_send_raw(instance, cmd, sizeof(cmd), resp, &resp_len, sizeof(resp));
    if(error != MfPlusErrorNone) return error;
    // status(1) + enc(16) + mac(8)
    if(resp_len < 1 + MF_PLUS_BLOCK_SIZE + MF_PLUS_MAC_SIZE || resp[0] != MF_PLUS_STATUS_OK) {
        return MfPlusErrorProtocol;
    }

    // The response MAC and the decryption IV both use the post-increment counter.
    session->r_ctr++;

    uint8_t mac_input[sizeof(payload) + MF_PLUS_BLOCK_SIZE];
    memcpy(&mac_input[0], payload, sizeof(payload));
    memcpy(&mac_input[sizeof(payload)], &resp[1], MF_PLUS_BLOCK_SIZE);
    uint8_t resp_mac[MF_PLUS_MAC_SIZE];
    mf_plus_crypto_calculate_mac(
        session->k_mac,
        MF_PLUS_STATUS_OK,
        session->r_ctr,
        session->ti,
        mac_input,
        sizeof(mac_input),
        resp_mac);
    if(memcmp(&resp[1 + MF_PLUS_BLOCK_SIZE], resp_mac, MF_PLUS_MAC_SIZE) != 0) {
        // Distinct from an access-denied status byte: a MAC mismatch means r_ctr is now desynced
        // (it was already advanced above), so the caller must abort the sector rather than retry.
        return MfPlusErrorAuth;
    }

    uint8_t iv[MF_PLUS_AES_BLOCK_SIZE];
    mf_plus_crypto_build_read_iv(session->ti, session->r_ctr, iv);
    mf_plus_crypto_cbc_decrypt(session->k_enc, iv, &resp[1], out->data, MF_PLUS_BLOCK_SIZE);

    return MfPlusErrorNone;
}

MfPlusError
    mf_plus_poller_read_signature(MfPlusPoller* instance, uint8_t* signature, bool* present) {
    furi_check(instance);
    furi_check(signature);
    furi_check(present);

    *present = false;

    const uint8_t cmd[2] = {MF_PLUS_CMD_READ_SIG, 0x00};
    uint8_t resp[64];
    size_t resp_len = 0;
    MfPlusError error =
        mf_plus_poller_send_raw(instance, cmd, sizeof(cmd), resp, &resp_len, sizeof(resp));
    if(error != MfPlusErrorNone) return error;

    // Cards vary: some prefix a 0x90 status byte, others return the raw 56 signature bytes.
    const uint8_t* sig = NULL;
    if(resp_len >= 1 + MF_PLUS_SIGNATURE_SIZE && resp[0] == MF_PLUS_STATUS_OK) {
        sig = &resp[1];
    } else if(resp_len >= MF_PLUS_SIGNATURE_SIZE) {
        sig = &resp[0];
    }

    if(sig != NULL) {
        memcpy(signature, sig, MF_PLUS_SIGNATURE_SIZE);
        *present = true;
    }
    // A card with no signature (EV0 / unsupported) is not an error.
    return MfPlusErrorNone;
}
