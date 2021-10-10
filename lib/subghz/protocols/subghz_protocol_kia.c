#include "subghz_protocol_kia.h"

struct SubGhzProtocolKIA {
    SubGhzProtocolCommon common;
};

typedef enum {
    KIADecoderStepReset = 0,
    KIADecoderStepCheckPreambula,
    KIADecoderStepSaveDuration,
    KIADecoderStepCheckDuration,
} KIADecoderStep;

SubGhzProtocolKIA* subghz_protocol_kia_alloc(void) {
    SubGhzProtocolKIA* instance = furi_alloc(sizeof(SubGhzProtocolKIA));

    instance->common.name = "KIA";
    instance->common.code_min_count_bit_for_found = 60;
    instance->common.te_short = 250;
    instance->common.te_long = 500;
    instance->common.te_delta = 100;
    instance->common.type_protocol = SubGhzProtocolCommonTypeDynamic;
    instance->common.to_string = (SubGhzProtocolCommonToStr)subghz_protocol_kia_to_str;
    instance->common.to_load_protocol =
        (SubGhzProtocolCommonLoadFromRAW)subghz_decoder_kia_to_load_protocol;

    return instance;
}

void subghz_protocol_kia_free(SubGhzProtocolKIA* instance) {
    furi_assert(instance);
    free(instance);
}

void subghz_protocol_kia_reset(SubGhzProtocolKIA* instance) {
    instance->common.parser_step = KIADecoderStepReset;
}

uint8_t subghz_protocol_kia_crc8(uint8_t* data, size_t len) {
    uint8_t crc = 0x08;
    size_t i, j;
    for(i = 0; i < len; i++) {
        crc ^= data[i];
        for(j = 0; j < 8; j++) {
            if((crc & 0x80) != 0)
                crc = (uint8_t)((crc << 1) ^ 0x7F);
            else
                crc <<= 1;
        }
    }
    return crc;
}

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolKIA instance
 */
void subghz_protocol_kia_check_remote_controller(SubGhzProtocolKIA* instance) {
    /*
    *   0x0F 0112 43B04EC 1 7D
    *   0x0F 0113 43B04EC 1 DF
    *   0x0F 0114 43B04EC 1 30
    *   0x0F 0115 43B04EC 2 13
    *   0x0F 0116 43B04EC 3 F5
    *         CNT  Serial K CRC8 Kia (CRC8, poly 0x7f, start_crc 0x08)
    */

    instance->common.serial = (uint32_t)((instance->common.code_last_found >> 12) & 0x0FFFFFFF);
    instance->common.btn = (instance->common.code_last_found >> 8) & 0x0F;
    instance->common.cnt = (instance->common.code_last_found >> 40) & 0xFFFF;
}

void subghz_protocol_kia_parse(SubGhzProtocolKIA* instance, bool level, uint32_t duration) {
    switch(instance->common.parser_step) {
    case KIADecoderStepReset:
        if((!level) &&
           (DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta)) {
            instance->common.parser_step = KIADecoderStepCheckPreambula;
            instance->common.te_last = duration;
            instance->common.header_count = 0;
        } else {
            instance->common.parser_step = KIADecoderStepReset;
        }
        break;
    case KIADecoderStepCheckPreambula:
        if(!level) {
            if((DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta) ||
               (DURATION_DIFF(duration, instance->common.te_long) < instance->common.te_delta)) {
                instance->common.te_last = duration;
            } else {
                instance->common.parser_step = KIADecoderStepReset;
            }
        } else if(
            (DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta) &&
            (DURATION_DIFF(instance->common.te_last, instance->common.te_short) <
             instance->common.te_delta)) {
            // Found header
            instance->common.header_count++;
            break;
        } else if(
            (DURATION_DIFF(duration, instance->common.te_long) < instance->common.te_delta) &&
            (DURATION_DIFF(instance->common.te_last, instance->common.te_long) <
             instance->common.te_delta)) {
            // Found start bit
            if(instance->common.header_count > 15) {
                instance->common.parser_step = KIADecoderStepSaveDuration;
                instance->common.code_found = 0;
                instance->common.code_count_bit = 1;
                subghz_protocol_common_add_bit(&instance->common, 1);
            } else {
                instance->common.parser_step = KIADecoderStepReset;
            }
        } else {
            instance->common.parser_step = KIADecoderStepReset;
        }
        break;
    case KIADecoderStepSaveDuration:
        if(!level) {
            if(duration >= (instance->common.te_long + instance->common.te_delta * 2)) {
                //Found stop bit
                instance->common.parser_step = KIADecoderStepReset;
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
                instance->common.parser_step = KIADecoderStepCheckDuration;
            }

        } else {
            instance->common.parser_step = KIADecoderStepReset;
        }
        break;
    case KIADecoderStepCheckDuration:
        if(level) {
            if((DURATION_DIFF(instance->common.te_last, instance->common.te_short) <
                instance->common.te_delta) &&
               (DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 0);
                instance->common.parser_step = KIADecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->common.te_last, instance->common.te_long) <
                 instance->common.te_delta) &&
                (DURATION_DIFF(duration, instance->common.te_long) < instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 1);
                instance->common.parser_step = KIADecoderStepSaveDuration;
            } else {
                instance->common.parser_step = KIADecoderStepReset;
            }
        } else {
            instance->common.parser_step = KIADecoderStepReset;
        }
        break;
    }
}

void subghz_protocol_kia_to_str(SubGhzProtocolKIA* instance, string_t output) {
    uint32_t code_found_hi = instance->common.code_last_found >> 32;
    uint32_t code_found_lo = instance->common.code_last_found & 0x00000000ffffffff;

    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:%08lX%08lX\r\n"
        "Sn:%07lX Btn:%lX Cnt:%04X\r\n",
        instance->common.name,
        instance->common.code_last_count_bit,
        code_found_hi,
        code_found_lo,
        instance->common.serial,
        instance->common.btn,
        instance->common.cnt);
}

void subghz_decoder_kia_to_load_protocol(SubGhzProtocolKIA* instance, void* context) {
    furi_assert(context);
    furi_assert(instance);
    SubGhzProtocolCommonLoad* data = context;
    instance->common.code_last_found = data->code_found;
    instance->common.code_last_count_bit = data->code_count_bit;
    subghz_protocol_kia_check_remote_controller(instance);
}