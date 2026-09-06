#include "galo_c02.h"

#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"
#include <lib/flipper_format/flipper_format_i.h>

#define TAG "SubGhzProtocolGaloC02"

#define GALO_C02_SYMBOL_COUNT       118U
#define GALO_C02_FRAME_SYMBOL_COUNT (GALO_C02_SYMBOL_COUNT + 1U)
#define GALO_C02_DATA_2_BIT_COUNT   (GALO_C02_SYMBOL_COUNT - 64U)

#define GALO_C02_TE_SHORT 480U
#define GALO_C02_TE_LONG  960U
#define GALO_C02_TE_DELTA 150U

#define GALO_C02_GAP       7680U
#define GALO_C02_GAP_DELTA 2000U

#define GALO_C02_REPEAT_DEFAULT 10U

typedef enum {
    GaloC02DecoderStepReset,
    GaloC02DecoderStepStart,
    GaloC02DecoderStepData,
    GaloC02DecoderStepTail,
    GaloC02DecoderStepGap,
} GaloC02DecoderStep;

struct SubGhzProtocolDecoderGaloC02 {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockGeneric generic;

    GaloC02DecoderStep parser_step;
    bool expected_level;
    uint16_t symbol_count;
};

struct SubGhzProtocolEncoderGaloC02 {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;

    size_t frame_index;
};

const SubGhzProtocolDecoder subghz_protocol_galo_c02_decoder = {
    .alloc = subghz_protocol_decoder_galo_c02_alloc,
    .free = subghz_protocol_decoder_galo_c02_free,

    .feed = subghz_protocol_decoder_galo_c02_feed,
    .reset = subghz_protocol_decoder_galo_c02_reset,

    .get_hash_data = subghz_protocol_decoder_galo_c02_get_hash_data,
    .serialize = subghz_protocol_decoder_galo_c02_serialize,
    .deserialize = subghz_protocol_decoder_galo_c02_deserialize,
    .get_string = subghz_protocol_decoder_galo_c02_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_galo_c02_encoder = {
    .alloc = subghz_protocol_encoder_galo_c02_alloc,
    .free = subghz_protocol_encoder_galo_c02_free,

    .deserialize = subghz_protocol_encoder_galo_c02_deserialize,
    .stop = subghz_protocol_encoder_galo_c02_stop,
    .yield = subghz_protocol_encoder_galo_c02_yield,
};

const SubGhzProtocol subghz_protocol_galo_c02 = {
    .name = SUBGHZ_PROTOCOL_GALO_C02_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_galo_c02_decoder,
    .encoder = &subghz_protocol_galo_c02_encoder,
};

static bool subghz_protocol_galo_c02_get_symbol(uint32_t duration, uint8_t* symbol) {
    bool short_match = DURATION_DIFF(duration, GALO_C02_TE_SHORT) < GALO_C02_TE_DELTA;
    bool long_match = DURATION_DIFF(duration, GALO_C02_TE_LONG) < GALO_C02_TE_DELTA;

    if(short_match == long_match) {
        return false;
    }

    *symbol = long_match ? 1U : 0U;
    return true;
}

static bool subghz_protocol_galo_c02_is_gap(uint32_t duration) {
    return DURATION_DIFF(duration, GALO_C02_GAP) <= GALO_C02_GAP_DELTA;
}

static SubGhzProtocolStatus subghz_protocol_galo_c02_read_data_2(
    SubGhzBlockGeneric* generic,
    FlipperFormat* flipper_format) {
    uint8_t data[sizeof(uint64_t)] = {0};

    if(!flipper_format_rewind(flipper_format)) {
        FURI_LOG_E(TAG, "Rewind error");
        return SubGhzProtocolStatusErrorParserOthers;
    }
    if(!flipper_format_read_hex(flipper_format, "Data", data, sizeof(data))) {
        FURI_LOG_E(TAG, "Missing Data");
        return SubGhzProtocolStatusErrorParserOthers;
    }

    generic->data_2 = 0;
    for(size_t i = 0; i < sizeof(data); i++) {
        generic->data_2 = (generic->data_2 << 8) | data[i];
    }

    return SubGhzProtocolStatusOk;
}

static void subghz_protocol_galo_c02_get_upload(SubGhzProtocolEncoderGaloC02* instance) {
    size_t index = 0;
    for(uint8_t i = 64; i > 0; i--) {
        uint32_t duration = bit_read(instance->generic.data, i - 1) ? GALO_C02_TE_LONG :
                                                                      GALO_C02_TE_SHORT;
        instance->encoder.upload[index] = level_duration_make((index & 1U) == 0U, duration);
        index++;
    }
    for(uint8_t i = GALO_C02_DATA_2_BIT_COUNT; i > 0; i--) {
        uint32_t duration = bit_read(instance->generic.data_2, i - 1) ? GALO_C02_TE_LONG :
                                                                        GALO_C02_TE_SHORT;
        instance->encoder.upload[index] = level_duration_make((index & 1U) == 0U, duration);
        index++;
    }

    instance->encoder.upload[index++] = level_duration_make(
        true, (instance->frame_index & 1U) ? GALO_C02_TE_LONG : GALO_C02_TE_SHORT);
    instance->encoder.upload[index++] = level_duration_make(false, GALO_C02_GAP);
    instance->encoder.size_upload = index;
}

void* subghz_protocol_encoder_galo_c02_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderGaloC02* instance = malloc(sizeof(SubGhzProtocolEncoderGaloC02));
    instance->base.protocol = &subghz_protocol_galo_c02;
    instance->generic.protocol_name = SUBGHZ_PROTOCOL_GALO_C02_NAME;
    instance->encoder.repeat = GALO_C02_REPEAT_DEFAULT;
    instance->encoder.size_upload = GALO_C02_FRAME_SYMBOL_COUNT + 1U;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    instance->encoder.front = 0;
    instance->generic.data = 0;
    instance->generic.data_2 = 0;
    instance->generic.data_count_bit = 0;
    instance->frame_index = 0;
    return instance;
}

void subghz_protocol_encoder_galo_c02_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderGaloC02* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

SubGhzProtocolStatus
    subghz_protocol_encoder_galo_c02_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderGaloC02* instance = context;
    instance->generic.data = 0;
    instance->generic.data_2 = 0;
    SubGhzProtocolStatus status = subghz_block_generic_deserialize_check_count_bit(
        &instance->generic, flipper_format, GALO_C02_SYMBOL_COUNT);

    if(status != SubGhzProtocolStatusOk) {
        return status;
    }
    status = subghz_protocol_galo_c02_read_data_2(&instance->generic, flipper_format);
    if(status != SubGhzProtocolStatusOk) {
        return status;
    }

    uint32_t repeat = GALO_C02_REPEAT_DEFAULT;
    if(flipper_format_read_uint32(flipper_format, "Repeat", &repeat, 1)) {
        instance->encoder.repeat = repeat;
    } else {
        instance->encoder.repeat = GALO_C02_REPEAT_DEFAULT;
    }

    instance->frame_index = 0;
    instance->encoder.front = 0;
    subghz_protocol_galo_c02_get_upload(instance);
    instance->encoder.is_running = true;
    return SubGhzProtocolStatusOk;
}

void subghz_protocol_encoder_galo_c02_stop(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderGaloC02* instance = context;
    instance->encoder.is_running = false;
    instance->encoder.front = 0;
    instance->frame_index = 0;
}

LevelDuration subghz_protocol_encoder_galo_c02_yield(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderGaloC02* instance = context;

    if(instance->encoder.repeat == 0 || !instance->encoder.is_running) {
        instance->encoder.is_running = false;
        return level_duration_reset();
    }

    LevelDuration ret = instance->encoder.upload[instance->encoder.front];

    if(++instance->encoder.front == instance->encoder.size_upload) {
        instance->encoder.front = 0;
        if(!subghz_block_generic_global.endless_tx) {
            instance->encoder.repeat--;
        }
        instance->frame_index++;

        if(instance->encoder.repeat != 0 || subghz_block_generic_global.endless_tx) {
            // The terminal symbol alternates between short and long in the captures.
            subghz_protocol_galo_c02_get_upload(instance);
        }
    }

    return ret;
}

void* subghz_protocol_decoder_galo_c02_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderGaloC02* instance = malloc(sizeof(SubGhzProtocolDecoderGaloC02));
    instance->base.protocol = &subghz_protocol_galo_c02;
    instance->generic.protocol_name = SUBGHZ_PROTOCOL_GALO_C02_NAME;
    instance->generic.data = 0;
    instance->generic.data_2 = 0;
    instance->generic.data_count_bit = 0;
    subghz_protocol_decoder_galo_c02_reset(instance);
    return instance;
}

void subghz_protocol_decoder_galo_c02_free(void* context) {
    furi_assert(context);
    free(context);
}

void subghz_protocol_decoder_galo_c02_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderGaloC02* instance = context;
    instance->parser_step = GaloC02DecoderStepReset;
    instance->expected_level = false;
    instance->symbol_count = 0;
}

void subghz_protocol_decoder_galo_c02_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderGaloC02* instance = context;
    uint8_t symbol = 0;

    switch(instance->parser_step) {
    case GaloC02DecoderStepReset:
        if(!level && subghz_protocol_galo_c02_is_gap(duration)) {
            instance->parser_step = GaloC02DecoderStepStart;
        }
        break;

    case GaloC02DecoderStepStart:
        if(!level && subghz_protocol_galo_c02_is_gap(duration)) {
            break;
        }
        if(!level || !subghz_protocol_galo_c02_get_symbol(duration, &symbol)) {
            instance->parser_step = GaloC02DecoderStepReset;
            break;
        }

        instance->generic.data = symbol != 0U;
        instance->generic.data_2 = 0;
        instance->symbol_count = 1;
        instance->expected_level = false;
        instance->parser_step = GaloC02DecoderStepData;
        break;

    case GaloC02DecoderStepData:
        if(level != instance->expected_level ||
           !subghz_protocol_galo_c02_get_symbol(duration, &symbol)) {
            instance->parser_step = GaloC02DecoderStepReset;
            break;
        }

        if(instance->symbol_count < 64U) {
            instance->generic.data = (instance->generic.data << 1) | (symbol != 0U);
        } else {
            instance->generic.data_2 = (instance->generic.data_2 << 1) | (symbol != 0U);
        }
        instance->symbol_count++;
        if(instance->symbol_count == GALO_C02_SYMBOL_COUNT) {
            instance->generic.data_count_bit = GALO_C02_SYMBOL_COUNT;
            instance->parser_step = GaloC02DecoderStepTail;
        } else {
            instance->expected_level = !instance->expected_level;
        }
        break;

    case GaloC02DecoderStepTail:
        if(!level || !subghz_protocol_galo_c02_get_symbol(duration, &symbol)) {
            instance->parser_step = GaloC02DecoderStepReset;
        } else {
            instance->parser_step = GaloC02DecoderStepGap;
        }
        break;

    case GaloC02DecoderStepGap:
        if(!level && subghz_protocol_galo_c02_is_gap(duration)) {
            if(instance->base.callback) {
                instance->base.callback(&instance->base, instance->base.context);
            }
            instance->parser_step = GaloC02DecoderStepStart;
        } else {
            instance->parser_step = GaloC02DecoderStepReset;
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_galo_c02_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderGaloC02* instance = context;
    uint8_t data[sizeof(uint64_t) * 2] = {0};
    for(size_t i = 0; i < sizeof(uint64_t); i++) {
        data[i] = (instance->generic.data >> (56U - i * 8U)) & 0xFFU;
        data[i + sizeof(uint64_t)] = (instance->generic.data_2 >> (56U - i * 8U)) & 0xFFU;
    }
    return subghz_protocol_blocks_add_bytes(data, sizeof(data));
}

SubGhzProtocolStatus subghz_protocol_decoder_galo_c02_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    furi_assert(flipper_format);
    furi_assert(preset);
    SubGhzProtocolDecoderGaloC02* instance = context;
    SubGhzProtocolStatus status =
        subghz_block_generic_serialize(&instance->generic, flipper_format, preset);

    uint8_t data[sizeof(uint64_t)] = {0};
    for(size_t i = 0; i < sizeof(data); i++) {
        data[sizeof(data) - i - 1] = (instance->generic.data_2 >> (i * 8)) & 0xFFU;
    }

    if(!flipper_format_rewind(flipper_format)) {
        FURI_LOG_E(TAG, "Rewind error");
        status = SubGhzProtocolStatusErrorParserOthers;
    }
    if((status == SubGhzProtocolStatusOk) &&
       !flipper_format_insert_or_update_hex(flipper_format, "Data", data, sizeof(data))) {
        FURI_LOG_E(TAG, "Unable to add Data");
        status = SubGhzProtocolStatusErrorParserOthers;
    }
    return status;
}

SubGhzProtocolStatus
    subghz_protocol_decoder_galo_c02_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderGaloC02* instance = context;
    instance->generic.data = 0;
    instance->generic.data_2 = 0;
    SubGhzProtocolStatus status = subghz_block_generic_deserialize_check_count_bit(
        &instance->generic, flipper_format, GALO_C02_SYMBOL_COUNT);
    if(status != SubGhzProtocolStatusOk) {
        return status;
    }
    return subghz_protocol_galo_c02_read_data_2(&instance->generic, flipper_format);
}

void subghz_protocol_decoder_galo_c02_get_string(void* context, FuriString* output) {
    furi_assert(context);
    furi_assert(output);
    SubGhzProtocolDecoderGaloC02* instance = context;

    furi_string_printf(
        output,
        "%s %ub\r\nKey: %016llX\r\nData: %016llX",
        SUBGHZ_PROTOCOL_GALO_C02_NAME,
        instance->generic.data_count_bit,
        instance->generic.data,
        instance->generic.data_2);
}
