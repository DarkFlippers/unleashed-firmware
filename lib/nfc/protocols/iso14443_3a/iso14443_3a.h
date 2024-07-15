#pragma once

#include <toolbox/bit_buffer.h>
#include <nfc/protocols/nfc_device_base_i.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ISO14443_3A_UID_4_BYTES  (4U)
#define ISO14443_3A_UID_7_BYTES  (7U)
#define ISO14443_3A_UID_10_BYTES (10U)
#define ISO14443_3A_MAX_UID_SIZE ISO14443_3A_UID_10_BYTES

#define ISO14443_3A_GUARD_TIME_US     (5000)
#define ISO14443_3A_FDT_POLL_FC       (1620)
#define ISO14443_3A_FDT_LISTEN_FC     (1172)
#define ISO14443_3A_POLLER_MASK_RX_FS ((ISO14443_3A_FDT_LISTEN_FC) / 2)
#define ISO14443_3A_POLL_POLL_MIN_US  (1100)

typedef enum {
    Iso14443_3aErrorNone,
    Iso14443_3aErrorNotPresent,
    Iso14443_3aErrorColResFailed,
    Iso14443_3aErrorBufferOverflow,
    Iso14443_3aErrorCommunication,
    Iso14443_3aErrorFieldOff,
    Iso14443_3aErrorWrongCrc,
    Iso14443_3aErrorTimeout,
} Iso14443_3aError;

typedef struct {
    uint8_t sens_resp[2];
} Iso14443_3aSensResp;

typedef struct {
    uint8_t sel_cmd;
    uint8_t sel_par;
    uint8_t data[4]; // max data bit is 32
} Iso14443_3aSddReq;

typedef struct {
    uint8_t nfcid[4];
    uint8_t bss;
} Iso14443_3aSddResp;

typedef struct {
    uint8_t sel_cmd;
    uint8_t sel_par;
    uint8_t nfcid[4];
    uint8_t bcc;
} Iso14443_3aSelReq;

typedef struct {
    uint8_t sak;
} Iso14443_3aSelResp;

typedef struct {
    uint8_t uid[ISO14443_3A_MAX_UID_SIZE];
    uint8_t uid_len;
    uint8_t atqa[2];
    uint8_t sak;
} Iso14443_3aData;

Iso14443_3aData* iso14443_3a_alloc(void);

void iso14443_3a_free(Iso14443_3aData* data);

void iso14443_3a_reset(Iso14443_3aData* data);

void iso14443_3a_copy(Iso14443_3aData* data, const Iso14443_3aData* other);

bool iso14443_3a_verify(Iso14443_3aData* data, const FuriString* device_type);

bool iso14443_3a_load(Iso14443_3aData* data, FlipperFormat* ff, uint32_t version);

bool iso14443_3a_save(const Iso14443_3aData* data, FlipperFormat* ff);

bool iso14443_3a_is_equal(const Iso14443_3aData* data, const Iso14443_3aData* other);

const char* iso14443_3a_get_device_name(const Iso14443_3aData* data, NfcDeviceNameType name_type);

const uint8_t* iso14443_3a_get_uid(const Iso14443_3aData* data, size_t* uid_len);

bool iso14443_3a_set_uid(Iso14443_3aData* data, const uint8_t* uid, size_t uid_len);

Iso14443_3aData* iso14443_3a_get_base_data(const Iso14443_3aData* data);

uint32_t iso14443_3a_get_cuid(const Iso14443_3aData* data);

// Getters and tests

bool iso14443_3a_supports_iso14443_4(const Iso14443_3aData* data);

uint8_t iso14443_3a_get_sak(const Iso14443_3aData* data);

void iso14443_3a_get_atqa(const Iso14443_3aData* data, uint8_t atqa[2]);

// Setters

void iso14443_3a_set_sak(Iso14443_3aData* data, uint8_t sak);

void iso14443_3a_set_atqa(Iso14443_3aData* data, const uint8_t atqa[2]);

#ifdef __cplusplus
}
#endif
