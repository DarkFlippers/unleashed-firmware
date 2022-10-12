#include <furi/furi.h>
#include <furi_hal.h>
#include "token_info.h"
#include "stdlib.h"
#include "common.h"
#include "../services/base32/base32.h"

TokenInfo* token_info_alloc() {
    TokenInfo* tokenInfo = malloc(sizeof(TokenInfo));
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

void token_info_set_secret(
    TokenInfo* token_info,
    const char* base32_token_secret,
    uint8_t token_secret_length,
    uint8_t* iv) {
    uint8_t* plain_secret = malloc(token_secret_length);
    int plain_secret_length =
        base32_decode((uint8_t*)base32_token_secret, plain_secret, token_secret_length);
    token_info->token_length = plain_secret_length;

    size_t remain = token_info->token_length % 16;
    if(remain) {
        token_info->token_length = token_info->token_length - remain + 16;
        uint8_t* plain_secret_aligned = malloc(token_info->token_length);
        memcpy(plain_secret_aligned, plain_secret, plain_secret_length);
        memset(plain_secret, 0, plain_secret_length);
        free(plain_secret);
        plain_secret = plain_secret_aligned;
    }

    token_info->token = malloc(token_info->token_length);

    furi_hal_crypto_store_load_key(CRYPTO_KEY_SLOT, iv);
    furi_hal_crypto_encrypt(plain_secret, token_info->token, token_info->token_length);
    furi_hal_crypto_store_unload_key(CRYPTO_KEY_SLOT);

    memset(plain_secret, 0, token_info->token_length);
    free(plain_secret);
}

uint8_t token_info_get_digits_count(TokenInfo* token_info) {
    switch(token_info->digits) {
    case TOTP_6_DIGITS:
        return 6;
    case TOTP_8_DIGITS:
        return 8;
    }

    return 6;
}
