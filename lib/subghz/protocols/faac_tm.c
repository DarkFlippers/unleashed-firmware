#include "faac_tm.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"
#include "common.h"

#define TAG "SubGhzProtocolFaacTm"

/* FAAC TM, 12 bit static code set by dip switches, AM, 433.92 and 868 MHz.
 * Frame: 35Te low gap, Te high start bit, then 12 bits sent as a low+high pair -
 * 1 = Te then 2Te, 0 = 2Te then Te. Te is nominally 400us and is measured per frame.
 */

#define FAAC_TM_HEADER_TE 35
#define FAAC_TM_TE_MIN    100
#define FAAC_TM_TE_MAX    1000

#define DIP_PATTERN "%c%c%c%c%c%c%c%c%c%c%c%c"
#define DATA_TO_DIP(dip)                                                                 \
    (dip & 0x800 ? '1' : '0'), (dip & 0x400 ? '1' : '0'), (dip & 0x200 ? '1' : '0'),     \
        (dip & 0x100 ? '1' : '0'), (dip & 0x080 ? '1' : '0'), (dip & 0x040 ? '1' : '0'), \
        (dip & 0x020 ? '1' : '0'), (dip & 0x010 ? '1' : '0'), (dip & 0x008 ? '1' : '0'), \
        (dip & 0x004 ? '1' : '0'), (dip & 0x002 ? '1' : '0'), (dip & 0x001 ? '1' : '0')

static const SubGhzBlockConst subghz_protocol_faac_tm_const = {
    .te_short = 400,
    .te_long = 800,
    .te_delta = 120,
    .min_count_bit_for_found = 12,
};

struct SubGhzProtocolDecoderFaacTm {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint32_t te;
};
SUBGHZ_ASSERT_DECODER_TE_LAYOUT(SubGhzProtocolDecoderFaacTm);

struct SubGhzProtocolEncoderFaacTm {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;

    uint32_t te;
};
SUBGHZ_ASSERT_ENCODER_GENERIC_LAYOUT(SubGhzProtocolEncoderFaacTm);

typedef enum {
    FaacTmDecoderStepReset = 0,
    FaacTmDecoderStepFoundStartBit,
    FaacTmDecoderStepSaveDuration,
    FaacTmDecoderStepCheckDuration,
} FaacTmDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_faac_tm_decoder = {
    .alloc = subghz_protocol_decoder_faac_tm_alloc,
    .free = subghz_protocol_decoder_common_free,

    .feed = subghz_protocol_decoder_faac_tm_feed,
    .reset = subghz_protocol_decoder_common_reset,

    .get_hash_data = subghz_protocol_decoder_common_get_hash_data,
    .serialize = subghz_protocol_decoder_common_serialize_te,
    .deserialize = subghz_protocol_decoder_faac_tm_deserialize,
    .get_string = subghz_protocol_decoder_faac_tm_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_faac_tm_encoder = {
    .alloc = subghz_protocol_encoder_faac_tm_alloc,
    .free = subghz_protocol_encoder_common_free,

    .deserialize = subghz_protocol_encoder_faac_tm_deserialize,
    .stop = subghz_protocol_encoder_common_stop,
    .yield = subghz_protocol_encoder_common_yield,
};

const SubGhzProtocol subghz_protocol_faac_tm = {
    .name = SUBGHZ_PROTOCOL_FAAC_TM_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_868 | SubGhzProtocolFlag_AM |
            SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save |
            SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_faac_tm_decoder,
    .encoder = &subghz_protocol_faac_tm_encoder,
};

void* subghz_protocol_encoder_faac_tm_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    return subghz_protocol_encoder_common_alloc(
        sizeof(SubGhzProtocolEncoderFaacTm), &subghz_protocol_faac_tm, 3, 52);
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderFaacTm instance
 * @return true On success
 */
static bool subghz_protocol_encoder_faac_tm_get_upload(SubGhzProtocolEncoderFaacTm* instance) {
    furi_assert(instance);
    size_t index = 0;
    size_t size_upload = (instance->generic.data_count_bit * 2) + 2;
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }
    //Send header
    instance->encoder.upload[index++] =
        level_duration_make(false, instance->te * FAAC_TM_HEADER_TE);
    //Send start bit
    instance->encoder.upload[index++] = level_duration_make(true, instance->te);
    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 0; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] = level_duration_make(false, instance->te);
            instance->encoder.upload[index++] = level_duration_make(true, instance->te * 2);
        } else {
            //send bit 0
            instance->encoder.upload[index++] = level_duration_make(false, instance->te * 2);
            instance->encoder.upload[index++] = level_duration_make(true, instance->te);
        }
    }
    return true;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_faac_tm_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderFaacTm* instance = context;
    SubGhzProtocolStatus res = SubGhzProtocolStatusError;
    do {
        res = subghz_block_generic_deserialize_check_count_bit(
            &instance->generic,
            flipper_format,
            subghz_protocol_faac_tm_const.min_count_bit_for_found);
        if(res != SubGhzProtocolStatusOk) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        // TE is optional, a file without a usable one is sent at the nominal rate
        uint32_t te = 0;
        instance->te = subghz_protocol_faac_tm_const.te_short;
        if(flipper_format_rewind(flipper_format) &&
           flipper_format_read_uint32(flipper_format, "TE", &te, 1) && (te > FAAC_TM_TE_MIN) &&
           (te < FAAC_TM_TE_MAX)) {
            instance->te = te;
        }
        // Optional value
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!subghz_protocol_encoder_faac_tm_get_upload(instance)) {
            res = SubGhzProtocolStatusErrorEncoderGetUpload;
            break;
        }
        instance->encoder.is_running = true;
    } while(false);

    return res;
}

void* subghz_protocol_decoder_faac_tm_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    return subghz_protocol_decoder_common_alloc(
        sizeof(SubGhzProtocolDecoderFaacTm), &subghz_protocol_faac_tm);
}

void subghz_protocol_decoder_faac_tm_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderFaacTm* instance = context;

    switch(instance->decoder.parser_step) {
    case FaacTmDecoderStepReset:
        if((!level) &&
           (DURATION_DIFF(duration, subghz_protocol_faac_tm_const.te_short * FAAC_TM_HEADER_TE) <
            subghz_protocol_faac_tm_const.te_delta * FAAC_TM_HEADER_TE)) {
            //Found header
            instance->decoder.parser_step = FaacTmDecoderStepFoundStartBit;
        }
        break;
    case FaacTmDecoderStepFoundStartBit:
        if(!level) {
            break;
        } else if(
            DURATION_DIFF(duration, subghz_protocol_faac_tm_const.te_short) <
            subghz_protocol_faac_tm_const.te_delta) {
            //Found start bit
            instance->decoder.parser_step = FaacTmDecoderStepSaveDuration;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->te = duration;
        } else {
            instance->decoder.parser_step = FaacTmDecoderStepReset;
        }
        break;
    case FaacTmDecoderStepSaveDuration:
        if(!level) { //save interval
            if(duration >= (subghz_protocol_faac_tm_const.te_short * 4)) {
                instance->decoder.parser_step = FaacTmDecoderStepFoundStartBit;
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_faac_tm_const.min_count_bit_for_found) {
                    // start bit plus 3 Te per bit
                    instance->te /= (instance->decoder.decode_count_bit * 3 + 1);

                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                break;
            }
            instance->decoder.te_last = duration;
            instance->te += duration;
            instance->decoder.parser_step = FaacTmDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = FaacTmDecoderStepReset;
        }
        break;
    case FaacTmDecoderStepCheckDuration:
        if(level) {
            instance->te += duration;
            if((DURATION_DIFF(instance->decoder.te_last, subghz_protocol_faac_tm_const.te_short) <
                subghz_protocol_faac_tm_const.te_delta) &&
               (DURATION_DIFF(duration, subghz_protocol_faac_tm_const.te_long) <
                subghz_protocol_faac_tm_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = FaacTmDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_faac_tm_const.te_long) <
                 subghz_protocol_faac_tm_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_faac_tm_const.te_short) <
                 subghz_protocol_faac_tm_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = FaacTmDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = FaacTmDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = FaacTmDecoderStepReset;
        }
        break;
    }
}

SubGhzProtocolStatus
    subghz_protocol_decoder_faac_tm_deserialize(void* context, FlipperFormat* flipper_format) {
    return subghz_protocol_decoder_common_deserialize_te(
        context, flipper_format, subghz_protocol_faac_tm_const.min_count_bit_for_found);
}

void subghz_protocol_decoder_faac_tm_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderFaacTm* instance = context;
    uint32_t data = (uint32_t)(instance->generic.data & 0xFFF);

    furi_string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%03lX    Te:%luus\r\n"
        // dips in the order they are sent
        "DIP:" DIP_PATTERN "\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        data,
        instance->te,
        DATA_TO_DIP(data));
}
