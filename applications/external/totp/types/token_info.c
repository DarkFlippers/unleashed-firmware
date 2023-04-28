#include "token_info.h"
#include <base32.h>
#include <base64.h>
#include <memset_s.h>
#include "common.h"
#include "../services/crypto/crypto.h"

TokenInfo* token_info_alloc() {
    TokenInfo* tokenInfo = malloc(sizeof(TokenInfo));
    furi_check(tokenInfo != NULL);
    tokenInfo->name = furi_string_alloc();
    token_info_set_defaults(tokenInfo);
    return tokenInfo;
}

void token_info_free(TokenInfo* token_info) {
    if(token_info == NULL) return;
    free(token_info->token);
    furi_string_free(token_info->name);
    free(token_info);
}

bool token_info_set_secret(
    TokenInfo* token_info,
    const char* plain_token_secret,
    size_t token_secret_length,
    PlainTokenSecretEncoding plain_token_secret_encoding,
    const uint8_t* iv) {
    if(token_secret_length == 0) return false;
    uint8_t* plain_secret;
    size_t plain_secret_length;
    size_t plain_secret_size;
    if(plain_token_secret_encoding == PlainTokenSecretEncodingBase32) {
        plain_secret_size = token_secret_length;
        plain_secret = malloc(plain_secret_size);
        furi_check(plain_secret != NULL);
        plain_secret_length =
            base32_decode((const uint8_t*)plain_token_secret, plain_secret, plain_secret_size);
    } else if(plain_token_secret_encoding == PlainTokenSecretEncodingBase64) {
        plain_secret_length = 0;
        plain_secret = base64_decode(
            (const uint8_t*)plain_token_secret,
            token_secret_length,
            &plain_secret_length,
            &plain_secret_size);
        furi_check(plain_secret != NULL);
    } else {
        return false;
    }

    bool result;
    if(plain_secret_length > 0) {
        if(token_info->token != NULL) {
            free(token_info->token);
        }

        token_info->token =
            totp_crypto_encrypt(plain_secret, plain_secret_length, iv, &token_info->token_length);
        result = true;
    } else {
        result = false;
    }

    memset_s(plain_secret, plain_secret_size, 0, plain_secret_size);
    free(plain_secret);
    return result;
}

bool token_info_set_digits_from_int(TokenInfo* token_info, uint8_t digits) {
    switch(digits) {
    case 5:
        token_info->digits = TotpFiveDigitsCount;
        return true;
    case 6:
        token_info->digits = TotpSixDigitsCount;
        return true;
    case 8:
        token_info->digits = TotpEightDigitsCount;
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

    if(furi_string_cmpi_str(str, TOTP_TOKEN_ALGO_STEAM_NAME) == 0) {
        token_info->algo = STEAM;
        return true;
    }

    return false;
}

bool token_info_set_algo_from_int(TokenInfo* token_info, uint8_t algo_code) {
    switch(algo_code) {
    case SHA1:
        token_info->algo = SHA1;
        break;
    case SHA256:
        token_info->algo = SHA256;
        break;
    case SHA512:
        token_info->algo = SHA512;
        break;
    case STEAM:
        token_info->algo = STEAM;
        break;
    default:
        return false;
    }

    return true;
}

char* token_info_get_algo_as_cstr(const TokenInfo* token_info) {
    switch(token_info->algo) {
    case SHA1:
        return TOTP_TOKEN_ALGO_SHA1_NAME;
    case SHA256:
        return TOTP_TOKEN_ALGO_SHA256_NAME;
    case SHA512:
        return TOTP_TOKEN_ALGO_SHA512_NAME;
    case STEAM:
        return TOTP_TOKEN_ALGO_STEAM_NAME;
    default:
        break;
    }

    return NULL;
}

bool token_info_set_automation_feature_from_str(TokenInfo* token_info, const FuriString* str) {
    if(furi_string_cmpi_str(str, TOTP_TOKEN_AUTOMATION_FEATURE_ENTER_AT_THE_END_NAME) == 0) {
        token_info->automation_features |= TokenAutomationFeatureEnterAtTheEnd;
        return true;
    }

    if(furi_string_cmpi_str(str, TOTP_TOKEN_AUTOMATION_FEATURE_TAB_AT_THE_END_NAME) == 0) {
        token_info->automation_features |= TokenAutomationFeatureTabAtTheEnd;
        return true;
    }

    if(furi_string_cmpi_str(str, TOTP_TOKEN_AUTOMATION_FEATURE_TYPE_SLOWER_NAME) == 0) {
        token_info->automation_features |= TokenAutomationFeatureTypeSlower;
        return true;
    }

    if(furi_string_cmpi_str(str, TOTP_TOKEN_AUTOMATION_FEATURE_NONE_NAME) == 0) {
        token_info->automation_features = TokenAutomationFeatureNone;
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

    clone->name = furi_string_alloc();
    furi_string_set(clone->name, src->name);

    return clone;
}

void token_info_set_defaults(TokenInfo* token_info) {
    furi_check(token_info != NULL);
    token_info->algo = SHA1;
    token_info->digits = TotpSixDigitsCount;
    token_info->duration = TOTP_TOKEN_DURATION_DEFAULT;
    token_info->automation_features = TokenAutomationFeatureNone;
    furi_string_reset(token_info->name);
}