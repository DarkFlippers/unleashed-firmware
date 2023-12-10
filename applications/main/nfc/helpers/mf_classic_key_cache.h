#pragma once

#include <nfc/protocols/mf_classic/mf_classic.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MfClassicKeyCache MfClassicKeyCache;

MfClassicKeyCache* mf_classic_key_cache_alloc();

void mf_classic_key_cache_free(MfClassicKeyCache* instance);

bool mf_classic_key_cache_load(MfClassicKeyCache* instance, const uint8_t* uid, size_t uid_len);

void mf_classic_key_cache_load_from_data(MfClassicKeyCache* instance, const MfClassicData* data);

bool mf_classic_key_cahce_get_next_key(
    MfClassicKeyCache* instance,
    uint8_t* sector_num,
    MfClassicKey* key,
    MfClassicKeyType* key_type);

bool mf_classic_key_cache_save(MfClassicKeyCache* instance, const MfClassicData* data);

void mf_classic_key_cache_reset(MfClassicKeyCache* instance);

#ifdef __cplusplus
}
#endif
