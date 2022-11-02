#pragma once

#include <stdlib.h>
#include <stdint.h>

#define OTP_ERROR (0)

/*
	Must compute HMAC using passed arguments,
	  output as char array through output.
	
	key is secret key.
	input is input number.
	output is an output buffer of the resulting HMAC operation.
	
	Must return 0 if error, or the length in bytes of the HMAC operation.
*/
typedef int (*TOTP_ALGO)(
    const uint8_t* key,
    size_t key_length,
    const uint8_t* input,
    size_t input_length,
    uint8_t* output);

/*
    Computes HMAC using SHA1
*/
extern const TOTP_ALGO TOTP_ALGO_SHA1;

/*
    Computes HMAC using SHA256
*/
extern const TOTP_ALGO TOTP_ALGO_SHA256;

/*
    Computes HMAC using SHA512
*/
extern const TOTP_ALGO TOTP_ALGO_SHA512;

/*
	Computes TOTP token
    Returns:
        TOTP token on success
        0 otherwise
*/
uint32_t totp_at(
    TOTP_ALGO algo,
    uint8_t digits,
    const uint8_t* plain_secret,
    size_t plain_secret_length,
    uint64_t for_time,
    float timezone,
    uint8_t interval);
