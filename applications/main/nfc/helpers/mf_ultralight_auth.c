#include "mf_ultralight_auth.h"

#include <furi.h>
#include <mbedtls/sha1.h>

MfUltralightAuth* mf_ultralight_auth_alloc(void) {
    MfUltralightAuth* instance = malloc(sizeof(MfUltralightAuth));

    return instance;
}

void mf_ultralight_auth_free(MfUltralightAuth* instance) {
    furi_assert(instance);

    free(instance);
}

void mf_ultralight_auth_reset(MfUltralightAuth* instance) {
    furi_assert(instance);

    instance->type = MfUltralightAuthTypeNone;
    memset(&instance->password, 0, sizeof(MfUltralightAuthPassword));
    memset(&instance->tdes_key, 0, sizeof(MfUltralightC3DesAuthKey));
    memset(&instance->pack, 0, sizeof(MfUltralightAuthPack));
}

bool mf_ultralight_generate_amiibo_pass(MfUltralightAuth* instance, uint8_t* uid, uint16_t uid_len) {
    furi_assert(instance);
    furi_assert(uid);

    bool generated = false;
    if(uid_len == 7) {
        instance->password.data[0] = uid[1] ^ uid[3] ^ 0xAA;
        instance->password.data[1] = uid[2] ^ uid[4] ^ 0x55;
        instance->password.data[2] = uid[3] ^ uid[5] ^ 0xAA;
        instance->password.data[3] = uid[4] ^ uid[6] ^ 0x55;
        generated = true;
    }

    return generated;
}

bool mf_ultralight_generate_xiaomi_pass(MfUltralightAuth* instance, uint8_t* uid, uint16_t uid_len) {
    furi_assert(instance);
    furi_assert(uid);

    uint8_t hash[20];
    bool generated = false;
    if(uid_len == 7) {
        mbedtls_sha1(uid, uid_len, hash);
        instance->password.data[0] = (hash[hash[0] % 20]);
        instance->password.data[1] = (hash[(hash[0] + 5) % 20]);
        instance->password.data[2] = (hash[(hash[0] + 13) % 20]);
        instance->password.data[3] = (hash[(hash[0] + 17) % 20]);
        generated = true;
    }

    return generated;
}
