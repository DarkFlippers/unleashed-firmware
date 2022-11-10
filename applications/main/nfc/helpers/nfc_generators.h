#pragma once

#include "../nfc_i.h"

typedef void (*NfcGeneratorFunc)(NfcDeviceData* data);

struct NfcGenerator {
    const char* name;
    NfcGeneratorFunc generator_func;
    NfcScene next_scene;
};

extern const NfcGenerator* const nfc_generators[];

void nfc_generate_mf_classic(NfcDeviceData* data, uint8_t uid_len, MfClassicType type);
