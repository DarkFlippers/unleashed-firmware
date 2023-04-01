#include <furi_hal.h>
#include "token_info.h"
#include "stdlib.h"
#include "common.h"
#include "../lib/base32/base32.h"
#include "../services/crypto/crypto.h"
#include "../lib/polyfills/memset_s.h"
#include "../lib/polyfills/strnlen.h"

TokenInfo* token_info_alloc() {
    TokenInfo* tokenInfo = malloc(sizeof(TokenInfo));
    furi_check(tokenInfo != NULL);
    tokenInfo->algo = SHA1;
    tokenInfo->digits = TOTP_6_DIGITS;
    tokenInfo->duration = TOTP_TOKEN_DURATION_DEFAULT;
    tokenInfo->automation_features = TOKEN_AUTOMATION_FEATURE_NONE;
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

bool token_info_set_duration_from_int(TokenInfo* token_info, uint8_t duration) {
    if(duration >= 15) {
        token_info->duration = duration;
        return true;
    }

    return false;
}

bool token_info_set_algo_from_str(TokenInfo* token_info, const FuriString* str) {
    if(furi_string_cmpi_str(str, TOTP_TOKEN_ALGO_SHA1_NAME) == 0) {
        token_info->algo = SHA1;
        return true;
    }

    if(furi_string_cmpi_str(str, TOTP_TOKEN_ALGO_SHA256_NAME) == 0) {
        token_info->algo = SHA256;
        return true;
    }

    if(furi_string_cmpi_str(str, TOTP_TOKEN_ALGO_SHA512_NAME) == 0) {
        token_info->algo = SHA512;
        return true;
    }

    return false;
}

char* token_info_get_algo_as_cstr(const TokenInfo* token_info) {
    switch(token_info->algo) {
    case SHA1:
        return TOTP_TOKEN_ALGO_SHA1_NAME;
    case SHA256:
        return TOTP_TOKEN_ALGO_SHA256_NAME;
    case SHA512:
        return TOTP_TOKEN_ALGO_SHA512_NAME;
    default:
        break;
    }

    return NULL;
}

bool token_info_set_automation_feature_from_str(TokenInfo* token_info, const FuriString* str) {
    if(furi_string_cmpi_str(str, TOTP_TOKEN_AUTOMATION_FEATURE_ENTER_AT_THE_END_NAME) == 0) {
        token_info->automation_features |= TOKEN_AUTOMATION_FEATURE_ENTER_AT_THE_END;
        return true;
    }

    if(furi_string_cmpi_str(str, TOTP_TOKEN_AUTOMATION_FEATURE_TAB_AT_THE_END_NAME) == 0) {
        token_info->automation_features |= TOKEN_AUTOMATION_FEATURE_TAB_AT_THE_END;
        return true;
    }

    if(furi_string_cmpi_str(str, TOTP_TOKEN_AUTOMATION_FEATURE_TYPE_SLOWER_NAME) == 0) {
        token_info->automation_features |= TOKEN_AUTOMATION_FEATURE_TYPE_SLOWER;
        return true;
    }

    if(furi_string_cmpi_str(str, TOTP_TOKEN_AUTOMATION_FEATURE_NONE_NAME) == 0) {
        token_info->automation_features = TOKEN_AUTOMATION_FEATURE_NONE;
        return true;
    }

    return false;
}

TokenInfo* token_info_clone(const TokenInfo* src) {
    TokenInfo* clone = token_info_alloc();
    memcpy(clone, src, sizeof(TokenInfo));

    clone->token = malloc(src->token_length);
    furi_check(clone->token != NULL);
    memcpy(clone->token, src->token, src->token_length);

    int name_length = strnlen(src->name, TOTP_TOKEN_MAX_LENGTH);
    clone->name = malloc(name_length + 1);
    furi_check(clone->name != NULL);
    strlcpy(clone->name, src->name, name_length + 1);

    return clone;
}