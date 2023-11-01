#pragma once

#include "../types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Key generation from simple data.
 * @param context Pointer to a SubGhzProtocolEncoderSecPlus_v2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number, 32 bit
 * @param btn Button number, 8 bit
 * @param cnt Container value, 28 bit
 * @param manufacture_name Name of manufacturer's key
 * @param preset Modulation, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_secplus_v2_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint32_t cnt,
    SubGhzRadioPreset* preset);

/**
 * Key generation from simple data.
 * @param context Pointer to a SubGhzProtocolEncoderKeeloq instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number, 28 bit
 * @param btn Button number, 4 bit
 * @param cnt Container value, 16 bit
 * @param manufacture_name Name of manufacturer's key
 * @param preset Modulation, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_keeloq_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    const char* manufacture_name,
    SubGhzRadioPreset* preset);

typedef struct SubGhzProtocolDecoderBinRAW SubGhzProtocolDecoderBinRAW;

void subghz_protocol_decoder_bin_raw_data_input_rssi(
    SubGhzProtocolDecoderBinRAW* instance,
    float rssi);

/**
 * Validation of fixed parts SubGhzProtocolDecoderSecPlus_v1.
 * @param fixed fixed parts
 * @return true On success
 */
bool subghz_protocol_secplus_v1_check_fixed(uint32_t fixed);

#ifdef __cplusplus
}
#endif
