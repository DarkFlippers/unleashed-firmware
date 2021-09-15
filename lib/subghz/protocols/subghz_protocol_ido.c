#include "subghz_protocol_ido.h"

struct SubGhzProtocolIDo {
    SubGhzProtocolCommon common;
};

typedef enum {
    IDoDecoderStepReset = 0,
    IDoDecoderStepFoundPreambula,
    IDoDecoderStepSaveDuration,
    IDoDecoderStepCheckDuration,
} IDoDecoderStep;

SubGhzProtocolIDo* subghz_protocol_ido_alloc(void) {
    SubGhzProtocolIDo* instance = furi_alloc(sizeof(SubGhzProtocolIDo));

    instance->common.name = "iDo 117/111"; // PT4301-X";
    instance->common.code_min_count_bit_for_found = 48;
    instance->common.te_short = 450;
    instance->common.te_long = 1450;
    instance->common.te_delta = 150;
    instance->common.type_protocol = SubGhzProtocolCommonTypeDynamic;
    instance->common.to_string = (SubGhzProtocolCommonToStr)subghz_protocol_ido_to_str;
    instance->common.to_load_protocol =
        (SubGhzProtocolCommonLoadFromRAW)subghz_decoder_ido_to_load_protocol;

    return instance;
}

void subghz_protocol_ido_free(SubGhzProtocolIDo* instance) {
    furi_assert(instance);
    free(instance);
}

/** Send bit 
 * 
 * @param instance - SubGhzProtocolIDo instance
 * @param bit - bit
 */
void subghz_protocol_ido_send_bit(SubGhzProtocolIDo* instance, uint8_t bit) {
    if(bit) {
        //send bit 1
        SUBGHZ_TX_PIN_HIGH();
        delay_us(instance->common.te_short);
        SUBGHZ_TX_PIN_LOW();
        delay_us(instance->common.te_short);
    } else {
        //send bit 0
        SUBGHZ_TX_PIN_HIGH();
        delay_us(instance->common.te_short);
        SUBGHZ_TX_PIN_LOW();
        delay_us(instance->common.te_long);
    }
}

void subghz_protocol_ido_send_key(
    SubGhzProtocolIDo* instance,
    uint64_t key,
    uint8_t bit,
    uint8_t repeat) {
    while(repeat--) {
        SUBGHZ_TX_PIN_HIGH();
        //Send header
        delay_us(instance->common.te_short * 10);
        SUBGHZ_TX_PIN_LOW();
        delay_us(instance->common.te_short * 10);
        //Send key data
        for(uint8_t i = bit; i > 0; i--) {
            subghz_protocol_ido_send_bit(instance, bit_read(key, i - 1));
        }
    }
}

void subghz_protocol_ido_reset(SubGhzProtocolIDo* instance) {
    instance->common.parser_step = IDoDecoderStepReset;
}

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolIDo instance
 */
void subghz_protocol_ido_check_remote_controller(SubGhzProtocolIDo* instance) {
    uint64_t code_found_reverse = subghz_protocol_common_reverse_key(
        instance->common.code_last_found, instance->common.code_last_count_bit);
    uint32_t code_fix = code_found_reverse & 0xFFFFFF;

    instance->common.serial = code_fix & 0xFFFFF;
    instance->common.btn = (code_fix >> 20) & 0x0F;
}

void subghz_protocol_ido_parse(SubGhzProtocolIDo* instance, bool level, uint32_t duration) {
    switch(instance->common.parser_step) {
    case IDoDecoderStepReset:
        if((level) && (DURATION_DIFF(duration, instance->common.te_short * 10) <
                       instance->common.te_delta * 5)) {
            instance->common.parser_step = IDoDecoderStepFoundPreambula;
        } else {
            instance->common.parser_step = IDoDecoderStepReset;
        }
        break;
    case IDoDecoderStepFoundPreambula:
        if((!level) && (DURATION_DIFF(duration, instance->common.te_short * 10) <
                        instance->common.te_delta * 5)) {
            //Found Preambula
            instance->common.parser_step = IDoDecoderStepSaveDuration;
            instance->common.code_found = 0;
            instance->common.code_count_bit = 0;
        } else {
            instance->common.parser_step = IDoDecoderStepReset;
        }
        break;
    case IDoDecoderStepSaveDuration:
        if(level) {
            if(duration >= (instance->common.te_short * 5 + instance->common.te_delta)) {
                instance->common.parser_step = IDoDecoderStepFoundPreambula;
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
                instance->common.parser_step = IDoDecoderStepCheckDuration;
            }

        } else {
            instance->common.parser_step = IDoDecoderStepReset;
        }
        break;
    case IDoDecoderStepCheckDuration:
        if(!level) {
            if((DURATION_DIFF(instance->common.te_last, instance->common.te_short) <
                instance->common.te_delta) &&
               (DURATION_DIFF(duration, instance->common.te_long) <
                instance->common.te_delta * 3)) {
                subghz_protocol_common_add_bit(&instance->common, 0);
                instance->common.parser_step = IDoDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->common.te_last, instance->common.te_short) <
                 instance->common.te_delta * 3) &&
                (DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 1);
                instance->common.parser_step = IDoDecoderStepSaveDuration;
            } else {
                instance->common.parser_step = IDoDecoderStepReset;
            }
        } else {
            instance->common.parser_step = IDoDecoderStepReset;
        }
        break;
    }
}

void subghz_protocol_ido_to_str(SubGhzProtocolIDo* instance, string_t output) {
    subghz_protocol_ido_check_remote_controller(instance);
    uint64_t code_found_reverse = subghz_protocol_common_reverse_key(
        instance->common.code_last_found, instance->common.code_last_count_bit);
    uint32_t code_fix = code_found_reverse & 0xFFFFFF;
    uint32_t code_hop = (code_found_reverse >> 24) & 0xFFFFFF;

    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%lX%08lX\r\n"
        "Fix:%06lX \r\n"
        "Hop:%06lX \r\n"
        "Sn:%05lX Btn:%lX\r\n",
        instance->common.name,
        instance->common.code_last_count_bit,
        (uint32_t)(instance->common.code_last_found >> 32),
        (uint32_t)instance->common.code_last_found,
        code_fix,
        code_hop,
        instance->common.serial,
        instance->common.btn);
}

void subghz_decoder_ido_to_load_protocol(SubGhzProtocolIDo* instance, void* context) {
    furi_assert(context);
    furi_assert(instance);
    SubGhzProtocolCommonLoad* data = context;
    instance->common.code_last_found = data->code_found;
    instance->common.code_last_count_bit = data->code_count_bit;
    subghz_protocol_ido_check_remote_controller(instance);
}