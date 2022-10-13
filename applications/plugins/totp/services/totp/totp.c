#include "totp.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "../hmac/hmac-sha1.h"
#include "../hmac/hmac-sha256.h"
#include "../hmac/hmac-sha512.h"
#include "../timezone_utils/timezone_utils.h"

#define UINT64_GET_BYTE(integer, index) ((integer >> (8 * index)) & 0xFF)

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
uint64_t totp_timecode(uint8_t interval, uint64_t for_time)
{
	return for_time/interval;
}

/*
	Converts an integer into an 8 byte array.
	
	out_bytes is the null-terminated output string already allocated
*/
void otp_num_to_bytes(uint64_t integer, uint8_t* out_bytes)
{   
    out_bytes[7] = UINT64_GET_BYTE(integer, 0);
    out_bytes[6] = UINT64_GET_BYTE(integer, 1);
    out_bytes[5] = UINT64_GET_BYTE(integer, 2);
    out_bytes[4] = UINT64_GET_BYTE(integer, 3);
    out_bytes[3] = UINT64_GET_BYTE(integer, 4);
    out_bytes[2] = UINT64_GET_BYTE(integer, 5);
    out_bytes[1] = UINT64_GET_BYTE(integer, 6);
    out_bytes[0] = UINT64_GET_BYTE(integer, 7);
}

/*
	Generates an OTP (One Time Password).
	
	input is a number used to generate the OTP
	out_str is the null-terminated output string already allocated
	
	Returns
			OTP code if otp code was successfully generated
		0 otherwise
*/
uint32_t otp_generate(TOTP_ALGO algo, uint8_t digits, const uint8_t* plain_secret, uint8_t plain_secret_length, uint64_t input)
{
	uint8_t* bytes = malloc(8);
    memset(bytes, 0, 8);
	uint8_t* hmac = malloc(64);
    memset(hmac, 0, 64);

    otp_num_to_bytes(input, bytes);
	
	int hmac_len = (*(algo))(plain_secret, plain_secret_length, bytes, 8, hmac);
	if (hmac_len == 0) {
		free(hmac);
        free(bytes);
        return OTP_ERROR;
    }
	
	uint64_t offset = (hmac[hmac_len-1] & 0xF);
	uint64_t i_code =
		((hmac[offset] & 0x7F) << 24 |
		(hmac[offset + 1] & 0xFF) << 16 |
		(hmac[offset + 2] & 0xFF) << 8 |
		(hmac[offset + 3] & 0xFF));
	i_code %= (uint64_t) pow(10, digits);

	free(hmac);
	free(bytes);
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
uint32_t totp_at(TOTP_ALGO algo, uint8_t digits, const uint8_t* plain_secret, uint8_t plain_secret_length, uint64_t for_time, float timezone, uint8_t interval)
{
    uint64_t for_time_adjusted = timezone_offset_apply(for_time, timezone_offset_from_hours(timezone));
	return otp_generate(algo, digits, plain_secret, plain_secret_length, totp_timecode(interval, for_time_adjusted));
}

static int totp_algo_sha1(const uint8_t* key, uint8_t key_length, const uint8_t* input, uint8_t input_length, uint8_t* output) { 
    hmac_sha1(key, key_length, input, input_length, output); 
    return HMAC_SHA1_RESULT_SIZE; 
}

static int totp_algo_sha256(const uint8_t* key, uint8_t key_length, const uint8_t* input, uint8_t input_length, uint8_t* output) { 
    hmac_sha256(key, key_length, input, input_length, output); 
    return HMAC_SHA256_RESULT_SIZE; 
}

static int totp_algo_sha512(const uint8_t* key, uint8_t key_length, const uint8_t* input, uint8_t input_length, uint8_t* output) { 
    hmac_sha512(key, key_length, input, input_length, output); 
    return HMAC_SHA512_RESULT_SIZE; 
}

const TOTP_ALGO TOTP_ALGO_SHA1 = (TOTP_ALGO)(&totp_algo_sha1);
const TOTP_ALGO TOTP_ALGO_SHA256 = (TOTP_ALGO)(&totp_algo_sha256);
const TOTP_ALGO TOTP_ALGO_SHA512 = (TOTP_ALGO)(&totp_algo_sha512);
