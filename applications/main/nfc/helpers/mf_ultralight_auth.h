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
} MfUltralightAuthType;

typedef struct {
    MfUltralightAuthType type;
    MfUltralightAuthPassword password;
    MfUltralightC3DesAuthKey tdes_key;
    MfUltralightAuthPack pack;
} MfUltralightAuth;

MfUltralightAuth* mf_ultralight_auth_alloc(void);

void mf_ultralight_auth_free(MfUltralightAuth* instance);

void mf_ultralight_auth_reset(MfUltralightAuth* instance);

bool mf_ultralight_generate_amiibo_pass(MfUltralightAuth* instance, uint8_t* uid, uint16_t uid_len);

bool mf_ultralight_generate_xiaomi_pass(MfUltralightAuth* instance, uint8_t* uid, uint16_t uid_len);

#ifdef __cplusplus
}
#endif
