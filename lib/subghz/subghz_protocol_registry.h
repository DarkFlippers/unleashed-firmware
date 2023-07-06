#pragma once

#include "registry.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const SubGhzProtocolRegistry subghz_protocol_registry;

typedef struct SubGhzProtocolDecoderBinRAW SubGhzProtocolDecoderBinRAW;

bool subghz_protocol_secplus_v2_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint32_t cnt,
    SubGhzRadioPreset* preset);

bool subghz_protocol_keeloq_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    const char* manufacture_name,
    SubGhzRadioPreset* preset);

void subghz_protocol_decoder_bin_raw_data_input_rssi(
    SubGhzProtocolDecoderBinRAW* instance,
    float rssi);

bool subghz_protocol_secplus_v1_check_fixed(uint32_t fixed);

#ifdef __cplusplus
}
#endif
