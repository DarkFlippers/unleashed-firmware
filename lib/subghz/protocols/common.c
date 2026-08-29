#include "common.h"

void subghz_protocol_encoder_common_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderCommon* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

void subghz_protocol_encoder_common_stop(void* context) {
    SubGhzProtocolEncoderCommon* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_common_yield(void* context) {
    SubGhzProtocolEncoderCommon* instance = context;

    if(instance->encoder.repeat == 0 || !instance->encoder.is_running) {
        instance->encoder.is_running = false;
        return level_duration_reset();
    }

    LevelDuration ret = instance->encoder.upload[instance->encoder.front];

    if(++instance->encoder.front == instance->encoder.size_upload) {
        if(!subghz_block_generic_global.endless_tx) instance->encoder.repeat--;
        instance->encoder.front = 0;
    }

    return ret;
}

void subghz_protocol_decoder_common_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderCommon* instance = context;
    free(instance);
}

void subghz_protocol_decoder_common_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderCommon* instance = context;
    instance->decoder.parser_step = 0;
}

uint8_t subghz_protocol_decoder_common_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderCommon* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

SubGhzProtocolStatus subghz_protocol_decoder_common_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderCommon* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}
