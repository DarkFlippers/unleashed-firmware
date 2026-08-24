#include "galo_c02.h"

#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"
#include <lib/flipper_format/flipper_format_i.h>
#include <lib/toolbox/stream/stream.h>

#include <string.h>

#define TAG "SubGhzProtocolGaloC02"

#define GALO_C02_SYMBOL_COUNT       118U
#define GALO_C02_FRAME_SYMBOL_COUNT (GALO_C02_SYMBOL_COUNT + 1U)
#define GALO_C02_DATA_SIZE          ((GALO_C02_SYMBOL_COUNT + 7U) / 8U)
#define GALO_C02_PREFIX_SIZE        6U

#define GALO_C02_TE_SHORT 480U
#define GALO_C02_TE_LONG  960U
#define GALO_C02_TE_DELTA 150U

#define GALO_C02_GAP       7680U
#define GALO_C02_GAP_DELTA 2000U

#define GALO_C02_REPEAT_DEFAULT 10U

// The first 48 normalized symbols were common to all supplied captures.
static const uint8_t galo_c02_prefix[GALO_C02_PREFIX_SIZE] = {
    0x00,
    0x03,
    0x8D,
    0x05,
    0xFE,
    0xFF,
};

typedef enum {
    GaloC02DecoderStepReset,
    GaloC02DecoderStepStart,
    GaloC02DecoderStepData,
    GaloC02DecoderStepTail,
    GaloC02DecoderStepGap,
} GaloC02DecoderStep;

struct SubGhzProtocolDecoderGaloC02 {
    SubGhzProtocolDecoderBase base;

    GaloC02DecoderStep parser_step;
    bool expected_level;
    uint16_t symbol_count;
    uint8_t data[GALO_C02_DATA_SIZE];
};

struct SubGhzProtocolEncoderGaloC02 {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    size_t frame_index;
    uint8_t data[GALO_C02_DATA_SIZE];
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

static void
    subghz_protocol_galo_c02_set_bit(uint8_t data[GALO_C02_DATA_SIZE], size_t index, bool value) {
    uint8_t mask = (uint8_t)(1U << (7U - (index & 0x7U)));

    if(value) {
        data[index >> 3] |= mask;
    } else {
        data[index >> 3] &= (uint8_t)~mask;
    }
}

static bool
    subghz_protocol_galo_c02_get_bit(const uint8_t data[GALO_C02_DATA_SIZE], size_t index) {
    return (data[index >> 3] >> (7U - (index & 0x7U))) & 1U;
}

static bool subghz_protocol_galo_c02_data_is_valid(const uint8_t data[GALO_C02_DATA_SIZE]) {
    return (data[GALO_C02_DATA_SIZE - 1U] & 0x03U) == 0U &&
           memcmp(data, galo_c02_prefix, GALO_C02_PREFIX_SIZE) == 0;
}

static SubGhzProtocolStatus subghz_protocol_galo_c02_read_data(
    FlipperFormat* flipper_format,
    uint8_t data[GALO_C02_DATA_SIZE]) {
    uint32_t bit_count = 0;

    if(!flipper_format_rewind(flipper_format)) {
        FURI_LOG_E(TAG, "Rewind error");
        return SubGhzProtocolStatusErrorParserOthers;
    }
    if(!flipper_format_read_uint32(flipper_format, "Bit", &bit_count, 1)) {
        FURI_LOG_E(TAG, "Missing Bit");
        return SubGhzProtocolStatusErrorParserBitCount;
    }
    if(bit_count != GALO_C02_SYMBOL_COUNT) {
        FURI_LOG_E(TAG, "Wrong number of bits in key");
        return SubGhzProtocolStatusErrorValueBitCount;
    }
    if(!flipper_format_read_hex(flipper_format, "Data", data, GALO_C02_DATA_SIZE)) {
        FURI_LOG_E(TAG, "Missing Data");
        return SubGhzProtocolStatusErrorParserOthers;
    }
    if(!subghz_protocol_galo_c02_data_is_valid(data)) {
        FURI_LOG_E(TAG, "Invalid Data");
        return SubGhzProtocolStatusErrorParserOthers;
    }

    return SubGhzProtocolStatusOk;
}

static SubGhzProtocolStatus subghz_protocol_galo_c02_serialize_data(
    const uint8_t data[GALO_C02_DATA_SIZE],
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    SubGhzProtocolStatus status = SubGhzProtocolStatusError;
    FuriString* preset_name = furi_string_alloc();

    do {
        stream_clean(flipper_format_get_raw_stream(flipper_format));
        if(!flipper_format_write_header_cstr(
               flipper_format, SUBGHZ_KEY_FILE_TYPE, SUBGHZ_KEY_FILE_VERSION)) {
            FURI_LOG_E(TAG, "Unable to add header");
            status = SubGhzProtocolStatusErrorParserHeader;
            break;
        }
        if(!flipper_format_write_uint32(flipper_format, "Frequency", &preset->frequency, 1)) {
            FURI_LOG_E(TAG, "Unable to add Frequency");
            status = SubGhzProtocolStatusErrorParserFrequency;
            break;
        }

        subghz_block_generic_get_preset_name(furi_string_get_cstr(preset->name), preset_name);
        if(!flipper_format_write_string_cstr(
               flipper_format, "Preset", furi_string_get_cstr(preset_name))) {
            FURI_LOG_E(TAG, "Unable to add Preset");
            status = SubGhzProtocolStatusErrorParserPreset;
            break;
        }
        if(!strcmp(furi_string_get_cstr(preset_name), "FuriHalSubGhzPresetCustom")) {
            if(!flipper_format_write_string_cstr(
                   flipper_format, "Custom_preset_module", "CC1101")) {
                FURI_LOG_E(TAG, "Unable to add Custom_preset_module");
                status = SubGhzProtocolStatusErrorParserCustomPreset;
                break;
            }
            if(!flipper_format_write_hex(
                   flipper_format, "Custom_preset_data", preset->data, preset->data_size)) {
                FURI_LOG_E(TAG, "Unable to add Custom_preset_data");
                status = SubGhzProtocolStatusErrorParserCustomPreset;
                break;
            }
        }
        if(!flipper_format_write_string_cstr(
               flipper_format, "Protocol", SUBGHZ_PROTOCOL_GALO_C02_NAME)) {
            FURI_LOG_E(TAG, "Unable to add Protocol");
            status = SubGhzProtocolStatusErrorParserProtocolName;
            break;
        }

        uint32_t bit_count = GALO_C02_SYMBOL_COUNT;
        if(!flipper_format_write_uint32(flipper_format, "Bit", &bit_count, 1)) {
            FURI_LOG_E(TAG, "Unable to add Bit");
            status = SubGhzProtocolStatusErrorParserBitCount;
            break;
        }
        if(!flipper_format_write_hex(flipper_format, "Data", data, GALO_C02_DATA_SIZE)) {
            FURI_LOG_E(TAG, "Unable to add Data");
            status = SubGhzProtocolStatusErrorParserOthers;
            break;
        }

        status = SubGhzProtocolStatusOk;
    } while(false);

    furi_string_free(preset_name);
    return status;
}

static void subghz_protocol_galo_c02_get_upload(SubGhzProtocolEncoderGaloC02* instance) {
    for(size_t i = 0; i < GALO_C02_SYMBOL_COUNT; i++) {
        uint32_t duration = subghz_protocol_galo_c02_get_bit(instance->data, i) ?
                                GALO_C02_TE_LONG :
                                GALO_C02_TE_SHORT;
        instance->encoder.upload[i] = level_duration_make((i & 1U) == 0U, duration);
    }

    size_t index = GALO_C02_SYMBOL_COUNT;
    instance->encoder.upload[index++] = level_duration_make(
        true, (instance->frame_index & 1U) ? GALO_C02_TE_LONG : GALO_C02_TE_SHORT);
    instance->encoder.upload[index++] = level_duration_make(false, GALO_C02_GAP);
    instance->encoder.size_upload = index;
}

void* subghz_protocol_encoder_galo_c02_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderGaloC02* instance = malloc(sizeof(SubGhzProtocolEncoderGaloC02));

    instance->base.protocol = &subghz_protocol_galo_c02;
    instance->encoder.repeat = GALO_C02_REPEAT_DEFAULT;
    instance->encoder.size_upload = GALO_C02_FRAME_SYMBOL_COUNT + 1U;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    instance->encoder.front = 0;
    instance->frame_index = 0;
    memset(instance->data, 0, sizeof(instance->data));
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
    SubGhzProtocolStatus status =
        subghz_protocol_galo_c02_read_data(flipper_format, instance->data);

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
    memset(instance->data, 0, sizeof(instance->data));
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

        memset(instance->data, 0, sizeof(instance->data));
        subghz_protocol_galo_c02_set_bit(instance->data, 0, symbol != 0U);
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

        subghz_protocol_galo_c02_set_bit(instance->data, instance->symbol_count, symbol != 0U);
        instance->symbol_count++;
        if(instance->symbol_count == GALO_C02_SYMBOL_COUNT) {
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
            if(subghz_protocol_galo_c02_data_is_valid(instance->data) && instance->base.callback) {
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
    return subghz_protocol_blocks_add_bytes(instance->data, sizeof(instance->data));
}

SubGhzProtocolStatus subghz_protocol_decoder_galo_c02_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    furi_assert(flipper_format);
    furi_assert(preset);
    SubGhzProtocolDecoderGaloC02* instance = context;

    return subghz_protocol_galo_c02_serialize_data(instance->data, flipper_format, preset);
}

SubGhzProtocolStatus
    subghz_protocol_decoder_galo_c02_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderGaloC02* instance = context;

    return subghz_protocol_galo_c02_read_data(flipper_format, instance->data);
}

void subghz_protocol_decoder_galo_c02_get_string(void* context, FuriString* output) {
    furi_assert(context);
    furi_assert(output);
    SubGhzProtocolDecoderGaloC02* instance = context;

    furi_string_cat_printf(
        output, "%s %ub\r\nData: 0x", SUBGHZ_PROTOCOL_GALO_C02_NAME, GALO_C02_SYMBOL_COUNT);
    for(size_t i = 0; i < sizeof(instance->data); i++) {
        furi_string_cat_printf(output, "%02X", instance->data[i]);
    }
}
