#pragma once

#include <nfc/protocols/nfc_device_base.h>

#include <core/string.h>
#include <toolbox/bit_buffer.h>
#include <flipper_format/flipper_format.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    Iso14443_3bErrorNone,
    Iso14443_3bErrorNotPresent,
    Iso14443_3bErrorColResFailed,
    Iso14443_3bErrorBufferOverflow,
    Iso14443_3bErrorCommunication,
    Iso14443_3bErrorFieldOff,
    Iso14443_3bErrorWrongCrc,
    Iso14443_3bErrorTimeout,
} Iso14443_3bError;

typedef enum {
    Iso14443_3bBitRateBoth106Kbit,
    Iso14443_3bBitRatePiccToPcd212Kbit,
    Iso14443_3bBitRatePiccToPcd424Kbit,
    Iso14443_3bBitRatePiccToPcd848Kbit,
    Iso14443_3bBitRatePcdToPicc212Kbit,
    Iso14443_3bBitRatePcdToPicc424Kbit,
    Iso14443_3bBitRatePcdToPicc848Kbit,
} Iso14443_3bBitRate;

typedef enum {
    Iso14443_3bFrameOptionNad,
    Iso14443_3bFrameOptionCid,
} Iso14443_3bFrameOption;

typedef struct Iso14443_3bData Iso14443_3bData;

// Virtual methods

Iso14443_3bData* iso14443_3b_alloc(void);

void iso14443_3b_free(Iso14443_3bData* data);

void iso14443_3b_reset(Iso14443_3bData* data);

void iso14443_3b_copy(Iso14443_3bData* data, const Iso14443_3bData* other);

bool iso14443_3b_verify(Iso14443_3bData* data, const FuriString* device_type);

bool iso14443_3b_load(Iso14443_3bData* data, FlipperFormat* ff, uint32_t version);

bool iso14443_3b_save(const Iso14443_3bData* data, FlipperFormat* ff);

bool iso14443_3b_is_equal(const Iso14443_3bData* data, const Iso14443_3bData* other);

const char* iso14443_3b_get_device_name(const Iso14443_3bData* data, NfcDeviceNameType name_type);

const uint8_t* iso14443_3b_get_uid(const Iso14443_3bData* data, size_t* uid_len);

bool iso14443_3b_set_uid(Iso14443_3bData* data, const uint8_t* uid, size_t uid_len);

Iso14443_3bData* iso14443_3b_get_base_data(const Iso14443_3bData* data);

// Getters and tests

bool iso14443_3b_supports_iso14443_4(const Iso14443_3bData* data);

bool iso14443_3b_supports_bit_rate(const Iso14443_3bData* data, Iso14443_3bBitRate bit_rate);

bool iso14443_3b_supports_frame_option(const Iso14443_3bData* data, Iso14443_3bFrameOption option);

const uint8_t* iso14443_3b_get_application_data(const Iso14443_3bData* data, size_t* data_size);

uint16_t iso14443_3b_get_frame_size_max(const Iso14443_3bData* data);

uint32_t iso14443_3b_get_fwt_fc_max(const Iso14443_3bData* data);

#ifdef __cplusplus
}
#endif
