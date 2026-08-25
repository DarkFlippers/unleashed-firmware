#pragma once

#include <lib/nfc/protocols/mf_ultralight/mf_ultralight.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MfUltralightAuthTypeNone,
    MfUltralightAuthTypeReader,
    MfUltralightAuthTypeManual,
    MfUltralightAuthTypeXiaomi,
    MfUltralightAuthTypeAmiibo,
    MfUltralightAuthTypeUidReveal, // UL-AES: user-requested real-UID reveal (all-zero UIDRetrKey)
} MfUltralightAuthType;

/** What became of the authentication the user asked for, so the read result can say. */
/**
 * Cleared by whoever starts a read, not by mf_ultralight_auth_reset() - it belongs to the
 * read that produced it, and the credential reset runs after the result is drawn.
 */
typedef enum {
    MfUltralightAuthOutcomeNone, /**< None was requested. */
    /** Never read, but it is what clears a Failed left by an earlier attempt in the
     * same unlock flow - the unlock helper deliberately does not reset mid-flow. */
    MfUltralightAuthOutcomeSuccess,
    MfUltralightAuthOutcomeFailed, /**< Attempted and rejected; costs an attempt if AUTHLIM is set. */
    MfUltralightAuthOutcomeSkippedUid, /**< Not attempted: no password derivable from this UID. */
} MfUltralightAuthOutcome;

typedef struct {
    MfUltralightAuthType type;
    MfUltralightAuthPassword password;
    MfUltralightC3DesAuthKey tdes_key;
    MfUltralightAesKey aes_key;
    MfUltralightAuthPack pack;
    MfUltralightAuthOutcome outcome;
} MfUltralightAuth;

MfUltralightAuth* mf_ultralight_auth_alloc(void);

void mf_ultralight_auth_free(MfUltralightAuth* instance);

void mf_ultralight_auth_reset(MfUltralightAuth* instance);

bool mf_ultralight_generate_amiibo_pass(MfUltralightAuth* instance, uint8_t* uid, uint16_t uid_len);

bool mf_ultralight_generate_xiaomi_pass(MfUltralightAuth* instance, uint8_t* uid, uint16_t uid_len);

#ifdef __cplusplus
}
#endif
