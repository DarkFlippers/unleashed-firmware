#include "subghz_protocol_scher_khan.h"

//https://phreakerclub.com/72
//https://phreakerclub.com/forum/showthread.php?t=7&page=2
//https://phreakerclub.com/forum/showthread.php?t=274&highlight=magicar
//!!!  https://phreakerclub.com/forum/showthread.php?t=489&highlight=magicar&page=5

struct SubGhzProtocolScherKhan {
    SubGhzProtocolCommon common;
    const char* protocol_name;
};

typedef enum {
    ScherKhanDecoderStepReset = 0,
    ScherKhanDecoderStepCheckPreambula,
    ScherKhanDecoderStepSaveDuration,
    ScherKhanDecoderStepCheckDuration,
} ScherKhanDecoderStep;

SubGhzProtocolScherKhan* subghz_protocol_scher_khan_alloc(void) {
    SubGhzProtocolScherKhan* instance = furi_alloc(sizeof(SubGhzProtocolScherKhan));

    instance->common.name = "Scher-Khan";
    instance->common.code_min_count_bit_for_found = 35;
    instance->common.te_short = 750;
    instance->common.te_long = 1100;
    instance->common.te_delta = 150;
    instance->common.type_protocol = SubGhzProtocolCommonTypeDynamic;
    instance->common.to_string = (SubGhzProtocolCommonToStr)subghz_protocol_scher_khan_to_str;
    instance->common.to_load_protocol =
        (SubGhzProtocolCommonLoadFromRAW)subghz_decoder_scher_khan_to_load_protocol;

    return instance;
}

void subghz_protocol_scher_khan_free(SubGhzProtocolScherKhan* instance) {
    furi_assert(instance);
    free(instance);
}

/** Send bit 
 * 
 * @param instance - SubGhzProtocolScherKhan instance
 * @param bit - bit
 */
// void subghz_protocol_scher_khan_send_bit(SubGhzProtocolScherKhan* instance, uint8_t bit) {
//     if(bit) {
//         //send bit 1
//         SUBGHZ_TX_PIN_HIGH();
//         delay_us(instance->common.te_long);
//         SUBGHZ_TX_PIN_LOW();
//         delay_us(instance->common.te_short);
//     } else {
//         //send bit 0
//         SUBGHZ_TX_PIN_HIGH();
//         delay_us(instance->common.te_short);
//         SUBGHZ_TX_PIN_LOW();
//         delay_us(instance->common.te_long);
//     }
// }

// void subghz_protocol_scher_khan_send_key(
//     SubGhzProtocolScherKhan* instance,
//     uint64_t key,
//     uint8_t bit,
//     uint8_t repeat) {
//     while(repeat--) {
//         SUBGHZ_TX_PIN_HIGH();
//         //Send header
//         delay_us(instance->common.te_long * 2);
//         SUBGHZ_TX_PIN_LOW();
//         delay_us(instance->common.te_long * 2);
//         //Send key data
//         for(uint8_t i = bit; i > 0; i--) {
//             subghz_protocol_scher_khan_send_bit(instance, bit_read(key, i - 1));
//         }
//     }
// }

void subghz_protocol_scher_khan_reset(SubGhzProtocolScherKhan* instance) {
    instance->common.parser_step = ScherKhanDecoderStepReset;
}

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolScherKhan instance
 */
void subghz_protocol_scher_khan_check_remote_controller(SubGhzProtocolScherKhan* instance) {
    /* 
    * MAGICAR 51 bit 00000001A99121DE83C3 MAGIC CODE, Dinamic
    * 0E8C1619E830C -> 000011101000110000010110 0001 1001 1110 1000001100001100
    * 0E8C1629D830D -> 000011101000110000010110 0010 1001 1101 1000001100001101
    * 0E8C1649B830E -> 000011101000110000010110 0100 1001 1011 1000001100001110
    * 0E8C16897830F -> 000011101000110000010110 1000 1001 0111 1000001100001111
    *                             Serial         Key  Ser ~Key   CNT
    */

    switch(instance->common.code_last_count_bit) {
    // case 35: //MAGIC CODE, Static
    //     instance->protocol_name = "MAGIC CODE, Static";
    //     break;
    case 51: //MAGIC CODE, Dinamic
        instance->protocol_name = "MAGIC CODE, Dinamic";
        instance->common.serial = ((instance->common.code_last_found >> 24) & 0xFFFFFF0) |
                                  ((instance->common.code_last_found >> 20) & 0x0F);
        instance->common.btn = (instance->common.code_last_found >> 24) & 0x0F;
        instance->common.cnt = instance->common.code_last_found & 0xFFFF;
        break;
        // case 57: //MAGIC CODE PRO / PRO2
        //     instance->protocol_name = "MAGIC CODE PRO / PRO2";
        //     break;

    default:
        instance->protocol_name = "Unknown";
        instance->common.serial = 0;
        instance->common.btn = 0;
        instance->common.cnt = 0;
        break;
    }
}

void subghz_protocol_scher_khan_parse(
    SubGhzProtocolScherKhan* instance,
    bool level,
    uint32_t duration) {
    switch(instance->common.parser_step) {
    case ScherKhanDecoderStepReset:
        if((level) &&
           (DURATION_DIFF(duration, instance->common.te_short * 2) < instance->common.te_delta)) {
            instance->common.parser_step = ScherKhanDecoderStepCheckPreambula;
            instance->common.te_last = duration;
            instance->common.header_count = 0;
        } else {
            instance->common.parser_step = ScherKhanDecoderStepReset;
        }
        break;
    case ScherKhanDecoderStepCheckPreambula:
        if(level) {
            if((DURATION_DIFF(duration, instance->common.te_short * 2) <
                instance->common.te_delta) ||
               (DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta)) {
                instance->common.te_last = duration;
            } else {
                instance->common.parser_step = ScherKhanDecoderStepReset;
            }
        } else if(
            (DURATION_DIFF(duration, instance->common.te_short * 2) < instance->common.te_delta) ||
            (DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta)) {
            if(DURATION_DIFF(instance->common.te_last, instance->common.te_short * 2) <
               instance->common.te_delta) {
                // Found header
                instance->common.header_count++;
                break;
            } else if(
                DURATION_DIFF(instance->common.te_last, instance->common.te_short) <
                instance->common.te_delta) {
                // Found start bit
                if(instance->common.header_count >= 2) {
                    instance->common.parser_step = ScherKhanDecoderStepSaveDuration;
                    instance->common.code_found = 0;
                    instance->common.code_count_bit = 1;
                } else {
                    instance->common.parser_step = ScherKhanDecoderStepReset;
                }
            } else {
                instance->common.parser_step = ScherKhanDecoderStepReset;
            }
        } else {
            instance->common.parser_step = ScherKhanDecoderStepReset;
        }
        break;
    case ScherKhanDecoderStepSaveDuration:
        if(level) {
            if(duration >= (instance->common.te_long + instance->common.te_delta * 2)) {
                //Found stop bit
                instance->common.parser_step = ScherKhanDecoderStepReset;
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
                instance->common.parser_step = ScherKhanDecoderStepCheckDuration;
            }

        } else {
            instance->common.parser_step = ScherKhanDecoderStepReset;
        }
        break;
    case ScherKhanDecoderStepCheckDuration:
        if(!level) {
            if((DURATION_DIFF(instance->common.te_last, instance->common.te_short) <
                instance->common.te_delta) &&
               (DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 0);
                instance->common.parser_step = ScherKhanDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(instance->common.te_last, instance->common.te_long) <
                 instance->common.te_delta) &&
                (DURATION_DIFF(duration, instance->common.te_long) < instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 1);
                instance->common.parser_step = ScherKhanDecoderStepSaveDuration;
            } else {
                instance->common.parser_step = ScherKhanDecoderStepReset;
            }
        } else {
            instance->common.parser_step = ScherKhanDecoderStepReset;
        }
        break;
    }
}

void subghz_protocol_scher_khan_to_str(SubGhzProtocolScherKhan* instance, string_t output) {
    subghz_protocol_scher_khan_check_remote_controller(instance);

    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%lX%08lX\r\n"
        "Sn:%07lX Btn:%lX Cnt:%04X\r\n"
        "Pt: %s\r\n",
        instance->common.name,
        instance->common.code_last_count_bit,
        (uint32_t)(instance->common.code_last_found >> 32),
        (uint32_t)instance->common.code_last_found,
        instance->common.serial,
        instance->common.btn,
        instance->common.cnt,
        instance->protocol_name);
}

void subghz_decoder_scher_khan_to_load_protocol(SubGhzProtocolScherKhan* instance, void* context) {
    furi_assert(context);
    furi_assert(instance);
    SubGhzProtocolCommonLoad* data = context;
    instance->common.code_last_found = data->code_found;
    instance->common.code_last_count_bit = data->code_count_bit;
    subghz_protocol_scher_khan_check_remote_controller(instance);
}