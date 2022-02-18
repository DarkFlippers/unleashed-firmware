#include "subghz_protocol_nero_sketch.h"

struct SubGhzProtocolNeroSketch {
    SubGhzProtocolCommon common;
};

typedef enum {
    NeroSketchDecoderStepReset = 0,
    NeroSketchDecoderStepCheckPreambula,
    NeroSketchDecoderStepSaveDuration,
    NeroSketchDecoderStepCheckDuration,
} NeroSketchDecoderStep;

SubGhzProtocolNeroSketch* subghz_protocol_nero_sketch_alloc(void) {
    SubGhzProtocolNeroSketch* instance = malloc(sizeof(SubGhzProtocolNeroSketch));

    instance->common.name = "Nero Sketch";
    instance->common.code_min_count_bit_for_found = 40;
    instance->common.te_short = 330;
    instance->common.te_long = 660;
    instance->common.te_delta = 150;
    instance->common.type_protocol = SubGhzProtocolCommonTypeStatic;
    instance->common.to_string = (SubGhzProtocolCommonToStr)subghz_protocol_nero_sketch_to_str;
    instance->common.to_save_file =
        (SubGhzProtocolCommonSaveFile)subghz_protocol_nero_sketch_to_save_file;
    instance->common.to_load_protocol_from_file =
        (SubGhzProtocolCommonLoadFromFile)subghz_protocol_nero_sketch_to_load_protocol_from_file;
    instance->common.to_load_protocol =
        (SubGhzProtocolCommonLoadFromRAW)subghz_decoder_nero_sketch_to_load_protocol;
    instance->common.get_upload_protocol =
        (SubGhzProtocolCommonEncoderGetUpLoad)subghz_protocol_nero_sketch_send_key;

    return instance;
}

void subghz_protocol_nero_sketch_free(SubGhzProtocolNeroSketch* instance) {
    furi_assert(instance);
    free(instance);
}

bool subghz_protocol_nero_sketch_send_key(
    SubGhzProtocolNeroSketch* instance,
    SubGhzProtocolCommonEncoder* encoder) {
    furi_assert(instance);
    furi_assert(encoder);
    size_t index = 0;
    encoder->size_upload = 47 * 2 + 2 + (instance->common.code_last_count_bit * 2) + 2;
    if(encoder->size_upload > SUBGHZ_ENCODER_UPLOAD_MAX_SIZE) return false;

    //Send header
    for(uint8_t i = 0; i < 47; i++) {
        encoder->upload[index++] = level_duration_make(true, (uint32_t)instance->common.te_short);
        encoder->upload[index++] = level_duration_make(false, (uint32_t)instance->common.te_short);
    }

    //Send start bit
    encoder->upload[index++] = level_duration_make(true, (uint32_t)instance->common.te_short * 4);
    encoder->upload[index++] = level_duration_make(false, (uint32_t)instance->common.te_short);

    //Send key data
    for(uint8_t i = instance->common.code_last_count_bit; i > 0; i--) {
        if(bit_read(instance->common.code_last_found, i - 1)) {
            //send bit 1
            encoder->upload[index++] =
                level_duration_make(true, (uint32_t)instance->common.te_long);
            encoder->upload[index++] =
                level_duration_make(false, (uint32_t)instance->common.te_short);
        } else {
            //send bit 0
            encoder->upload[index++] =
                level_duration_make(true, (uint32_t)instance->common.te_short);
            encoder->upload[index++] =
                level_duration_make(false, (uint32_t)instance->common.te_long);
        }
    }

    //Send stop bit
    encoder->upload[index++] = level_duration_make(true, (uint32_t)instance->common.te_short * 3);
    encoder->upload[index++] = level_duration_make(false, (uint32_t)instance->common.te_short);

    return true;
}

void subghz_protocol_nero_sketch_reset(SubGhzProtocolNeroSketch* instance) {
    instance->common.parser_step = NeroSketchDecoderStepReset;
}

void subghz_protocol_nero_sketch_parse(
    SubGhzProtocolNeroSketch* instance,
    bool level,
    uint32_t duration) {
    switch(instance->common.parser_step) {
    case NeroSketchDecoderStepReset:
        if((level) &&
           (DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta)) {
            instance->common.parser_step = NeroSketchDecoderStepCheckPreambula;
            instance->common.te_last = duration;
            instance->common.header_count = 0;
        }
        break;
    case NeroSketchDecoderStepCheckPreambula:
        if(level) {
            if((DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta) ||
               (DURATION_DIFF(duration, instance->common.te_short * 4) <
                instance->common.te_delta)) {
                instance->common.te_last = duration;
            } else {
                instance->common.parser_step = NeroSketchDecoderStepReset;
            }
        } else if(DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta) {
            if(DURATION_DIFF(instance->common.te_last, instance->common.te_short) <
               instance->common.te_delta) {
                // Found header
                instance->common.header_count++;
                break;
            } else if(
                DURATION_DIFF(instance->common.te_last, instance->common.te_short * 4) <
                instance->common.te_delta) {
                // Found start bit
                if(instance->common.header_count > 40) {
                    instance->common.parser_step = NeroSketchDecoderStepSaveDuration;
                    instance->common.code_found = 0;
                    instance->common.code_count_bit = 0;
                } else {
                    instance->common.parser_step = NeroSketchDecoderStepReset;
                }
            } else {
                instance->common.parser_step = NeroSketchDecoderStepReset;
            }
        } else {
            instance->common.parser_step = NeroSketchDecoderStepReset;
        }
        break;
    case NeroSketchDecoderStepSaveDuration:
        if(level) {
            if(duration >= (instance->common.te_short * 2 + instance->common.te_delta * 2)) {
                //Found stop bit
                instance->common.parser_step = NeroSketchDecoderStepReset;
                if(instance->common.code_count_bit >=
                   instance->common.code_min_count_bit_for_found) {
                    instance->common.code_last_found = instance->common.code_found;
                    instance->common.code_last_count_bit = instance->common.code_count_bit;
                    if(instance->common.callback)
                        instance->common.callback(
                            (SubGhzProtocolCommon*)instance, instance->common.context);
                }
                instance->common.code_found = 0;
                instance->common.code_count_bit = 0;
                break;
            } else {
                instance->common.te_last = duration;
                instance->common.parser_step = NeroSketchDecoderStepCheckDuration;
            }

        } else {
            instance->common.parser_step = NeroSketchDecoderStepReset;
        }
        break;
    case NeroSketchDecoderStepCheckDuration:
        if(!level) {
            if((DURATION_DIFF(instance->common.te_last, instance->common.te_short) <
                instance->common.te_delta) &&
               (DURATION_DIFF(duration, instance->common.te_long) < instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 0);
                instance->common.parser_step = NeroSketchDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->common.te_last, instance->common.te_long) <
                 instance->common.te_delta) &&
                (DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 1);
                instance->common.parser_step = NeroSketchDecoderStepSaveDuration;
            } else {
                instance->common.parser_step = NeroSketchDecoderStepReset;
            }
        } else {
            instance->common.parser_step = NeroSketchDecoderStepReset;
        }
        break;
    }
}

void subghz_protocol_nero_sketch_to_str(SubGhzProtocolNeroSketch* instance, string_t output) {
    uint32_t code_found_hi = instance->common.code_last_found >> 32;
    uint32_t code_found_lo = instance->common.code_last_found & 0x00000000ffffffff;

    uint64_t code_found_reverse = subghz_protocol_common_reverse_key(
        instance->common.code_last_found, instance->common.code_last_count_bit);

    uint32_t code_found_reverse_hi = code_found_reverse >> 32;
    uint32_t code_found_reverse_lo = code_found_reverse & 0x00000000ffffffff;

    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%lX%08lX\r\n"
        "Yek:0x%lX%08lX\r\n",
        instance->common.name,
        instance->common.code_last_count_bit,
        code_found_hi,
        code_found_lo,
        code_found_reverse_hi,
        code_found_reverse_lo);
}

bool subghz_protocol_nero_sketch_to_save_file(
    SubGhzProtocolNeroSketch* instance,
    FlipperFormat* flipper_format) {
    return subghz_protocol_common_to_save_file((SubGhzProtocolCommon*)instance, flipper_format);
}

bool subghz_protocol_nero_sketch_to_load_protocol_from_file(
    FlipperFormat* flipper_format,
    SubGhzProtocolNeroSketch* instance,
    const char* file_path) {
    return subghz_protocol_common_to_load_protocol_from_file(
        (SubGhzProtocolCommon*)instance, flipper_format);
}

void subghz_decoder_nero_sketch_to_load_protocol(SubGhzProtocolNeroSketch* instance, void* context) {
    furi_assert(context);
    furi_assert(instance);
    SubGhzProtocolCommonLoad* data = context;
    instance->common.code_last_found = data->code_found;
    instance->common.code_last_count_bit = data->code_count_bit;
}
