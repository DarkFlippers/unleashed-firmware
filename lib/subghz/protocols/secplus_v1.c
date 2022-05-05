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
    .alloc = NULL,
    .free = NULL,

    .deserialize = NULL,
    .stop = NULL,
    .yield = NULL,
};

const SubGhzProtocol subghz_protocol_secplus_v1 = {
    .name = SUBGHZ_PROTOCOL_SECPLUS_V1_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_315 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &subghz_protocol_secplus_v1_decoder,
    .encoder = &subghz_protocol_secplus_v1_encoder,
};

void* subghz_protocol_decoder_secplus_v1_alloc(SubGhzEnvironment* environment) {
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
 * Security+ 1.0 half-message decoding
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
    uint32_t frequency,
    FuriHalSubGhzPreset preset) {
    furi_assert(context);
    SubGhzProtocolDecoderSecPlus_v1* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, frequency, preset);
}

bool subghz_protocol_decoder_secplus_v1_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderSecPlus_v1* instance = context;
    return subghz_block_generic_deserialize(&instance->generic, flipper_format);
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

        if(0 <= pin && pin <= 9999) {
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
