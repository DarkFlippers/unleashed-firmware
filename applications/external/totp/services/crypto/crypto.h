#pragma once

#include "../../types/plugin_state.h"

typedef uint8_t CryptoSeedIVResult;

enum CryptoSeedIVResults {

    /**
     * @brief IV seeding operation failed
     */
    CryptoSeedIVResultFailed = 0b00,

    /**
     * @brief IV seeding operation succeeded
     */
    CryptoSeedIVResultFlagSuccess = 0b01,

    /**
     * @brief As a part of IV seeding operation new crypto verify data has been generated
     */
    CryptoSeedIVResultFlagNewCryptoVerifyData = 0b10
};

/**
 * @brief Encrypts plain data using built-in certificate and given initialization vector (IV)
 * @param plain_data plain data to be encrypted
 * @param plain_data_length plain data length
 * @param iv initialization vector (IV) to be used to encrypt plain data
 * @param[out] encrypted_data_length encrypted data length
 * @return Encrypted data
 */
uint8_t* totp_crypto_encrypt(
    const uint8_t* plain_data,
    const size_t plain_data_length,
    const uint8_t* iv,
    size_t* encrypted_data_length);

/**
 * @brief Decrypts encrypted data using built-in certificate and given initialization vector (IV)
 * @param encrypted_data encrypted data to be decrypted
 * @param encrypted_data_length encrypted data length
 * @param iv initialization vector (IV) to be used to encrypt plain data
 * @param[out] decrypted_data_length decrypted data length
 * @return Decrypted data
 */
uint8_t* totp_crypto_decrypt(
    const uint8_t* encrypted_data,
    const size_t encrypted_data_length,
    const uint8_t* iv,
    size_t* decrypted_data_length);

/**
 * @brief Seed initialization vector (IV) using user's PIN
 * @param plugin_state application state
 * @param pin user's PIN
 * @param pin_length user's PIN length
 * @return Results of seeding IV
 */
CryptoSeedIVResult
    totp_crypto_seed_iv(PluginState* plugin_state, const uint8_t* pin, uint8_t pin_length);

/**
 * @brief Verifies whether cryptographic information (certificate + IV) is valid and can be used for encryption and decryption
 * @param plugin_state application state
 * @return \c true if cryptographic information is valid; \c false otherwise
 */
bool totp_crypto_verify_key(const PluginState* plugin_state);