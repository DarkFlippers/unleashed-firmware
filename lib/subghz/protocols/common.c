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

SubGhzProtocolStatus subghz_protocol_common_append_data_2(
    SubGhzProtocolStatus status,
    SubGhzBlockGeneric* generic,
    FlipperFormat* flipper_format) {
    uint8_t key_data[sizeof(uint64_t)] = {0};
    for(size_t i = 0; i < sizeof(uint64_t); i++) {
        key_data[sizeof(uint64_t) - i - 1] = (generic->data_2 >> (i * 8)) & 0xFF;
    }

    if(!flipper_format_rewind(flipper_format)) {
        FURI_LOG_E(generic->protocol_name, "Rewind error");
        status = SubGhzProtocolStatusErrorParserOthers;
    }

    if((status == SubGhzProtocolStatusOk) &&
       !flipper_format_insert_or_update_hex(flipper_format, "Data", key_data, sizeof(uint64_t))) {
        FURI_LOG_E(generic->protocol_name, "Unable to add Data");
        status = SubGhzProtocolStatusErrorParserOthers;
    }
    return status;
}

SubGhzProtocolStatus subghz_protocol_decoder_common_serialize_data_2(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderCommon* instance = context;
    SubGhzProtocolStatus ret =
        subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
    return subghz_protocol_common_append_data_2(ret, &instance->generic, flipper_format);
}

SubGhzProtocolStatus subghz_protocol_decoder_common_serialize_te(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderCommonTe* instance = context;
    SubGhzProtocolStatus ret =
        subghz_block_generic_serialize(&instance->common.generic, flipper_format, preset);
    if((ret == SubGhzProtocolStatusOk) &&
       !flipper_format_write_uint32(flipper_format, "TE", &instance->te, 1)) {
        FURI_LOG_E(instance->common.generic.protocol_name, "Unable to add TE");
        ret = SubGhzProtocolStatusErrorParserTe;
    }
    return ret;
}

void* subghz_protocol_decoder_common_alloc(size_t instance_size, const SubGhzProtocol* protocol) {
    SubGhzProtocolDecoderCommon* instance = malloc(instance_size);
    instance->base.protocol = protocol;
    instance->generic.protocol_name = protocol->name;
    return instance;
}

void* subghz_protocol_encoder_common_alloc(
    size_t instance_size,
    const SubGhzProtocol* protocol,
    size_t repeat,
    size_t size_upload) {
    SubGhzProtocolEncoderCommonGeneric* instance = malloc(instance_size);
    instance->common.base.protocol = protocol;
    instance->generic.protocol_name = protocol->name;
    instance->common.encoder.repeat = repeat;
    instance->common.encoder.size_upload = size_upload;
    instance->common.encoder.upload = malloc(size_upload * sizeof(LevelDuration));
    instance->common.encoder.is_running = false;
    return instance;
}
