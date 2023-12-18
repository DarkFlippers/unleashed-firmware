#pragma once

#include <toolbox/bit_buffer.h>
#include <nfc/protocols/nfc_device_base_i.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FELICA_IDM_SIZE (8U)
#define FELICA_PMM_SIZE (8U)

#define FELICA_GUARD_TIME_US (20000U)
#define FELICA_FDT_POLL_FC (10000U)
#define FELICA_POLL_POLL_MIN_US (1280U)

#define FELICA_FDT_LISTEN_FC (1172)

#define FELICA_SYSTEM_CODE_CODE (0xFFFFU)
#define FELICA_TIME_SLOT_1 (0x00U)
#define FELICA_TIME_SLOT_2 (0x01U)
#define FELICA_TIME_SLOT_4 (0x03U)
#define FELICA_TIME_SLOT_8 (0x07U)
#define FELICA_TIME_SLOT_16 (0x0FU)

typedef enum {
    FelicaErrorNone,
    FelicaErrorNotPresent,
    FelicaErrorColResFailed,
    FelicaErrorBufferOverflow,
    FelicaErrorCommunication,
    FelicaErrorFieldOff,
    FelicaErrorWrongCrc,
    FelicaErrorProtocol,
    FelicaErrorTimeout,
} FelicaError;

typedef struct {
    uint8_t data[FELICA_IDM_SIZE];
} FelicaIDm;

typedef struct {
    uint8_t data[FELICA_PMM_SIZE];
} FelicaPMm;

typedef struct {
    FelicaIDm idm;
    FelicaPMm pmm;
} FelicaData;

extern const NfcDeviceBase nfc_device_felica;

FelicaData* felica_alloc();

void felica_free(FelicaData* data);

void felica_reset(FelicaData* data);

void felica_copy(FelicaData* data, const FelicaData* other);

bool felica_verify(FelicaData* data, const FuriString* device_type);

bool felica_load(FelicaData* data, FlipperFormat* ff, uint32_t version);

bool felica_save(const FelicaData* data, FlipperFormat* ff);

bool felica_is_equal(const FelicaData* data, const FelicaData* other);

const char* felica_get_device_name(const FelicaData* data, NfcDeviceNameType name_type);

const uint8_t* felica_get_uid(const FelicaData* data, size_t* uid_len);

bool felica_set_uid(FelicaData* data, const uint8_t* uid, size_t uid_len);

FelicaData* felica_get_base_data(const FelicaData* data);

#ifdef __cplusplus
}
#endif
