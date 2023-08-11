#pragma once

#include <stdint.h>
#include "../services/crypto/constants.h"

typedef struct {
    /**
     * @brief Crypto key slot to be used
     */
    uint8_t crypto_key_slot;

    /**
     * @brief Crypto algorithms version to be used
     */
    uint8_t crypto_version;

    /**
     * @brief Initialization vector (IV) to be used for encryption\decryption 
     */
    uint8_t iv[CRYPTO_IV_LENGTH];

    /**
     * @brief Randomly-generated salt
     */
    uint8_t salt[CRYPTO_SALT_LENGTH];

    /**
     * @brief Encrypted well-known data
     */
    uint8_t* crypto_verify_data;

    /**
     * @brief Encrypted well-known data length
     */
    size_t crypto_verify_data_length;

    /**
     * @brief Whether user's PIN is required or not 
     */
    bool pin_required;
} CryptoSettings;