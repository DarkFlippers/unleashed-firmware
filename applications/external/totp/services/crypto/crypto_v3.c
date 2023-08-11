#include "crypto_v3.h"
#include <stdlib.h>
#include <furi.h>
#include <furi_hal_crypto.h>
#include <furi_hal_random.h>
#include <furi_hal_version.h>
#include "../../types/common.h"
#include "../../config/wolfssl/config.h"
#include <wolfssl/wolfcrypt/hmac.h>
#include <wolfssl/wolfcrypt/pwdbased.h>
#include "memset_s.h"
#include "constants.h"
#include "polyfills.h"

#define CRYPTO_ALIGNMENT_FACTOR (16)
#define PBKDF2_ITERATIONS_COUNT (200)

static const uint8_t* get_device_uid() {
    return (const uint8_t*)UID64_BASE; //-V566
}

static uint8_t get_device_uid_length() {
    return furi_hal_version_uid_size();
}

static const uint8_t* get_crypto_verify_key() {
    return get_device_uid();
}

static uint8_t get_crypto_verify_key_length() {
    return get_device_uid_length();
}

uint8_t* totp_crypto_encrypt_v3(
    const uint8_t* plain_data,
    const size_t plain_data_length,
    const CryptoSettings* crypto_settings,
    size_t* encrypted_data_length) {
    uint8_t* encrypted_data;
    size_t remain = plain_data_length % CRYPTO_ALIGNMENT_FACTOR;
    if(remain) {
        size_t plain_data_aligned_length = plain_data_length - remain + CRYPTO_ALIGNMENT_FACTOR;
        uint8_t* plain_data_aligned = malloc(plain_data_aligned_length);
        furi_check(plain_data_aligned != NULL);
        memset(plain_data_aligned, 0, plain_data_aligned_length);
        memcpy(plain_data_aligned, plain_data, plain_data_length);

        encrypted_data = malloc(plain_data_aligned_length);
        furi_check(encrypted_data != NULL);
        *encrypted_data_length = plain_data_aligned_length;

        furi_check(
            furi_hal_crypto_enclave_load_key(crypto_settings->crypto_key_slot, crypto_settings->iv),
            "Encryption failed: enclave_load_key");
        furi_check(
            furi_hal_crypto_encrypt(plain_data_aligned, encrypted_data, plain_data_aligned_length),
            "Encryption failed: encrypt");
        furi_check(
            furi_hal_crypto_enclave_unload_key(crypto_settings->crypto_key_slot),
            "Encryption failed: enclave_unload_key");

        memset_s(plain_data_aligned, plain_data_aligned_length, 0, plain_data_aligned_length);
        free(plain_data_aligned);
    } else {
        encrypted_data = malloc(plain_data_length);
        furi_check(encrypted_data != NULL);
        *encrypted_data_length = plain_data_length;

        furi_check(
            furi_hal_crypto_enclave_load_key(crypto_settings->crypto_key_slot, crypto_settings->iv),
            "Encryption failed: enclave_load_key");
        furi_check(
            furi_hal_crypto_encrypt(plain_data, encrypted_data, plain_data_length),
            "Encryption failed: encrypt");
        furi_check(
            furi_hal_crypto_enclave_unload_key(crypto_settings->crypto_key_slot),
            "Encryption failed: enclave_unload_key");
    }

    return encrypted_data;
}

uint8_t* totp_crypto_decrypt_v3(
    const uint8_t* encrypted_data,
    const size_t encrypted_data_length,
    const CryptoSettings* crypto_settings,
    size_t* decrypted_data_length) {
    *decrypted_data_length = encrypted_data_length;
    uint8_t* decrypted_data = malloc(*decrypted_data_length);
    furi_check(decrypted_data != NULL);
    furi_check(
        furi_hal_crypto_enclave_load_key(crypto_settings->crypto_key_slot, crypto_settings->iv),
        "Decryption failed: enclave_load_key");
    furi_check(
        furi_hal_crypto_decrypt(encrypted_data, decrypted_data, encrypted_data_length),
        "Decryption failed: decrypt");
    furi_check(
        furi_hal_crypto_enclave_unload_key(crypto_settings->crypto_key_slot),
        "Decryption failed: enclave_unload_key");
    return decrypted_data;
}

CryptoSeedIVResult totp_crypto_seed_iv_v3(
    CryptoSettings* crypto_settings,
    const uint8_t* pin,
    uint8_t pin_length) {
    CryptoSeedIVResult result;
    if(crypto_settings->crypto_verify_data == NULL) {
        FURI_LOG_I(LOGGING_TAG, "Generating new salt");
        furi_hal_random_fill_buf(&crypto_settings->salt[0], CRYPTO_SALT_LENGTH);
    }

    const uint8_t* device_uid = get_device_uid();
    uint8_t device_uid_length = get_device_uid_length();

    uint8_t pbkdf_key_length = device_uid_length;
    if(pin != NULL && pin_length > 0) {
        pbkdf_key_length += pin_length;
    }

    uint8_t* pbkdf_key = malloc(pbkdf_key_length);
    furi_check(pbkdf_key != NULL);

    memcpy(pbkdf_key, device_uid, device_uid_length);

    if(pin != NULL && pin_length > 0) {
        memcpy(pbkdf_key + device_uid_length, pin, pin_length);
    }

    uint8_t pbkdf_output[WC_SHA512_DIGEST_SIZE] = {0};

    int pbkdf_result_code = wc_PBKDF2(
        &pbkdf_output[0],
        pbkdf_key,
        pbkdf_key_length,
        &crypto_settings->salt[0],
        CRYPTO_SALT_LENGTH,
        PBKDF2_ITERATIONS_COUNT,
        WC_SHA512_DIGEST_SIZE,
        WC_SHA512);

    memset_s(pbkdf_key, pbkdf_key_length, 0, pbkdf_key_length);
    free(pbkdf_key);

    if(pbkdf_result_code == 0) {
        uint8_t offset = pbkdf_output[WC_SHA512_DIGEST_SIZE - 1] %
                         (WC_SHA512_DIGEST_SIZE - CRYPTO_IV_LENGTH - 1);
        memcpy(&crypto_settings->iv[0], &pbkdf_output[offset], CRYPTO_IV_LENGTH);
        result = CryptoSeedIVResultFlagSuccess;
        if(crypto_settings->crypto_verify_data == NULL) {
            const uint8_t* crypto_vkey = get_crypto_verify_key();
            uint8_t crypto_vkey_length = get_crypto_verify_key_length();
            FURI_LOG_I(LOGGING_TAG, "Generating crypto verify data");
            crypto_settings->crypto_verify_data = malloc(crypto_vkey_length);
            furi_check(crypto_settings->crypto_verify_data != NULL);
            crypto_settings->crypto_verify_data_length = crypto_vkey_length;

            crypto_settings->crypto_verify_data = totp_crypto_encrypt_v3(
                crypto_vkey,
                crypto_vkey_length,
                crypto_settings,
                &crypto_settings->crypto_verify_data_length);

            crypto_settings->pin_required = pin != NULL && pin_length > 0;

            result |= CryptoSeedIVResultFlagNewCryptoVerifyData;
        }
    } else {
        result = CryptoSeedIVResultFailed;
    }

    memset_s(&pbkdf_output[0], WC_SHA512_DIGEST_SIZE, 0, WC_SHA512_DIGEST_SIZE);

    return result;
}

bool totp_crypto_verify_key_v3(const CryptoSettings* crypto_settings) {
    size_t decrypted_key_length;
    uint8_t* decrypted_key = totp_crypto_decrypt_v3(
        crypto_settings->crypto_verify_data,
        crypto_settings->crypto_verify_data_length,
        crypto_settings,
        &decrypted_key_length);

    const uint8_t* crypto_vkey = get_crypto_verify_key();
    uint8_t crypto_vkey_length = get_crypto_verify_key_length();
    bool key_valid = true;
    for(uint8_t i = 0; i < crypto_vkey_length && key_valid; i++) {
        if(decrypted_key[i] != crypto_vkey[i]) key_valid = false;
    }

    free(decrypted_key);

    return key_valid;
}