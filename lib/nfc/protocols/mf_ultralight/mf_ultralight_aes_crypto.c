#include "mf_ultralight_aes_crypto.h"

#include <string.h>
#include <stdbool.h>
#include <furi.h>
#include <mbedtls/aes.h>

// Largest secure-messaging payload the MAC is computed over (a 64-page FAST_READ response = 256 B);
// the 2-byte CmdCtr is prepended.
#define MF_ULTRALIGHT_AES_CMAC_MSG_MAX (256)

void mf_ultralight_aes_rol16(uint8_t* data) {
    uint8_t first = data[0];
    for(uint8_t i = 1; i < 16; i++) {
        data[i - 1] = data[i];
    }
    data[15] = first;
}

// Rotate a 16-byte block right by one byte (undo one rol16).
static void mf_ultralight_aes_ror16(uint8_t* data) {
    uint8_t last = data[15];
    for(uint8_t i = 15; i > 0; i--) {
        data[i] = data[i - 1];
    }
    data[0] = last;
}

void mf_ultralight_aes_cmac(const uint8_t* key, const uint8_t* data, size_t len, uint8_t* mac) {
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

static void mf_ultralight_aes_cmac8(const uint8_t* mac16, uint8_t* out8) {
    for(int i = 0; i < 8; i++)
        out8[i] = mac16[2 * i + 1];
}

void mf_ultralight_aes_cmac8_ctr(
    const uint8_t* key,
    uint16_t counter,
    const uint8_t* data,
    size_t len,
    uint8_t* out8) {
    furi_check(len <= MF_ULTRALIGHT_AES_CMAC_MSG_MAX);
    // MAC input = 2-byte CmdCtr (LSB first) || data (§8.8.3). `data` may be empty (the standalone
    // MAC over CmdCtr that replaces a WRITE ACK).
    uint8_t msg[2 + MF_ULTRALIGHT_AES_CMAC_MSG_MAX];
    msg[0] = counter & 0xFF;
    msg[1] = (counter >> 8) & 0xFF;
    if(len) memcpy(msg + 2, data, len);
    uint8_t mac16[16];
    mf_ultralight_aes_cmac(key, msg, 2 + len, mac16);
    mf_ultralight_aes_cmac8(mac16, out8);
}

void mf_ultralight_aes_derive_session_key(
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
