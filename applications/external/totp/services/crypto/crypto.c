#include "crypto.h"
#include <furi_hal_crypto.h>
#include <furi_hal_random.h>
#include <furi_hal_version.h>
#include "../config/config.h"
#include "../../types/common.h"
#include "memset_s.h"

#define CRYPTO_KEY_SLOT (2)
#define CRYPTO_VERIFY_KEY "FFF_Crypto_pass"
#define CRYPTO_VERIFY_KEY_LENGTH (16)
#define CRYPTO_ALIGNMENT_FACTOR (16)

uint8_t* totp_crypto_encrypt(
    const uint8_t* plain_data,
    const size_t plain_data_length,
    const uint8_t* iv,
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

        furi_hal_crypto_store_load_key(CRYPTO_KEY_SLOT, iv);
        furi_hal_crypto_encrypt(plain_data_aligned, encrypted_data, plain_data_aligned_length);
        furi_hal_crypto_store_unload_key(CRYPTO_KEY_SLOT);

        memset_s(plain_data_aligned, plain_data_aligned_length, 0, plain_data_aligned_length);
        free(plain_data_aligned);
    } else {
        encrypted_data = malloc(plain_data_length);
        furi_check(encrypted_data != NULL);
        *encrypted_data_length = plain_data_length;

        furi_hal_crypto_store_load_key(CRYPTO_KEY_SLOT, iv);
        furi_hal_crypto_encrypt(plain_data, encrypted_data, plain_data_length);
        furi_hal_crypto_store_unload_key(CRYPTO_KEY_SLOT);
    }

    return encrypted_data;
}

uint8_t* totp_crypto_decrypt(
    const uint8_t* encrypted_data,
    const size_t encrypted_data_length,
    const uint8_t* iv,
    size_t* decrypted_data_length) {
    *decrypted_data_length = encrypted_data_length;
    uint8_t* decrypted_data = malloc(*decrypted_data_length);
    furi_check(decrypted_data != NULL);
    furi_hal_crypto_store_load_key(CRYPTO_KEY_SLOT, iv);
    furi_hal_crypto_decrypt(encrypted_data, decrypted_data, encrypted_data_length);
    furi_hal_crypto_store_unload_key(CRYPTO_KEY_SLOT);
    return decrypted_data;
}

bool totp_crypto_seed_iv(PluginState* plugin_state, const uint8_t* pin, uint8_t pin_length) {
    if(plugin_state->crypto_verify_data == NULL) {
        FURI_LOG_D(LOGGING_TAG, "Generating new IV");
        furi_hal_random_fill_buf(&plugin_state->base_iv[0], TOTP_IV_SIZE);
    }

    memcpy(&plugin_state->iv[0], &plugin_state->base_iv[0], TOTP_IV_SIZE);
    if(pin != NULL && pin_length > 0) {
        uint8_t max_i;
        if(pin_length > TOTP_IV_SIZE) {
            max_i = TOTP_IV_SIZE;
        } else {
            max_i = pin_length;
        }

        for(uint8_t i = 0; i < max_i; i++) {
            plugin_state->iv[i] = plugin_state->iv[i] ^ (uint8_t)(pin[i] * (i + 1));
        }
    } else {
        uint8_t max_i;
        size_t uid_size = furi_hal_version_uid_size();
        if(uid_size > TOTP_IV_SIZE) {
            max_i = TOTP_IV_SIZE;
        } else {
            max_i = uid_size;
        }

        const uint8_t* uid = furi_hal_version_uid();
        for(uint8_t i = 0; i < max_i; i++) {
            plugin_state->iv[i] = plugin_state->iv[i] ^ uid[i];
        }
    }

    bool result = true;
    if(plugin_state->crypto_verify_data == NULL) {
        FURI_LOG_D(LOGGING_TAG, "Generating crypto verify data");
        plugin_state->crypto_verify_data = malloc(CRYPTO_VERIFY_KEY_LENGTH);
        furi_check(plugin_state->crypto_verify_data != NULL);
        plugin_state->crypto_verify_data_length = CRYPTO_VERIFY_KEY_LENGTH;

        plugin_state->crypto_verify_data = totp_crypto_encrypt(
            (uint8_t*)CRYPTO_VERIFY_KEY,
            CRYPTO_VERIFY_KEY_LENGTH,
            &plugin_state->iv[0],
            &plugin_state->crypto_verify_data_length);

        plugin_state->pin_set = pin != NULL && pin_length > 0;

        result = totp_config_file_update_crypto_signatures(plugin_state) ==
                 TotpConfigFileUpdateSuccess;
    }

    return result;
}

bool totp_crypto_verify_key(const PluginState* plugin_state) {
    size_t decrypted_key_length;
    const uint8_t* decrypted_key = totp_crypto_decrypt(
        plugin_state->crypto_verify_data,
        plugin_state->crypto_verify_data_length,
        &plugin_state->iv[0],
        &decrypted_key_length);

    bool key_valid = true;
    for(uint8_t i = 0; i < CRYPTO_VERIFY_KEY_LENGTH && key_valid; i++) {
        if(decrypted_key[i] != CRYPTO_VERIFY_KEY[i]) key_valid = false;
    }

    return key_valid;
}