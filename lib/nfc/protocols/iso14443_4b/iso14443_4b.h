#pragma once

#include <nfc/protocols/iso14443_3b/iso14443_3b.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    Iso14443_4bErrorNone,
    Iso14443_4bErrorNotPresent,
    Iso14443_4bErrorProtocol,
    Iso14443_4bErrorTimeout,
} Iso14443_4bError;

typedef struct Iso14443_4bData Iso14443_4bData;

// Virtual methods

Iso14443_4bData* iso14443_4b_alloc(void);

void iso14443_4b_free(Iso14443_4bData* data);

void iso14443_4b_reset(Iso14443_4bData* data);

void iso14443_4b_copy(Iso14443_4bData* data, const Iso14443_4bData* other);

bool iso14443_4b_verify(Iso14443_4bData* data, const FuriString* device_type);

bool iso14443_4b_load(Iso14443_4bData* data, FlipperFormat* ff, uint32_t version);

bool iso14443_4b_save(const Iso14443_4bData* data, FlipperFormat* ff);

bool iso14443_4b_is_equal(const Iso14443_4bData* data, const Iso14443_4bData* other);

const char* iso14443_4b_get_device_name(const Iso14443_4bData* data, NfcDeviceNameType name_type);

const uint8_t* iso14443_4b_get_uid(const Iso14443_4bData* data, size_t* uid_len);

bool iso14443_4b_set_uid(Iso14443_4bData* data, const uint8_t* uid, size_t uid_len);

Iso14443_3bData* iso14443_4b_get_base_data(const Iso14443_4bData* data);

#ifdef __cplusplus
}
#endif
