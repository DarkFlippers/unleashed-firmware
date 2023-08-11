#include "crypto_v1.h"
#ifdef TOTP_OBSOLETE_CRYPTO_V1_COMPATIBILITY_ENABLED
#include <stdlib.h>
#include <furi.h>
#include <furi_hal_crypto.h>
#include <furi_hal_random.h>
#include <furi_hal_version.h>
#include "../../types/common.h"
#include "memset_s.h"
#include "polyfills.h"

#define CRYPTO_KEY_SLOT (2)
#define CRYPTO_VERIFY_KEY_LENGTH (16)
#define CRYPTO_ALIGNMENT_FACTOR (16)
#define TOTP_IV_SIZE (16)

static const char* CRYPTO_VERIFY_KEY = "FFF_Crypto_pass";

uint8_t* totp_crypto_encrypt_v1(
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

        furi_hal_crypto_enclave_load_key(CRYPTO_KEY_SLOT, crypto_settings->iv);
        furi_hal_crypto_encrypt(plain_data_aligned, encrypted_data, plain_data_aligned_length);
        furi_hal_crypto_enclave_unload_key(CRYPTO_KEY_SLOT);

        memset_s(plain_data_aligned, plain_data_aligned_length, 0, plain_data_aligned_length);
        free(plain_data_aligned);
    } else {
        encrypted_data = malloc(plain_data_length);
        furi_check(encrypted_data != NULL);
        *encrypted_data_length = plain_data_length;

        furi_hal_crypto_enclave_load_key(CRYPTO_KEY_SLOT, crypto_settings->iv);
        furi_hal_crypto_encrypt(plain_data, encrypted_data, plain_data_length);
        furi_hal_crypto_enclave_unload_key(CRYPTO_KEY_SLOT);
    }

    return encrypted_data;
}

uint8_t* totp_crypto_decrypt_v1(
    const uint8_t* encrypted_data,
    const size_t encrypted_data_length,
    const CryptoSettings* crypto_settings,
    size_t* decrypted_data_length) {
    *decrypted_data_length = encrypted_data_length;
    uint8_t* decrypted_data = malloc(*decrypted_data_length);
    furi_check(decrypted_data != NULL);
    furi_hal_crypto_enclave_load_key(CRYPTO_KEY_SLOT, crypto_settings->iv);
    furi_hal_crypto_decrypt(encrypted_data, decrypted_data, encrypted_data_length);
    furi_hal_crypto_enclave_unload_key(CRYPTO_KEY_SLOT);
    return decrypted_data;
}

CryptoSeedIVResult totp_crypto_seed_iv_v1(
    CryptoSettings* crypto_settings,
    const uint8_t* pin,
    uint8_t pin_length) {
    CryptoSeedIVResult result;
    if(crypto_settings->crypto_verify_data == NULL) {
        FURI_LOG_I(LOGGING_TAG, "Generating new IV");
        furi_hal_random_fill_buf(&crypto_settings->salt[0], CRYPTO_SALT_LENGTH);
    }

    memcpy(&crypto_settings->iv[0], &crypto_settings->salt[0], TOTP_IV_SIZE);
    if(pin != NULL && pin_length > 0) {
        uint8_t max_i;
        if(pin_length > TOTP_IV_SIZE) {
            max_i = TOTP_IV_SIZE;
        } else {
            max_i = pin_length;
        }

        for(uint8_t i = 0; i < max_i; i++) {
            crypto_settings->iv[i] = crypto_settings->iv[i] ^ (uint8_t)(pin[i] * (i + 1));
        }
    } else {
        uint8_t max_i;
        size_t uid_size = furi_hal_version_uid_size();
        if(uid_size > TOTP_IV_SIZE) {
            max_i = TOTP_IV_SIZE;
        } else {
            max_i = uid_size;
        }

        const uint8_t* uid = (const uint8_t*)UID64_BASE; //-V566
        for(uint8_t i = 0; i < max_i; i++) {
            crypto_settings->iv[i] = crypto_settings->iv[i] ^ uid[i];
        }
    }

    result = CryptoSeedIVResultFlagSuccess;
    if(crypto_settings->crypto_verify_data == NULL) {
        FURI_LOG_I(LOGGING_TAG, "Generating crypto verify data");
        crypto_settings->crypto_verify_data = malloc(CRYPTO_VERIFY_KEY_LENGTH);
        furi_check(crypto_settings->crypto_verify_data != NULL);
        crypto_settings->crypto_verify_data_length = CRYPTO_VERIFY_KEY_LENGTH;

        crypto_settings->crypto_verify_data = totp_crypto_encrypt_v1(
            (const uint8_t*)CRYPTO_VERIFY_KEY,
            CRYPTO_VERIFY_KEY_LENGTH,
            crypto_settings,
            &crypto_settings->crypto_verify_data_length);

        crypto_settings->pin_required = pin != NULL && pin_length > 0;

        result |= CryptoSeedIVResultFlagNewCryptoVerifyData;
    }

    return result;
}

bool totp_crypto_verify_key_v1(const CryptoSettings* crypto_settings) {
    size_t decrypted_key_length;
    uint8_t* decrypted_key = totp_crypto_decrypt_v1(
        crypto_settings->crypto_verify_data,
        crypto_settings->crypto_verify_data_length,
        crypto_settings,
        &decrypted_key_length);

    bool key_valid = true;
    for(uint8_t i = 0; i < CRYPTO_VERIFY_KEY_LENGTH && key_valid; i++) {
        if(decrypted_key[i] != CRYPTO_VERIFY_KEY[i]) key_valid = false;
    }

    free(decrypted_key);

    return key_valid;
}
#endif