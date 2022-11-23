#pragma once

#include <inttypes.h>

typedef uint8_t TokenHashAlgo;
typedef uint8_t TokenDigitsCount;

/**
 * @brief Hashing algorithm to be used to generate token
 */
enum TokenHashAlgos {
    /**
     * @brief SHA1 hashing algorithm
     */
    SHA1,

    /**
     * @brief SHA256 hashing algorithm
     */
    SHA256,

    /**
     * @brief SHA512 hashing algorithm
     */
    SHA512
};

/**
 * @brief Token digits count to be generated.
 */
enum TokenDigitsCounts {
    /**
     * @brief 6 digits
     */
    TOTP_6_DIGITS = 6,

    /**
     * @brief 8 digits
     */
    TOTP_8_DIGITS = 8
};

#define TOTP_TOKEN_DIGITS_MAX_COUNT 8

/**
 * @brief TOTP token information
 */
typedef struct {
    /**
     * @brief Encrypted token secret 
     */
    uint8_t* token;

    /**
     * @brief Encrypted token secret length 
     */
    size_t token_length;

    /**
     * @brief User-friendly token name 
     */
    char* name;

    /**
     * @brief Hashing algorithm
     */
    TokenHashAlgo algo;

    /**
     * @brief Desired TOTP token length 
     */
    TokenDigitsCount digits;
} TokenInfo;

/**
 * @brief Allocates a new instance of \c TokenInfo
 * @return 
 */
TokenInfo* token_info_alloc();

/**
 * @brief Disposes all the resources allocated by the given \c TokenInfo instance
 * @param token_info instance to be disposed
 */
void token_info_free(TokenInfo* token_info);

/**
 * @brief Encrypts & sets plain token secret to the given instance of \c TokenInfo
 * @param token_info instance where secret should be updated
 * @param base32_token_secret plain token secret in Base32 format
 * @param token_secret_length plain token secret length
 * @param iv initialization vecor (IV) to be used for encryption
 * @return \c true if token successfully set; \c false otherwise
 */
bool token_info_set_secret(
    TokenInfo* token_info,
    const char* base32_token_secret,
    size_t token_secret_length,
    const uint8_t* iv);

/**
 * @brief Sets token digits count from \c uint8_t value
 * @param token_info instance whichs token digits count length should be updated
 * @param digits desired token digits count length
 * @return \c true if token digits count length has been updated; \c false p
 */
bool token_info_set_digits_from_int(TokenInfo* token_info, uint8_t digits);
