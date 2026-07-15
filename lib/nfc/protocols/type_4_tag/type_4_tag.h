#pragma once

#include <lib/nfc/protocols/iso14443_4a/iso14443_4a.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TYPE_4_TAG_MF_DESFIRE_NDEF_SIZE (2048U - sizeof(uint16_t))

typedef enum {
    Type4TagErrorNone,
    Type4TagErrorNotPresent,
    Type4TagErrorProtocol,
    Type4TagErrorTimeout,
    Type4TagErrorWrongFormat,
    Type4TagErrorNotSupported,
    Type4TagErrorApduFailed,
    Type4TagErrorCardUnformatted,
    Type4TagErrorCardLocked,
    Type4TagErrorCustomCommand,
} Type4TagError;

typedef enum {
    Type4TagPlatformUnknown,
    Type4TagPlatformNtag4xx,
    Type4TagPlatformMfDesfire,
} Type4TagPlatform;

typedef struct {
    Iso14443_4aData* iso14443_4a_data;
    FuriString* device_name;
    // Tag specific data
    bool is_tag_specific;
    Type4TagPlatform platform;
    FuriString* platform_name;
    union {
        struct {
            uint8_t minor : 4;
            uint8_t major : 4;
        };
        uint8_t value;
    } t4t_version;
    uint16_t chunk_max_read;
    uint16_t chunk_max_write;
    uint16_t ndef_file_id;
    uint16_t ndef_max_len;
    uint8_t ndef_read_lock;
    uint8_t ndef_write_lock;
    // Data contained, not tag specific
    SimpleArray* ndef_data;
} Type4TagData;

extern const NfcDeviceBase nfc_device_type_4_tag;

// Virtual methods

Type4TagData* type_4_tag_alloc(void);

void type_4_tag_free(Type4TagData* data);

void type_4_tag_reset(Type4TagData* data);

void type_4_tag_copy(Type4TagData* data, const Type4TagData* other);

bool type_4_tag_verify(Type4TagData* data, const FuriString* device_type);

bool type_4_tag_load(Type4TagData* data, FlipperFormat* ff, uint32_t version);

bool type_4_tag_save(const Type4TagData* data, FlipperFormat* ff);

bool type_4_tag_is_equal(const Type4TagData* data, const Type4TagData* other);

const char* type_4_tag_get_device_name(const Type4TagData* data, NfcDeviceNameType name_type);

const uint8_t* type_4_tag_get_uid(const Type4TagData* data, size_t* uid_len);

bool type_4_tag_set_uid(Type4TagData* data, const uint8_t* uid, size_t uid_len);

Iso14443_4aData* type_4_tag_get_base_data(const Type4TagData* data);

#ifdef __cplusplus
}
#endif
