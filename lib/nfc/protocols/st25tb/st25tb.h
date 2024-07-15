#pragma once

#include <nfc/protocols/nfc_device_base_i.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ST25TB_UID_SIZE (8U)

//#define ST25TB_FDT_FC (4205U)
#define ST25TB_FDT_FC           (8494U)
#define ST25TB_GUARD_TIME_US    (5000U)
#define ST25TB_POLL_POLL_MIN_US (1280U)

#define ST25TB_MAX_BLOCKS       (128U)
#define ST25TB_SYSTEM_OTP_BLOCK (0xFFU)
#define ST25TB_BLOCK_SIZE       (4U)

typedef enum {
    St25tbErrorNone,
    St25tbErrorNotPresent,
    St25tbErrorColResFailed,
    St25tbErrorBufferOverflow,
    St25tbErrorCommunication,
    St25tbErrorFieldOff,
    St25tbErrorWrongCrc,
    St25tbErrorTimeout,
    St25tbErrorWriteFailed,
} St25tbError;

typedef enum {
    St25tbType512At,
    St25tbType512Ac,
    St25tbTypeX512,
    St25tbType02k,
    St25tbType04k,
    St25tbTypeX4k,
    St25tbTypeNum,
} St25tbType;

typedef struct {
    uint8_t uid[ST25TB_UID_SIZE];
    St25tbType type;
    uint32_t blocks[ST25TB_MAX_BLOCKS];
    uint32_t system_otp_block;
} St25tbData;

extern const NfcDeviceBase nfc_device_st25tb;

St25tbData* st25tb_alloc(void);

void st25tb_free(St25tbData* data);

void st25tb_reset(St25tbData* data);

void st25tb_copy(St25tbData* data, const St25tbData* other);

bool st25tb_verify(St25tbData* data, const FuriString* device_type);

bool st25tb_load(St25tbData* data, FlipperFormat* ff, uint32_t version);

bool st25tb_save(const St25tbData* data, FlipperFormat* ff);

bool st25tb_is_equal(const St25tbData* data, const St25tbData* other);

uint8_t st25tb_get_block_count(St25tbType type);

const char* st25tb_get_device_name(const St25tbData* data, NfcDeviceNameType name_type);

const uint8_t* st25tb_get_uid(const St25tbData* data, size_t* uid_len);

bool st25tb_set_uid(St25tbData* data, const uint8_t* uid, size_t uid_len);

St25tbData* st25tb_get_base_data(const St25tbData* data);

St25tbType st25tb_get_type_from_uid(const uint8_t* uid);

#ifdef __cplusplus
}
#endif
