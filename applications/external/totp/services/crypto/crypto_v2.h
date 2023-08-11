#pragma once

#include "../../config/app/config.h"
#ifdef TOTP_OBSOLETE_CRYPTO_V2_COMPATIBILITY_ENABLED
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../../types/crypto_settings.h"
#include "common_types.h"

/**
 * @brief Encrypts plain data using built-in certificate and given initialization vector (IV)
 * @param plain_data plain data to be encrypted
 * @param plain_data_length plain data length
 * @param crypto_settings crypto settings
 * @param[out] encrypted_data_length encrypted data length
 * @return Encrypted data
 */
uint8_t* totp_crypto_encrypt_v2(
    const uint8_t* plain_data,
    const size_t plain_data_length,
    const CryptoSettings* crypto_settings,
    size_t* encrypted_data_length);

/**
 * @brief Decrypts encrypted data using built-in certificate and given initialization vector (IV)
 * @param encrypted_data encrypted data to be decrypted
 * @param encrypted_data_length encrypted data length
 * @param crypto_settings crypto settings
 * @param[out] decrypted_data_length decrypted data length
 * @return Decrypted data
 */
uint8_t* totp_crypto_decrypt_v2(
    const uint8_t* encrypted_data,
    const size_t encrypted_data_length,
    const CryptoSettings* crypto_settings,
    size_t* decrypted_data_length);

/**
 * @brief Seed initialization vector (IV) using user's PIN
 * @param crypto_settings crypto settings
 * @param pin user's PIN
 * @param pin_length user's PIN length
 * @return Results of seeding IV
 */
CryptoSeedIVResult
    totp_crypto_seed_iv_v2(CryptoSettings* crypto_settings, const uint8_t* pin, uint8_t pin_length);

/**
 * @brief Verifies whether cryptographic information (certificate + IV) is valid and can be used for encryption and decryption
 * @param crypto_settings crypto settings
 * @return \c true if cryptographic information is valid; \c false otherwise
 */
bool totp_crypto_verify_key_v2(const CryptoSettings* crypto_settings);
#endif
