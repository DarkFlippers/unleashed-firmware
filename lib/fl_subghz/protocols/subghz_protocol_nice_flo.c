#include "subghz_protocol_nice_flo.h"

/*
 * Help
 * https://phreakerclub.com/447
 *
 */

struct SubGhzProtocolNiceFlo {
    SubGhzProtocolCommon common;
};

SubGhzProtocolNiceFlo* subghz_protocol_nice_flo_alloc() {
    SubGhzProtocolNiceFlo* instance = furi_alloc(sizeof(SubGhzProtocolNiceFlo));

    instance->common.name = "Nice FLO";
    instance->common.code_min_count_bit_for_found = 12;
    instance->common.te_shot = 700;
    instance->common.te_long = 1400;
    instance->common.te_delta = 200;
    
    return instance;
}

void subghz_protocol_nice_flo_free(SubGhzProtocolNiceFlo* instance) {
    furi_assert(instance);
    free(instance);
}

void subghz_protocol_nice_flo_send_bit(SubGhzProtocolNiceFlo* instance, uint8_t bit) {
    if (bit) {
        //send bit 1
        SUBGHZ_TX_PIN_LOW();
        delay_us(instance->common.te_long);
        SUBGHZ_TX_PIN_HIGTH();
        delay_us(instance->common.te_shot);
    } else {
        //send bit 0
        SUBGHZ_TX_PIN_LOW();
        delay_us(instance->common.te_shot);
        SUBGHZ_TX_PIN_HIGTH();
        delay_us(instance->common.te_long);
    }
}

void subghz_protocol_nice_flo_send_key(SubGhzProtocolNiceFlo* instance, uint64_t key, uint8_t bit, uint8_t repeat) {
    while (repeat--) {
        //Send header
        SUBGHZ_TX_PIN_LOW();
        delay_us(instance->common.te_shot * 34);     //+2 interval v bit 1
        //Send start bit
        subghz_protocol_nice_flo_send_bit(instance, 1);
        //Send key data
        for (uint8_t i = bit; i > 0; i--) {
            subghz_protocol_nice_flo_send_bit(instance, bit_read(key, i - 1));
        }
    }
}

void subghz_protocol_nice_flo_parse(SubGhzProtocolNiceFlo* instance, LevelPair data) {
    switch (instance->common.parser_step) {
    case 0:
        if ((data.level == ApiHalSubGhzCaptureLevelLow)
                && (DURATION_DIFF(data.duration,instance->common.te_shot * 36)< instance->common.te_delta * 36)) {
            //Found header Nice Flo
            instance->common.parser_step = 1;
        } else {
            instance->common.parser_step = 0;
        }
        break;
    case 1:
        if (data.level == ApiHalSubGhzCaptureLevelLow) {
            break;
        } else if (DURATION_DIFF(data.duration,instance->common.te_shot)< instance->common.te_delta) {
            //Found start bit Nice Flo
            instance->common.parser_step = 2;
            instance->common.code_found = 0;
            instance->common.code_count_bit = 0;
        } else {
            instance->common.parser_step = 0;
        }
        break;
    case 2:
        if (data.level == ApiHalSubGhzCaptureLevelLow) { //save interval
            if (data.duration >= (instance->common.te_shot * 4)) {
                instance->common.parser_step = 1;
                if (instance->common.code_count_bit>= instance->common.code_min_count_bit_for_found) {

                    //ToDo out data display
                    if (instance->common.callback) instance->common.callback((SubGhzProtocolCommon*)instance, instance->common.context);
                }
                break;
            }
            instance->common.te_last = data.duration;
            instance->common.parser_step = 3;
        } else {
            instance->common.parser_step = 0;
        }
        break;
    case 3:
        if (data.level == ApiHalSubGhzCaptureLevelHigh) {
            if ((DURATION_DIFF(instance->common.te_last,instance->common.te_shot) < instance->common.te_delta)
                    && (DURATION_DIFF(data.duration,instance->common.te_long)< instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 0);
                instance->common.parser_step = 2;
            } else if ((DURATION_DIFF(instance->common.te_last,instance->common.te_long)< instance->common.te_delta)
                    && (DURATION_DIFF(data.duration,instance->common.te_shot)< instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 1);
                instance->common.parser_step = 2;
            } else
                instance->common.parser_step = 0;
        } else {
            instance->common.parser_step = 0;
        }
        break;
    }
}
