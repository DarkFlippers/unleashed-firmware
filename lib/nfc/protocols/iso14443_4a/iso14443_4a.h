#pragma once

#include <nfc/protocols/iso14443_3a/iso14443_3a.h>

#include <lib/toolbox/simple_array.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    Iso14443_4aErrorNone,
    Iso14443_4aErrorNotPresent,
    Iso14443_4aErrorProtocol,
    Iso14443_4aErrorTimeout,
} Iso14443_4aError;

typedef enum {
    Iso14443_4aBitRateBoth106Kbit,
    Iso14443_4aBitRatePiccToPcd212Kbit,
    Iso14443_4aBitRatePiccToPcd424Kbit,
    Iso14443_4aBitRatePiccToPcd848Kbit,
    Iso14443_4aBitRatePcdToPicc212Kbit,
    Iso14443_4aBitRatePcdToPicc424Kbit,
    Iso14443_4aBitRatePcdToPicc848Kbit,
} Iso14443_4aBitRate;

typedef enum {
    Iso14443_4aFrameOptionNad,
    Iso14443_4aFrameOptionCid,
} Iso14443_4aFrameOption;

typedef struct {
    uint8_t tl;
    uint8_t t0;
    uint8_t ta_1;
    uint8_t tb_1;
    uint8_t tc_1;
    SimpleArray* t1_tk;
} Iso14443_4aAtsData;

typedef struct {
    Iso14443_3aData* iso14443_3a_data;
    Iso14443_4aAtsData ats_data;
} Iso14443_4aData;

// Virtual methods

Iso14443_4aData* iso14443_4a_alloc(void);

void iso14443_4a_free(Iso14443_4aData* data);

void iso14443_4a_reset(Iso14443_4aData* data);

void iso14443_4a_copy(Iso14443_4aData* data, const Iso14443_4aData* other);

bool iso14443_4a_verify(Iso14443_4aData* data, const FuriString* device_type);

bool iso14443_4a_load(Iso14443_4aData* data, FlipperFormat* ff, uint32_t version);

bool iso14443_4a_save(const Iso14443_4aData* data, FlipperFormat* ff);

bool iso14443_4a_is_equal(const Iso14443_4aData* data, const Iso14443_4aData* other);

const char* iso14443_4a_get_device_name(const Iso14443_4aData* data, NfcDeviceNameType name_type);

const uint8_t* iso14443_4a_get_uid(const Iso14443_4aData* data, size_t* uid_len);

bool iso14443_4a_set_uid(Iso14443_4aData* data, const uint8_t* uid, size_t uid_len);

Iso14443_3aData* iso14443_4a_get_base_data(const Iso14443_4aData* data);

// Getters & Tests

uint16_t iso14443_4a_get_frame_size_max(const Iso14443_4aData* data);

uint32_t iso14443_4a_get_fwt_fc_max(const Iso14443_4aData* data);

const uint8_t* iso14443_4a_get_historical_bytes(const Iso14443_4aData* data, uint32_t* count);

bool iso14443_4a_supports_bit_rate(const Iso14443_4aData* data, Iso14443_4aBitRate bit_rate);

bool iso14443_4a_supports_frame_option(const Iso14443_4aData* data, Iso14443_4aFrameOption option);

#ifdef __cplusplus
}
#endif
