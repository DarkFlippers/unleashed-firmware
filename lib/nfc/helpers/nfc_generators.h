#pragma once

#include "../nfc_device.h"

typedef void (*NfcGeneratorFunc)(NfcDeviceData* data);

typedef struct {
    const char* name;
    NfcGeneratorFunc generator_func;
} NfcGenerator;

extern const NfcGenerator* const nfc_generators[];

void nfc_generate_mf_classic(NfcDeviceData* data, uint8_t uid_len, MfClassicType type);
