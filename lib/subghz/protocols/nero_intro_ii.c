#include "nero_intro_ii.h"

#include "../blocks/generic.h"
#include "common.h"
#include "nero_intro_ii_code.h"

#define INTRO_DATA_MASK UINT64_C(0x1FFFFFFFFFF)

typedef struct {
    SubGhzProtocolDecoderBase base;
    SubGhzBlockGeneric generic;
    NeroIntroIICode code;
    uint8_t corrections;
} SubGhzProtocolDecoderNeroIntroII;

static void* intro_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderNeroIntroII* instance = malloc(sizeof(*instance));
    memset(instance, 0, sizeof(*instance));
    instance->base.protocol = &subghz_protocol_nero_intro_ii;
    instance->generic.protocol_name = SUBGHZ_PROTOCOL_NERO_INTRO_II_NAME;
    return instance;
}

static void intro_free(void* context) {
    free(context);
}

static void intro_reset(void* context) {
    SubGhzProtocolDecoderNeroIntroII* instance = context;
    nero_intro_ii_code_reset(&instance->code);
}

static void intro_feed(void* context, bool level, uint32_t duration) {
    SubGhzProtocolDecoderNeroIntroII* instance = context;
    NeroIntroIIFrame frame;
    if(!nero_intro_ii_code_feed(&instance->code, level, duration, &frame)) return;
    instance->generic.data = 0;
    instance->generic.data_2 = 0;
    for(unsigned i = 0; i < NERO_INTRO_II_BITS; i++) {
        bool bit = (frame.symbols[i / 7] >> (6 - (i % 7))) & 1;
        if(i < 64) {
            instance->generic.data = (instance->generic.data << 1) | bit;
        } else {
            instance->generic.data_2 = (instance->generic.data_2 << 1) | bit;
        }
    }
    instance->generic.data_count_bit = NERO_INTRO_II_BITS;
    instance->corrections = frame.corrections;
    /* Receiver callbacks reset every decoder. The just-consumed idle may
     * already contain the next frame's first sync chip; retain that tail. */
    uint64_t sync = instance->code.sync;
    uint8_t sync_count = instance->code.sync_count;
    if(instance->base.callback) instance->base.callback(&instance->base, instance->base.context);
    instance->code.sync = sync;
    instance->code.sync_count = sync_count;
}

static uint8_t intro_hash(void* context) {
    SubGhzProtocolDecoderNeroIntroII* instance = context;
    uint32_t hash = 2166136261u;
    for(unsigned i = 0; i < 8; i++) {
        hash = (hash ^ ((instance->generic.data >> (i * 8)) & 0xff)) * 16777619u;
        hash = (hash ^ ((instance->generic.data_2 >> (i * 8)) & 0xff)) * 16777619u;
    }
    return (hash >> 24) ^ (hash >> 16) ^ (hash >> 8) ^ hash;
}

static SubGhzProtocolStatus
    intro_serialize(void* context, FlipperFormat* format, SubGhzRadioPreset* preset) {
    SubGhzProtocolDecoderNeroIntroII* instance = context;
    SubGhzProtocolStatus status =
        subghz_block_generic_serialize(&instance->generic, format, preset);
    return subghz_protocol_common_append_data_2(status, &instance->generic, format);
}

static SubGhzProtocolStatus intro_deserialize(void* context, FlipperFormat* format) {
    SubGhzProtocolDecoderNeroIntroII* instance = context;
    SubGhzProtocolStatus status = subghz_block_generic_deserialize_check_count_bit(
        &instance->generic, format, NERO_INTRO_II_BITS);
    if(status != SubGhzProtocolStatusOk) return status;
    if(instance->generic.data_count_bit != NERO_INTRO_II_BITS)
        return SubGhzProtocolStatusErrorValueBitCount;
    uint8_t bytes[8];
    if(!flipper_format_rewind(format) || !flipper_format_read_hex(format, "Data", bytes, 8))
        return SubGhzProtocolStatusErrorParserOthers;
    uint64_t data = 0;
    for(unsigned i = 0; i < 8; i++)
        data = (data << 8) | bytes[i];
    if(data & ~INTRO_DATA_MASK) return SubGhzProtocolStatusErrorParserKey;
    instance->generic.data_2 = data;
    instance->corrections = 0;
    return SubGhzProtocolStatusOk;
}

static void intro_string(void* context, FuriString* output) {
    SubGhzProtocolDecoderNeroIntroII* instance = context;
    subghz_block_generic_global.btn_is_available = false;
    subghz_block_generic_global.cnt_is_available = false;
    furi_string_cat_printf(
        output,
        "%s %ubit\r\n"
        "Key:%08lX%08lX\r\n"
        "Data:%03lX%08lX\r\n"
        "Line code, edits:%u\r\n"
        "Dynamic fields unknown\r\n",
        instance->generic.protocol_name,
        NERO_INTRO_II_BITS,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)instance->generic.data,
        (uint32_t)(instance->generic.data_2 >> 32),
        (uint32_t)instance->generic.data_2,
        instance->corrections);
}

static const SubGhzProtocolDecoder intro_decoder = {
    .alloc = intro_alloc,
    .free = intro_free,
    .feed = intro_feed,
    .reset = intro_reset,
    .get_hash_data = intro_hash,
    .serialize = intro_serialize,
    .deserialize = intro_deserialize,
    .get_string = intro_string,
};

/* Saving stores the observed line-code symbols. No authentication generator
 * has been established, so this protocol intentionally has no encoder. */
const SubGhzProtocol subghz_protocol_nero_intro_ii = {
    .name = SUBGHZ_PROTOCOL_NERO_INTRO_II_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Load,
    .decoder = &intro_decoder,
    .encoder = NULL,
};
