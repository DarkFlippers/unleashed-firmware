#pragma once

#define PLUGIN_APP_ID      "mfkey"
#define PLUGIN_API_VERSION 1

typedef struct {
    const char* name;
    bool (*napi_mf_classic_mfkey32_nonces_check_presence)();
    bool (*napi_mf_classic_nested_nonces_check_presence)();
    MfClassicNonceArray* (
        *napi_mf_classic_nonce_array_alloc)(KeysDict*, bool, KeysDict*, ProgramState*);
    void (*napi_mf_classic_nonce_array_free)(MfClassicNonceArray*);
} MfkeyPlugin;
