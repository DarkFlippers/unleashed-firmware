#include "chamberlain_code.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolChamb_Code"

#define CHAMBERLAIN_CODE_BIT_STOP 0b0001
#define CHAMBERLAIN_CODE_BIT_1 0b0011
#define CHAMBERLAIN_CODE_BIT_0 0b0111

#define CHAMBERLAIN_7_CODE_MASK 0xF000000FF0F
#define CHAMBERLAIN_8_CODE_MASK 0xF00000F00F
#define CHAMBERLAIN_9_CODE_MASK 0xF000000000F

#define CHAMBERLAIN_7_CODE_MASK_CHECK 0x10000001101
#define CHAMBERLAIN_8_CODE_MASK_CHECK 0x1000001001
#define CHAMBERLAIN_9_CODE_MASK_CHECK 0x10000000001

#define CHAMBERLAIN_7_CODE_DIP_PATTERN "%c%c%c%c%c%c%c"
#define CHAMBERLAIN_7_CODE_DATA_TO_DIP(dip)                                                 \
    (dip & 0x0040 ? '1' : '0'), (dip & 0x0020 ? '1' : '0'), (dip & 0x0010 ? '1' : '0'),     \
        (dip & 0x0008 ? '1' : '0'), (dip & 0x0004 ? '1' : '0'), (dip & 0x0002 ? '1' : '0'), \
        (dip & 0x0001 ? '1' : '0')

#define CHAMBERLAIN_8_CODE_DIP_PATTERN "%c%c%c%c%cx%c%c"
#define CHAMBERLAIN_8_CODE_DATA_TO_DIP(dip)                                                 \
    (dip & 0x0080 ? '1' : '0'), (dip & 0x0040 ? '1' : '0'), (dip & 0x0020 ? '1' : '0'),     \
        (dip & 0x0010 ? '1' : '0'), (dip & 0x0008 ? '1' : '0'), (dip & 0x0001 ? '1' : '0'), \
        (dip & 0x0002 ? '1' : '0')

#define CHAMBERLAIN_9_CODE_DIP_PATTERN "%c%c%c%c%c%c%c%c%c"
#define CHAMBERLAIN_9_CODE_DATA_TO_DIP(dip)                                                 \
    (dip & 0x0100 ? '1' : '0'), (dip & 0x0080 ? '1' : '0'), (dip & 0x0040 ? '1' : '0'),     \
        (dip & 0x0020 ? '1' : '0'), (dip & 0x0010 ? '1' : '0'), (dip & 0x0008 ? '1' : '0'), \
        (dip & 0x0001 ? '1' : '0'), (dip & 0x0002 ? '1' : '0'), (dip & 0x0004 ? '1' : '0')

static const SubGhzBlockConst subghz_protocol_chamb_code_const = {
    .te_short = 1000,
    .te_long = 3000,
    .te_delta = 200,
    .min_count_bit_for_found = 10,
};

struct SubGhzProtocolDecoderChamb_Code {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderChamb_Code {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    Chamb_CodeDecoderStepReset = 0,
    Chamb_CodeDecoderStepFoundStartBit,
    Chamb_CodeDecoderStepSaveDuration,
    Chamb_CodeDecoderStepCheckDuration,
} Chamb_CodeDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_chamb_code_decoder = {
    .alloc = subghz_protocol_decoder_chamb_code_alloc,
    .free = subghz_protocol_decoder_chamb_code_free,

    .feed = subghz_protocol_decoder_chamb_code_feed,
    .reset = subghz_protocol_decoder_chamb_code_reset,

    .get_hash_data = subghz_protocol_decoder_chamb_code_get_hash_data,
    .serialize = subghz_protocol_decoder_chamb_code_serialize,
    .deserialize = subghz_protocol_decoder_chamb_code_deserialize,
    .get_string = subghz_protocol_decoder_chamb_code_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_chamb_code_encoder = {
    .alloc = subghz_protocol_encoder_chamb_code_alloc,
    .free = subghz_protocol_encoder_chamb_code_free,

    .deserialize = subghz_protocol_encoder_chamb_code_deserialize,
    .stop = subghz_protocol_encoder_chamb_code_stop,
    .yield = subghz_protocol_encoder_chamb_code_yield,
};

const SubGhzProtocol subghz_protocol_chamb_code = {
    .name = SUBGHZ_PROTOCOL_CHAMB_CODE_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_315 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_chamb_code_decoder,
    .encoder = &subghz_protocol_chamb_code_encoder,
};

void* subghz_protocol_encoder_chamb_code_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderChamb_Code* instance = malloc(sizeof(SubGhzProtocolEncoderChamb_Code));

    instance->base.protocol = &subghz_protocol_chamb_code;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 10;
    instance->encoder.size_upload = 24;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_running = false;
    return instance;
}

void subghz_protocol_encoder_chamb_code_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderChamb_Code* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

static uint64_t subghz_protocol_chamb_bit_to_code(uint64_t data, uint8_t size) {
    uint64_t data_res = 0;
    for(uint8_t i = 0; i < size; i++) {
        if(!(bit_read(data, size - i - 1))) {
            data_res = data_res << 4 | CHAMBERLAIN_CODE_BIT_0;
        } else {
            data_res = data_res << 4 | CHAMBERLAIN_CODE_BIT_1;
        }
    }
    return data_res;
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderChamb_Code instance
 * @return true On success
 */
static bool
    subghz_protocol_encoder_chamb_code_get_upload(SubGhzProtocolEncoderChamb_Code* instance) {
    furi_assert(instance);

    uint64_t data = subghz_protocol_chamb_bit_to_code(
        instance->generic.data, instance->generic.data_count_bit);

    switch(instance->generic.data_count_bit) {
    case 7:
        data = ((data >> 4) << 16) | (data & 0xF) << 4 | CHAMBERLAIN_7_CODE_MASK_CHECK;
        break;
    case 8:
        data = ((data >> 12) << 16) | (data & 0xFF) << 4 | CHAMBERLAIN_8_CODE_MASK_CHECK;
        break;
    case 9:
        data = (data << 4) | CHAMBERLAIN_9_CODE_MASK_CHECK;
        break;

    default:
        FURI_LOG_E(TAG, "Invalid bits count");
        return false;
        break;
    }
#define UPLOAD_HEX_DATA_SIZE 10
    uint8_t upload_hex_data[UPLOAD_HEX_DATA_SIZE] = {0};
    size_t upload_hex_count_bit = 0;

    //insert guard time
    for(uint8_t i = 0; i < 36; i++) {
        subghz_protocol_blocks_set_bit_array(
            0, upload_hex_data, upload_hex_count_bit++, UPLOAD_HEX_DATA_SIZE);
    }

    //insert data
    switch(instance->generic.data_count_bit) {
    case 7:
    case 9:
        for(uint8_t i = 44; i > 0; i--) {
            if(!bit_read(data, i - 1)) {
                subghz_protocol_blocks_set_bit_array(
                    0, upload_hex_data, upload_hex_count_bit++, UPLOAD_HEX_DATA_SIZE);
            } else {
                subghz_protocol_blocks_set_bit_array(
                    1, upload_hex_data, upload_hex_count_bit++, UPLOAD_HEX_DATA_SIZE);
            }
        }
        break;
    case 8:
        for(uint8_t i = 40; i > 0; i--) {
            if(!bit_read(data, i - 1)) {
                subghz_protocol_blocks_set_bit_array(
                    0, upload_hex_data, upload_hex_count_bit++, UPLOAD_HEX_DATA_SIZE);
            } else {
                subghz_protocol_blocks_set_bit_array(
                    1, upload_hex_data, upload_hex_count_bit++, UPLOAD_HEX_DATA_SIZE);
            }
        }
        break;
    }

    instance->encoder.size_upload = subghz_protocol_blocks_get_upload(
        upload_hex_data,
        upload_hex_count_bit,
        instance->encoder.upload,
        instance->encoder.size_upload,
        subghz_protocol_chamb_code_const.te_short);

    return true;
}

bool subghz_protocol_encoder_chamb_code_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderChamb_Code* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        if(instance->generic.data_count_bit >
           subghz_protocol_chamb_code_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        //optional parameter parameter
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!subghz_protocol_encoder_chamb_code_get_upload(instance)) break;
        instance->encoder.is_running = true;

        res = true;
    } while(false);

    return res;
}

void subghz_protocol_encoder_chamb_code_stop(void* context) {
    SubGhzProtocolEncoderChamb_Code* instance = context;
    instance->encoder.is_running = false;
}

LevelDuration subghz_protocol_encoder_chamb_code_yield(void* context) {
    SubGhzProtocolEncoderChamb_Code* instance = context;

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

void* subghz_protocol_decoder_chamb_code_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderChamb_Code* instance = malloc(sizeof(SubGhzProtocolDecoderChamb_Code));
    instance->base.protocol = &subghz_protocol_chamb_code;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_chamb_code_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderChamb_Code* instance = context;
    free(instance);
}

void subghz_protocol_decoder_chamb_code_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderChamb_Code* instance = context;
    instance->decoder.parser_step = Chamb_CodeDecoderStepReset;
}

static bool subghz_protocol_chamb_code_to_bit(uint64_t* data, uint8_t size) {
    uint64_t data_tmp = data[0];
    uint64_t data_res = 0;
    for(uint8_t i = 0; i < size; i++) {
        if((data_tmp & 0xF) == CHAMBERLAIN_CODE_BIT_0) {
            bit_write(data_res, i, 0);
        } else if((data_tmp & 0xF) == CHAMBERLAIN_CODE_BIT_1) {
            bit_write(data_res, i, 1);
        } else {
            return false;
        }
        data_tmp >>= 4;
    }
    data[0] = data_res;
    return true;
}

static bool subghz_protocol_decoder_chamb_code_check_mask_and_parse(
    SubGhzProtocolDecoderChamb_Code* instance) {
    furi_assert(instance);
    if(instance->decoder.decode_count_bit >
       subghz_protocol_chamb_code_const.min_count_bit_for_found + 1)
        return false;

    if((instance->decoder.decode_data & CHAMBERLAIN_7_CODE_MASK) ==
       CHAMBERLAIN_7_CODE_MASK_CHECK) {
        instance->decoder.decode_count_bit = 7;
        instance->decoder.decode_data &= ~CHAMBERLAIN_7_CODE_MASK;
        instance->decoder.decode_data = (instance->decoder.decode_data >> 12) |
                                        ((instance->decoder.decode_data >> 4) & 0xF);
    } else if(
        (instance->decoder.decode_data & CHAMBERLAIN_8_CODE_MASK) ==
        CHAMBERLAIN_8_CODE_MASK_CHECK) {
        instance->decoder.decode_count_bit = 8;
        instance->decoder.decode_data &= ~CHAMBERLAIN_8_CODE_MASK;
        instance->decoder.decode_data = instance->decoder.decode_data >> 4 |
                                        CHAMBERLAIN_CODE_BIT_0 << 8; //DIP 6 no use
    } else if(
        (instance->decoder.decode_data & CHAMBERLAIN_9_CODE_MASK) ==
        CHAMBERLAIN_9_CODE_MASK_CHECK) {
        instance->decoder.decode_count_bit = 9;
        instance->decoder.decode_data &= ~CHAMBERLAIN_9_CODE_MASK;
        instance->decoder.decode_data >>= 4;
    } else {
        return false;
    }
    return subghz_protocol_chamb_code_to_bit(
        &instance->decoder.decode_data, instance->decoder.decode_count_bit);
}

void subghz_protocol_decoder_chamb_code_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderChamb_Code* instance = context;
    switch(instance->decoder.parser_step) {
    case Chamb_CodeDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_chamb_code_const.te_short * 39) <
                        subghz_protocol_chamb_code_const.te_delta * 20)) {
            //Found header Chamb_Code
            instance->decoder.parser_step = Chamb_CodeDecoderStepFoundStartBit;
        }
        break;
    case Chamb_CodeDecoderStepFoundStartBit:
        if((level) && (DURATION_DIFF(duration, subghz_protocol_chamb_code_const.te_short) <
                       subghz_protocol_chamb_code_const.te_delta)) {
            //Found start bit Chamb_Code
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.decode_data = instance->decoder.decode_data << 4 |
                                            CHAMBERLAIN_CODE_BIT_STOP;
            instance->decoder.decode_count_bit++;
            instance->decoder.parser_step = Chamb_CodeDecoderStepSaveDuration;
        } else {
            instance->decoder.parser_step = Chamb_CodeDecoderStepReset;
        }
        break;
    case Chamb_CodeDecoderStepSaveDuration:
        if(!level) { //save interval
            if(duration > subghz_protocol_chamb_code_const.te_short * 5) {
                if(instance->decoder.decode_count_bit >=
                   subghz_protocol_chamb_code_const.min_count_bit_for_found) {
                    instance->generic.serial = 0x0;
                    instance->generic.btn = 0x0;
                    if(subghz_protocol_decoder_chamb_code_check_mask_and_parse(instance)) {
                        instance->generic.data = instance->decoder.decode_data;
                        instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                    }
                }
                instance->decoder.parser_step = Chamb_CodeDecoderStepReset;
            } else {
                instance->decoder.te_last = duration;
                instance->decoder.parser_step = Chamb_CodeDecoderStepCheckDuration;
            }
        } else {
            instance->decoder.parser_step = Chamb_CodeDecoderStepReset;
        }
        break;
    case Chamb_CodeDecoderStepCheckDuration:
        if(level) {
            if((DURATION_DIFF( //Found stop bit Chamb_Code
                    instance->decoder.te_last,
                    subghz_protocol_chamb_code_const.te_short * 3) <
                subghz_protocol_chamb_code_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_chamb_code_const.te_short) <
                subghz_protocol_chamb_code_const.te_delta)) {
                instance->decoder.decode_data = instance->decoder.decode_data << 4 |
                                                CHAMBERLAIN_CODE_BIT_STOP;
                instance->decoder.decode_count_bit++;
                instance->decoder.parser_step = Chamb_CodeDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_chamb_code_const.te_short * 2) <
                 subghz_protocol_chamb_code_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_chamb_code_const.te_short * 2) <
                 subghz_protocol_chamb_code_const.te_delta)) {
                instance->decoder.decode_data = instance->decoder.decode_data << 4 |
                                                CHAMBERLAIN_CODE_BIT_1;
                instance->decoder.decode_count_bit++;
                instance->decoder.parser_step = Chamb_CodeDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_chamb_code_const.te_short) <
                 subghz_protocol_chamb_code_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_chamb_code_const.te_short * 3) <
                 subghz_protocol_chamb_code_const.te_delta)) {
                instance->decoder.decode_data = instance->decoder.decode_data << 4 |
                                                CHAMBERLAIN_CODE_BIT_0;
                instance->decoder.decode_count_bit++;
                instance->decoder.parser_step = Chamb_CodeDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = Chamb_CodeDecoderStepReset;
            }

        } else {
            instance->decoder.parser_step = Chamb_CodeDecoderStepReset;
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_chamb_code_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderChamb_Code* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool subghz_protocol_decoder_chamb_code_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderChamb_Code* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

bool subghz_protocol_decoder_chamb_code_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderChamb_Code* instance = context;
    bool ret = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            break;
        }
        if(instance->generic.data_count_bit >
           subghz_protocol_chamb_code_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        ret = true;
    } while(false);
    return ret;
}

void subghz_protocol_decoder_chamb_code_get_string(void* context, string_t output) {
    furi_assert(context);
    SubGhzProtocolDecoderChamb_Code* instance = context;

    uint32_t code_found_lo = instance->generic.data & 0x00000000ffffffff;

    uint64_t code_found_reverse = subghz_protocol_blocks_reverse_key(
        instance->generic.data, instance->generic.data_count_bit);

    uint32_t code_found_reverse_lo = code_found_reverse & 0x00000000ffffffff;

    string_cat_printf(
        output,
        "%s %db\r\n"
        "Key:0x%03lX\r\n"
        "Yek:0x%03lX\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        code_found_lo,
        code_found_reverse_lo);

    switch(instance->generic.data_count_bit) {
    case 7:
        string_cat_printf(
            output,
            "DIP:" CHAMBERLAIN_7_CODE_DIP_PATTERN "\r\n",
            CHAMBERLAIN_7_CODE_DATA_TO_DIP(code_found_lo));
        break;
    case 8:
        string_cat_printf(
            output,
            "DIP:" CHAMBERLAIN_8_CODE_DIP_PATTERN "\r\n",
            CHAMBERLAIN_8_CODE_DATA_TO_DIP(code_found_lo));
        break;
    case 9:
        string_cat_printf(
            output,
            "DIP:" CHAMBERLAIN_9_CODE_DIP_PATTERN "\r\n",
            CHAMBERLAIN_9_CODE_DATA_TO_DIP(code_found_lo));
        break;

    default:
        break;
    }
}
