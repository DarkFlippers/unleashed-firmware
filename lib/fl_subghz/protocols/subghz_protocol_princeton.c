#include "subghz_protocol_princeton.h"

/*
 * Help
 * https://phreakerclub.com/447
 *
 */

struct SubGhzProtocolPrinceton {
    SubGhzProtocolCommon common;
};

SubGhzProtocolPrinceton* subghz_protocol_princeton_alloc(void) {
    SubGhzProtocolPrinceton* instance = furi_alloc(sizeof(SubGhzProtocolPrinceton));

    instance->common.name = "Princeton";
    instance->common.code_min_count_bit_for_found = 24;
    instance->common.te_shot = 450;//150;
    instance->common.te_long = 1350;//450;
    instance->common.te_delta = 200;//50;

    return instance;
}

void subghz_protocol_princeton_free(SubGhzProtocolPrinceton* instance) {
    furi_assert(instance);
    free(instance);
}

void subghz_protocol_princeton_send_bit(SubGhzProtocolPrinceton* instance, uint8_t bit) {
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

void subghz_protocol_princeton_send_key(SubGhzProtocolPrinceton* instance, uint64_t key, uint8_t bit,uint8_t repeat) {
    while (repeat--) {
        SUBGHZ_TX_PIN_LOW();
        //Send start bit
        subghz_protocol_princeton_send_bit(instance, 1);
        //Send header
        delay_us(instance->common.te_shot * 33); //+2 interval v bit 1
        //Send key data
        for (uint8_t i = bit; i > 0; i--) {
            subghz_protocol_princeton_send_bit(instance, bit_read(key, i - 1));
        }
    }
}

void subghz_protocol_princeton_parse(SubGhzProtocolPrinceton* instance, LevelPair data) {
    switch (instance->common.parser_step) {
    case 0:
        if ((data.level == ApiHalSubGhzCaptureLevelLow)
                && (DURATION_DIFF(data.duration,instance->common.te_shot * 36)< instance->common.te_delta * 36)) {
            //Found Preambula
            instance->common.parser_step = 1;
            instance->common.code_found = 0;
            instance->common.code_count_bit = 0;
        } else {
            instance->common.parser_step = 0;
        }
        break;
    case 1:
        //save duration
        if (data.level == ApiHalSubGhzCaptureLevelHigh) {
            instance->common.te_last = data.duration;
            instance->common.parser_step = 2;
        }
        break;
    case 2:
        if (data.level == ApiHalSubGhzCaptureLevelLow) {
            if (data.duration>= (instance->common.te_shot * 10+ instance->common.te_delta)) {
                instance->common.parser_step = 1;
                if (instance->common.code_count_bit>= instance->common.code_min_count_bit_for_found) {
                    //ToDo out data display
                    if (instance->common.callback) instance->common.callback((SubGhzProtocolCommon*)instance, instance->common.context);
                }
                instance->common.code_found = 0;
                instance->common.code_count_bit = 0;
                break;
            }

            if ((DURATION_DIFF(instance->common.te_last,instance->common.te_shot)< instance->common.te_delta)
                    && (DURATION_DIFF(data.duration,instance->common.te_long)< instance->common.te_delta*3)) {
                subghz_protocol_common_add_bit(&instance->common, 0);
                instance->common.parser_step = 1;
            } else if ((DURATION_DIFF(instance->common.te_last,instance->common.te_long)< instance->common.te_delta*3)
                    && (DURATION_DIFF(data.duration,instance->common.te_shot)< instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 1);
                instance->common.parser_step = 1;
            } else {
                instance->common.parser_step = 0;
            }
        } else {
            instance->common.parser_step = 0;
        }
        break;
    }
}
