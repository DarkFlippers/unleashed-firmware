#include "crypto_v2.h"
#include <stdlib.h>
#include <furi.h>
#include <furi_hal_crypto.h>
#include <furi_hal_random.h>
#include <furi_hal_version.h>
#include "../../types/common.h"
#include "../hmac/hmac_sha512.h"
#include "memset_s.h"
#include "constants.h"

#define CRYPTO_ALIGNMENT_FACTOR (16)

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

uint8_t* totp_crypto_encrypt_v2(
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
            furi_hal_crypto_store_load_key(crypto_settings->crypto_key_slot, crypto_settings->iv),
            "Encryption failed: store_load_key");
        furi_check(
            furi_hal_crypto_encrypt(plain_data_aligned, encrypted_data, plain_data_aligned_length),
            "Encryption failed: encrypt");
        furi_check(
            furi_hal_crypto_store_unload_key(crypto_settings->crypto_key_slot),
            "Encryption failed: store_unload_key");

        memset_s(plain_data_aligned, plain_data_aligned_length, 0, plain_data_aligned_length);
        free(plain_data_aligned);
    } else {
        encrypted_data = malloc(plain_data_length);
        furi_check(encrypted_data != NULL);
        *encrypted_data_length = plain_data_length;

        furi_check(
            furi_hal_crypto_store_load_key(crypto_settings->crypto_key_slot, crypto_settings->iv),
            "Encryption failed: store_load_key");
        furi_check(
            furi_hal_crypto_encrypt(plain_data, encrypted_data, plain_data_length),
            "Encryption failed: encrypt");
        furi_check(
            furi_hal_crypto_store_unload_key(crypto_settings->crypto_key_slot),
            "Encryption failed: store_unload_key");
    }

    return encrypted_data;
}

uint8_t* totp_crypto_decrypt_v2(
    const uint8_t* encrypted_data,
    const size_t encrypted_data_length,
    const CryptoSettings* crypto_settings,
    size_t* decrypted_data_length) {
    *decrypted_data_length = encrypted_data_length;
    uint8_t* decrypted_data = malloc(*decrypted_data_length);
    furi_check(decrypted_data != NULL);
    furi_check(
        furi_hal_crypto_store_load_key(crypto_settings->crypto_key_slot, crypto_settings->iv),
        "Decryption failed: store_load_key");
    furi_check(
        furi_hal_crypto_decrypt(encrypted_data, decrypted_data, encrypted_data_length),
        "Decryption failed: decrypt");
    furi_check(
        furi_hal_crypto_store_unload_key(crypto_settings->crypto_key_slot),
        "Decryption failed: store_unload_key");
    return decrypted_data;
}

CryptoSeedIVResult totp_crypto_seed_iv_v2(
    CryptoSettings* crypto_settings,
    const uint8_t* pin,
    uint8_t pin_length) {
    CryptoSeedIVResult result;
    if(crypto_settings->crypto_verify_data == NULL) {
        FURI_LOG_I(LOGGING_TAG, "Generating new IV");
        furi_hal_random_fill_buf(&crypto_settings->base_iv[0], CRYPTO_IV_LENGTH);
    }

    memcpy(&crypto_settings->iv[0], &crypto_settings->base_iv[0], CRYPTO_IV_LENGTH);

    const uint8_t* device_uid = get_device_uid();
    uint8_t device_uid_length = get_device_uid_length();

    uint8_t hmac_key_length = device_uid_length;
    if(pin != NULL && pin_length > 0) {
        hmac_key_length += pin_length;
    }

    uint8_t* hmac_key = malloc(hmac_key_length);
    furi_check(hmac_key != NULL);

    memcpy(hmac_key, device_uid, device_uid_length);

    if(pin != NULL && pin_length > 0) {
        memcpy(hmac_key + device_uid_length, pin, pin_length);
    }

    uint8_t hmac[HMAC_SHA512_RESULT_SIZE] = {0};
    int hmac_result_code = hmac_sha512(
        hmac_key, hmac_key_length, &crypto_settings->base_iv[0], CRYPTO_IV_LENGTH, &hmac[0]);

    memset_s(hmac_key, hmac_key_length, 0, hmac_key_length);
    free(hmac_key);

    if(hmac_result_code == 0) {
        uint8_t offset =
            hmac[HMAC_SHA512_RESULT_SIZE - 1] % (HMAC_SHA512_RESULT_SIZE - CRYPTO_IV_LENGTH - 1);
        memcpy(&crypto_settings->iv[0], &hmac[offset], CRYPTO_IV_LENGTH);

        result = CryptoSeedIVResultFlagSuccess;
        if(crypto_settings->crypto_verify_data == NULL) {
            const uint8_t* crypto_vkey = get_crypto_verify_key();
            uint8_t crypto_vkey_length = get_crypto_verify_key_length();
            FURI_LOG_I(LOGGING_TAG, "Generating crypto verify data");
            crypto_settings->crypto_verify_data = malloc(crypto_vkey_length);
            furi_check(crypto_settings->crypto_verify_data != NULL);
            crypto_settings->crypto_verify_data_length = crypto_vkey_length;

            crypto_settings->crypto_verify_data = totp_crypto_encrypt_v2(
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

    return result;
}

bool totp_crypto_verify_key_v2(const CryptoSettings* crypto_settings) {
    size_t decrypted_key_length;
    uint8_t* decrypted_key = totp_crypto_decrypt_v2(
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