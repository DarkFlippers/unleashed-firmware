#include "subghz_protocol_came.h"
#include "subghz_protocol_common.h"

/*
 * Help
 * https://phreakerclub.com/447
 *
 */

struct SubGhzProtocolCame {
    SubGhzProtocolCommon common;
};

SubGhzProtocolCame* subghz_protocol_came_alloc() {
    SubGhzProtocolCame* instance = furi_alloc(sizeof(SubGhzProtocolCame));

    instance->common.name = "Came";
    instance->common.code_min_count_bit_for_found = 12;
    instance->common.te_shot = 320;
    instance->common.te_long = 640;
    instance->common.te_delta = 150;

    return instance;
}

void subghz_protocol_came_free(SubGhzProtocolCame* instance) {
    furi_assert(instance);
    free(instance);
}

/** Send bit 
 * 
 * @param instance - SubGhzProtocolCame instance
 * @param bit - bit
 */
void subghz_protocol_came_send_bit(SubGhzProtocolCame* instance, uint8_t bit) {
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

void subghz_protocol_came_send_key(SubGhzProtocolCame* instance, uint64_t key, uint8_t bit, uint8_t repeat) {
    while (repeat--) {
        //Send header
        SUBGHZ_TX_PIN_LOW();
        delay_us(instance->common.te_shot * 34);     //+2 interval v bit 1
        //Send start bit
        subghz_protocol_came_send_bit(instance, 1);
        //Send key data
        for (uint8_t i = bit; i > 0; i--) {
            subghz_protocol_came_send_bit(instance, bit_read(key, i - 1));
        }
    }
}

void subghz_protocol_came_reset(SubGhzProtocolCame* instance) {
    instance->common.parser_step = 0;
}

void subghz_protocol_came_parse(SubGhzProtocolCame* instance, bool level, uint32_t duration) {
    switch (instance->common.parser_step) {
    case 0:
        if ((!level)
                && (DURATION_DIFF(duration, instance->common.te_shot * 51)< instance->common.te_delta * 51)) { //Need protocol 36 te_shot
            //Found header CAME
            instance->common.parser_step = 1;
        } else {
            instance->common.parser_step = 0;
        }
        break;
    case 1:
        if (!level) {
            break;
        } else if (DURATION_DIFF(duration, instance->common.te_shot)< instance->common.te_delta) {
            //Found start bit CAME
            instance->common.parser_step = 2;
            instance->common.code_found = 0;
            instance->common.code_count_bit = 0;
        } else {
            instance->common.parser_step = 0;
        }
        break;
    case 2:
        if (!level) { //save interval
            if (duration >= (instance->common.te_shot * 4)) {
                instance->common.parser_step = 1;
                if (instance->common.code_count_bit>= instance->common.code_min_count_bit_for_found) {


                    instance->common.serial = 0x0;
                    instance->common.btn = 0x0;
                    if (instance->common.callback)
                        instance->common.callback((SubGhzProtocolCommon*)instance, instance->common.context);
                
                }
                break;
            }
            instance->common.te_last = duration;
            instance->common.parser_step = 3;
        } else {
            instance->common.parser_step = 0;
        }
        break;
    case 3:
        if (level) {
            if ((DURATION_DIFF(instance->common.te_last,instance->common.te_shot) < instance->common.te_delta)
                    && (DURATION_DIFF(duration, instance->common.te_long)< instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 0);
                instance->common.parser_step = 2;
            } else if ((DURATION_DIFF(instance->common.te_last,instance->common.te_long)< instance->common.te_delta)
                    && (DURATION_DIFF(duration, instance->common.te_shot)< instance->common.te_delta)) {
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
