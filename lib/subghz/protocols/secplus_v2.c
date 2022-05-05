#include "secplus_v2.h"
#include <lib/toolbox/manchester_decoder.h>
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

/*
* Help
* https://github.com/argilo/secplus
* https://github.com/merbanan/rtl_433/blob/master/src/devices/secplus_v2.c
*/

#define TAG "SubGhzProtocoSecPlus_v2"

#define SECPLUS_V2_HEADER 0x3C0000000000
#define SECPLUS_V2_HEADER_MASK 0xFFFF3C0000000000
#define SECPLUS_V2_PACKET_1 0x000000000000
#define SECPLUS_V2_PACKET_2 0x010000000000
#define SECPLUS_V2_PACKET_MASK 0x30000000000

static const SubGhzBlockConst subghz_protocol_secplus_v2_const = {
    .te_short = 250,
    .te_long = 500,
    .te_delta = 110,
    .min_count_bit_for_found = 62,
};

struct SubGhzProtocolDecoderSecPlus_v2 {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    ManchesterState manchester_saved_state;
    uint64_t secplus_packet_1;
};

struct SubGhzProtocolEncoderSecPlus_v2 {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    SecPlus_v2DecoderStepReset = 0,
    SecPlus_v2DecoderStepDecoderData,
} SecPlus_v2DecoderStep;

const SubGhzProtocolDecoder subghz_protocol_secplus_v2_decoder = {
    .alloc = subghz_protocol_decoder_secplus_v2_alloc,
    .free = subghz_protocol_decoder_secplus_v2_free,

    .feed = subghz_protocol_decoder_secplus_v2_feed,
    .reset = subghz_protocol_decoder_secplus_v2_reset,

    .get_hash_data = subghz_protocol_decoder_secplus_v2_get_hash_data,
    .serialize = subghz_protocol_decoder_secplus_v2_serialize,
    .deserialize = subghz_protocol_decoder_secplus_v2_deserialize,
    .get_string = subghz_protocol_decoder_secplus_v2_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_secplus_v2_encoder = {
    .alloc = NULL,
    .free = NULL,

    .deserialize = NULL,
    .stop = NULL,
    .yield = NULL,
};

const SubGhzProtocol subghz_protocol_secplus_v2 = {
    .name = SUBGHZ_PROTOCOL_SECPLUS_V2_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_315 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &subghz_protocol_secplus_v2_decoder,
    .encoder = &subghz_protocol_secplus_v2_encoder,
};

void* subghz_protocol_decoder_secplus_v2_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolDecoderSecPlus_v2* instance = malloc(sizeof(SubGhzProtocolDecoderSecPlus_v2));
    instance->base.protocol = &subghz_protocol_secplus_v2;
    instance->generic.protocol_name = instance->base.protocol->name;

    return instance;
}

void subghz_protocol_decoder_secplus_v2_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderSecPlus_v2* instance = context;
    free(instance);
}

void subghz_protocol_decoder_secplus_v2_reset(void* context) {
    furi_assert(context);
    // SubGhzProtocolDecoderSecPlus_v2* instance = context;
    // does not reset the decoder because you need to get 2 parts of the package
}

static bool subghz_protocol_secplus_v2_check_packet(SubGhzProtocolDecoderSecPlus_v2* instance) {
    if((instance->decoder.decode_data & SECPLUS_V2_HEADER_MASK) == SECPLUS_V2_HEADER) {
        if((instance->decoder.decode_data & SECPLUS_V2_PACKET_MASK) == SECPLUS_V2_PACKET_1) {
            instance->secplus_packet_1 = instance->decoder.decode_data;
        } else if(
            ((instance->decoder.decode_data & SECPLUS_V2_PACKET_MASK) == SECPLUS_V2_PACKET_2) &&
            (instance->secplus_packet_1)) {
            return true;
        }
    }
    return false;
}

void subghz_protocol_decoder_secplus_v2_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderSecPlus_v2* instance = context;

    ManchesterEvent event = ManchesterEventReset;
    switch(instance->decoder.parser_step) {
    case SecPlus_v2DecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_secplus_v2_const.te_long * 130) <
                        subghz_protocol_secplus_v2_const.te_delta * 100)) {
            //Found header Security+ 2.0
            instance->decoder.parser_step = SecPlus_v2DecoderStepDecoderData;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->secplus_packet_1 = 0;
            manchester_advance(
                instance->manchester_saved_state,
                ManchesterEventReset,
                &instance->manchester_saved_state,
                NULL);
            manchester_advance(
                instance->manchester_saved_state,
                ManchesterEventLongHigh,
                &instance->manchester_saved_state,
                NULL);
            manchester_advance(
                instance->manchester_saved_state,
                ManchesterEventShortLow,
                &instance->manchester_saved_state,
                NULL);
        }
        break;
    case SecPlus_v2DecoderStepDecoderData:
        if(!level) {
            if(DURATION_DIFF(duration, subghz_protocol_secplus_v2_const.te_short) <
               subghz_protocol_secplus_v2_const.te_delta) {
                event = ManchesterEventShortLow;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_secplus_v2_const.te_long) <
                subghz_protocol_secplus_v2_const.te_delta) {
                event = ManchesterEventLongLow;
            } else if(
                duration >= (subghz_protocol_secplus_v2_const.te_long * 2 +
                             subghz_protocol_secplus_v2_const.te_delta)) {
                if(instance->decoder.decode_count_bit >=
                   subghz_protocol_secplus_v2_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    if(subghz_protocol_secplus_v2_check_packet(instance)) {
                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                        instance->decoder.parser_step = SecPlus_v2DecoderStepReset;
                    }
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventReset,
                    &instance->manchester_saved_state,
                    NULL);
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventLongHigh,
                    &instance->manchester_saved_state,
                    NULL);
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventShortLow,
                    &instance->manchester_saved_state,
                    NULL);
            } else {
                instance->decoder.parser_step = SecPlus_v2DecoderStepReset;
            }
        } else {
            if(DURATION_DIFF(duration, subghz_protocol_secplus_v2_const.te_short) <
               subghz_protocol_secplus_v2_const.te_delta) {
                event = ManchesterEventShortHigh;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_secplus_v2_const.te_long) <
                subghz_protocol_secplus_v2_const.te_delta) {
                event = ManchesterEventLongHigh;
            } else {
                instance->decoder.parser_step = SecPlus_v2DecoderStepReset;
            }
        }
        if(event != ManchesterEventReset) {
            bool data;
            bool data_ok = manchester_advance(
                instance->manchester_saved_state, event, &instance->manchester_saved_state, &data);

            if(data_ok) {
                instance->decoder.decode_data = (instance->decoder.decode_data << 1) | data;
                instance->decoder.decode_count_bit++;
            }
        }
        break;
    }
}

/** 
 * Security+ 2.0 half-message decoding
 * @param data data 
 * @param roll_array[] return roll_array part
 * @param fixed[] return fixed part
 * @return true On success
 */

static bool
    subghz_protocol_secplus_v2_decode_half(uint64_t data, uint8_t roll_array[], uint32_t* fixed) {
    uint8_t order = (data >> 34) & 0x0f;
    uint8_t invert = (data >> 30) & 0x0f;
    uint16_t p[3] = {0};

    for(int i = 29; i >= 0; i -= 3) {
        p[0] = p[0] << 1 | bit_read(data, i);
        p[1] = p[1] << 1 | bit_read(data, i - 1);
        p[2] = p[2] << 1 | bit_read(data, i - 2);
    }

    // selectively invert buffers
    switch(invert) {
    case 0x00: // 0b0000 (True, True, False),
        p[0] = ~p[0] & 0x03FF;
        p[1] = ~p[1] & 0x03FF;
        break;
    case 0x01: // 0b0001 (False, True, False),
        p[1] = ~p[1] & 0x03FF;
        break;
    case 0x02: // 0b0010 (False, False, True),
        p[2] = ~p[2] & 0x03FF;
        break;
    case 0x04: // 0b0100 (True, True, True),
        p[0] = ~p[0] & 0x03FF;
        p[1] = ~p[1] & 0x03FF;
        p[2] = ~p[2] & 0x03FF;
        break;
    case 0x05: // 0b0101 (True, False, True),
    case 0x0a: // 0b1010 (True, False, True),
        p[0] = ~p[0] & 0x03FF;
        p[2] = ~p[2] & 0x03FF;
        break;
    case 0x06: // 0b0110 (False, True, True),
        p[1] = ~p[1] & 0x03FF;
        p[2] = ~p[2] & 0x03FF;
        break;
    case 0x08: // 0b1000 (True, False, False),
        p[0] = ~p[0] & 0x03FF;
        break;
    case 0x09: // 0b1001 (False, False, False),
        break;
    default:
        FURI_LOG_E(TAG, "Invert FAIL");
        return false;
    }

    uint16_t a = p[0], b = p[1], c = p[2];

    // selectively reorder buffers
    switch(order) {
    case 0x06: // 0b0110  2, 1, 0],
    case 0x09: // 0b1001  2, 1, 0],
        p[2] = a;
        p[1] = b;
        p[0] = c;
        break;
    case 0x08: // 0b1000  1, 2, 0],
    case 0x04: // 0b0100  1, 2, 0],
        p[1] = a;
        p[2] = b;
        p[0] = c;
        break;
    case 0x01: // 0b0001 2, 0, 1],
        p[2] = a;
        p[0] = b;
        p[1] = c;
        break;
    case 0x00: // 0b0000  0, 2, 1],
        p[0] = a;
        p[2] = b;
        p[1] = c;
        break;
    case 0x05: // 0b0101 1, 0, 2],
        p[1] = a;
        p[0] = b;
        p[2] = c;
        break;
    case 0x02: // 0b0010 0, 1, 2],
    case 0x0A: // 0b1010 0, 1, 2],
        p[0] = a;
        p[1] = b;
        p[2] = c;
        break;
    default:
        FURI_LOG_E(TAG, "Order FAIL");
        return false;
    }

    data = order << 4 | invert;
    int k = 0;
    for(int i = 6; i >= 0; i -= 2) {
        roll_array[k++] = (data >> i) & 0x03;
        if(roll_array[k] == 3) {
            FURI_LOG_E(TAG, "Roll_Array FAIL");
            return false;
        }
    }

    for(int i = 8; i >= 0; i -= 2) {
        roll_array[k++] = (p[2] >> i) & 0x03;
        if(roll_array[k] == 3) {
            FURI_LOG_E(TAG, "Roll_Array FAIL");
            return false;
        }
    }

    fixed[0] = p[0] << 10 | p[1];
    return true;
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 * @param packet_1 first part of the message
 */
static void
    subghz_protocol_secplus_v2_remote_controller(SubGhzBlockGeneric* instance, uint64_t packet_1) {
    uint32_t fixed_1[1];
    uint8_t roll_1[9] = {0};
    uint32_t fixed_2[1];
    uint8_t roll_2[9] = {0};
    uint8_t rolling_digits[18] = {0};

    if(subghz_protocol_secplus_v2_decode_half(packet_1, roll_1, fixed_1) &&
       subghz_protocol_secplus_v2_decode_half(instance->data, roll_2, fixed_2)) {
        rolling_digits[0] = roll_2[8];
        rolling_digits[1] = roll_1[8];

        rolling_digits[2] = roll_2[4];
        rolling_digits[3] = roll_2[5];
        rolling_digits[4] = roll_2[6];
        rolling_digits[5] = roll_2[7];

        rolling_digits[6] = roll_1[4];
        rolling_digits[7] = roll_1[5];
        rolling_digits[8] = roll_1[6];
        rolling_digits[9] = roll_1[7];

        rolling_digits[10] = roll_2[0];
        rolling_digits[11] = roll_2[1];
        rolling_digits[12] = roll_2[2];
        rolling_digits[13] = roll_2[3];

        rolling_digits[14] = roll_1[0];
        rolling_digits[15] = roll_1[1];
        rolling_digits[16] = roll_1[2];
        rolling_digits[17] = roll_1[3];

        uint32_t rolling = 0;
        for(int i = 0; i < 18; i++) {
            rolling = (rolling * 3) + rolling_digits[i];
        }
        // Max value = 2^28 (268435456)
        if(rolling >= 0x10000000) {
            FURI_LOG_E(TAG, "Rolling FAIL");
            instance->cnt = 0;
            instance->btn = 0;
            instance->serial = 0;
        } else {
            instance->cnt = subghz_protocol_blocks_reverse_key(rolling, 28);
            instance->btn = fixed_1[0] >> 12;
            instance->serial = fixed_1[0] << 20 | fixed_2[0];
        }
    } else {
        instance->cnt = 0;
        instance->btn = 0;
        instance->serial = 0;
    }
}

uint8_t subghz_protocol_decoder_secplus_v2_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderSecPlus_v2* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool subghz_protocol_decoder_secplus_v2_serialize(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t frequency,
    FuriHalSubGhzPreset preset) {
    furi_assert(context);
    SubGhzProtocolDecoderSecPlus_v2* instance = context;
    bool res =
        subghz_block_generic_serialize(&instance->generic, flipper_format, frequency, preset);

    uint8_t key_data[sizeof(uint64_t)] = {0};
    for(size_t i = 0; i < sizeof(uint64_t); i++) {
        key_data[sizeof(uint64_t) - i - 1] = (instance->secplus_packet_1 >> i * 8) & 0xFF;
    }

    if(res &&
       !flipper_format_write_hex(flipper_format, "Secplus_packet_1", key_data, sizeof(uint64_t))) {
        FURI_LOG_E(TAG, "Unable to add Secplus_packet_1");
        res = false;
    }
    return res;
}

bool subghz_protocol_decoder_secplus_v2_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderSecPlus_v2* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        uint8_t key_data[sizeof(uint64_t)] = {0};
        if(!flipper_format_read_hex(
               flipper_format, "Secplus_packet_1", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Missing Secplus_packet_1");
            break;
        }
        for(uint8_t i = 0; i < sizeof(uint64_t); i++) {
            instance->secplus_packet_1 = instance->secplus_packet_1 << 8 | key_data[i];
        }
        res = true;
    } while(false);

    return res;
}

void subghz_protocol_decoder_secplus_v2_get_string(void* context, string_t output) {
    furi_assert(context);
    SubGhzProtocolDecoderSecPlus_v2* instance = context;
    subghz_protocol_secplus_v2_remote_controller(&instance->generic, instance->secplus_packet_1);

    string_cat_printf(
        output,
        "%s %db\r\n"
        "Pk1:0x%lX%08lX\r\n"
        "Pk2:0x%lX%08lX\r\n"
        "Sn:0x%08lX  Btn:0x%01X\r\n"
        "Cnt:0x%03X\r\n",

        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->secplus_packet_1 >> 32),
        (uint32_t)instance->secplus_packet_1,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)instance->generic.data,
        instance->generic.serial,
        instance->generic.btn,
        instance->generic.cnt);
}
