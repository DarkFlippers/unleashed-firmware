#pragma once

#include <inttypes.h>

typedef enum { SHA1, SHA256, SHA512 } TokenHashAlgo;

typedef enum { TOTP_6_DIGITS, TOTP_8_DIGITS } TokenDigitsCount;

typedef struct {
    uint8_t* token;
    size_t token_length;
    char* name;
    TokenHashAlgo algo;
    TokenDigitsCount digits;
} TokenInfo;

TokenInfo* token_info_alloc();
void token_info_free(TokenInfo* token_info);
bool token_info_set_secret(
    TokenInfo* token_info,
    const char* base32_token_secret,
    size_t token_secret_length,
    const uint8_t* iv);
uint8_t token_info_get_digits_count(const TokenInfo* token_info);
