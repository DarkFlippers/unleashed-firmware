#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <furi/furi.h>

#define TOTP_TOKEN_DURATION_DEFAULT 30

#define TOTP_TOKEN_ALGO_SHA1_NAME "sha1"
#define TOTP_TOKEN_ALGO_SHA256_NAME "sha256"
#define TOTP_TOKEN_ALGO_SHA512_NAME "sha512"
#define TOTP_TOKEN_MAX_LENGTH 255

#define TOTP_TOKEN_AUTOMATION_FEATURE_NONE_NAME "none"
#define TOTP_TOKEN_AUTOMATION_FEATURE_ENTER_AT_THE_END_NAME "enter"
#define TOTP_TOKEN_AUTOMATION_FEATURE_TAB_AT_THE_END_NAME "tab"
#define TOTP_TOKEN_AUTOMATION_FEATURE_TYPE_SLOWER_NAME "slower"

typedef uint8_t TokenHashAlgo;
typedef uint8_t TokenDigitsCount;
typedef uint8_t TokenAutomationFeature;

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

/**
 * @brief Token automation features.
 */
enum TokenAutomationFeatures {
    /**
     * @brief No features enabled
     */
    TOKEN_AUTOMATION_FEATURE_NONE = 0b000,

    /**
     * @brief Press "Enter" key at the end as a part of token input automation
     */
    TOKEN_AUTOMATION_FEATURE_ENTER_AT_THE_END = 0b001,

    /**
     * @brief Press "Tab" key at the end as a part of token input automation
     */
    TOKEN_AUTOMATION_FEATURE_TAB_AT_THE_END = 0b010,

    /**
     * @brief Press keys slower and wait longer between keystrokes
     */
    TOKEN_AUTOMATION_FEATURE_TYPE_SLOWER = 0b100
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

    /**
     * @brief Desired TOTP token duration in seconds
     */
    uint8_t duration;

    /**
     * @brief Token input automation features
     */
    TokenAutomationFeature automation_features;
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
 * @return \c true if token digits count length has been updated; \c false otherwise
 */
bool token_info_set_digits_from_int(TokenInfo* token_info, uint8_t digits);

/**
 * @brief Sets token duration from \c uint8_t value
 * @param token_info instance whichs token digits count length should be updated
 * @param duration desired token duration in seconds
 * @return \c true if token duration has been updated; \c false otherwise
 */
bool token_info_set_duration_from_int(TokenInfo* token_info, uint8_t duration);

/**
 * @brief Sets token hashing algorithm from \c str value
 * @param token_info instance whichs token hashing algorithm should be updated
 * @param str desired token algorithm
 * @return \c true if token hahsing algorithm has been updated; \c false otherwise
 */
bool token_info_set_algo_from_str(TokenInfo* token_info, const FuriString* str);

/**
 * @brief Gets token hahsing algorithm name as C-string
 * @param token_info instance which token hahsing algorithm name should be returned
 * @return token hashing algorithm name as C-string
 */
char* token_info_get_algo_as_cstr(const TokenInfo* token_info);

/**
 * @brief Sets token automation feature from \c str value
 * @param token_info instance whichs token automation feature should be updated
 * @param str desired token automation feature
 * @return \c true if token automation feature has been set; \c false otherwise
 */
bool token_info_set_automation_feature_from_str(TokenInfo* token_info, const FuriString* str);

/**
 * @brief Clones \c TokenInfo instance
 * @param src instance to clone
 * @return cloned instance
 */
TokenInfo* token_info_clone(const TokenInfo* src);