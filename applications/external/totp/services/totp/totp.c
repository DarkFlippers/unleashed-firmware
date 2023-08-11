#include "totp.h"

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <timezone_utils.h>
#include "../../config/wolfssl/config.h"
#include <wolfssl/wolfcrypt/hmac.h>

#define HMAC_MAX_RESULT_SIZE WC_SHA512_DIGEST_SIZE

static uint64_t swap_uint64(uint64_t val) {
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
    return (val << 32) | (val >> 32);
}

/**
 * @brief Generates the timeblock for a time in seconds.
 *        Timeblocks are the amount of intervals in a given time. For example,
 *        if 1,000,000 seconds has passed for 30 second intervals, you would get
 *        33,333 timeblocks (intervals), where timeblock++ is effectively +30 seconds.
 * @param interval in seconds
 * @param for_time a time in seconds to get the current timeblocks
 * @return Timeblock given \p for_time using \p interval
 */
uint64_t totp_timecode(uint8_t interval, uint64_t for_time) {
    return for_time / interval;
}

/**
 * @brief Generates an OTP (One Time Password)
 * @param algo hashing algorithm to be used
 * @param plain_secret plain token secret
 * @param plain_secret_length plain token secret length
 * @param input input data for OTP code generation
 * @return OTP code if code was successfully generated; 0 otherwise
 */
uint64_t otp_generate(
    TOTP_ALGO algo,
    const uint8_t* plain_secret,
    size_t plain_secret_length,
    uint64_t input) {
    uint8_t hmac[HMAC_MAX_RESULT_SIZE] = {0};

    uint64_t input_swapped = swap_uint64(input);

    int hmac_len =
        (*algo)(plain_secret, plain_secret_length, (uint8_t*)&input_swapped, 8, &hmac[0]);
    if(hmac_len == 0) {
        return OTP_ERROR;
    }

    uint64_t offset = (hmac[hmac_len - 1] & 0xF);
    uint64_t i_code =
        ((hmac[offset] & 0x7F) << 24 | (hmac[offset + 1] & 0xFF) << 16 |
         (hmac[offset + 2] & 0xFF) << 8 | (hmac[offset + 3] & 0xFF));

    return i_code;
}

uint64_t totp_at(
    TOTP_ALGO algo,
    const uint8_t* plain_secret,
    size_t plain_secret_length,
    uint64_t for_time,
    float timezone,
    uint8_t interval) {
    uint64_t for_time_adjusted =
        timezone_offset_apply(for_time, timezone_offset_from_hours(timezone));
    return otp_generate(
        algo, plain_secret, plain_secret_length, totp_timecode(interval, for_time_adjusted));
}

static int totp_algo_common(
    int type,
    const uint8_t* key,
    size_t key_length,
    const uint8_t* input,
    size_t input_length,
    uint8_t* output) {
    Hmac hmac;
    int ret = wc_HmacSetKey(&hmac, type, key, key_length);
    if(ret == 0) {
        ret = wc_HmacUpdate(&hmac, input, input_length);
    }

    if(ret == 0) {
        ret = wc_HmacFinal(&hmac, output);
    }

    wc_HmacFree(&hmac);
    return ret == 0 ? wc_HmacSizeByType(type) : 0;
}

static int totp_algo_sha1(
    const uint8_t* key,
    size_t key_length,
    const uint8_t* input,
    size_t input_length,
    uint8_t* output) {
    return totp_algo_common(WC_SHA, key, key_length, input, input_length, output);
}

static int totp_algo_sha256(
    const uint8_t* key,
    size_t key_length,
    const uint8_t* input,
    size_t input_length,
    uint8_t* output) {
    return totp_algo_common(WC_SHA256, key, key_length, input, input_length, output);
}

static int totp_algo_sha512(
    const uint8_t* key,
    size_t key_length,
    const uint8_t* input,
    size_t input_length,
    uint8_t* output) {
    return totp_algo_common(WC_SHA512, key, key_length, input, input_length, output);
}

const TOTP_ALGO TOTP_ALGO_SHA1 = (TOTP_ALGO)(&totp_algo_sha1);
const TOTP_ALGO TOTP_ALGO_SHA256 = (TOTP_ALGO)(&totp_algo_sha256);
const TOTP_ALGO TOTP_ALGO_SHA512 = (TOTP_ALGO)(&totp_algo_sha512);
