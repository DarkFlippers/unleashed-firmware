#include "totp.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "../hmac/hmac_sha1.h"
#include "../hmac/hmac_sha256.h"
#include "../hmac/hmac_sha512.h"
#include "../hmac/byteswap.h"
#include "../timezone_utils/timezone_utils.h"

/*
	Generates the timeblock for a time in seconds.
	
	Timeblocks are the amount of intervals in a given time. For example,
	if 1,000,000 seconds has passed for 30 second intervals, you would get
	33,333 timeblocks (intervals), where timeblock++ is effectively +30 seconds.
	
	for_time is a time in seconds to get the current timeblocks
	
	Returns
			timeblock given for_time, using data->interval
		error, 0
*/
uint64_t totp_timecode(uint8_t interval, uint64_t for_time) {
    return for_time / interval;
}

/*
	Generates an OTP (One Time Password).
	
	input is a number used to generate the OTP
	out_str is the null-terminated output string already allocated
	
	Returns
			OTP code if otp code was successfully generated
		0 otherwise
*/
uint32_t otp_generate(
    TOTP_ALGO algo,
    uint8_t digits,
    const uint8_t* plain_secret,
    uint8_t plain_secret_length,
    uint64_t input) {
    uint8_t* hmac = malloc(64);
    memset(hmac, 0, 64);

    uint64_t input_swapped = swap_uint64(input);

    int hmac_len = (*(algo))(plain_secret, plain_secret_length, (uint8_t*)&input_swapped, 8, hmac);
    if(hmac_len == 0) {
        free(hmac);
        return OTP_ERROR;
    }

    uint64_t offset = (hmac[hmac_len - 1] & 0xF);
    uint64_t i_code =
        ((hmac[offset] & 0x7F) << 24 | (hmac[offset + 1] & 0xFF) << 16 |
         (hmac[offset + 2] & 0xFF) << 8 | (hmac[offset + 3] & 0xFF));
    i_code %= (uint64_t)pow(10, digits);

    free(hmac);
    return i_code;
}

/*
	Generates a OTP key using the totp algorithm.
	
	for_time is the time the generated key will be created for
	offset is a timeblock adjustment for the generated key
	out_str is the null-terminated output string already allocated
	
	Returns
			TOTP code if otp code was successfully generated
		0 otherwise
*/
uint32_t totp_at(
    TOTP_ALGO algo,
    uint8_t digits,
    const uint8_t* plain_secret,
    uint8_t plain_secret_length,
    uint64_t for_time,
    float timezone,
    uint8_t interval) {
    uint64_t for_time_adjusted =
        timezone_offset_apply(for_time, timezone_offset_from_hours(timezone));
    return otp_generate(
        algo,
        digits,
        plain_secret,
        plain_secret_length,
        totp_timecode(interval, for_time_adjusted));
}

static int totp_algo_sha1(
    const uint8_t* key,
    uint8_t key_length,
    const uint8_t* input,
    uint8_t input_length,
    uint8_t* output) {
    hmac_sha1(key, key_length, input, input_length, output);
    return HMAC_SHA1_RESULT_SIZE;
}

static int totp_algo_sha256(
    const uint8_t* key,
    uint8_t key_length,
    const uint8_t* input,
    uint8_t input_length,
    uint8_t* output) {
    hmac_sha256(key, key_length, input, input_length, output);
    return HMAC_SHA256_RESULT_SIZE;
}

static int totp_algo_sha512(
    const uint8_t* key,
    uint8_t key_length,
    const uint8_t* input,
    uint8_t input_length,
    uint8_t* output) {
    hmac_sha512(key, key_length, input, input_length, output);
    return HMAC_SHA512_RESULT_SIZE;
}

const TOTP_ALGO TOTP_ALGO_SHA1 = (TOTP_ALGO)(&totp_algo_sha1);
const TOTP_ALGO TOTP_ALGO_SHA256 = (TOTP_ALGO)(&totp_algo_sha256);
const TOTP_ALGO TOTP_ALGO_SHA512 = (TOTP_ALGO)(&totp_algo_sha512);
