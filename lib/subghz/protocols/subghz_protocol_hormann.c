#include "subghz_protocol_hormann.h"
#include "subghz_protocol_common.h"

struct SubGhzProtocolHormann {
    SubGhzProtocolCommon common;
};

typedef enum {
    HormannDecoderStepReset = 0,
    HormannDecoderStepFoundStartHeader,
    HormannDecoderStepFoundHeader,
    HormannDecoderStepFoundStartBit,
    HormannDecoderStepSaveDuration,
    HormannDecoderStepCheckDuration,
} HormannDecoderStep;

SubGhzProtocolHormann* subghz_protocol_hormann_alloc() {
    SubGhzProtocolHormann* instance = malloc(sizeof(SubGhzProtocolHormann));

    instance->common.name = "Hormann HSM";
    instance->common.code_min_count_bit_for_found = 44;
    instance->common.te_short = 511;
    instance->common.te_long = 1022;
    instance->common.te_delta = 200;
    instance->common.type_protocol = SubGhzProtocolCommonTypeStatic;
    instance->common.to_string = (SubGhzProtocolCommonToStr)subghz_protocol_hormann_to_str;
    instance->common.to_save_file =
        (SubGhzProtocolCommonSaveFile)subghz_protocol_hormann_to_save_file;
    instance->common.to_load_protocol_from_file =
        (SubGhzProtocolCommonLoadFromFile)subghz_protocol_hormann_to_load_protocol_from_file;
    instance->common.to_load_protocol =
        (SubGhzProtocolCommonLoadFromRAW)subghz_decoder_hormann_to_load_protocol;
    instance->common.get_upload_protocol =
        (SubGhzProtocolCommonEncoderGetUpLoad)subghz_protocol_hormann_send_key;

    return instance;
}

void subghz_protocol_hormann_free(SubGhzProtocolHormann* instance) {
    furi_assert(instance);
    free(instance);
}

bool subghz_protocol_hormann_send_key(
    SubGhzProtocolHormann* instance,
    SubGhzProtocolCommonEncoder* encoder) {
    furi_assert(instance);
    furi_assert(encoder);

    size_t index = 0;
    encoder->size_upload = 3 + (instance->common.code_last_count_bit * 2 + 2) * 20 + 1;
    if(encoder->size_upload > SUBGHZ_ENCODER_UPLOAD_MAX_SIZE) return false;
    //Send header
    encoder->upload[index++] =
        level_duration_make(false, (uint32_t)instance->common.te_short * 64);
    encoder->upload[index++] = level_duration_make(true, (uint32_t)instance->common.te_short * 64);
    encoder->upload[index++] =
        level_duration_make(false, (uint32_t)instance->common.te_short * 64);
    encoder->repeat = 10;

    for(size_t repeat = 0; repeat < 20; repeat++) {
        //Send start bit
        encoder->upload[index++] =
            level_duration_make(true, (uint32_t)instance->common.te_short * 24);
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
    }
    encoder->upload[index++] = level_duration_make(true, (uint32_t)instance->common.te_short * 24);
    return true;
}

void subghz_protocol_hormann_reset(SubGhzProtocolHormann* instance) {
    instance->common.parser_step = HormannDecoderStepReset;
}

void subghz_protocol_hormann_parse(SubGhzProtocolHormann* instance, bool level, uint32_t duration) {
    switch(instance->common.parser_step) {
    case HormannDecoderStepReset:
        if((level) && (DURATION_DIFF(duration, instance->common.te_short * 64) <
                       instance->common.te_delta * 64)) {
            instance->common.parser_step = HormannDecoderStepFoundStartHeader;
        }
        break;
    case HormannDecoderStepFoundStartHeader:
        if((!level) && (DURATION_DIFF(duration, instance->common.te_short * 64) <
                        instance->common.te_delta * 64)) {
            instance->common.parser_step = HormannDecoderStepFoundHeader;
        } else {
            instance->common.parser_step = HormannDecoderStepReset;
        }
        break;
    case HormannDecoderStepFoundHeader:
        if((level) && (DURATION_DIFF(duration, instance->common.te_short * 24) <
                       instance->common.te_delta * 24)) {
            instance->common.parser_step = HormannDecoderStepFoundStartBit;
        } else {
            instance->common.parser_step = HormannDecoderStepReset;
        }
        break;
    case HormannDecoderStepFoundStartBit:
        if((!level) &&
           (DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta)) {
            instance->common.parser_step = HormannDecoderStepSaveDuration;
            instance->common.code_found = 0;
            instance->common.code_count_bit = 0;
        } else {
            instance->common.parser_step = HormannDecoderStepReset;
        }
        break;
    case HormannDecoderStepSaveDuration:
        if(level) { //save interval
            if(duration >= (instance->common.te_short * 5)) {
                instance->common.parser_step = HormannDecoderStepFoundStartBit;
                if(instance->common.code_count_bit >=
                   instance->common.code_min_count_bit_for_found) {
                    instance->common.serial = 0x0;
                    instance->common.btn = 0x0;

                    instance->common.code_last_found = instance->common.code_found;
                    instance->common.code_last_count_bit = instance->common.code_count_bit;

                    if(instance->common.callback)
                        instance->common.callback(
                            (SubGhzProtocolCommon*)instance, instance->common.context);
                }
                break;
            }
            instance->common.te_last = duration;
            instance->common.parser_step = HormannDecoderStepCheckDuration;
        } else {
            instance->common.parser_step = HormannDecoderStepReset;
        }
        break;
    case HormannDecoderStepCheckDuration:
        if(!level) {
            if((DURATION_DIFF(instance->common.te_last, instance->common.te_short) <
                instance->common.te_delta) &&
               (DURATION_DIFF(duration, instance->common.te_long) < instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 0);
                instance->common.parser_step = HormannDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->common.te_last, instance->common.te_long) <
                 instance->common.te_delta) &&
                (DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 1);
                instance->common.parser_step = HormannDecoderStepSaveDuration;
            } else
                instance->common.parser_step = HormannDecoderStepReset;
        } else {
            instance->common.parser_step = HormannDecoderStepReset;
        }
        break;
    }
}

void subghz_protocol_hormann_to_str(SubGhzProtocolHormann* instance, string_t output) {
    uint32_t code_found_hi = instance->common.code_last_found >> 32;
    uint32_t code_found_lo = instance->common.code_last_found & 0x00000000ffffffff;
    instance->common.btn = (instance->common.code_last_found >> 4) & 0xF;

    string_cat_printf(
        output,
        "%s\r\n"
        "%dbit\r\n"
        "Key:0x%03lX%08lX\r\n"
        "Btn:0x%01X",
        instance->common.name,
        instance->common.code_last_count_bit,
        code_found_hi,
        code_found_lo,
        instance->common.btn);
}

bool subghz_protocol_hormann_to_save_file(
    SubGhzProtocolHormann* instance,
    FlipperFormat* flipper_format) {
    return subghz_protocol_common_to_save_file((SubGhzProtocolCommon*)instance, flipper_format);
}

bool subghz_protocol_hormann_to_load_protocol_from_file(
    FlipperFormat* flipper_format,
    SubGhzProtocolHormann* instance,
    const char* file_path) {
    return subghz_protocol_common_to_load_protocol_from_file(
        (SubGhzProtocolCommon*)instance, flipper_format);
}

void subghz_decoder_hormann_to_load_protocol(SubGhzProtocolHormann* instance, void* context) {
    furi_assert(context);
    furi_assert(instance);
    SubGhzProtocolCommonLoad* data = context;
    instance->common.code_last_found = data->code_found;
    instance->common.code_last_count_bit = data->code_count_bit;
}
