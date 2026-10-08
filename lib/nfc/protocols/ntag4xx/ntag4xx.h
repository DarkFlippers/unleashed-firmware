#pragma once

#include <lib/nfc/protocols/iso14443_4a/iso14443_4a.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NTAG4XX_UID_SIZE            (7)
#define NTAG4XX_BATCH_SIZE          (4)
#define NTAG4XX_BATCH_EXTRA_BITS    4
#define NTAG4XX_FAB_KEY_SIZE_BITS_4 4
#define NTAG4XX_FAB_KEY_SIZE_BITS_1 1
#define NTAG4XX_PROD_WEEK_SIZE_BITS 7

#define NTAG4XX_CMD_GET_VERSION (0x60)

typedef enum {
    Ntag4xxErrorNone,
    Ntag4xxErrorNotPresent,
    Ntag4xxErrorProtocol,
    Ntag4xxErrorTimeout,
} Ntag4xxError;

typedef enum {
    Ntag4xxType413DNA,
    Ntag4xxType424DNA,
    Ntag4xxType424DNATT,
    Ntag4xxType426QDNA,
    Ntag4xxType426QDNATT,

    Ntag4xxTypeUnknown,
    Ntag4xxTypeNum,
} Ntag4xxType;

#pragma pack(push, 1)
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

    uint8_t uid[NTAG4XX_UID_SIZE];
    // [36b batch][5b fab key][7b prod week]
    // 5b fab key is split 4b in last byte of batch and 1b in prod week
    // Due to endianness, they appear swapped in the struct definition
    uint8_t batch[NTAG4XX_BATCH_SIZE];
    struct {
        uint8_t fab_key_4b : NTAG4XX_FAB_KEY_SIZE_BITS_4;
        uint8_t batch_extra : NTAG4XX_BATCH_EXTRA_BITS;
    };
    struct {
        uint8_t prod_week : NTAG4XX_PROD_WEEK_SIZE_BITS;
        uint8_t fab_key_1b : NTAG4XX_FAB_KEY_SIZE_BITS_1;
    };
    uint8_t prod_year;
    struct {
        uint8_t fab_key_id;
    } optional;
} Ntag4xxVersion;
#pragma pack(pop)

typedef struct {
    Iso14443_4aData* iso14443_4a_data;
    Ntag4xxVersion version;
    FuriString* device_name;
} Ntag4xxData;

extern const NfcDeviceBase nfc_device_ntag4xx;

// Virtual methods

Ntag4xxData* ntag4xx_alloc(void);

void ntag4xx_free(Ntag4xxData* data);

void ntag4xx_reset(Ntag4xxData* data);

void ntag4xx_copy(Ntag4xxData* data, const Ntag4xxData* other);

bool ntag4xx_verify(Ntag4xxData* data, const FuriString* device_type);

bool ntag4xx_load(Ntag4xxData* data, FlipperFormat* ff, uint32_t version);

bool ntag4xx_save(const Ntag4xxData* data, FlipperFormat* ff);

bool ntag4xx_is_equal(const Ntag4xxData* data, const Ntag4xxData* other);

const char* ntag4xx_get_device_name(const Ntag4xxData* data, NfcDeviceNameType name_type);

const uint8_t* ntag4xx_get_uid(const Ntag4xxData* data, size_t* uid_len);

bool ntag4xx_set_uid(Ntag4xxData* data, const uint8_t* uid, size_t uid_len);

Iso14443_4aData* ntag4xx_get_base_data(const Ntag4xxData* data);

// Helpers

Ntag4xxType ntag4xx_get_type_from_version(const Ntag4xxVersion* const version);

#ifdef __cplusplus
}
#endif
