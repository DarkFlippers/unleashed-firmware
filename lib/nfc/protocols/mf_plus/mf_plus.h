#pragma once

#include <lib/nfc/protocols/iso14443_4a/iso14443_4a.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MF_PLUS_UID_SIZE_MAX (7)
#define MF_PLUS_BATCH_SIZE   (5)

#define MF_PLUS_CMD_GET_VERSION (0x60)

typedef enum {
    MfPlusErrorNone,
    MfPlusErrorUnknown,
    MfPlusErrorNotPresent,
    MfPlusErrorProtocol,
    MfPlusErrorAuth,
    MfPlusErrorPartialRead,
    MfPlusErrorTimeout,
} MfPlusError;

typedef enum {
    MfPlusTypePlus,
    MfPlusTypeEV1,
    MfPlusTypeEV2,
    MfPlusTypeS,
    MfPlusTypeSE,
    MfPlusTypeX,

    MfPlusTypeUnknown,
    MfPlusTypeNum,
} MfPlusType;

typedef enum {
    MfPlusSize1K,
    MfPlusSize2K,
    MfPlusSize4K,

    MfPlusSizeUnknown,
    MfPlusSizeNum,
} MfPlusSize;

typedef enum {
    MfPlusSecurityLevel0,
    MfPlusSecurityLevel1,
    MfPlusSecurityLevel2,
    MfPlusSecurityLevel3,

    MfPlusSecurityLevelUnknown,
    MfPlusSecurityLevelNum,
} MfPlusSecurityLevel;

typedef struct {
    uint8_t hw_vendor;
    uint8_t hw_type;
    uint8_t hw_subtype;
    uint8_t hw_major;
    uint8_t hw_minor;
    uint8_t hw_storage;
    uint8_t hw_proto;

    uint8_t sw_vendor;
    uint8_t sw_type;
    uint8_t sw_subtype;
    uint8_t sw_major;
    uint8_t sw_minor;
    uint8_t sw_storage;
    uint8_t sw_proto;

    uint8_t uid[MF_PLUS_UID_SIZE_MAX];
    uint8_t batch[MF_PLUS_BATCH_SIZE];
    uint8_t prod_week;
    uint8_t prod_year;
} MfPlusVersion;

typedef struct {
    Iso14443_4aData* iso14443_4a_data;
    MfPlusVersion version;
    MfPlusType type;
    MfPlusSize size;
    MfPlusSecurityLevel security_level;
    FuriString* device_name;
} MfPlusData;

extern const NfcDeviceBase nfc_device_mf_plus;

MfPlusData* mf_plus_alloc(void);

void mf_plus_free(MfPlusData* data);

void mf_plus_reset(MfPlusData* data);

void mf_plus_copy(MfPlusData* data, const MfPlusData* other);

bool mf_plus_verify(MfPlusData* data, const FuriString* device_type);

bool mf_plus_load(MfPlusData* data, FlipperFormat* ff, uint32_t version);

bool mf_plus_save(const MfPlusData* data, FlipperFormat* ff);

bool mf_plus_is_equal(const MfPlusData* data, const MfPlusData* other);

const char* mf_plus_get_device_name(const MfPlusData* data, NfcDeviceNameType name_type);

const uint8_t* mf_plus_get_uid(const MfPlusData* data, size_t* uid_len);

bool mf_plus_set_uid(MfPlusData* data, const uint8_t* uid, size_t uid_len);

Iso14443_4aData* mf_plus_get_base_data(const MfPlusData* data);

#ifdef __cplusplus
}
#endif
