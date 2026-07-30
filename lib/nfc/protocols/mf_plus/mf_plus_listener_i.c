#include "mf_plus_listener_i.h"
#include "mf_plus_i.h"
#include "mf_plus_crypto.h"

#include <furi.h>
#include <furi_hal_random.h>

#define TAG "MfPlusListener"

#define MF_PLUS_CMD_READ_ENC      (0x31)
#define MF_PLUS_CMD_READ_PLAIN    (0x33)
#define MF_PLUS_CMD_WRITE_ENC     (0xA1)
#define MF_PLUS_CMD_WRITE_PLAIN   (0xA3)
#define MF_PLUS_STATUS_OK         (0x90)
#define MF_PLUS_CONFIG_BLOCK_HIGH (0xB0)

// SL3 rejection status: "command not available at the current card state". A Plus in SL3 answers
// this to the SL0-only WritePerso command, which is how a reader's info probe confirms Plus + SL3.
#define MF_PLUS_STATUS_CMD_UNAVAILABLE (0x0B)

// Authentication error status. A real card answers this to a wrong key or a malformed auth step
// rather than going silent, so a reader -- or a key check like PM3 `hf mfp chk` -- cleanly advances
// to the next key instead of desyncing when it times out waiting for a response.
#define MF_PLUS_STATUS_AUTH_ERROR (0x06)

// GetVersion chaining status: "additional data frame expected" -- the card sends it on every frame
// but the last. The final frame ends 0x90 (OK), NOT 0x00 like DESFire (captured from a real EV1).
#define MF_PLUS_STATUS_ADDITIONAL_FRAME (0xAF)
// ISO7816 status-word high byte: a wrapped native command's response trails 0x91 + <native status>.
#define MF_PLUS_ISO_SW1                 (0x91)

static void mf_plus_listener_send(MfPlusListener* instance, const uint8_t* data, size_t len) {
    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_copy_bytes(instance->tx_buffer, data, len);
    iso14443_4a_listener_send_block(instance->iso14443_4a_listener, instance->tx_buffer);
}

// Answer with a single NXP native-command status byte. A real card NAKs an unsupported or
// unavailable command (e.g. 0x0B "command not available") rather than going silent, so the reader
// gets a frame instead of a timeout -- which is what lets an info scan complete cleanly.
static void mf_plus_listener_send_status(MfPlusListener* instance, uint8_t status) {
    mf_plus_listener_send(instance, &status, sizeof(status));
}

// Resolve the AES key for a 16-bit key address, from the keys recovered into MfPlusData. Sector
// keys live at 0x4000 + 2*sector + key_type; admin keys at the 0x90xx addresses (enum skips 0x9002).
// Returns false when the key was never recovered (the card would not be able to authenticate it).
static bool
    mf_plus_listener_resolve_key(MfPlusListener* instance, uint16_t key_id, MfPlusKey* key) {
    const MfPlusData* data = instance->data;
    const uint8_t sector_count = mf_plus_get_sector_count(data->size);

    if(key_id >= 0x4000U && key_id < 0x4000U + 2U * sector_count) {
        const uint8_t sector = (uint8_t)((key_id - 0x4000U) >> 1);
        const MfPlusKeyType key_type = (MfPlusKeyType)((key_id - 0x4000U) & 1U);
        if(!mf_plus_is_key_found(data, sector, key_type)) return false;
        *key = (key_type == MfPlusKeyTypeB) ? data->key_b[sector] : data->key_a[sector];
        return true;
    }

    MfPlusAdminKeyType admin_type;
    if(!mf_plus_admin_key_type_from_address(key_id, &admin_type)) return false;
    if(!mf_plus_is_admin_key_found(data, admin_type)) return false;
    *key = data->admin_key[admin_type];
    return true;
}

// Resolve the plaintext of a requested block: data blocks (high 0x00) or config blocks (high 0xB0).
// Returns false if the address is out of range or that block was never recovered.
static bool mf_plus_listener_resolve_block(
    MfPlusListener* instance,
    uint8_t block_low,
    uint8_t block_high,
    MfPlusBlock* out) {
    const MfPlusData* data = instance->data;

    if(block_high == 0x00) {
        if(block_low >= mf_plus_get_block_count(data->size)) return false;
        if(!mf_plus_is_block_read(data, block_low)) return false;
        *out = data->block[block_low];
        return true;
    }
    if(block_high == MF_PLUS_CONFIG_BLOCK_HIGH) {
        if(block_low >= MF_PLUS_CONFIG_BLOCK_NUM) return false;
        if(!mf_plus_is_config_block_read(data, block_low)) return false;
        *out = data->config_block[block_low];
        return true;
    }
    return false;
}

NfcCommand mf_plus_listener_auth_first_handler(MfPlusListener* instance, const BitBuffer* rx) {
    // [AUTH_FIRST, key_id_lo, key_id_hi, 0x00]
    if(bit_buffer_get_size_bytes(rx) < 4) {
        FURI_LOG_D(TAG, "AUTH_FIRST frame too short");
        mf_plus_listener_send_status(instance, MF_PLUS_STATUS_AUTH_ERROR);
        return NfcCommandContinue;
    }
    const uint16_t key_id = bit_buffer_get_byte(rx, 1) |
                            ((uint16_t)bit_buffer_get_byte(rx, 2) << 8);

    // A fresh AUTH_FIRST always restarts authentication.
    mf_plus_listener_reset_session(instance);

    if(!mf_plus_listener_resolve_key(instance, key_id, &instance->auth_key)) {
        // We never recovered this sector's key. A real card still answers AUTH_FIRST (it holds every
        // key) and only rejects a wrong key at AUTH_CONTINUE -- and some readers (e.g. a U-Prox
        // encoder) abort the whole exchange if AUTH_FIRST itself is NAKed. So run the handshake with
        // a random placeholder key: phase 1 completes normally, then the reader's phase-2 token can't
        // echo our RndB, so we NAK at AUTH_CONTINUE exactly as a real card rejects a wrong key.
        FURI_LOG_D(TAG, "AUTH_FIRST for unrecovered key 0x%04X (placeholder)", key_id);
        furi_hal_random_fill_buf(instance->auth_key.data, MF_PLUS_KEY_SIZE);
    }

    // Card picks RndB and returns E(key, RndB); the poller ECB-decrypts it with the same key.
    furi_hal_random_fill_buf(instance->rnd_b, sizeof(instance->rnd_b));

    uint8_t resp[1 + MF_PLUS_AES_BLOCK_SIZE];
    resp[0] = MF_PLUS_STATUS_OK;
    mf_plus_crypto_ecb_encrypt(instance->auth_key.data, instance->rnd_b, &resp[1]);

    mf_plus_listener_send(instance, resp, sizeof(resp));
    instance->state = MfPlusListenerStateAuthFirstDone;
    return NfcCommandContinue;
}

NfcCommand mf_plus_listener_auth_continue_handler(MfPlusListener* instance, const BitBuffer* rx) {
    // [AUTH_CONTINUE, E(key, iv=0, rrot(RndA) || rol(RndB)) (32)]
    if(instance->state != MfPlusListenerStateAuthFirstDone) {
        FURI_LOG_D(TAG, "AUTH_CONTINUE without a prior AUTH_FIRST (state=%d)", instance->state);
        mf_plus_listener_send_status(instance, MF_PLUS_STATUS_AUTH_ERROR);
        return NfcCommandContinue;
    }
    if(bit_buffer_get_size_bytes(rx) < 1 + 2 * MF_PLUS_AES_BLOCK_SIZE) {
        FURI_LOG_D(TAG, "AUTH_CONTINUE frame too short");
        mf_plus_listener_send_status(instance, MF_PLUS_STATUS_AUTH_ERROR);
        return NfcCommandContinue;
    }

    const uint8_t zero_iv[MF_PLUS_AES_BLOCK_SIZE] = {0};
    uint8_t dec[2 * MF_PLUS_AES_BLOCK_SIZE];
    mf_plus_crypto_cbc_decrypt(
        instance->auth_key.data, zero_iv, bit_buffer_get_data(rx) + 1, dec, sizeof(dec));

    // dec = rrot(RndA) || rol(RndB). The RndB echo proves the reader holds our key.
    uint8_t rnd_b_rot[MF_PLUS_AES_BLOCK_SIZE];
    mf_plus_crypto_rotate_left(instance->rnd_b, rnd_b_rot);
    if(memcmp(&dec[MF_PLUS_AES_BLOCK_SIZE], rnd_b_rot, MF_PLUS_AES_BLOCK_SIZE) != 0) {
        // Wrong key: the reader's token doesn't echo our RndB. NAK so a key check (PM3 `hf mfp chk`)
        // records a miss and advances instead of hanging on a timeout, which would desync its scan.
        FURI_LOG_D(TAG, "AUTH_CONTINUE RndB echo mismatch");
        mf_plus_listener_reset_session(instance);
        mf_plus_listener_send_status(instance, MF_PLUS_STATUS_AUTH_ERROR);
        return NfcCommandContinue;
    }

    const uint8_t* rnd_a_rrot = &dec[0]; // the value the card treats as RndA (rrot(RndA))

    // Establish the session: fresh TI, counters reset, session keys from rrot(RndA) + RndB (the
    // same inputs the poller derives from).
    MfPlusListenerSession* s = &instance->session;
    furi_hal_random_fill_buf(s->ti, sizeof(s->ti));
    s->r_ctr = 0;
    s->w_ctr = 0;
    mf_plus_crypto_derive_session_keys(
        instance->auth_key.data, rnd_a_rrot, instance->rnd_b, s->k_enc, s->k_mac);

    // Response = E(key, iv=0, TI(4) || RndA'(16) || caps). The reader expects rol(RndA). One
    // rotate-left of the extracted value satisfies both readers: for this port's poller (which put
    // rrot(RndA) here) it yields RndA (its 3rd accepted variant), and for a PM3-style reader (which
    // put RndA here) it yields rol(RndA). The cap bytes after are unused by the poller.
    uint8_t rnd_a_echo[MF_PLUS_AES_BLOCK_SIZE];
    mf_plus_crypto_rotate_left(rnd_a_rrot, rnd_a_echo);

    uint8_t plain[2 * MF_PLUS_AES_BLOCK_SIZE] = {0};
    memcpy(&plain[0], s->ti, 4);
    memcpy(&plain[4], rnd_a_echo, MF_PLUS_AES_BLOCK_SIZE);

    uint8_t resp[1 + 2 * MF_PLUS_AES_BLOCK_SIZE];
    resp[0] = MF_PLUS_STATUS_OK;
    mf_plus_crypto_cbc_encrypt(instance->auth_key.data, zero_iv, plain, &resp[1], sizeof(plain));

    mf_plus_listener_send(instance, resp, sizeof(resp));
    instance->state = MfPlusListenerStateAuthenticated;
    return NfcCommandContinue;
}

NfcCommand
    mf_plus_listener_read_handler(MfPlusListener* instance, const BitBuffer* rx, bool plain) {
    // [READ, block_lo, block_hi, count, cmd_mac(8)]. Encrypted (0x31) and plaintext (0x33) reads
    // are identical except for the opcode fed into the MAC and whether the data is encrypted.
    if(instance->state != MfPlusListenerStateAuthenticated) {
        FURI_LOG_D(TAG, "READ before authentication (state=%d)", instance->state);
        return NfcCommandContinue;
    }
    if(bit_buffer_get_size_bytes(rx) < 4 + MF_PLUS_MAC_SIZE) {
        FURI_LOG_D(TAG, "READ frame too short");
        return NfcCommandContinue;
    }

    const uint8_t block_low = bit_buffer_get_byte(rx, 1);
    const uint8_t block_high = bit_buffer_get_byte(rx, 2);
    const uint8_t count = bit_buffer_get_byte(rx, 3);
    if(count != 0x01) {
        FURI_LOG_D(TAG, "READ count %u unsupported (single-block only)", count);
        return NfcCommandContinue;
    }

    MfPlusListenerSession* s = &instance->session;
    const uint8_t opcode = plain ? MF_PLUS_CMD_READ_PLAIN : MF_PLUS_CMD_READ_ENC;

    // Verify the command MAC (pre-increment r_ctr), same layout the poller builds.
    const uint8_t payload[3] = {block_low, block_high, count};
    uint8_t cmd_mac[MF_PLUS_MAC_SIZE];
    mf_plus_crypto_calculate_mac(
        s->k_mac, opcode, s->r_ctr, s->ti, payload, sizeof(payload), cmd_mac);
    if(memcmp(bit_buffer_get_data(rx) + 4, cmd_mac, MF_PLUS_MAC_SIZE) != 0) {
        FURI_LOG_D(TAG, "READ command MAC mismatch");
        return NfcCommandContinue;
    }

    MfPlusBlock block;
    if(!mf_plus_listener_resolve_block(instance, block_low, block_high, &block)) {
        FURI_LOG_D(TAG, "READ of unavailable block hi=0x%02X lo=%u", block_high, block_low);
        return NfcCommandContinue;
    }

    // The response MAC always uses the post-increment counter; an encrypted (0x31) read's IV too.
    s->r_ctr++;

    // Data travels as ciphertext for 0x31, plaintext for 0x33. The response MAC covers the bytes as
    // transmitted, so it is computed over whichever form goes on the wire.
    uint8_t data[MF_PLUS_BLOCK_SIZE];
    if(plain) {
        memcpy(data, block.data, MF_PLUS_BLOCK_SIZE);
    } else {
        uint8_t iv[MF_PLUS_AES_BLOCK_SIZE];
        mf_plus_crypto_build_read_iv(s->ti, s->r_ctr, s->w_ctr, iv);
        mf_plus_crypto_cbc_encrypt(s->k_enc, iv, block.data, data, MF_PLUS_BLOCK_SIZE);
    }

    uint8_t mac_input[sizeof(payload) + MF_PLUS_BLOCK_SIZE];
    memcpy(&mac_input[0], payload, sizeof(payload));
    memcpy(&mac_input[sizeof(payload)], data, MF_PLUS_BLOCK_SIZE);
    uint8_t resp_mac[MF_PLUS_MAC_SIZE];
    mf_plus_crypto_calculate_mac(
        s->k_mac, MF_PLUS_STATUS_OK, s->r_ctr, s->ti, mac_input, sizeof(mac_input), resp_mac);

    uint8_t resp[1 + MF_PLUS_BLOCK_SIZE + MF_PLUS_MAC_SIZE];
    resp[0] = MF_PLUS_STATUS_OK;
    memcpy(&resp[1], data, MF_PLUS_BLOCK_SIZE);
    memcpy(&resp[1 + MF_PLUS_BLOCK_SIZE], resp_mac, MF_PLUS_MAC_SIZE);

    mf_plus_listener_send(instance, resp, sizeof(resp));
    return NfcCommandContinue;
}

// Store a 16-byte plaintext a reader just wrote, resolving the 2-byte address to a sector key
// (0x40xx), an admin key (0x90xx), a data block (high 0x00) or a config block (0xB0xx). Updates the
// matching MfPlusData field and its found/read mask so the emulate-exit diff persists it to the
// .shd. Returns false for an address this card doesn't expose (the caller NAKs).
static bool mf_plus_listener_store_write(
    MfPlusListener* instance,
    uint16_t block_addr,
    const uint8_t* plain) {
    MfPlusData* data = instance->data;
    const uint8_t sector_count = mf_plus_get_sector_count(data->size);

    if(block_addr >= 0x4000U && block_addr < 0x4000U + 2U * sector_count) {
        const uint8_t sector = (uint8_t)((block_addr - 0x4000U) >> 1);
        const MfPlusKeyType key_type = (MfPlusKeyType)((block_addr - 0x4000U) & 1U);
        MfPlusKey key;
        memcpy(key.data, plain, MF_PLUS_KEY_SIZE);
        mf_plus_set_key_found(data, sector, key_type, &key);
        return true;
    }

    MfPlusAdminKeyType admin_type;
    if(mf_plus_admin_key_type_from_address(block_addr, &admin_type)) {
        MfPlusKey key;
        memcpy(key.data, plain, MF_PLUS_KEY_SIZE);
        mf_plus_set_admin_key_found(data, admin_type, &key);
        return true;
    }

    if(block_addr < mf_plus_get_block_count(data->size)) {
        MfPlusBlock block;
        memcpy(block.data, plain, MF_PLUS_BLOCK_SIZE);
        mf_plus_set_block_read(data, block_addr, &block);
        return true;
    }

    if((block_addr >> 8) == MF_PLUS_CONFIG_BLOCK_HIGH &&
       (block_addr & 0xFF) < MF_PLUS_CONFIG_BLOCK_NUM) {
        MfPlusBlock block;
        memcpy(block.data, plain, MF_PLUS_BLOCK_SIZE);
        mf_plus_set_config_block_read(data, (uint8_t)(block_addr & 0xFF), &block);
        return true;
    }

    return false;
}

NfcCommand
    mf_plus_listener_write_handler(MfPlusListener* instance, const BitBuffer* rx, bool encrypted) {
    // [WRITE, BNr_lo, BNr_hi, data(16), cmd_mac(8)]. 0xA1 = encrypted (keys; the block is
    // CBC-decrypted with the write IV), 0xA3 = plain (data blocks carry plaintext). Both MAC the
    // command; a reader writes only after authenticating.
    if(instance->state != MfPlusListenerStateAuthenticated) {
        FURI_LOG_D(TAG, "WRITE before authentication (state=%d)", instance->state);
        mf_plus_listener_send_status(instance, MF_PLUS_STATUS_CMD_UNAVAILABLE);
        return NfcCommandContinue;
    }
    if(bit_buffer_get_size_bytes(rx) < 3 + MF_PLUS_BLOCK_SIZE + MF_PLUS_MAC_SIZE) {
        FURI_LOG_D(TAG, "WRITE frame too short");
        mf_plus_listener_send_status(instance, MF_PLUS_STATUS_CMD_UNAVAILABLE);
        return NfcCommandContinue;
    }

    const uint8_t block_low = bit_buffer_get_byte(rx, 1);
    const uint8_t block_high = bit_buffer_get_byte(rx, 2);
    const uint16_t block_addr = block_low | ((uint16_t)block_high << 8);
    const uint8_t* wire_data = bit_buffer_get_data(rx) + 3; // 16 data bytes as transmitted
    const uint8_t* received_mac = bit_buffer_get_data(rx) + 3 + MF_PLUS_BLOCK_SIZE;

    MfPlusListenerSession* s = &instance->session;
    const uint8_t opcode = encrypted ? MF_PLUS_CMD_WRITE_ENC : MF_PLUS_CMD_WRITE_PLAIN;

    // Verify the command MAC (pre-increment w_ctr) over {BNr_lo, BNr_hi, data-as-transmitted}.
    uint8_t mac_input[2 + MF_PLUS_BLOCK_SIZE];
    mac_input[0] = block_low;
    mac_input[1] = block_high;
    memcpy(&mac_input[2], wire_data, MF_PLUS_BLOCK_SIZE);
    uint8_t cmd_mac[MF_PLUS_MAC_SIZE];
    mf_plus_crypto_calculate_mac(
        s->k_mac, opcode, s->w_ctr, s->ti, mac_input, sizeof(mac_input), cmd_mac);
    if(memcmp(received_mac, cmd_mac, MF_PLUS_MAC_SIZE) != 0) {
        FURI_LOG_D(TAG, "WRITE command MAC mismatch");
        mf_plus_listener_send_status(instance, MF_PLUS_STATUS_AUTH_ERROR);
        return NfcCommandContinue;
    }

    // Recover the plaintext: 0xA1 CBC-decrypts with the write IV (pre-increment w_ctr); 0xA3 already
    // carries plaintext.
    uint8_t plain[MF_PLUS_BLOCK_SIZE];
    if(encrypted) {
        uint8_t iv[MF_PLUS_AES_BLOCK_SIZE];
        mf_plus_crypto_build_write_iv(s->ti, s->r_ctr, s->w_ctr, iv);
        mf_plus_crypto_cbc_decrypt(s->k_enc, iv, wire_data, plain, MF_PLUS_BLOCK_SIZE);
    } else {
        memcpy(plain, wire_data, MF_PLUS_BLOCK_SIZE);
    }

    if(!mf_plus_listener_store_write(instance, block_addr, plain)) {
        FURI_LOG_D(TAG, "WRITE to unsupported address 0x%04X", block_addr);
        mf_plus_listener_send_status(instance, MF_PLUS_STATUS_CMD_UNAVAILABLE);
        return NfcCommandContinue;
    }

    // Write accepted: advance w_ctr, then MAC the OK status over the post-increment counter.
    s->w_ctr++;
    uint8_t resp_mac[MF_PLUS_MAC_SIZE];
    mf_plus_crypto_calculate_mac(s->k_mac, MF_PLUS_STATUS_OK, s->w_ctr, s->ti, NULL, 0, resp_mac);

    uint8_t resp[1 + MF_PLUS_MAC_SIZE];
    resp[0] = MF_PLUS_STATUS_OK;
    memcpy(&resp[1], resp_mac, MF_PLUS_MAC_SIZE);
    mf_plus_listener_send(instance, resp, sizeof(resp));
    return NfcCommandContinue;
}

NfcCommand mf_plus_listener_read_signature_handler(MfPlusListener* instance, const BitBuffer* rx) {
    // [READ_SIG, 0x00]. The originality signature is level-independent and needs no auth.
    UNUSED(rx);
    if(!instance->data->signature_present) {
        // No signature to serve (an EV0/S/X part has none): NAK like a card that doesn't support the
        // command, so the reader's info scan gets a response instead of a timeout.
        FURI_LOG_D(TAG, "READ_SIG but card has no originality signature");
        mf_plus_listener_send_status(instance, MF_PLUS_STATUS_CMD_UNAVAILABLE);
        return NfcCommandContinue;
    }

    uint8_t resp[1 + MF_PLUS_SIGNATURE_SIZE];
    resp[0] = MF_PLUS_STATUS_OK;
    memcpy(&resp[1], instance->data->signature, MF_PLUS_SIGNATURE_SIZE);

    mf_plus_listener_send(instance, resp, sizeof(resp));
    return NfcCommandContinue;
}

NfcCommand mf_plus_listener_write_perso_handler(MfPlusListener* instance, const BitBuffer* rx) {
    // The probe fires WritePerso at an invalid block purely to read its rejection status, and a real
    // card needs neither auth nor a particular state to reject it -- so ignore the frame and
    // unconditionally answer the SL3 status. (This listener only ever emulates SL3.)
    UNUSED(rx);
    mf_plus_listener_send_status(instance, MF_PLUS_STATUS_CMD_UNAVAILABLE);
    return NfcCommandContinue;
}

NfcCommand mf_plus_listener_unsupported_handler(MfPlusListener* instance, const BitBuffer* rx) {
    // Any command this listener doesn't implement (non-first auth 0x76, ...). A real card NAKs it
    // rather than staying silent, so answer 0x0B: this keeps a reader's info scan from stalling on
    // "no response", and reading it back still falls through to ATS-based typing.
    UNUSED(rx);
    mf_plus_listener_send_status(instance, MF_PLUS_STATUS_CMD_UNAVAILABLE);
    return NfcCommandContinue;
}

// True once a real GetVersion answer exists: only EV1/EV2 (NXP vendor 0x04, family nibble 0x02)
// carry a version; EV0 parts (S/X/SE) leave it zeroed and must NAK GetVersion like the real chip.
static bool mf_plus_listener_has_version(const MfPlusListener* instance) {
    const MfPlusVersion* v = &instance->data->version;
    return v->hw_vendor == 0x04 && (v->hw_type & 0x0F) == 0x02;
}

// One GetVersion frame: `chunk` (7 HW/SW bytes, or the 14-byte UID/batch/date tail) prefixed by the
// native status (native form: status + data) or suffixed by the ISO7816 status word 0x91 + status
// (wrapped form: data + 91 + status). Reproduces a real EV1's 0x60/0xAF exchange byte-for-byte.
static void mf_plus_listener_send_version_frame(
    MfPlusListener* instance,
    bool wrapped,
    uint8_t status,
    const uint8_t* chunk,
    size_t chunk_len) {
    uint8_t resp[MF_PLUS_BLOCK_SIZE + 2]; // longest chunk is 14 bytes, plus up to 2 status bytes
    size_t len = 0;
    if(wrapped) {
        memcpy(resp, chunk, chunk_len);
        len = chunk_len;
        resp[len++] = MF_PLUS_ISO_SW1;
        resp[len++] = status;
    } else {
        resp[len++] = status;
        memcpy(&resp[len], chunk, chunk_len);
        len += chunk_len;
    }
    mf_plus_listener_send(instance, resp, len);
}

NfcCommand mf_plus_listener_get_version_handler(MfPlusListener* instance, bool wrapped) {
    if(!mf_plus_listener_has_version(instance)) {
        // EV0 (S/X/SE) has no GetVersion; NAK 0x0B like the real chip (this is TagInfo's EV gate).
        instance->get_version_stage = 0;
        FURI_LOG_D(TAG, "GetVersion but card is EV0 (no version)");
        mf_plus_listener_send_status(instance, MF_PLUS_STATUS_CMD_UNAVAILABLE);
        return NfcCommandContinue;
    }
    // MfPlusVersion is a packed 28-byte record (7 HW || 7 SW || 7 UID || 5 batch || week || year), so
    // replaying it in three chunks reproduces the real 0x60 -> 0xAF -> 0xAF exchange. Frame 1 = the 7
    // HW bytes with the "more frames" status.
    const uint8_t* v = (const uint8_t*)&instance->data->version;
    mf_plus_listener_send_version_frame(
        instance, wrapped, MF_PLUS_STATUS_ADDITIONAL_FRAME, &v[0], 7);
    instance->get_version_stage = 1;
    return NfcCommandContinue;
}

NfcCommand mf_plus_listener_get_version_continue_handler(MfPlusListener* instance, bool wrapped) {
    const uint8_t* v = (const uint8_t*)&instance->data->version;
    if(instance->get_version_stage == 1) {
        // Frame 2: the 7 SW bytes, still "more frames".
        mf_plus_listener_send_version_frame(
            instance, wrapped, MF_PLUS_STATUS_ADDITIONAL_FRAME, &v[7], 7);
        instance->get_version_stage = 2;
    } else {
        // Frame 3 (final): UID + batch + production date, ended with 0x90 (OK) -- a real MFP ends
        // non-zero, unlike DESFire's 0x00.
        mf_plus_listener_send_version_frame(instance, wrapped, MF_PLUS_STATUS_OK, &v[14], 14);
        instance->get_version_stage = 0;
    }
    return NfcCommandContinue;
}
