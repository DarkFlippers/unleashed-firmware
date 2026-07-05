#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MF_PLUS_AES_BLOCK_SIZE (16)
#define MF_PLUS_AES_KEY_SIZE   (16)
#define MF_PLUS_MAC_SIZE       (8)

/**
 * MIFARE Plus SL3 secure-messaging crypto primitives (AES-128 based).
 *
 * Algorithm is the NXP AN10922 / MIFARE Plus SL3 scheme, cross-validated against
 * Proxmark3 (mifare4.c + mfp_data_crypt in cmdhfmfp.c) and the flipperzero-mfp-reader
 * fork. Built on firmware mbedtls (no third-party AES). The data-encryption IVs follow
 * PM3's mfp_data_crypt() exactly (verified against source: a single low counter byte, so
 * a session is bound to < 256 data ops). The read IV is proven — PM3 and the fork agree
 * and both read real cards; the WRITE IV follows PM3's proven layout but is not yet
 * hardware-validated in this port — confirm with a real-card write trace before the
 * write / emulation paths (PR6/7). See plan review §0a.
 */

/** AES-128 ECB, single 16-byte block. */
void mf_plus_crypto_ecb_encrypt(
    const uint8_t key[MF_PLUS_AES_KEY_SIZE],
    const uint8_t input[MF_PLUS_AES_BLOCK_SIZE],
    uint8_t output[MF_PLUS_AES_BLOCK_SIZE]);

void mf_plus_crypto_ecb_decrypt(
    const uint8_t key[MF_PLUS_AES_KEY_SIZE],
    const uint8_t input[MF_PLUS_AES_BLOCK_SIZE],
    uint8_t output[MF_PLUS_AES_BLOCK_SIZE]);

/** AES-128 CBC. length must be a multiple of 16. input/output may alias. */
void mf_plus_crypto_cbc_encrypt(
    const uint8_t key[MF_PLUS_AES_KEY_SIZE],
    const uint8_t iv[MF_PLUS_AES_BLOCK_SIZE],
    const uint8_t* input,
    uint8_t* output,
    size_t length);

void mf_plus_crypto_cbc_decrypt(
    const uint8_t key[MF_PLUS_AES_KEY_SIZE],
    const uint8_t iv[MF_PLUS_AES_BLOCK_SIZE],
    const uint8_t* input,
    uint8_t* output,
    size_t length);

/** AES-CMAC (NIST SP 800-38B / RFC 4493). mac = full 16 bytes. */
void mf_plus_crypto_cmac(
    const uint8_t key[MF_PLUS_AES_KEY_SIZE],
    const uint8_t* data,
    size_t length,
    uint8_t mac[MF_PLUS_AES_BLOCK_SIZE]);

/** MFP truncated MAC: the 8 odd-indexed bytes of the full CMAC. */
void mf_plus_crypto_cmac8(
    const uint8_t key[MF_PLUS_AES_KEY_SIZE],
    const uint8_t* data,
    size_t length,
    uint8_t mac[MF_PLUS_MAC_SIZE]);

/**
 * Derive the SL3 session keys from the sector/admin key and the auth randoms.
 * Byte ranges below are inclusive; outputs k_enc/k_mac must not alias `key`.
 *   Kenc = AES(key, RndA[11..15] || RndB[11..15] || (RndA[4..8] ^ RndB[4..8]) || 0x11)
 *   Kmac = AES(key, RndA[7..11]  || RndB[7..11]  || (RndA[0..4] ^ RndB[0..4]) || 0x22)
 */
void mf_plus_crypto_derive_session_keys(
    const uint8_t key[MF_PLUS_AES_KEY_SIZE],
    const uint8_t rnd_a[MF_PLUS_AES_BLOCK_SIZE],
    const uint8_t rnd_b[MF_PLUS_AES_BLOCK_SIZE],
    uint8_t k_enc[MF_PLUS_AES_KEY_SIZE],
    uint8_t k_mac[MF_PLUS_AES_KEY_SIZE]);

/**
 * CMAC8 over cmd(1) || ctr_lo(1) || ctr_hi(1) || TI(4) || data(N).
 * The caller assembles `data` (block number, count, cipher payload, ...) per the
 * PM3-validated read/write MAC layouts; this only prefixes cmd+counter+TI.
 */
void mf_plus_crypto_calculate_mac(
    const uint8_t k_mac[MF_PLUS_AES_KEY_SIZE],
    uint8_t cmd,
    uint16_t counter,
    const uint8_t ti[4],
    const uint8_t* data,
    size_t data_length,
    uint8_t mac[MF_PLUS_MAC_SIZE]);

/**
 * Data-encryption IV for an encrypted READ response, matching PM3 mfp_data_crypt():
 * R_ctr low byte at IV[0]/[4]/[8], TI at IV[12..15], rest zero. Only the counter low byte
 * is used (bounds a session to < 256 reads). Proven: PM3 and the fork agree here and both
 * read real cards.
 */
void mf_plus_crypto_build_read_iv(
    const uint8_t ti[4],
    uint16_t r_ctr,
    uint8_t iv[MF_PLUS_AES_BLOCK_SIZE]);

/**
 * Data-encryption IV for an encrypted WRITE command, matching PM3 mfp_data_crypt():
 * W_ctr low byte at IV[7]/[11]/[15], TI at IV[0..3], rest zero. (Replaces the fork's
 * divergent 16-bit two-counter layout — verified against PM3 source; see plan review §0a.)
 * PM3's wrbl uses this on real cards, but this port's write path is not yet
 * hardware-validated — confirm with a captured write trace before the write / emulation
 * paths (PR6/7).
 */
void mf_plus_crypto_build_write_iv(
    const uint8_t ti[4],
    uint16_t w_ctr,
    uint8_t iv[MF_PLUS_AES_BLOCK_SIZE]);

/** Rotate a 16-byte block left by one byte (auth token helper). */
void mf_plus_crypto_rotate_left(
    const uint8_t input[MF_PLUS_AES_BLOCK_SIZE],
    uint8_t output[MF_PLUS_AES_BLOCK_SIZE]);

#ifdef __cplusplus
}
#endif
