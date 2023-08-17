#pragma once

#include "../nfc_device.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*NfcGeneratorFunc)(NfcDeviceData* data);

typedef struct {
    const char* name;
    NfcGeneratorFunc generator_func;
} NfcGenerator;

extern const NfcGenerator* const nfc_generators[];

void nfc_generate_mf_classic(NfcDeviceData* data, uint8_t uid_len, MfClassicType type);

void nfc_generate_mf_classic_ext(
    NfcDeviceData* data,
    uint8_t uid_len,
    MfClassicType type,
    bool random_uid,
    uint8_t* uid);

#ifdef __cplusplus
}
#endif