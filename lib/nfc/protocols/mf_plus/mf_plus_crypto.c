#include "mf_plus_crypto.h"

#include <furi.h>
#include <string.h>
#include <stdbool.h>
#include <mbedtls/aes.h>

/* ---- AES-128 primitives (firmware mbedtls, software) ---- */

static void mf_plus_crypto_aes_ecb(
    bool encrypt,
    const uint8_t key[MF_PLUS_AES_KEY_SIZE],
    const uint8_t input[MF_PLUS_AES_BLOCK_SIZE],
    uint8_t output[MF_PLUS_AES_BLOCK_SIZE]) {
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    if(encrypt) {
        furi_check(mbedtls_aes_setkey_enc(&ctx, key, 128) == 0);
        furi_check(mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_ENCRYPT, input, output) == 0);
    } else {
        furi_check(mbedtls_aes_setkey_dec(&ctx, key, 128) == 0);
        furi_check(mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_DECRYPT, input, output) == 0);
    }
    mbedtls_aes_free(&ctx);
}

static void mf_plus_crypto_aes_cbc(
    bool encrypt,
    const uint8_t key[MF_PLUS_AES_KEY_SIZE],
    const uint8_t iv[MF_PLUS_AES_BLOCK_SIZE],
    const uint8_t* input,
    uint8_t* output,
    size_t length) {
    // CBC needs whole 16-byte blocks; a zero/partial length is a caller-contract violation
    // (mbedtls would no-op or error) — fail loud rather than skip or emit crypto silently.
    furi_check(length != 0 && (length % MF_PLUS_AES_BLOCK_SIZE) == 0);

    // mbedtls updates the IV in place; keep the caller's const IV intact.
    uint8_t iv_local[MF_PLUS_AES_BLOCK_SIZE];
    memcpy(iv_local, iv, MF_PLUS_AES_BLOCK_SIZE);

    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    if(encrypt) {
        furi_check(mbedtls_aes_setkey_enc(&ctx, key, 128) == 0);
        furi_check(
            mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, length, iv_local, input, output) ==
            0);
    } else {
        furi_check(mbedtls_aes_setkey_dec(&ctx, key, 128) == 0);
        furi_check(
            mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, length, iv_local, input, output) ==
            0);
    }
    mbedtls_aes_free(&ctx);
}

void mf_plus_crypto_ecb_encrypt(
    const uint8_t key[MF_PLUS_AES_KEY_SIZE],
    const uint8_t input[MF_PLUS_AES_BLOCK_SIZE],
    uint8_t output[MF_PLUS_AES_BLOCK_SIZE]) {
    mf_plus_crypto_aes_ecb(true, key, input, output);
}

void mf_plus_crypto_ecb_decrypt(
    const uint8_t key[MF_PLUS_AES_KEY_SIZE],
    const uint8_t input[MF_PLUS_AES_BLOCK_SIZE],
    uint8_t output[MF_PLUS_AES_BLOCK_SIZE]) {
    mf_plus_crypto_aes_ecb(false, key, input, output);
}

void mf_plus_crypto_cbc_encrypt(
    const uint8_t key[MF_PLUS_AES_KEY_SIZE],
    const uint8_t iv[MF_PLUS_AES_BLOCK_SIZE],
    const uint8_t* input,
    uint8_t* output,
    size_t length) {
    mf_plus_crypto_aes_cbc(true, key, iv, input, output, length);
}

void mf_plus_crypto_cbc_decrypt(
    const uint8_t key[MF_PLUS_AES_KEY_SIZE],
    const uint8_t iv[MF_PLUS_AES_BLOCK_SIZE],
    const uint8_t* input,
    uint8_t* output,
    size_t length) {
    mf_plus_crypto_aes_cbc(false, key, iv, input, output, length);
}

/* ---- AES-CMAC (RFC 4493) on top of AES-ECB ---- */

static void mf_plus_crypto_cmac_shift_left(
    const uint8_t in[MF_PLUS_AES_BLOCK_SIZE],
    uint8_t out[MF_PLUS_AES_BLOCK_SIZE]) {
    uint8_t carry = 0;
    for(int i = MF_PLUS_AES_BLOCK_SIZE - 1; i >= 0; i--) {
        out[i] = (uint8_t)((in[i] << 1) | carry);
        carry = (in[i] >> 7) & 1;
    }
}

static void mf_plus_crypto_cmac_subkeys(
    const uint8_t key[MF_PLUS_AES_KEY_SIZE],
    uint8_t k1[MF_PLUS_AES_BLOCK_SIZE],
    uint8_t k2[MF_PLUS_AES_BLOCK_SIZE]) {
    const uint8_t rb = 0x87;
    uint8_t l[MF_PLUS_AES_BLOCK_SIZE];
    memset(l, 0, sizeof(l));
    mf_plus_crypto_ecb_encrypt(key, l, l);

    mf_plus_crypto_cmac_shift_left(l, k1);
    if(l[0] & 0x80) k1[MF_PLUS_AES_BLOCK_SIZE - 1] ^= rb;

    mf_plus_crypto_cmac_shift_left(k1, k2);
    if(k1[0] & 0x80) k2[MF_PLUS_AES_BLOCK_SIZE - 1] ^= rb;
}

void mf_plus_crypto_cmac(
    const uint8_t key[MF_PLUS_AES_KEY_SIZE],
    const uint8_t* data,
    size_t length,
    uint8_t mac[MF_PLUS_AES_BLOCK_SIZE]) {
    uint8_t k1[MF_PLUS_AES_BLOCK_SIZE], k2[MF_PLUS_AES_BLOCK_SIZE];
    mf_plus_crypto_cmac_subkeys(key, k1, k2);

    size_t n = (length + MF_PLUS_AES_BLOCK_SIZE - 1) / MF_PLUS_AES_BLOCK_SIZE;
    if(n == 0) n = 1;
    const bool last_complete = (length > 0) && (length % MF_PLUS_AES_BLOCK_SIZE == 0);

    uint8_t x[MF_PLUS_AES_BLOCK_SIZE];
    memset(x, 0, sizeof(x));

    for(size_t i = 0; i < n - 1; i++) {
        uint8_t y[MF_PLUS_AES_BLOCK_SIZE];
        for(size_t j = 0; j < MF_PLUS_AES_BLOCK_SIZE; j++) {
            y[j] = x[j] ^ data[i * MF_PLUS_AES_BLOCK_SIZE + j];
        }
        mf_plus_crypto_ecb_encrypt(key, y, x);
    }

    uint8_t last[MF_PLUS_AES_BLOCK_SIZE];
    const size_t offset = (n - 1) * MF_PLUS_AES_BLOCK_SIZE;
    if(last_complete) {
        for(size_t j = 0; j < MF_PLUS_AES_BLOCK_SIZE; j++) {
            last[j] = data[offset + j] ^ k1[j];
        }
    } else {
        memset(last, 0, sizeof(last));
        const size_t rem = length - offset;
        memcpy(last, &data[offset], rem);
        last[rem] = 0x80;
        for(size_t j = 0; j < MF_PLUS_AES_BLOCK_SIZE; j++) {
            last[j] ^= k2[j];
        }
    }

    uint8_t y[MF_PLUS_AES_BLOCK_SIZE];
    for(size_t j = 0; j < MF_PLUS_AES_BLOCK_SIZE; j++) {
        y[j] = x[j] ^ last[j];
    }
    mf_plus_crypto_ecb_encrypt(key, y, mac);
}

void mf_plus_crypto_cmac8(
    const uint8_t key[MF_PLUS_AES_KEY_SIZE],
    const uint8_t* data,
    size_t length,
    uint8_t mac[MF_PLUS_MAC_SIZE]) {
    uint8_t full[MF_PLUS_AES_BLOCK_SIZE];
    mf_plus_crypto_cmac(key, data, length, full);
    for(size_t i = 0; i < MF_PLUS_MAC_SIZE; i++) {
        mac[i] = full[i * 2 + 1];
    }
}

/* ---- Session keys (AN10922) ---- */

void mf_plus_crypto_derive_session_keys(
    const uint8_t key[MF_PLUS_AES_KEY_SIZE],
    const uint8_t rnd_a[MF_PLUS_AES_BLOCK_SIZE],
    const uint8_t rnd_b[MF_PLUS_AES_BLOCK_SIZE],
    uint8_t k_enc[MF_PLUS_AES_KEY_SIZE],
    uint8_t k_mac[MF_PLUS_AES_KEY_SIZE]) {
    uint8_t sv[MF_PLUS_AES_BLOCK_SIZE];

    // Kenc: RndA[11..15] || RndB[11..15] || (RndA[4..8] ^ RndB[4..8]) || 0x11
    memcpy(&sv[0], &rnd_a[11], 5);
    memcpy(&sv[5], &rnd_b[11], 5);
    for(int i = 0; i < 5; i++) {
        sv[10 + i] = rnd_a[4 + i] ^ rnd_b[4 + i];
    }
    sv[15] = 0x11;
    mf_plus_crypto_ecb_encrypt(key, sv, k_enc);

    // Kmac: RndA[7..11] || RndB[7..11] || (RndA[0..4] ^ RndB[0..4]) || 0x22
    memcpy(&sv[0], &rnd_a[7], 5);
    memcpy(&sv[5], &rnd_b[7], 5);
    for(int i = 0; i < 5; i++) {
        sv[10 + i] = rnd_a[i] ^ rnd_b[i];
    }
    sv[15] = 0x22;
    mf_plus_crypto_ecb_encrypt(key, sv, k_mac);
}

// Cap on the payload appended to the fixed stack buffer below; keeps the bound check and
// the buffer size locked together. Real callers MAC one 16-byte block at a time.
#define MF_PLUS_CRYPTO_MAC_MAX_DATA (256)

void mf_plus_crypto_calculate_mac(
    const uint8_t k_mac[MF_PLUS_AES_KEY_SIZE],
    uint8_t cmd,
    uint16_t counter,
    const uint8_t ti[4],
    const uint8_t* data,
    size_t data_length,
    uint8_t mac[MF_PLUS_MAC_SIZE]) {
    // Header is cmd(1)+ctr(2)+TI(4)=7 bytes.
    furi_check(data_length <= MF_PLUS_CRYPTO_MAC_MAX_DATA);

    uint8_t buf[7 + MF_PLUS_CRYPTO_MAC_MAX_DATA];
    size_t n = 0;
    buf[n++] = cmd;
    buf[n++] = (uint8_t)(counter & 0xFF);
    buf[n++] = (uint8_t)((counter >> 8) & 0xFF);
    memcpy(&buf[n], ti, 4);
    n += 4;
    if(data && data_length > 0) {
        memcpy(&buf[n], data, data_length);
        n += data_length;
    }
    mf_plus_crypto_cmac8(k_mac, buf, n, mac);
}

/* ---- Data-encryption IVs (see header for the write-IV validation caveat) ---- */

void mf_plus_crypto_build_read_iv(
    const uint8_t ti[4],
    uint16_t r_ctr,
    uint8_t iv[MF_PLUS_AES_BLOCK_SIZE]) {
    // PM3 mfp_data_crypt (read, rev=true): R_ctr low byte at IV[0]/[4]/[8], TI at IV[12..15].
    memset(iv, 0, MF_PLUS_AES_BLOCK_SIZE);
    const uint8_t ctr = (uint8_t)(r_ctr & 0xFF);
    for(size_t i = 0; i < 9; i += 4) {
        iv[i] = ctr;
    }
    memcpy(&iv[12], ti, 4);
}

void mf_plus_crypto_build_write_iv(
    const uint8_t ti[4],
    uint16_t w_ctr,
    uint8_t iv[MF_PLUS_AES_BLOCK_SIZE]) {
    // PM3 mfp_data_crypt (write, rev=false): W_ctr low byte at IV[7]/[11]/[15], TI at IV[0..3]
    // (the loop also writes IV[3], which the TI copy then overwrites).
    memset(iv, 0, MF_PLUS_AES_BLOCK_SIZE);
    const uint8_t ctr = (uint8_t)(w_ctr & 0xFF);
    for(size_t i = 3; i < MF_PLUS_AES_BLOCK_SIZE; i += 4) {
        iv[i] = ctr;
    }
    memcpy(&iv[0], ti, 4);
}

void mf_plus_crypto_rotate_left(
    const uint8_t input[MF_PLUS_AES_BLOCK_SIZE],
    uint8_t output[MF_PLUS_AES_BLOCK_SIZE]) {
    memcpy(output, &input[1], MF_PLUS_AES_BLOCK_SIZE - 1);
    output[MF_PLUS_AES_BLOCK_SIZE - 1] = input[0];
}
