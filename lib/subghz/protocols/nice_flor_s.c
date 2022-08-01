#include "nice_flor_s.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

/*
 * https://phreakerclub.com/1615
 * https://phreakerclub.com/forum/showthread.php?t=2360
 * https://vrtp.ru/index.php?showtopic=27867
 */

#define TAG "SubGhzProtocoNiceFlorS"

static const SubGhzBlockConst subghz_protocol_nice_flor_s_const = {
    .te_short = 500,
    .te_long = 1000,
    .te_delta = 300,
    .min_count_bit_for_found = 52,
};

struct SubGhzProtocolDecoderNiceFlorS {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    const char* nice_flor_s_rainbow_table_file_name;
};

struct SubGhzProtocolEncoderNiceFlorS {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    NiceFlorSDecoderStepReset = 0,
    NiceFlorSDecoderStepCheckHeader,
    NiceFlorSDecoderStepFoundHeader,
    NiceFlorSDecoderStepSaveDuration,
    NiceFlorSDecoderStepCheckDuration,
} NiceFlorSDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_nice_flor_s_decoder = {
    .alloc = subghz_protocol_decoder_nice_flor_s_alloc,
    .free = subghz_protocol_decoder_nice_flor_s_free,

    .feed = subghz_protocol_decoder_nice_flor_s_feed,
    .reset = subghz_protocol_decoder_nice_flor_s_reset,

    .get_hash_data = subghz_protocol_decoder_nice_flor_s_get_hash_data,
    .serialize = subghz_protocol_decoder_nice_flor_s_serialize,
    .deserialize = subghz_protocol_decoder_nice_flor_s_deserialize,
    .get_string = subghz_protocol_decoder_nice_flor_s_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_nice_flor_s_encoder = {
    .alloc = NULL,
    .free = NULL,

    .deserialize = NULL,
    .stop = NULL,
    .yield = NULL,
};

const SubGhzProtocol subghz_protocol_nice_flor_s = {
    .name = SUBGHZ_PROTOCOL_NICE_FLOR_S_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_868 | SubGhzProtocolFlag_AM |
            SubGhzProtocolFlag_Decodable,

    .decoder = &subghz_protocol_nice_flor_s_decoder,
    .encoder = &subghz_protocol_nice_flor_s_encoder,
};

/** 
 * Read bytes from rainbow table
 * @param file_name Full path to rainbow table the file 
 * @param address Byte address in file
 * @return data
 */
static uint8_t
    subghz_protocol_nice_flor_s_get_byte_in_file(const char* file_name, uint32_t address) {
    if(!file_name) return 0;

    uint8_t buffer[1] = {0};
    if(subghz_keystore_raw_get_data(file_name, address, buffer, sizeof(uint8_t))) {
        return buffer[0];
    } else {
        return 0;
    }
}

static inline void subghz_protocol_decoder_nice_flor_s_magic_xor(uint8_t* p, uint8_t k) {
    for(uint8_t i = 1; i < 6; i++) {
        p[i] ^= k;
    }
}

uint64_t subghz_protocol_nice_flor_s_encrypt(uint64_t data, const char* file_name) {
    uint8_t* p = (uint8_t*)&data;

    uint8_t k = 0;
    for(uint8_t y = 0; y < 2; y++) {
        k = subghz_protocol_nice_flor_s_get_byte_in_file(file_name, p[0] & 0x1f);
        subghz_protocol_decoder_nice_flor_s_magic_xor(p, k);

        p[5] &= 0x0f;
        p[0] ^= k & 0xe0;
        k = subghz_protocol_nice_flor_s_get_byte_in_file(file_name, p[0] >> 3) + 0x25;
        subghz_protocol_decoder_nice_flor_s_magic_xor(p, k);

        p[5] &= 0x0f;
        p[0] ^= k & 0x7;
        if(y == 0) {
            k = p[0];
            p[0] = p[1];
            p[1] = k;
        }
    }

    p[5] = ~p[5] & 0x0f;
    k = ~p[4];
    p[4] = ~p[0];
    p[0] = ~p[2];
    p[2] = k;
    k = ~p[3];
    p[3] = ~p[1];
    p[1] = k;

    return data;
}

static uint64_t
    subghz_protocol_nice_flor_s_decrypt(SubGhzBlockGeneric* instance, const char* file_name) {
    furi_assert(instance);
    uint64_t data = instance->data;
    uint8_t* p = (uint8_t*)&data;

    uint8_t k = 0;

    k = ~p[4];
    p[5] = ~p[5];
    p[4] = ~p[2];
    p[2] = ~p[0];
    p[0] = k;
    k = ~p[3];
    p[3] = ~p[1];
    p[1] = k;

    for(uint8_t y = 0; y < 2; y++) {
        k = subghz_protocol_nice_flor_s_get_byte_in_file(file_name, p[0] >> 3) + 0x25;
        subghz_protocol_decoder_nice_flor_s_magic_xor(p, k);

        p[5] &= 0x0f;
        p[0] ^= k & 0x7;
        k = subghz_protocol_nice_flor_s_get_byte_in_file(file_name, p[0] & 0x1f);
        subghz_protocol_decoder_nice_flor_s_magic_xor(p, k);

        p[5] &= 0x0f;
        p[0] ^= k & 0xe0;

        if(y == 0) {
            k = p[0];
            p[0] = p[1];
            p[1] = k;
        }
    }
    return data;
}

void* subghz_protocol_decoder_nice_flor_s_alloc(SubGhzEnvironment* environment) {
    SubGhzProtocolDecoderNiceFlorS* instance = malloc(sizeof(SubGhzProtocolDecoderNiceFlorS));
    instance->base.protocol = &subghz_protocol_nice_flor_s;
    instance->generic.protocol_name = instance->base.protocol->name;
    instance->nice_flor_s_rainbow_table_file_name =
        subghz_environment_get_nice_flor_s_rainbow_table_file_name(environment);
    if(instance->nice_flor_s_rainbow_table_file_name) {
        FURI_LOG_I(
            TAG, "Loading rainbow table from %s", instance->nice_flor_s_rainbow_table_file_name);
    }
    return instance;
}

void subghz_protocol_decoder_nice_flor_s_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlorS* instance = context;
    instance->nice_flor_s_rainbow_table_file_name = NULL;
    free(instance);
}

void subghz_protocol_decoder_nice_flor_s_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlorS* instance = context;
    instance->decoder.parser_step = NiceFlorSDecoderStepReset;
}

void subghz_protocol_decoder_nice_flor_s_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlorS* instance = context;

    switch(instance->decoder.parser_step) {
    case NiceFlorSDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_nice_flor_s_const.te_short * 38) <
                        subghz_protocol_nice_flor_s_const.te_delta * 38)) {
            //Found start header Nice Flor-S
            instance->decoder.parser_step = NiceFlorSDecoderStepCheckHeader;
        }
        break;
    case NiceFlorSDecoderStepCheckHeader:
        if((level) && (DURATION_DIFF(duration, subghz_protocol_nice_flor_s_const.te_short * 3) <
                       subghz_protocol_nice_flor_s_const.te_delta * 3)) {
            //Found next header Nice Flor-S
            instance->decoder.parser_step = NiceFlorSDecoderStepFoundHeader;
        } else {
            instance->decoder.parser_step = NiceFlorSDecoderStepReset;
        }
        break;
    case NiceFlorSDecoderStepFoundHeader:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_nice_flor_s_const.te_short * 3) <
                        subghz_protocol_nice_flor_s_const.te_delta * 3)) {
            //Found header Nice Flor-S
            instance->decoder.parser_step = NiceFlorSDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        } else {
            instance->decoder.parser_step = NiceFlorSDecoderStepReset;
        }
        break;
    case NiceFlorSDecoderStepSaveDuration:
        if(level) {
            if(DURATION_DIFF(duration, subghz_protocol_nice_flor_s_const.te_short * 3) <
               subghz_protocol_nice_flor_s_const.te_delta) {
                //Found STOP bit
                instance->decoder.parser_step = NiceFlorSDecoderStepReset;
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_nice_flor_s_const.min_count_bit_for_found) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                break;
            } else {
                //save interval
                instance->decoder.te_last = duration;
                instance->decoder.parser_step = NiceFlorSDecoderStepCheckDuration;
            }
        }
        break;
    case NiceFlorSDecoderStepCheckDuration:
        if(!level) {
            if((DURATION_DIFF(
                    instance->decoder.te_last, subghz_protocol_nice_flor_s_const.te_short) <
                subghz_protocol_nice_flor_s_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_nice_flor_s_const.te_long) <
                subghz_protocol_nice_flor_s_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = NiceFlorSDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_nice_flor_s_const.te_long) <
                 subghz_protocol_nice_flor_s_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_nice_flor_s_const.te_short) <
                 subghz_protocol_nice_flor_s_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = NiceFlorSDecoderStepSaveDuration;
            } else
                instance->decoder.parser_step = NiceFlorSDecoderStepReset;
        } else {
            instance->decoder.parser_step = NiceFlorSDecoderStepReset;
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 * @param file_name Full path to rainbow table the file 
 */
static void subghz_protocol_nice_flor_s_remote_controller(
    SubGhzBlockGeneric* instance,
    const char* file_name) {
    /*
    * Packet format Nice Flor-s: START-P0-P1-P2-P3-P4-P5-P6-P7-STOP
    * P0 (4-bit)    - button positional code - 1:0x1, 2:0x2, 3:0x4, 4:0x8;
    * P1 (4-bit)    - batch repetition number, calculated by the formula:
    * P1 = 0xF ^ P0 ^ n; where n changes from 1 to 15, then 0, and then in a circle
    * key 1: {0xE,0xF,0xC,0xD,0xA,0xB,0x8,0x9,0x6,0x7,0x4,0x5,0x2,0x3,0x0,0x1};
    * key 2: {0xD,0xC,0xF,0xE,0x9,0x8,0xB,0xA,0x5,0x4,0x7,0x6,0x1,0x0,0x3,0x2};
    * key 3: {0xB,0xA,0x9,0x8,0xF,0xE,0xD,0xC,0x3,0x2,0x1,0x0,0x7,0x6,0x5,0x4};
    * key 4: {0x7,0x6,0x5,0x4,0x3,0x2,0x1,0x0,0xF,0xE,0xD,0xC,0xB,0xA,0x9,0x8};
    * P2 (4-bit)    - part of the serial number, P2 = (K ^ S3) & 0xF;
    * P3 (byte)     - the major part of the encrypted index
    * P4 (byte)     - the low-order part of the encrypted index
    * P5 (byte)     - part of the serial number, P5 = K ^ S2;
    * P6 (byte)     - part of the serial number, P6 = K ^ S1;
    * P7 (byte)     - part of the serial number, P7 = K ^ S0;
    * K (byte)      - depends on P3 and P4, K = Fk(P3, P4);
    * S3,S2,S1,S0   - serial number of the console 28 bit.
    *
    * data    => 0x1c5783607f7b3     key  serial  cnt
    * decrypt => 0x10436c6820444 => 0x1  0436c682 0444
    * 
    */
    if(!file_name) {
        instance->cnt = 0;
        instance->serial = 0;
        instance->btn = 0;
    } else {
        uint64_t decrypt = subghz_protocol_nice_flor_s_decrypt(instance, file_name);
        instance->cnt = decrypt & 0xFFFF;
        instance->serial = (decrypt >> 16) & 0xFFFFFFF;
        instance->btn = (decrypt >> 48) & 0xF;
    }
}

uint8_t subghz_protocol_decoder_nice_flor_s_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlorS* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool subghz_protocol_decoder_nice_flor_s_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlorS* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

bool subghz_protocol_decoder_nice_flor_s_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlorS* instance = context;
    bool ret = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_nice_flor_s_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        ret = true;
    } while(false);
    return ret;
}

void subghz_protocol_decoder_nice_flor_s_get_string(void* context, string_t output) {
    furi_assert(context);
    SubGhzProtocolDecoderNiceFlorS* instance = context;

    subghz_protocol_nice_flor_s_remote_controller(
        &instance->generic, instance->nice_flor_s_rainbow_table_file_name);
    uint32_t code_found_hi = instance->generic.data >> 32;
    uint32_t code_found_lo = instance->generic.data & 0x00000000ffffffff;

    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%lX%08lX\r\n"
        "Sn:%05lX\r\n"
        "Cnt:%04X Btn:%02lX\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        code_found_hi,
        code_found_lo,
        instance->generic.serial,
        instance->generic.cnt,
        instance->generic.btn);
}
