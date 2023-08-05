#include "token_info.h"
#include <furi/core/check.h>
#include <base32.h>
#include <base64.h>
#include <memset_s.h>
#include "common.h"
#include "../services/crypto/crypto_facade.h"

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
    const CryptoSettings* crypto_settings) {
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

        token_info->token = totp_crypto_encrypt(
            plain_secret, plain_secret_length, crypto_settings, &token_info->token_length);
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
        token_info->digits = TokenDigitsCountFive;
        return true;
    case 6:
        token_info->digits = TokenDigitsCountSix;
        return true;
    case 8:
        token_info->digits = TokenDigitsCountEight;
        return true;
    default:
        break;
    }

    return false;
}

bool token_info_set_duration_from_int(TokenInfo* token_info, uint8_t duration) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if(duration >= TokenDurationMin && duration <= TokenDurationMax) { //-V560
        token_info->duration = duration;
        return true;
    }
#pragma GCC diagnostic pop

    return false;
}

bool token_info_set_algo_from_str(TokenInfo* token_info, const FuriString* str) {
    if(furi_string_cmpi_str(str, TOKEN_HASH_ALGO_SHA1_NAME) == 0) {
        token_info->algo = TokenHashAlgoSha1;
        return true;
    }

    if(furi_string_cmpi_str(str, TOKEN_HASH_ALGO_SHA256_NAME) == 0) {
        token_info->algo = TokenHashAlgoSha256;
        return true;
    }

    if(furi_string_cmpi_str(str, TOKEN_HASH_ALGO_SHA512_NAME) == 0) {
        token_info->algo = TokenHashAlgoSha512;
        return true;
    }

    if(furi_string_cmpi_str(str, TOKEN_HASH_ALGO_STEAM_NAME) == 0) {
        token_info->algo = TokenHashAlgoSteam;
        return true;
    }

    return false;
}

bool token_info_set_algo_from_int(TokenInfo* token_info, uint8_t algo_code) {
    switch(algo_code) {
    case TokenHashAlgoSha1:
        token_info->algo = TokenHashAlgoSha1;
        break;
    case TokenHashAlgoSha256:
        token_info->algo = TokenHashAlgoSha256;
        break;
    case TokenHashAlgoSha512:
        token_info->algo = TokenHashAlgoSha512;
        break;
    case TokenHashAlgoSteam:
        token_info->algo = TokenHashAlgoSteam;
        break;
    default:
        return false;
    }

    return true;
}

const char* token_info_get_algo_as_cstr(const TokenInfo* token_info) {
    switch(token_info->algo) {
    case TokenHashAlgoSha1:
        return TOKEN_HASH_ALGO_SHA1_NAME;
    case TokenHashAlgoSha256:
        return TOKEN_HASH_ALGO_SHA256_NAME;
    case TokenHashAlgoSha512:
        return TOKEN_HASH_ALGO_SHA512_NAME;
    case TokenHashAlgoSteam:
        return TOKEN_HASH_ALGO_STEAM_NAME;
    default:
        break;
    }

    return NULL;
}

bool token_info_set_automation_feature_from_str(TokenInfo* token_info, const FuriString* str) {
    if(furi_string_cmpi_str(str, TOKEN_AUTOMATION_FEATURE_ENTER_AT_THE_END_NAME) == 0) {
        token_info->automation_features |= TokenAutomationFeatureEnterAtTheEnd;
        return true;
    }

    if(furi_string_cmpi_str(str, TOKEN_AUTOMATION_FEATURE_TAB_AT_THE_END_NAME) == 0) {
        token_info->automation_features |= TokenAutomationFeatureTabAtTheEnd;
        return true;
    }

    if(furi_string_cmpi_str(str, TOKEN_AUTOMATION_FEATURE_TYPE_SLOWER_NAME) == 0) {
        token_info->automation_features |= TokenAutomationFeatureTypeSlower;
        return true;
    }

    if(furi_string_cmpi_str(str, TOKEN_AUTOMATION_FEATURE_NONE_NAME) == 0) {
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
    token_info->algo = TokenHashAlgoDefault;
    token_info->digits = TokenDigitsCountDefault;
    token_info->duration = TokenDurationDefault;
    token_info->automation_features = TokenAutomationFeatureNone;
    furi_string_reset(token_info->name);
}