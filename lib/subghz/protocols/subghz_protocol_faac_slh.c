#include "subghz_protocol_faac_slh.h"

struct SubGhzProtocolFaacSLH {
    SubGhzProtocolCommon common;
};

typedef enum {
    FaacSLHDecoderStepReset = 0,
    FaacSLHDecoderStepFoundPreambula,
    FaacSLHDecoderStepSaveDuration,
    FaacSLHDecoderStepCheckDuration,
} FaacSLHDecoderStep;

SubGhzProtocolFaacSLH* subghz_protocol_faac_slh_alloc(void) {
    SubGhzProtocolFaacSLH* instance = furi_alloc(sizeof(SubGhzProtocolFaacSLH));

    instance->common.name = "Faac SLH";
    instance->common.code_min_count_bit_for_found = 64;
    instance->common.te_short = 255;
    instance->common.te_long = 595;
    instance->common.te_delta = 100;
    instance->common.type_protocol = SubGhzProtocolCommonTypeDynamic;
    instance->common.to_string = (SubGhzProtocolCommonToStr)subghz_protocol_faac_slh_to_str;
    instance->common.to_load_protocol =
        (SubGhzProtocolCommonLoadFromRAW)subghz_decoder_faac_slh_to_load_protocol;

    return instance;
}

void subghz_protocol_faac_slh_free(SubGhzProtocolFaacSLH* instance) {
    furi_assert(instance);
    free(instance);
}

/** Send bit 
 * 
 * @param instance - SubGhzProtocolFaacSLH instance
 * @param bit - bit
 */
void subghz_protocol_faac_slh_send_bit(SubGhzProtocolFaacSLH* instance, uint8_t bit) {
    if(bit) {
        //send bit 1
        SUBGHZ_TX_PIN_HIGH();
        delay_us(instance->common.te_long);
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

void subghz_protocol_faac_slh_send_key(
    SubGhzProtocolFaacSLH* instance,
    uint64_t key,
    uint8_t bit,
    uint8_t repeat) {
    while(repeat--) {
        SUBGHZ_TX_PIN_HIGH();
        //Send header
        delay_us(instance->common.te_long * 2);
        SUBGHZ_TX_PIN_LOW();
        delay_us(instance->common.te_long * 2);
        //Send key data
        for(uint8_t i = bit; i > 0; i--) {
            subghz_protocol_faac_slh_send_bit(instance, bit_read(key, i - 1));
        }
    }
}

void subghz_protocol_faac_slh_reset(SubGhzProtocolFaacSLH* instance) {
    instance->common.parser_step = FaacSLHDecoderStepReset;
}

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolFaacSLH instance
 */
void subghz_protocol_faac_slh_check_remote_controller(SubGhzProtocolFaacSLH* instance) {
    uint64_t code_found_reverse = subghz_protocol_common_reverse_key(
        instance->common.code_last_found, instance->common.code_last_count_bit);
    uint32_t code_fix = code_found_reverse & 0xFFFFFFFF;
    //uint32_t code_hop = (code_found_reverse >> 24) & 0xFFFFF;

    instance->common.serial = code_fix & 0xFFFFFFF;
    instance->common.btn = (code_fix >> 28) & 0x0F;
}

void subghz_protocol_faac_slh_parse(SubGhzProtocolFaacSLH* instance, bool level, uint32_t duration) {
    switch(instance->common.parser_step) {
    case FaacSLHDecoderStepReset:
        if((level) && (DURATION_DIFF(duration, instance->common.te_long * 2) <
                       instance->common.te_delta * 3)) {
            instance->common.parser_step = FaacSLHDecoderStepFoundPreambula;
        }
        break;
    case FaacSLHDecoderStepFoundPreambula:
        if((!level) && (DURATION_DIFF(duration, instance->common.te_long * 2) <
                        instance->common.te_delta * 3)) {
            //Found Preambula
            instance->common.parser_step = FaacSLHDecoderStepSaveDuration;
            instance->common.code_found = 0;
            instance->common.code_count_bit = 0;
        } else {
            instance->common.parser_step = FaacSLHDecoderStepReset;
        }
        break;
    case FaacSLHDecoderStepSaveDuration:
        if(level) {
            if(duration >= (instance->common.te_short * 3 + instance->common.te_delta)) {
                instance->common.parser_step = FaacSLHDecoderStepFoundPreambula;
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
                instance->common.parser_step = FaacSLHDecoderStepCheckDuration;
            }

        } else {
            instance->common.parser_step = FaacSLHDecoderStepReset;
        }
        break;
    case FaacSLHDecoderStepCheckDuration:
        if(!level) {
            if((DURATION_DIFF(instance->common.te_last, instance->common.te_short) <
                instance->common.te_delta) &&
               (DURATION_DIFF(duration, instance->common.te_long) < instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 0);
                instance->common.parser_step = FaacSLHDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->common.te_last, instance->common.te_long) <
                 instance->common.te_delta) &&
                (DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 1);
                instance->common.parser_step = FaacSLHDecoderStepSaveDuration;
            } else {
                instance->common.parser_step = FaacSLHDecoderStepReset;
            }
        } else {
            instance->common.parser_step = FaacSLHDecoderStepReset;
        }
        break;
    }
}

void subghz_protocol_faac_slh_to_str(SubGhzProtocolFaacSLH* instance, string_t output) {
    subghz_protocol_faac_slh_check_remote_controller(instance);
    uint64_t code_found_reverse = subghz_protocol_common_reverse_key(
        instance->common.code_last_found, instance->common.code_last_count_bit);
    uint32_t code_fix = code_found_reverse & 0xFFFFFFFF;
    uint32_t code_hop = (code_found_reverse >> 32) & 0xFFFFFFFF;

    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:%lX%08lX\r\n"
        "Fix:%08lX \r\n"
        "Hop:%08lX \r\n"
        "Sn:%07lX Btn:%lX\r\n",
        instance->common.name,
        instance->common.code_last_count_bit,
        (uint32_t)(instance->common.code_last_found >> 32),
        (uint32_t)instance->common.code_last_found,
        code_fix,
        code_hop,
        instance->common.serial,
        instance->common.btn);
}

void subghz_decoder_faac_slh_to_load_protocol(SubGhzProtocolFaacSLH* instance, void* context) {
    furi_assert(context);
    furi_assert(instance);
    SubGhzProtocolCommonLoad* data = context;
    instance->common.code_last_found = data->code_found;
    instance->common.code_last_count_bit = data->code_count_bit;
    subghz_protocol_faac_slh_check_remote_controller(instance);
}