#include "secplus_v1.h"
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

/*
* Help
* https://github.com/argilo/secplus
* https://github.com/merbanan/rtl_433/blob/master/src/devices/secplus_v1.c
*/

#define TAG "SubGhzProtocoSecPlus_v1"

#define SECPLUS_V1_BIT_ERR -1 //0b0000
#define SECPLUS_V1_BIT_0 0 //0b0001
#define SECPLUS_V1_BIT_1 1 //0b0011
#define SECPLUS_V1_BIT_2 2 //0b0111

#define SECPLUS_V1_PACKET_1_HEADER 0x00
#define SECPLUS_V1_PACKET_2_HEADER 0x02
#define SECPLUS_V1_PACKET_1_INDEX_BASE 0
#define SECPLUS_V1_PACKET_2_INDEX_BASE 21
#define SECPLUS_V1_PACKET_1_ACCEPTED (1 << 0)
#define SECPLUS_V1_PACKET_2_ACCEPTED (1 << 1)

static const SubGhzBlockConst subghz_protocol_secplus_v1_const = {
    .te_short = 500,
    .te_long = 1500,
    .te_delta = 100,
    .min_count_bit_for_found = 21,
};

struct SubGhzProtocolDecoderSecPlus_v1 {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint8_t packet_accepted;
    uint8_t base_packet_index;
    uint8_t data_array[44];
};

struct SubGhzProtocolEncoderSecPlus_v1 {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;

    uint8_t data_array[44];
};

typedef enum {
    SecPlus_v1DecoderStepReset = 0,
    SecPlus_v1DecoderStepSearchStartBit,
    SecPlus_v1DecoderStepSaveDuration,
    SecPlus_v1DecoderStepDecoderData,
} SecPlus_v1DecoderStep;

const SubGhzProtocolDecoder subghz_protocol_secplus_v1_decoder = {
    .alloc = subghz_protocol_decoder_secplus_v1_alloc,
    .free = subghz_protocol_decoder_secplus_v1_free,

    .feed = subghz_protocol_decoder_secplus_v1_feed,
    .reset = subghz_protocol_decoder_secplus_v1_reset,

    .get_hash_data = subghz_protocol_decoder_secplus_v1_get_hash_data,
    .serialize = subghz_protocol_decoder_secplus_v1_serialize,
    .deserialize = subghz_protocol_decoder_secplus_v1_deserialize,
    .get_string = subghz_protocol_decoder_secplus_v1_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_secplus_v1_encoder = {
    .alloc = subghz_protocol_encoder_secplus_v1_alloc,
    .free = subghz_protocol_encoder_secplus_v1_free,

    .deserialize = subghz_protocol_encoder_secplus_v1_deserialize,
    .stop = subghz_protocol_encoder_secplus_v1_stop,
    .yield = subghz_protocol_encoder_secplus_v1_yield,
};

const SubGhzProtocol subghz_protocol_secplus_v1 = {
    .name = SUBGHZ_PROTOCOL_SECPLUS_V1_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_315 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_secplus_v1_decoder,
    .encoder = &subghz_protocol_secplus_v1_encoder,
};

void* subghz_protocol_encoder_secplus_v1_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderSecPlus_v1* instance = malloc(sizeof(SubGhzProtocolEncoderSecPlus_v1));

    instance->base.protocol = &subghz_protocol_secplus_v1;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 128;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_secplus_v1_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderSecPlus_v1* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderSecPlus_v1 instance
 * @return true On success
 */
static bool
    subghz_protocol_encoder_secplus_v1_get_upload(SubGhzProtocolEncoderSecPlus_v1* instance) {
    furi_assert(instance);
    size_t index = 0;
    size_t size_upload = (instance->generic.data_count_bit * 2);
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Encoder size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }

    //Send header packet 1
    instance->encoder.upload[index++] = level_duration_make(
        false, (uint32_t)subghz_protocol_secplus_v1_const.te_short * (116 + 3));
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_secplus_v1_const.te_short);

    //Send data packet 1
    for(uint8_t i = SECPLUS_V1_PACKET_1_INDEX_BASE + 1; i < SECPLUS_V1_PACKET_1_INDEX_BASE + 21;
        i++) {
        switch(instance->data_array[i]) {
        case SECPLUS_V1_BIT_0:
            instance->encoder.upload[index++] = level_duration_make(
                false, (uint32_t)subghz_protocol_secplus_v1_const.te_short * 3);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_secplus_v1_const.te_short);
            break;
        case SECPLUS_V1_BIT_1:
            instance->encoder.upload[index++] = level_duration_make(
                false, (uint32_t)subghz_protocol_secplus_v1_const.te_short * 2);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_secplus_v1_const.te_short * 2);
            break;
        case SECPLUS_V1_BIT_2:
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_secplus_v1_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_secplus_v1_const.te_short * 3);
            break;

        default:
            FURI_LOG_E(TAG, "Encoder error, wrong bit type");
            return false;
            break;
        }
    }

    //Send header packet 2
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_secplus_v1_const.te_short * (116));
    instance->encoder.upload[index++] =
        level_duration_make(true, (uint32_t)subghz_protocol_secplus_v1_const.te_short * 3);

    //Send data packet 2
    for(uint8_t i = SECPLUS_V1_PACKET_2_INDEX_BASE + 1; i < SECPLUS_V1_PACKET_2_INDEX_BASE + 21;
        i++) {
        switch(instance->data_array[i]) {
        case SECPLUS_V1_BIT_0:
            instance->encoder.upload[index++] = level_duration_make(
                false, (uint32_t)subghz_protocol_secplus_v1_const.te_short * 3);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_secplus_v1_const.te_short);
            break;
        case SECPLUS_V1_BIT_1:
            instance->encoder.upload[index++] = level_duration_make(
                false, (uint32_t)subghz_protocol_secplus_v1_const.te_short * 2);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_secplus_v1_const.te_short * 2);
            break;
        case SECPLUS_V1_BIT_2:
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_secplus_v1_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_secplus_v1_const.te_short * 3);
            break;

        default:
            FURI_LOG_E(TAG, "Encoder error, wrong bit type.");
            return false;
            break;
        }
    }

    return true;
}

/** 
 * Security+ 1.0 message encoding
 * @param instance SubGhzProtocolEncoderSecPlus_v1* 
 */

static bool subghz_protocol_secplus_v1_encode(SubGhzProtocolEncoderSecPlus_v1* instance) {
    uint32_t fixed = (instance->generic.data >> 32) & 0xFFFFFFFF;
    uint32_t rolling = instance->generic.data & 0xFFFFFFFF;

    uint8_t rolling_array[20] = {0};
    uint8_t fixed_array[20] = {0};
    uint32_t acc = 0;

    //increment the counter
    rolling += 2;

    //update data
    instance->generic.data &= 0xFFFFFFFF00000000;
    instance->generic.data |= rolling;

    if(rolling > 0xFFFFFFFF) {
        rolling = 0xE6000000;
    }
    if(fixed > 0xCFD41B90) {
        FURI_LOG_E("TAG", "Encode wrong fixed data");
        return false;
    }

    rolling = subghz_protocol_blocks_reverse_key(rolling, 32);

    for(int i = 19; i > -1; i--) {
        rolling_array[i] = rolling % 3;
        rolling /= 3;
        fixed_array[i] = fixed % 3;
        fixed /= 3;
    }

    instance->data_array[SECPLUS_V1_PACKET_1_INDEX_BASE] = SECPLUS_V1_PACKET_1_HEADER;
    instance->data_array[SECPLUS_V1_PACKET_2_INDEX_BASE] = SECPLUS_V1_PACKET_2_HEADER;

    //encode packet 1
    for(uint8_t i = 1; i < 11; i++) {
        acc += rolling_array[i - 1];
        instance->data_array[i * 2 - 1] = rolling_array[i - 1];
        acc += fixed_array[i - 1];
        instance->data_array[i * 2] = acc % 3;
    }

    acc = 0;
    //encode packet 2
    for(uint8_t i = 11; i < 21; i++) {
        acc += rolling_array[i - 1];
        instance->data_array[i * 2] = rolling_array[i - 1];
        acc += fixed_array[i - 1];
        instance->data_array[i * 2 + 1] = acc % 3;
    }

    return true;
}

bool subghz_protocol_encoder_secplus_v1_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderSecPlus_v1* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        if(instance->generic.data_count_bit !=
           2 * subghz_protocol_secplus_v1_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!subghz_protocol_secplus_v1_encode(instance)) {
            break;
        }
        if(!subghz_protocol_encoder_secplus_v1_get_upload(instance)) {
            break;
        }

        uint8_t key_data[sizeof(uint64_t)] = {0};
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (instance->generic.data >> i * 8) & 0xFF;
        }
        if(!flipper_format_update_hex(flipper_format, "Key", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to add Key");
            break;
        }

        instance->encoder.is_running = true;

        res = true;
    } while(false);

    return res;
}

void subghz_protocol_encoder_secplus_v1_stop(void* context) {
    SubGhzProtocolEncoderSecPlus_v1* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_secplus_v1_yield(void* context) {
    SubGhzProtocolEncoderSecPlus_v1* instance = context;

    if(instance->encoder.repeat == 0 || !instance->encoder.is_running) {
        instance->encoder.is_running = false;
        return level_duration_reset();
    }

    LevelDuration ret = instance->encoder.upload[instance->encoder.front];

    if(++instance->encoder.front == instance->encoder.size_upload) {
        instance->encoder.repeat--;
        instance->encoder.front = 0;
    }

    return ret;
}

void* subghz_protocol_decoder_secplus_v1_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderSecPlus_v1* instance = malloc(sizeof(SubGhzProtocolDecoderSecPlus_v1));
    instance->base.protocol = &subghz_protocol_secplus_v1;
    instance->generic.protocol_name = instance->base.protocol->name;

    return instance;
}

void subghz_protocol_decoder_secplus_v1_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderSecPlus_v1* instance = context;
    free(instance);
}

void subghz_protocol_decoder_secplus_v1_reset(void* context) {
    furi_assert(context);
    // SubGhzProtocolDecoderSecPlus_v1* instance = context;
    // does not reset the decoder because you need to get 2 parts of the package
}

/** 
 * Security+ 1.0 message decoding
 * @param instance SubGhzProtocolDecoderSecPlus_v1* 
 */

static void subghz_protocol_secplus_v1_decode(SubGhzProtocolDecoderSecPlus_v1* instance) {
    uint32_t rolling = 0;
    uint32_t fixed = 0;
    uint32_t acc = 0;
    uint8_t digit = 0;

    //decode packet 1
    for(uint8_t i = 1; i < 21; i += 2) {
        digit = instance->data_array[i];
        rolling = (rolling * 3) + digit;
        acc += digit;

        digit = (60 + instance->data_array[i + 1] - acc) % 3;
        fixed = (fixed * 3) + digit;
        acc += digit;
    }

    acc = 0;
    //decode packet 2
    for(uint8_t i = 22; i < 42; i += 2) {
        digit = instance->data_array[i];
        rolling = (rolling * 3) + digit;
        acc += digit;

        digit = (60 + instance->data_array[i + 1] - acc) % 3;
        fixed = (fixed * 3) + digit;
        acc += digit;
    }

    rolling = subghz_protocol_blocks_reverse_key(rolling, 32);
    instance->generic.data = (uint64_t)fixed << 32 | rolling;

    instance->generic.data_count_bit =
        subghz_protocol_secplus_v1_const.min_count_bit_for_found * 2;
}

void subghz_protocol_decoder_secplus_v1_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderSecPlus_v1* instance = context;

    switch(instance->decoder.parser_step) {
    case SecPlus_v1DecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_secplus_v1_const.te_short * 120) <
                        subghz_protocol_secplus_v1_const.te_delta * 120)) {
            //Found header Security+ 1.0
            instance->decoder.parser_step = SecPlus_v1DecoderStepSearchStartBit;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->packet_accepted = 0;
            memset(instance->data_array, 0, sizeof(instance->data_array));
        }
        break;
    case SecPlus_v1DecoderStepSearchStartBit:
        if(level) {
            if(DURATION_DIFF(duration, subghz_protocol_secplus_v1_const.te_short) <
               subghz_protocol_secplus_v1_const.te_delta) {
                instance->base_packet_index = SECPLUS_V1_PACKET_1_INDEX_BASE;
                instance
                    ->data_array[instance->decoder.decode_count_bit + instance->base_packet_index] =
                    SECPLUS_V1_BIT_0;
                instance->decoder.decode_count_bit++;
                instance->decoder.parser_step = SecPlus_v1DecoderStepSaveDuration;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_secplus_v1_const.te_long) <
                subghz_protocol_secplus_v1_const.te_delta) {
                instance->base_packet_index = SECPLUS_V1_PACKET_2_INDEX_BASE;
                instance
                    ->data_array[instance->decoder.decode_count_bit + instance->base_packet_index] =
                    SECPLUS_V1_BIT_2;
                instance->decoder.decode_count_bit++;
                instance->decoder.parser_step = SecPlus_v1DecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = SecPlus_v1DecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = SecPlus_v1DecoderStepReset;
        }
        break;
    case SecPlus_v1DecoderStepSaveDuration:
        if(!level) { //save interval
            if(DURATION_DIFF(duration, subghz_protocol_secplus_v1_const.te_short * 120) <
               subghz_protocol_secplus_v1_const.te_delta * 120) {
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_secplus_v1_const.min_count_bit_for_found) {
                    if(instance->base_packet_index == SECPLUS_V1_PACKET_1_INDEX_BASE)
                        instance->packet_accepted |= SECPLUS_V1_PACKET_1_ACCEPTED;
                    if(instance->base_packet_index == SECPLUS_V1_PACKET_2_INDEX_BASE)
                        instance->packet_accepted |= SECPLUS_V1_PACKET_2_ACCEPTED;

                    if(instance->packet_accepted ==
                       (SECPLUS_V1_PACKET_1_ACCEPTED | SECPLUS_V1_PACKET_2_ACCEPTED)) {
                        subghz_protocol_secplus_v1_decode(instance);

                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                        instance->decoder.parser_step = SecPlus_v1DecoderStepReset;
                    }
                }
                instance->decoder.parser_step = SecPlus_v1DecoderStepSearchStartBit;
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
            } else {
                instance->decoder.te_last = duration;
                instance->decoder.parser_step = SecPlus_v1DecoderStepDecoderData;
            }
        } else {
            instance->decoder.parser_step = SecPlus_v1DecoderStepReset;
        }
        break;
    case SecPlus_v1DecoderStepDecoderData:
        if(level && (instance->decoder.decode_count_bit <=
                     subghz_protocol_secplus_v1_const.min_count_bit_for_found)) {
            if((DURATION_DIFF(
                    instance->decoder.te_last, subghz_protocol_secplus_v1_const.te_short * 3) <
                subghz_protocol_secplus_v1_const.te_delta * 3) &&
               (DURATION_DIFF(duration, subghz_protocol_secplus_v1_const.te_short) <
                subghz_protocol_secplus_v1_const.te_delta)) {
                instance
                    ->data_array[instance->decoder.decode_count_bit + instance->base_packet_index] =
                    SECPLUS_V1_BIT_0;
                instance->decoder.decode_count_bit++;
                instance->decoder.parser_step = SecPlus_v1DecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_secplus_v1_const.te_short * 2) <
                 subghz_protocol_secplus_v1_const.te_delta * 2) &&
                (DURATION_DIFF(duration, subghz_protocol_secplus_v1_const.te_short * 2) <
                 subghz_protocol_secplus_v1_const.te_delta * 2)) {
                instance
                    ->data_array[instance->decoder.decode_count_bit + instance->base_packet_index] =
                    SECPLUS_V1_BIT_1;
                instance->decoder.decode_count_bit++;
                instance->decoder.parser_step = SecPlus_v1DecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_secplus_v1_const.te_short) <
                 subghz_protocol_secplus_v1_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_secplus_v1_const.te_short * 3) <
                 subghz_protocol_secplus_v1_const.te_delta * 3)) {
                instance
                    ->data_array[instance->decoder.decode_count_bit + instance->base_packet_index] =
                    SECPLUS_V1_BIT_2;
                instance->decoder.decode_count_bit++;
                instance->decoder.parser_step = SecPlus_v1DecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = SecPlus_v1DecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = SecPlus_v1DecoderStepReset;
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_secplus_v1_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderSecPlus_v1* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool subghz_protocol_decoder_secplus_v1_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderSecPlus_v1* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

bool subghz_protocol_decoder_secplus_v1_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderSecPlus_v1* instance = context;
    bool ret = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            break;
        }
        if(instance->generic.data_count_bit !=
           2 * subghz_protocol_secplus_v1_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        ret = true;
    } while(false);
    return ret;
}

bool subghz_protocol_secplus_v1_check_fixed(uint32_t fixed) {
    //uint8_t id0 = (fixed / 3) % 3;
    uint8_t id1 = (fixed / 9) % 3;
    uint8_t btn = fixed % 3;

    do {
        if(id1 == 0) return false;
        if(!(btn == 0 || btn == 1 || btn == 2)) return false;
    } while(false);
    return true;
}

void subghz_protocol_decoder_secplus_v1_get_string(void* context, string_t output) {
    furi_assert(context);
    SubGhzProtocolDecoderSecPlus_v1* instance = context;

    uint32_t fixed = (instance->generic.data >> 32) & 0xFFFFFFFF;
    instance->generic.cnt = instance->generic.data & 0xFFFFFFFF;

    instance->generic.btn = fixed % 3;
    uint8_t id0 = (fixed / 3) % 3;
    uint8_t id1 = (fixed / 9) % 3;
    uint16_t pin = 0;

    string_cat_printf(
        output,
        "%s %db\r\n"
        "Key:0x%lX%08lX\r\n"
        "id1:%d id0:%d",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)instance->generic.data,
        id1,
        id0);

    if(id1 == 0) {
        // (fixed // 3**3) % (3**7)    3^3=27  3^73=72187

        instance->generic.serial = (fixed / 27) % 2187;
        // pin = (fixed // 3**10) % (3**9)  3^10=59049 3^9=19683
        pin = (fixed / 59049) % 19683;

        if(pin <= 9999) {
            string_cat_printf(output, " pin:%d", pin);
        } else if(10000 <= pin && pin <= 11029) {
            string_cat_printf(output, " pin:enter");
        }

        int pin_suffix = 0;
        // pin_suffix = (fixed // 3**19) % 3   3^19=1162261467
        pin_suffix = (fixed / 1162261467) % 3;

        if(pin_suffix == 1) {
            string_cat_printf(output, " #\r\n");
        } else if(pin_suffix == 2) {
            string_cat_printf(output, " *\r\n");
        } else {
            string_cat_printf(output, "\r\n");
        }
        string_cat_printf(
            output,
            "Sn:0x%08lX\r\n"
            "Cnt:0x%03X\r\n"
            "Sw_id:0x%X\r\n",
            instance->generic.serial,
            instance->generic.cnt,
            instance->generic.btn);
    } else {
        //id = fixed / 27;
        instance->generic.serial = fixed / 27;
        if(instance->generic.btn == 1) {
            string_cat_printf(output, " Btn:left\r\n");
        } else if(instance->generic.btn == 0) {
            string_cat_printf(output, " Btn:middle\r\n");
        } else if(instance->generic.btn == 2) {
            string_cat_printf(output, " Btn:right\r\n");
        }

        string_cat_printf(
            output,
            "Sn:0x%08lX\r\n"
            "Cnt:0x%03X\r\n"
            "Sw_id:0x%X\r\n",
            instance->generic.serial,
            instance->generic.cnt,
            instance->generic.btn);
    }
}
