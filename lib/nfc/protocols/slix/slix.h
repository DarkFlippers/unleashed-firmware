#pragma once

#include <nfc/protocols/iso15693_3/iso15693_3.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SLIX_BLOCK_SIZE     (4U)
#define SLIX_SIGNATURE_SIZE (32U)

#define SLIX_COUNTER_BLOCK_NUM (79U)

#define SLIX_PP_CONDITION_RL (1U << 0)
#define SLIX_PP_CONDITION_WL (1U << 1)
#define SLIX_PP_CONDITION_RH (1U << 4)
#define SLIX_PP_CONDITION_WH (1U << 5)

#define SLIX_FEATURE_FLAG_UM_PP              (1UL << 0)
#define SLIX_FEATURE_FLAG_COUNTER            (1UL << 1)
#define SLIX_FEATURE_FLAG_EAS_ID             (1UL << 2)
#define SLIX_FEATURE_FLAG_EAS_PP             (1UL << 3)
#define SLIX_FEATURE_FLAG_AFI_PP             (1UL << 4)
#define SLIX_FEATURE_FLAG_INVENTORY_READ_EXT (1UL << 5)
#define SLIX_FEATURE_FLAG_EAS_IR             (1UL << 6)
#define SLIX_FEATURE_FLAG_ORIGINALITY_SIG    (1UL << 8)
#define SLIX_FEATURE_FLAG_ORIGINALITY_SIG_PP (1UL << 9)
#define SLIX_FEATURE_FLAG_PERSISTENT_QUIET   (1UL << 10)
#define SLIX_FEATURE_FLAG_PRIVACY            (1UL << 12)
#define SLIX_FEATURE_FLAG_DESTROY            (1UL << 13)
#define SLIX_FEATURE_EXT                     (1UL << 31)

#define SLIX_TYPE_FEATURE_READ            (1U << 0)
#define SLIX_TYPE_FEATURE_WRITE           (1U << 1)
#define SLIX_TYPE_FEATURE_PRIVACY         (1U << 2)
#define SLIX_TYPE_FEATURE_DESTROY         (1U << 3)
#define SLIX_TYPE_FEATURE_EAS             (1U << 4)
#define SLIX_TYPE_FEATURE_SIGNATURE       (1U << 5)
#define SLIX_TYPE_FEATURE_PROTECTION      (1U << 6)
#define SLIX_TYPE_FEATURE_NFC_SYSTEM_INFO (1U << 7)

typedef uint32_t SlixTypeFeatures;

typedef enum {
    SlixErrorNone,
    SlixErrorTimeout,
    SlixErrorFormat,
    SlixErrorNotSupported,
    SlixErrorInternal,
    SlixErrorWrongPassword,
    SlixErrorUidMismatch,
    SlixErrorUnknown,
} SlixError;

typedef enum {
    SlixTypeSlix,
    SlixTypeSlixS,
    SlixTypeSlixL,
    SlixTypeSlix2,

    SlixTypeCount,
    SlixTypeUnknown,
} SlixType;

typedef enum {
    SlixPasswordTypeRead,
    SlixPasswordTypeWrite,
    SlixPasswordTypePrivacy,
    SlixPasswordTypeDestroy,
    SlixPasswordTypeEasAfi,
    SlixPasswordTypeCount,
} SlixPasswordType;

typedef uint32_t SlixPassword;
typedef uint8_t SlixSignature[SLIX_SIGNATURE_SIZE];
typedef bool SlixPrivacy;
typedef uint16_t SlixRandomNumber;

typedef struct {
    uint8_t pointer;
    uint8_t condition;
} SlixProtection;

typedef struct {
    bool eas;
    bool ppl;
} SlixLockBits;

typedef struct {
    SlixProtection protection;
    SlixLockBits lock_bits;
} SlixSystemInfo;

typedef enum {
    SlixCapabilitiesDefault,
    SlixCapabilitiesAcceptAllPasswords,

    SlixCapabilitiesCount,
} SlixCapabilities;

typedef struct {
    Iso15693_3Data* iso15693_3_data;
    SlixSystemInfo system_info;
    SlixSignature signature;
    SlixPassword passwords[SlixPasswordTypeCount];
    SlixPrivacy privacy;
    SlixCapabilities capabilities;
} SlixData;

SlixData* slix_alloc(void);

void slix_free(SlixData* data);

void slix_reset(SlixData* data);

void slix_copy(SlixData* data, const SlixData* other);

bool slix_verify(SlixData* data, const FuriString* device_type);

bool slix_load(SlixData* data, FlipperFormat* ff, uint32_t version);

bool slix_save(const SlixData* data, FlipperFormat* ff);

bool slix_is_equal(const SlixData* data, const SlixData* other);

const char* slix_get_device_name(const SlixData* data, NfcDeviceNameType name_type);

const uint8_t* slix_get_uid(const SlixData* data, size_t* uid_len);

bool slix_set_uid(SlixData* data, const uint8_t* uid, size_t uid_len);

const Iso15693_3Data* slix_get_base_data(const SlixData* data);

// Getters and tests

SlixType slix_get_type(const SlixData* data);

SlixPassword slix_get_password(const SlixData* data, SlixPasswordType password_type);

uint16_t slix_get_counter(const SlixData* data);

bool slix_is_privacy_mode(const SlixData* data);

bool slix_is_block_protected(
    const SlixData* data,
    SlixPasswordType password_type,
    uint8_t block_num);

bool slix_is_counter_increment_protected(const SlixData* data);

// Static methods
bool slix_type_has_features(SlixType slix_type, SlixTypeFeatures features);

bool slix_type_supports_password(SlixType slix_type, SlixPasswordType password_type);

#ifdef __cplusplus
}
#endif
