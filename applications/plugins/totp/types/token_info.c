#include <furi/furi.h>
#include <furi_hal.h>
#include "token_info.h"
#include "stdlib.h"
#include "common.h"
#include "../lib/base32/base32.h"
#include "../services/crypto/crypto.h"
#include "../lib/polyfills/memset_s.h"

TokenInfo* token_info_alloc() {
    TokenInfo* tokenInfo = malloc(sizeof(TokenInfo));
    furi_check(tokenInfo != NULL);
    tokenInfo->algo = SHA1;
    tokenInfo->digits = TOTP_6_DIGITS;
    return tokenInfo;
}

void token_info_free(TokenInfo* token_info) {
    if(token_info == NULL) return;
    free(token_info->name);
    free(token_info->token);
    free(token_info);
}

bool token_info_set_secret(
    TokenInfo* token_info,
    const char* base32_token_secret,
    size_t token_secret_length,
    const uint8_t* iv) {
    if(token_secret_length == 0) return false;

    uint8_t* plain_secret = malloc(token_secret_length);
    furi_check(plain_secret != NULL);
    int plain_secret_length =
        base32_decode((const uint8_t*)base32_token_secret, plain_secret, token_secret_length);
    bool result;
    if(plain_secret_length > 0) {
        token_info->token =
            totp_crypto_encrypt(plain_secret, plain_secret_length, iv, &token_info->token_length);
        result = true;
    } else {
        result = false;
    }

    memset_s(plain_secret, token_secret_length, 0, token_secret_length);
    free(plain_secret);
    return result;
}

bool token_info_set_digits_from_int(TokenInfo* token_info, uint8_t digits) {
    switch(digits) {
    case 6:
        token_info->digits = TOTP_6_DIGITS;
        return true;
    case 8:
        token_info->digits = TOTP_8_DIGITS;
        return true;
    default:
        break;
    }

    return false;
}
