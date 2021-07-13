#include "subghz_protocol_nero_sketch.h"


struct SubGhzProtocolNeroSketch {
    SubGhzProtocolCommon common;
};

SubGhzProtocolNeroSketch* subghz_protocol_nero_sketch_alloc(void) {
    SubGhzProtocolNeroSketch* instance = furi_alloc(sizeof(SubGhzProtocolNeroSketch));

    instance->common.name = "Nero Sketch"; 
    instance->common.code_min_count_bit_for_found = 40;
    instance->common.te_shot = 330;
    instance->common.te_long = 660;
    instance->common.te_delta = 150;
    instance->common.to_string = (SubGhzProtocolCommonToStr)subghz_protocol_nero_sketch_to_str;

    return instance;
}

void subghz_protocol_nero_sketch_free(SubGhzProtocolNeroSketch* instance) {
    furi_assert(instance);
    free(instance);
}

/** Send bit 
 * 
 * @param instance - SubGhzProtocolNeroSketch instance
 * @param bit - bit
 */
void subghz_protocol_nero_sketch_send_bit(SubGhzProtocolNeroSketch* instance, uint8_t bit) {
    if (bit) {
        //send bit 1
        SUBGHZ_TX_PIN_HIGTH();
        delay_us(instance->common.te_long);
        SUBGHZ_TX_PIN_LOW();
        delay_us(instance->common.te_shot);
    } else {
        //send bit 0
        SUBGHZ_TX_PIN_HIGTH();
        delay_us(instance->common.te_shot);
        SUBGHZ_TX_PIN_LOW();
        delay_us(instance->common.te_long);
    }
}

void subghz_protocol_nero_sketch_send_key(SubGhzProtocolNeroSketch* instance, uint64_t key, uint8_t bit,uint8_t repeat) {
    while (repeat--) {
        //Send header
        for(uint8_t i = 0; i < 47; i++){
           SUBGHZ_TX_PIN_HIGTH(); 
            delay_us(instance->common.te_shot);
            SUBGHZ_TX_PIN_LOW();
            delay_us(instance->common.te_shot);
        }

        //Send start bit
        SUBGHZ_TX_PIN_HIGTH(); 
        delay_us(instance->common.te_shot*4);
        SUBGHZ_TX_PIN_LOW();
        delay_us(instance->common.te_shot);

        //Send key data
        for (uint8_t i = bit; i > 0; i--) {
            subghz_protocol_nero_sketch_send_bit(instance, bit_read(key, i - 1));
        }
        //Send stop bit
        SUBGHZ_TX_PIN_HIGTH(); 
        delay_us(instance->common.te_shot*3);
        SUBGHZ_TX_PIN_LOW();
        delay_us(instance->common.te_shot);
    }
}

void subghz_protocol_nero_sketch_reset(SubGhzProtocolNeroSketch* instance) {
    instance->common.parser_step = 0;
}

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolNeroSketch instance
 */
void subghz_protocol_nero_sketch_check_remote_controller(SubGhzProtocolNeroSketch* instance) {
    //пока не понятно с серийником, но код статический
    // uint64_t code_found_reverse = subghz_protocol_common_reverse_key(instance->common.code_found, instance->common.code_count_bit);
    // uint32_t code_fix = code_found_reverse & 0xFFFFFFFF;
    // //uint32_t code_hop = (code_found_reverse >> 24) & 0xFFFFF;

    // instance->common.serial = code_fix & 0xFFFFFFF;
    // instance->common.btn = (code_fix >> 28) & 0x0F;

    if (instance->common.callback) instance->common.callback((SubGhzProtocolCommon*)instance, instance->common.context);

}

void subghz_protocol_nero_sketch_parse(SubGhzProtocolNeroSketch* instance, bool level, uint32_t duration) {
    switch (instance->common.parser_step) {
    case 0:
        if ((level)
                && (DURATION_DIFF(duration,instance->common.te_shot)< instance->common.te_delta)) {
            instance->common.parser_step = 1;
            instance->common.te_last = duration;
            instance->common.header_count = 0;
        } else {
            instance->common.parser_step = 0;
        }
        break;
    case 1:
       if (level){
            if((DURATION_DIFF(duration,instance->common.te_shot)< instance->common.te_delta )
                || (DURATION_DIFF(duration,instance->common.te_shot*4)< instance->common.te_delta)) {
                instance->common.te_last = duration;
            } else {
                instance->common.parser_step = 0;
            }
        } else if(DURATION_DIFF(duration,instance->common.te_shot)< instance->common.te_delta){
            if(DURATION_DIFF(instance->common.te_last,instance->common.te_shot)< instance->common.te_delta){
                // Found header
                instance->common.header_count++;
                break;
            }else if(DURATION_DIFF(instance->common.te_last,instance->common.te_shot*4)< instance->common.te_delta){
                 // Found start bit
                 if(instance->common.header_count>40) {
                    instance->common.parser_step = 2;
                    instance->common.code_found = 0;
                    instance->common.code_count_bit = 0;
                 }else {
                    instance->common.parser_step = 0;
                 }
            } else {
                instance->common.parser_step = 0;
            }
        } else {
            instance->common.parser_step = 0;
        }
        break;
    case 2:
        if (level) {
            if (duration >= (instance->common.te_shot * 2 + instance->common.te_delta*2)) {
                //Found stop bit
                instance->common.parser_step = 0;
                if (instance->common.code_count_bit>= instance->common.code_min_count_bit_for_found) {
                    subghz_protocol_nero_sketch_check_remote_controller(instance);
                }
                instance->common.code_found = 0;
                instance->common.code_count_bit = 0;
                break;
            } else {
                instance->common.te_last = duration;
                instance->common.parser_step = 3;
            }

        }else{
            instance->common.parser_step = 0;
        }
        break;
    case 3:
        if(!level){
                if ((DURATION_DIFF(instance->common.te_last,instance->common.te_shot)< instance->common.te_delta)
                    && (DURATION_DIFF(duration,instance->common.te_long)< instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 0);
                instance->common.parser_step = 2;
            } else if ((DURATION_DIFF(instance->common.te_last,instance->common.te_long )< instance->common.te_delta)
                    && (DURATION_DIFF(duration,instance->common.te_shot)< instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 1);
                instance->common.parser_step = 2;
            } else {
                instance->common.parser_step = 0;
            }
        } else {
            instance->common.parser_step = 0;
        }
        break;
    }
}

void subghz_protocol_nero_sketch_to_str(SubGhzProtocolNeroSketch* instance, string_t output) {
    
    uint32_t code_found_hi = instance->common.code_found >> 32;
    uint32_t code_found_lo = instance->common.code_found & 0x00000000ffffffff;

    uint64_t code_found_reverse = subghz_protocol_common_reverse_key(instance->common.code_found, instance->common.code_count_bit);

    uint32_t code_found_reverse_hi = code_found_reverse>>32;
    uint32_t code_found_reverse_lo = code_found_reverse&0x00000000ffffffff;

    //uint32_t rev_hi =

    string_cat_printf(output,
                      "Protocol %s, %d Bit\r\n"
                      " KEY:0x%lX%08lX\r\n"
                      " YEK:0x%lX%08lX\r\n",
                      instance->common.name,
                      instance->common.code_count_bit,
                      code_found_hi,
                      code_found_lo,
                      code_found_reverse_hi,
                      code_found_reverse_lo
                      );
}
