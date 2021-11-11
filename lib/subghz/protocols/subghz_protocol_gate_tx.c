#include "subghz_protocol_gate_tx.h"

struct SubGhzProtocolGateTX {
    SubGhzProtocolCommon common;
};

typedef enum {
    GateTXDecoderStepReset = 0,
    GateTXDecoderStepFoundStartBit,
    GateTXDecoderStepSaveDuration,
    GateTXDecoderStepCheckDuration,
} GateTXDecoderStep;

SubGhzProtocolGateTX* subghz_protocol_gate_tx_alloc(void) {
    SubGhzProtocolGateTX* instance = furi_alloc(sizeof(SubGhzProtocolGateTX));

    instance->common.name = "GateTX";
    instance->common.code_min_count_bit_for_found = 24;
    instance->common.te_short = 350;
    instance->common.te_long = 700;
    instance->common.te_delta = 100;
    instance->common.type_protocol = SubGhzProtocolCommonTypeStatic;
    instance->common.to_string = (SubGhzProtocolCommonToStr)subghz_protocol_gate_tx_to_str;
    instance->common.to_save_file =
        (SubGhzProtocolCommonSaveFile)subghz_protocol_gate_tx_to_save_file;
    instance->common.to_load_protocol_from_file =
        (SubGhzProtocolCommonLoadFromFile)subghz_protocol_gate_tx_to_load_protocol_from_file;
    instance->common.to_load_protocol =
        (SubGhzProtocolCommonLoadFromRAW)subghz_decoder_gate_tx_to_load_protocol;
    instance->common.get_upload_protocol =
        (SubGhzProtocolCommonEncoderGetUpLoad)subghz_protocol_gate_tx_send_key;
    return instance;
}

void subghz_protocol_gate_tx_free(SubGhzProtocolGateTX* instance) {
    furi_assert(instance);
    free(instance);
}

bool subghz_protocol_gate_tx_send_key(
    SubGhzProtocolGateTX* instance,
    SubGhzProtocolCommonEncoder* encoder) {
    furi_assert(instance);
    furi_assert(encoder);
    size_t index = 0;
    encoder->size_upload = (instance->common.code_last_count_bit * 2) + 2;
    if(encoder->size_upload > SUBGHZ_ENCODER_UPLOAD_MAX_SIZE) return false;
    //Send header
    encoder->upload[index++] =
        level_duration_make(false, (uint32_t)instance->common.te_short * 49);
    //Send start bit
    encoder->upload[index++] = level_duration_make(true, (uint32_t)instance->common.te_long);
    //Send key data
    for(uint8_t i = instance->common.code_last_count_bit; i > 0; i--) {
        if(bit_read(instance->common.code_last_found, i - 1)) {
            //send bit 1
            encoder->upload[index++] =
                level_duration_make(false, (uint32_t)instance->common.te_long);
            encoder->upload[index++] =
                level_duration_make(true, (uint32_t)instance->common.te_short);
        } else {
            //send bit 0
            encoder->upload[index++] =
                level_duration_make(false, (uint32_t)instance->common.te_short);
            encoder->upload[index++] =
                level_duration_make(true, (uint32_t)instance->common.te_long);
        }
    }
    return true;
}

void subghz_protocol_gate_tx_reset(SubGhzProtocolGateTX* instance) {
    instance->common.parser_step = GateTXDecoderStepReset;
}

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolFaacSLH instance
 */
void subghz_protocol_gate_tx_check_remote_controller(SubGhzProtocolGateTX* instance) {
    uint32_t code_found_reverse = subghz_protocol_common_reverse_key(
        instance->common.code_last_found, instance->common.code_last_count_bit);

    instance->common.serial = (code_found_reverse & 0xFF) << 12 |
                              ((code_found_reverse >> 8) & 0xFF) << 4 |
                              ((code_found_reverse >> 20) & 0x0F);
    instance->common.btn = ((code_found_reverse >> 16) & 0x0F);
}

void subghz_protocol_gate_tx_parse(SubGhzProtocolGateTX* instance, bool level, uint32_t duration) {
    switch(instance->common.parser_step) {
    case GateTXDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, instance->common.te_short * 47) <
                        instance->common.te_delta * 47)) {
            //Found Preambula
            instance->common.parser_step = GateTXDecoderStepFoundStartBit;
        }
        break;
    case GateTXDecoderStepFoundStartBit:
        if(level &&
           ((DURATION_DIFF(duration, instance->common.te_long) < instance->common.te_delta * 3))) {
            //Found start bit
            instance->common.parser_step = GateTXDecoderStepSaveDuration;
            instance->common.code_found = 0;
            instance->common.code_count_bit = 0;
        } else {
            instance->common.parser_step = GateTXDecoderStepReset;
        }
        break;
    case GateTXDecoderStepSaveDuration:
        if(!level) {
            if(duration >= (instance->common.te_short * 10 + instance->common.te_delta)) {
                instance->common.parser_step = GateTXDecoderStepFoundStartBit;
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
                instance->common.parser_step = GateTXDecoderStepCheckDuration;
            }
        }
        break;
    case GateTXDecoderStepCheckDuration:
        if(level) {
            if((DURATION_DIFF(instance->common.te_last, instance->common.te_short) <
                instance->common.te_delta) &&
               (DURATION_DIFF(duration, instance->common.te_long) <
                instance->common.te_delta * 3)) {
                subghz_protocol_common_add_bit(&instance->common, 0);
                instance->common.parser_step = GateTXDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->common.te_last, instance->common.te_long) <
                 instance->common.te_delta * 3) &&
                (DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 1);
                instance->common.parser_step = GateTXDecoderStepSaveDuration;
            } else {
                instance->common.parser_step = GateTXDecoderStepReset;
            }
        } else {
            instance->common.parser_step = GateTXDecoderStepReset;
        }
        break;
    }
}

void subghz_protocol_gate_tx_to_str(SubGhzProtocolGateTX* instance, string_t output) {
    subghz_protocol_gate_tx_check_remote_controller(instance);
    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:%06lX\r\n"
        "Sn:%05lX  Btn:%lX\r\n",
        instance->common.name,
        instance->common.code_last_count_bit,
        (uint32_t)(instance->common.code_last_found & 0xFFFFFF),
        instance->common.serial,
        instance->common.btn);
}

bool subghz_protocol_gate_tx_to_save_file(
    SubGhzProtocolGateTX* instance,
    FlipperFile* flipper_file) {
    return subghz_protocol_common_to_save_file((SubGhzProtocolCommon*)instance, flipper_file);
}

bool subghz_protocol_gate_tx_to_load_protocol_from_file(
    FlipperFile* flipper_file,
    SubGhzProtocolGateTX* instance,
    const char* file_path) {
    if(subghz_protocol_common_to_load_protocol_from_file(
           (SubGhzProtocolCommon*)instance, flipper_file)) {
        subghz_protocol_gate_tx_check_remote_controller(instance);
        return true;
    }
    return false;
}

void subghz_decoder_gate_tx_to_load_protocol(SubGhzProtocolGateTX* instance, void* context) {
    furi_assert(context);
    furi_assert(instance);
    SubGhzProtocolCommonLoad* data = context;
    instance->common.code_last_found = data->code_found;
    instance->common.code_last_count_bit = data->code_count_bit;
    subghz_protocol_gate_tx_check_remote_controller(instance);
}
