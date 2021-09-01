#include "subghz_protocol_star_line.h"
#include "subghz_protocol_keeloq_common.h"

#include "../subghz_keystore.h"

#include <furi.h>

#include <m-string.h>
#include <m-array.h>


struct SubGhzProtocolStarLine {
    SubGhzProtocolCommon common;
    SubGhzKeystore* keystore;
    const char* manufacture_name;
};

SubGhzProtocolStarLine* subghz_protocol_star_line_alloc(SubGhzKeystore* keystore) {
    SubGhzProtocolStarLine* instance = furi_alloc(sizeof(SubGhzProtocolStarLine));

    instance->keystore = keystore;

    instance->common.name = "Star Line"; 
    instance->common.code_min_count_bit_for_found = 64;
    instance->common.te_short = 250;
    instance->common.te_long = 500;
    instance->common.te_delta = 120;
    instance->common.type_protocol = TYPE_PROTOCOL_DYNAMIC;
    instance->common.to_string = (SubGhzProtocolCommonToStr)subghz_protocol_star_line_to_str;
    instance->common.to_load_protocol =
        (SubGhzProtocolCommonLoadFromRAW)subghz_decoder_star_line_to_load_protocol;

    return instance;
}

void subghz_protocol_star_line_free(SubGhzProtocolStarLine* instance) {
    furi_assert(instance);
    free(instance);
}

const char* subghz_protocol_star_line_find_and_get_manufacture_name (void* context){
    SubGhzProtocolStarLine* instance = context;
    subghz_protocol_star_line_check_remote_controller(instance);
    return instance->manufacture_name;
}

const char* subghz_protocol_star_line_get_manufacture_name (void* context){
    SubGhzProtocolStarLine* instance = context;
    return instance->manufacture_name;
}

/** Send bit 
 * 
 * @param instance - SubGhzProtocolStarLine instance
 * @param bit - bit
 */
void subghz_protocol_star_line_send_bit(SubGhzProtocolStarLine* instance, uint8_t bit) {
    if (bit) {
        //send bit 1
        SUBGHZ_TX_PIN_HIGH();
        delay_us(instance->common.te_long);
        SUBGHZ_TX_PIN_LOW();
        delay_us(instance->common.te_long);
    } else {
        //send bit 0
        SUBGHZ_TX_PIN_HIGH();
        delay_us(instance->common.te_short);
        SUBGHZ_TX_PIN_LOW();
        delay_us(instance->common.te_short);
    }
}

void subghz_protocol_star_line_send_key(SubGhzProtocolStarLine* instance, uint64_t key, uint8_t bit,uint8_t repeat) {
    while (repeat--) {
        //Send header
        for(uint8_t i = 0; i < 6; i++){
            SUBGHZ_TX_PIN_HIGH();
            delay_us(instance->common.te_long * 2);
            SUBGHZ_TX_PIN_LOW();
            delay_us(instance->common.te_long * 2); 
        } 
        //Send Start bit ??????????
        //Send key data
        for (uint8_t i = bit; i > 0; i--) {
            subghz_protocol_star_line_send_bit(instance, bit_read(key, i - 1));
        }
        //Send Stop bit ??????????
    }
}

void subghz_protocol_star_line_reset(SubGhzProtocolStarLine* instance) {
    instance->common.parser_step = 0;
}

/** Checking the accepted code against the database manafacture key
 * 
 * @param instance SubGhzProtocolStarLine instance
 * @param fix fix part of the parcel
 * @param hop hop encrypted part of the parcel
 * @return true on successful search
 */
uint8_t subghz_protocol_star_line_check_remote_controller_selector(SubGhzProtocolStarLine* instance, uint32_t fix , uint32_t hop) {
    uint16_t end_serial = (uint16_t)(fix&0xFF);
    uint8_t btn = (uint8_t)(fix>>24);
    uint32_t decrypt = 0;
    uint64_t man_normal_learning;

    for
        M_EACH(manufacture_code, *subghz_keystore_get_data(instance->keystore), SubGhzKeyArray_t) {
            switch (manufacture_code->type){
                case KEELOQ_LEARNING_SIMPLE:
                    //Simple Learning
                    decrypt = subghz_protocol_keeloq_common_decrypt(hop, manufacture_code->key);
                    if((decrypt>>24 == btn) && ((((uint16_t)(decrypt>>16)) & 0x00FF) == end_serial)){
                        instance->manufacture_name = string_get_cstr(manufacture_code->name);
                        instance->common.cnt = decrypt & 0x0000FFFF;
                        return 1;
                    }
                break;
                case KEELOQ_LEARNING_NORMAL:
                    // Normal_Learning
                    // https://phreakerclub.com/forum/showpost.php?p=43557&postcount=37
                    man_normal_learning = subghz_protocol_keeloq_common_normal_learning(fix, manufacture_code->key);
                    decrypt=subghz_protocol_keeloq_common_decrypt(hop, man_normal_learning);
                    if( (decrypt>>24 ==btn)&& ((((uint16_t)(decrypt>>16))&0x00FF)==end_serial)){ 
                        instance->manufacture_name = string_get_cstr(manufacture_code->name);
                        instance->common.cnt = decrypt & 0x0000FFFF;
                        return 1;
                    }
                break;
                case KEELOQ_LEARNING_UNKNOWN:
                    // Simple Learning
                    decrypt=subghz_protocol_keeloq_common_decrypt(hop, manufacture_code->key);
                    if( (decrypt>>24 ==btn) && ((((uint16_t)(decrypt>>16))&0x00FF)==end_serial)){
                        instance->manufacture_name = string_get_cstr(manufacture_code->name);
                        instance->common.cnt = decrypt & 0x0000FFFF;
                        return 1;
                    }
                    // Check for mirrored man
                    uint64_t man_rev=0;
                    uint64_t man_rev_byte=0;
                    for(uint8_t i=0; i<64; i+=8){
                        man_rev_byte=(uint8_t)(manufacture_code->key >> i);
                        man_rev = man_rev  | man_rev_byte << (56-i);
                    }
                    decrypt=subghz_protocol_keeloq_common_decrypt(hop, man_rev);
                    if( (decrypt>>24 ==btn) && ((((uint16_t)(decrypt>>16))&0x00FF)==end_serial)){
                      instance->manufacture_name = string_get_cstr(manufacture_code->name);
                      instance->common.cnt= decrypt&0x0000FFFF;
                      return 1;
                    }
                    //###########################
                    // Normal_Learning
                    // https://phreakerclub.com/forum/showpost.php?p=43557&postcount=37
                    man_normal_learning = subghz_protocol_keeloq_common_normal_learning(fix, manufacture_code->key);
                    decrypt=subghz_protocol_keeloq_common_decrypt(hop, man_normal_learning);
                    if( (decrypt>>24 ==btn)&& ((((uint16_t)(decrypt>>16))&0x00FF)==end_serial)){
                        instance->manufacture_name = string_get_cstr(manufacture_code->name);
                        instance->common.cnt= decrypt&0x0000FFFF;
                        return 1;
                    }
                    // Check for mirrored man
                    man_rev=0;
                    man_rev_byte=0;
                    for(uint8_t i=0; i<64; i+=8){
                        man_rev_byte = (uint8_t)(manufacture_code->key >> i);
                        man_rev = man_rev  | man_rev_byte << (56-i);
                    }
                    man_normal_learning = subghz_protocol_keeloq_common_normal_learning(fix, man_rev);
                    decrypt=subghz_protocol_keeloq_common_decrypt(hop, man_normal_learning);
                    if( (decrypt>>24 ==btn) && ((((uint16_t)(decrypt>>16))&0x00FF)==end_serial)){
                        instance->manufacture_name = string_get_cstr(manufacture_code->name);
                        instance->common.cnt= decrypt&0x0000FFFF;
                        return 1;
                    }
                break;
            }
        }

    instance->manufacture_name = "Unknown";
    instance->common.cnt=0;

    return 0;
}

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolStarLine instance
 */
void subghz_protocol_star_line_check_remote_controller(SubGhzProtocolStarLine* instance) {
    uint64_t key = subghz_protocol_common_reverse_key(instance->common.code_last_found, instance->common.code_last_count_bit);
    uint32_t key_fix = key >> 32;
    uint32_t key_hop = key & 0x00000000ffffffff;

    subghz_protocol_star_line_check_remote_controller_selector(instance, key_fix, key_hop);

    instance ->common.serial= key_fix&0x00FFFFFF;
    instance->common.btn = key_fix >> 24;
}

void subghz_protocol_star_line_parse(SubGhzProtocolStarLine* instance, bool level, uint32_t duration) {
    switch (instance->common.parser_step) {
    case 0:
        if (level){
            if(DURATION_DIFF(duration,instance->common.te_long * 2)< instance->common.te_delta * 2) {
                instance->common.parser_step = 1;
                instance->common.header_count++;
            } else if(instance->common.header_count>4){
                instance->common.code_found = 0;
                instance->common.code_count_bit = 0;
                instance->common.te_last = duration;
                instance->common.parser_step = 3;
            }
        }else{
            instance->common.parser_step = 0;
            instance->common.header_count = 0;
        }
        break;
    case 1:
        if ((!level)
                && (DURATION_DIFF(duration,instance->common.te_long * 2)< instance->common.te_delta * 2)) {
            //Found Preambula
            instance->common.parser_step = 0;
        } else {
            instance->common.header_count = 0;
            instance->common.parser_step = 0;
        }
        break;
    case 2:
        if (level) {
            if (duration >= (instance->common.te_long  + instance->common.te_delta)) {
                instance->common.parser_step = 0;
                if (instance->common.code_count_bit>= instance->common.code_min_count_bit_for_found) {
                    if(instance->common.code_last_found != instance->common.code_found){
                        instance->common.code_last_found = instance->common.code_found;
                        instance->common.code_last_count_bit = instance->common.code_count_bit;
                        if (instance->common.callback) instance->common.callback((SubGhzProtocolCommon*)instance, instance->common.context);
                    }
                }
                instance->common.code_found = 0;
                instance->common.code_count_bit = 0;
                instance->common.header_count = 0;
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
                if ((DURATION_DIFF(instance->common.te_last,instance->common.te_short)< instance->common.te_delta)
                    && (DURATION_DIFF(duration,instance->common.te_short)< instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 0);
                instance->common.parser_step = 2;
            } else if ((DURATION_DIFF(instance->common.te_last,instance->common.te_long )< instance->common.te_delta)
                    && (DURATION_DIFF(duration,instance->common.te_long)< instance->common.te_delta)) {
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

void subghz_protocol_star_line_to_str(SubGhzProtocolStarLine* instance, string_t output) {
    subghz_protocol_star_line_check_remote_controller(instance);
    uint32_t code_found_hi = instance->common.code_last_found >> 32;
    uint32_t code_found_lo = instance->common.code_last_found & 0x00000000ffffffff;

    uint64_t code_found_reverse = subghz_protocol_common_reverse_key(instance->common.code_last_found, instance->common.code_last_count_bit);

    uint32_t code_found_reverse_hi = code_found_reverse>>32;
    uint32_t code_found_reverse_lo = code_found_reverse&0x00000000ffffffff;
    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%lX%lX\r\n"
        "Fix:0x%08lX     Cnt:%04X\r\n"
        "Hop:0x%08lX     Btn:%02lX\r\n"
        "MF:%s\r\n"
        "Sn:0x%07lX \r\n",
        instance->common.name,
        instance->common.code_last_count_bit,
        code_found_hi,
        code_found_lo,
        code_found_reverse_hi,
        instance->common.cnt,
        code_found_reverse_lo,
        instance->common.btn,
        instance->manufacture_name,
        instance->common.serial
    );
}

void subghz_decoder_star_line_to_load_protocol(
    SubGhzProtocolStarLine* instance,
    void* context) {
    furi_assert(context);
    furi_assert(instance);
    SubGhzProtocolCommonLoad* data = context;
    instance->common.code_last_found = data->code_found;
    instance->common.code_last_count_bit = data->code_count_bit;
    subghz_protocol_star_line_check_remote_controller(instance);
}