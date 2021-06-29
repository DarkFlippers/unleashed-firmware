#include "subghz_protocol_common.h"
#include <stdio.h>

void subghz_protocol_common_add_bit(SubGhzProtocolCommon *common, uint8_t bit){
    common->code_found = common->code_found <<1 | bit;
    common->code_count_bit++;
}

uint8_t subghz_protocol_common_check_interval (SubGhzProtocolCommon *common, uint32_t interval, uint16_t interval_check) {
    if ((interval_check >= (interval - common->te_delta))&&(interval_check <= (interval + common->te_delta))){
        return 1;
    } else {
        return 0;
    }
}

uint64_t subghz_protocol_common_reverse_key(uint64_t key, uint8_t count_bit){
    uint64_t key_reverse=0;
    for(uint8_t i=0; i<count_bit; i++) {
        key_reverse=key_reverse<<1|bit_read(key,i);
    }
    return key_reverse;
}

void subghz_protocol_common_set_callback(SubGhzProtocolCommon* common, SubGhzProtocolCommonCallback callback, void* context) {
    common->callback = callback;
    common->context = context;
}

void subghz_protocol_common_to_str(SubGhzProtocolCommon* instance, string_t output) {
    if (instance->to_string) {
        instance->to_string(instance, output);
    } else {
        uint32_t code_found_hi = instance->code_found >> 32;
        uint32_t code_found_lo = instance->code_found & 0x00000000ffffffff;

        uint64_t code_found_reverse = subghz_protocol_common_reverse_key(instance->code_found, instance->code_count_bit);

        uint32_t code_found_reverse_hi = code_found_reverse>>32;
        uint32_t code_found_reverse_lo = code_found_reverse&0x00000000ffffffff;

        if (code_found_hi>0) {
            string_cat_printf(
                output,
                "Protocol %s, %d Bit\r\n"
                " KEY:0x%lX%08lX\r\n"
                " YEK:0x%lX%08lX\r\n",
                instance->name,
                instance->code_count_bit,
                code_found_hi,
                code_found_lo,
                code_found_reverse_hi,
                code_found_reverse_lo
            );
        } else {
            string_cat_printf(
                output,
                "Protocol %s, %d Bit\r\n"
                " KEY:0x%lX%lX\r\n"
                " YEK:0x%lX%lX\r\n",
                instance->name,
                instance->code_count_bit,
                code_found_hi,
                code_found_lo,
                code_found_reverse_hi,
                code_found_reverse_lo
            );
        }
    }
}
