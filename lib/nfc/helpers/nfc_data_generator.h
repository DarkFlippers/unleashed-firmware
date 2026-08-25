#pragma once

#include <nfc/nfc_device.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NfcDataGeneratorTypeMfUltralight,
    NfcDataGeneratorTypeMfUltralightEV1_11,
    NfcDataGeneratorTypeMfUltralightEV1_H11,
    NfcDataGeneratorTypeMfUltralightEV1_21,
    NfcDataGeneratorTypeMfUltralightEV1_H21,
    NfcDataGeneratorTypeMfUltralightC,
    NfcDataGeneratorTypeMfUltralightAES,
    NfcDataGeneratorTypeNTAG203,
    NfcDataGeneratorTypeNTAG213,
    NfcDataGeneratorTypeNTAG215,
    NfcDataGeneratorTypeNTAG216,
    NfcDataGeneratorTypeNTAGI2C1k,
    NfcDataGeneratorTypeNTAGI2C2k,
    NfcDataGeneratorTypeNTAGI2CPlus1k,
    NfcDataGeneratorTypeNTAGI2CPlus2k,

    NfcDataGeneratorTypeMfClassicMini,
    NfcDataGeneratorTypeMfClassic1k_4b,
    NfcDataGeneratorTypeMfClassic1k_7b,
    NfcDataGeneratorTypeMfClassic4k_4b,
    NfcDataGeneratorTypeMfClassic4k_7b,

    NfcDataGeneratorTypeMfPlusSE_4b,
    NfcDataGeneratorTypeMfPlusSE_7b,
    NfcDataGeneratorTypeMfPlusS2k_4b,
    NfcDataGeneratorTypeMfPlusS2k_7b,
    NfcDataGeneratorTypeMfPlusS4k_4b,
    NfcDataGeneratorTypeMfPlusS4k_7b,
    NfcDataGeneratorTypeMfPlusX2k_4b,
    NfcDataGeneratorTypeMfPlusX2k_7b,
    NfcDataGeneratorTypeMfPlusX4k_4b,
    NfcDataGeneratorTypeMfPlusX4k_7b,
    NfcDataGeneratorTypeMfPlusEV1_2k_4b,
    NfcDataGeneratorTypeMfPlusEV1_2k_7b,
    NfcDataGeneratorTypeMfPlusEV1_4k_4b,
    NfcDataGeneratorTypeMfPlusEV1_4k_7b,
    NfcDataGeneratorTypeMfPlusEV2_2k_4b,
    NfcDataGeneratorTypeMfPlusEV2_2k_7b,
    NfcDataGeneratorTypeMfPlusEV2_4k_4b,
    NfcDataGeneratorTypeMfPlusEV2_4k_7b,

    NfcDataGeneratorTypeNum,

} NfcDataGeneratorType;

const char* nfc_data_generator_get_name(NfcDataGeneratorType type);

void nfc_data_generator_fill_data(NfcDataGeneratorType type, NfcDevice* nfc_device);

#ifdef __cplusplus
}
#endif
