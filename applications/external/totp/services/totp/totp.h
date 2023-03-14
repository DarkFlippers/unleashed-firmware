#pragma once

#include <stdlib.h>
#include <stdint.h>

#define OTP_ERROR (0)

/**
 * @brief Must compute HMAC using passed arguments, output as char array through output.
 *        \p key is secret key buffer.
 *        \p key_length is secret key buffer length.
 *        \p input is input buffer.
 *        \p input_length is input buffer length.
 *	      \p output is an output buffer of the resulting HMAC operation.
 *        Must return 0 if error, or the length in bytes of the HMAC operation.
 */
typedef int (*TOTP_ALGO)(
    const uint8_t* key,
    size_t key_length,
    const uint8_t* input,
    size_t input_length,
    uint8_t* output);

/**
 * @brief Computes HMAC using SHA1
 */
extern const TOTP_ALGO TOTP_ALGO_SHA1;

/**
 * @brief Computes HMAC using SHA256
 */
extern const TOTP_ALGO TOTP_ALGO_SHA256;

/**
 * @brief Computes HMAC using SHA512
 */
extern const TOTP_ALGO TOTP_ALGO_SHA512;

/**
 * @brief Generates a OTP key using the totp algorithm.
 * @param algo hashing algorithm to be used
 * @param digits desired TOTP code length
 * @param plain_secret plain token secret
 * @param plain_secret_length plain token secret length
 * @param for_time the time the generated key will be created for
 * @param timezone UTC timezone adjustment for the generated key
 * @param interval token lifetime in seconds
 * @return TOTP code if code was successfully generated; 0 otherwise
 */
uint32_t totp_at(
    TOTP_ALGO algo,
    uint8_t digits,
    const uint8_t* plain_secret,
    size_t plain_secret_length,
    uint64_t for_time,
    float timezone,
    uint8_t interval);
