#include "came_atomo.h"
#include <lib/toolbox/manchester_decoder.h>
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocoCameAtomo"

#define SUBGHZ_NO_CAME_ATOMO_RAINBOW_TABLE 0xFFFFFFFFFFFFFFFF

static const SubGhzBlockConst subghz_protocol_came_atomo_const = {
    .te_short = 600,
    .te_long = 1200,
    .te_delta = 250,
    .min_count_bit_for_found = 62,
};

struct SubGhzProtocolDecoderCameAtomo {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    ManchesterState manchester_saved_state;
    const char* came_atomo_rainbow_table_file_name;
};

struct SubGhzProtocolEncoderCameAtomo {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    CameAtomoDecoderStepReset = 0,
    CameAtomoDecoderStepDecoderData,
} CameAtomoDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_came_atomo_decoder = {
    .alloc = subghz_protocol_decoder_came_atomo_alloc,
    .free = subghz_protocol_decoder_came_atomo_free,

    .feed = subghz_protocol_decoder_came_atomo_feed,
    .reset = subghz_protocol_decoder_came_atomo_reset,

    .get_hash_data = subghz_protocol_decoder_came_atomo_get_hash_data,
    .serialize = subghz_protocol_decoder_came_atomo_serialize,
    .deserialize = subghz_protocol_decoder_came_atomo_deserialize,
    .get_string = subghz_protocol_decoder_came_atomo_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_came_atomo_encoder = {
    .alloc = NULL,
    .free = NULL,

    .deserialize = NULL,
    .stop = NULL,
    .yield = NULL,
};

const SubGhzProtocol subghz_protocol_came_atomo = {
    .name = SUBGHZ_PROTOCOL_CAME_ATOMO_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable,

    .decoder = &subghz_protocol_came_atomo_decoder,
    .encoder = &subghz_protocol_came_atomo_encoder,
};

void* subghz_protocol_decoder_came_atomo_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolDecoderCameAtomo* instance = malloc(sizeof(SubGhzProtocolDecoderCameAtomo));
    instance->base.protocol = &subghz_protocol_came_atomo;
    instance->generic.protocol_name = instance->base.protocol->name;
    instance->came_atomo_rainbow_table_file_name =
        subghz_environment_get_came_atomo_rainbow_table_file_name(environment);
    if(instance->came_atomo_rainbow_table_file_name) {
        FURI_LOG_I(
            TAG, "Loading rainbow table from %s", instance->came_atomo_rainbow_table_file_name);
    }
    return instance;
}

void subghz_protocol_decoder_came_atomo_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderCameAtomo* instance = context;
    instance->came_atomo_rainbow_table_file_name = NULL;
    free(instance);
}

void subghz_protocol_decoder_came_atomo_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderCameAtomo* instance = context;
    instance->decoder.parser_step = CameAtomoDecoderStepReset;
    manchester_advance(
        instance->manchester_saved_state,
        ManchesterEventReset,
        &instance->manchester_saved_state,
        NULL);
}

void subghz_protocol_decoder_came_atomo_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderCameAtomo* instance = context;

    ManchesterEvent event = ManchesterEventReset;
    switch(instance->decoder.parser_step) {
    case CameAtomoDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_came_atomo_const.te_long * 60) <
                        subghz_protocol_came_atomo_const.te_delta * 40)) {
            //Found header CAME
            instance->decoder.parser_step = CameAtomoDecoderStepDecoderData;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 1;
            manchester_advance(
                instance->manchester_saved_state,
                ManchesterEventReset,
                &instance->manchester_saved_state,
                NULL);
            manchester_advance(
                instance->manchester_saved_state,
                ManchesterEventShortLow,
                &instance->manchester_saved_state,
                NULL);
        }
        break;
    case CameAtomoDecoderStepDecoderData:
        if(!level) {
            if(DURATION_DIFF(duration, subghz_protocol_came_atomo_const.te_short) <
               subghz_protocol_came_atomo_const.te_delta) {
                event = ManchesterEventShortLow;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_came_atomo_const.te_long) <
                subghz_protocol_came_atomo_const.te_delta) {
                event = ManchesterEventLongLow;
            } else if(
                duration >= ((uint32_t)subghz_protocol_came_atomo_const.te_long * 2 +
                             subghz_protocol_came_atomo_const.te_delta)) {
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_came_atomo_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 1;
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventReset,
                    &instance->manchester_saved_state,
                    NULL);
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventShortLow,
                    &instance->manchester_saved_state,
                    NULL);
            } else {
                instance->decoder.parser_step = CameAtomoDecoderStepReset;
            }
        } else {
            if(DURATION_DIFF(duration, subghz_protocol_came_atomo_const.te_short) <
               subghz_protocol_came_atomo_const.te_delta) {
                event = ManchesterEventShortHigh;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_came_atomo_const.te_long) <
                subghz_protocol_came_atomo_const.te_delta) {
                event = ManchesterEventLongHigh;
            } else {
                instance->decoder.parser_step = CameAtomoDecoderStepReset;
            }
        }
        if(event != ManchesterEventReset) {
            bool data;
            bool data_ok = manchester_advance(
                instance->manchester_saved_state, event, &instance->manchester_saved_state, &data);

            if(data_ok) {
                instance->decoder.decode_data = (instance->decoder.decode_data << 1) | !data;
                instance->decoder.decode_count_bit++;
            }
        }
        break;
    }
}

/** 
 * Read bytes from rainbow table
 * @param file_name Full path to rainbow table the file 
 * @param number_atomo_magic_xor number in the array
 * @return atomo_magic_xor
 */
static uint64_t subghz_protocol_came_atomo_get_magic_xor_in_file(
    const char* file_name,
    uint8_t number_atomo_magic_xor) {
    if(!strcmp(file_name, "")) return SUBGHZ_NO_CAME_ATOMO_RAINBOW_TABLE;

    uint8_t buffer[sizeof(uint64_t)] = {0};
    uint32_t address = number_atomo_magic_xor * sizeof(uint64_t);
    uint64_t atomo_magic_xor = 0;

    if(subghz_keystore_raw_get_data(file_name, address, buffer, sizeof(uint64_t))) {
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            atomo_magic_xor = (atomo_magic_xor << 8) | buffer[i];
        }
    } else {
        atomo_magic_xor = SUBGHZ_NO_CAME_ATOMO_RAINBOW_TABLE;
    }
    return atomo_magic_xor;
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 * @param file_name Full path to rainbow table the file
 */
static void subghz_protocol_came_atomo_remote_controller(
    SubGhzBlockGeneric* instance,
    const char* file_name) {
    /* 
    * 0x1fafef3ed0f7d9ef
    * 0x185fcc1531ee86e7
    * 0x184fa96912c567ff
    * 0x187f8a42f3dc38f7
    * 0x186f63915492a5cd
    * 0x181f40bab58bfac5
    * 0x180f25c696a01bdd
    * 0x183f06ed77b944d5
    * 0x182ef661d83d21a9
    * 0x18ded54a39247ea1
    * 0x18ceb0361a0f9fb9
    * 0x18fe931dfb16c0b1
    * 0x18ee7ace5c585d8b
    * ........ 
    * transmission consists of 99 parcels with increasing counter while holding down the button
    * with each new press, the counter in the encrypted part increases
    * 
    * 0x1FAFF13ED0F7D9EF
    * 0x1FAFF11ED0F7D9EF
    * 0x1FAFF10ED0F7D9EF
    * 0x1FAFF0FED0F7D9EF
    * 0x1FAFF0EED0F7D9EF
    * 0x1FAFF0DED0F7D9EF
    * 0x1FAFF0CED0F7D9EF
    * 0x1FAFF0BED0F7D9EF
    * 0x1FAFF0AED0F7D9EF 
    * 
    *                   where     0x1FAF - parcel counter, 0хF0A - button press counter,
    *                           0xED0F7D9E - serial number, 0хF -  key
    * 0x1FAF parcel counter - 1 in the parcel queue ^ 0x185F =  0x07F0
    * 0x185f ^ 0x185F = 0x0000
    * 0x184f ^ 0x185F = 0x0010
    * 0x187f ^ 0x185F = 0x0020
    * .....
    * 0x182e ^ 0x185F = 0x0071 
    * 0x18de ^ 0x185F = 0x0081
    * .....
    * 0x1e43 ^ 0x185F = 0x061C
    *                           where the last nibble is incremented every 8 samples
    * 
    * Decode
    * 
    * 0x1cf6931dfb16c0b1 => 0x1cf6
    * 0x1cf6 ^ 0x185F = 0x04A9
    * 0x04A9 => 0x04A = 74 (dec)
    * 74+1 % 32(atomo_magic_xor) = 11
    * GET atomo_magic_xor[11] = 0xXXXXXXXXXXXXXXXX
    * 0x931dfb16c0b1 ^ 0xXXXXXXXXXXXXXXXX =  0xEF3ED0F7D9EF
    * 0xEF3 ED0F7D9E F  => 0xEF3 - CNT, 0xED0F7D9E - SN, 0xF - key
    * 
    * */

    uint16_t parcel_counter = instance->data >> 48;
    parcel_counter = parcel_counter ^ 0x185F;
    parcel_counter >>= 4;
    uint8_t ind = (parcel_counter + 1) % 32;
    uint64_t temp_data = instance->data & 0x0000FFFFFFFFFFFF;
    uint64_t atomo_magic_xor = subghz_protocol_came_atomo_get_magic_xor_in_file(file_name, ind);

    if(atomo_magic_xor != SUBGHZ_NO_CAME_ATOMO_RAINBOW_TABLE) {
        temp_data = temp_data ^ atomo_magic_xor;
        instance->cnt = temp_data >> 36;
        instance->serial = (temp_data >> 4) & 0x000FFFFFFFF;
        instance->btn = temp_data & 0xF;
    } else {
        instance->cnt = 0;
        instance->serial = 0;
        instance->btn = 0;
    }
}

uint8_t subghz_protocol_decoder_came_atomo_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderCameAtomo* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool subghz_protocol_decoder_came_atomo_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderCameAtomo* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

bool subghz_protocol_decoder_came_atomo_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderCameAtomo* instance = context;
    bool ret = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_came_atomo_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        ret = true;
    } while(false);
    return ret;
}

void subghz_protocol_decoder_came_atomo_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderCameAtomo* instance = context;
    subghz_protocol_came_atomo_remote_controller(
        &instance->generic, instance->came_atomo_rainbow_table_file_name);
    uint32_t code_found_hi = instance->generic.data >> 32;
    uint32_t code_found_lo = instance->generic.data & 0x00000000ffffffff;

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Key:0x%lX%08lX\r\n"
        "Sn:0x%08lX  Btn:0x%01X\r\n"
        "Cnt:0x%03lX\r\n",

        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        code_found_hi,
        code_found_lo,
        instance->generic.serial,
        instance->generic.btn,
        instance->generic.cnt);
}
