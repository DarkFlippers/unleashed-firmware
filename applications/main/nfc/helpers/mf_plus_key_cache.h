#pragma once

#include <nfc/protocols/mf_plus/mf_plus.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MfPlusKeyCache MfPlusKeyCache;

MfPlusKeyCache* mf_plus_key_cache_alloc(void);

void mf_plus_key_cache_free(MfPlusKeyCache* instance);

// Persist every recovered SL3 key of `data` (per-sector AES A/B + 0x90xx admin keys) to
// /ext/nfc/.cache/{UID}.mfpkeys, so a later read of the same card authenticates from the cache
// instead of re-running the whole dictionary attack. A no-op returning true when the card has no
// recovered keys (nothing worth caching). Instance-free, mirroring mf_classic_key_cache_save.
bool mf_plus_key_cache_save(const MfPlusData* data);

// Load {UID}.mfpkeys into `instance`. Returns true only if a valid cache existed and parsed; on any
// failure the instance is left empty.
bool mf_plus_key_cache_load(MfPlusKeyCache* instance, const uint8_t* uid, size_t uid_len);

// Targeted lookup of a cached sector key. Returns false if that (sector, key type) isn't cached.
bool mf_plus_key_cache_get_sector_key(
    const MfPlusKeyCache* instance,
    uint8_t sector,
    MfPlusKeyType key_type,
    MfPlusKey* out_key);

// Targeted lookup of a cached admin (0x90xx) key. Returns false if that admin key isn't cached.
bool mf_plus_key_cache_get_admin_key(
    const MfPlusKeyCache* instance,
    MfPlusAdminKeyType admin_type,
    MfPlusKey* out_key);

void mf_plus_key_cache_reset(MfPlusKeyCache* instance);

#ifdef __cplusplus
}
#endif
