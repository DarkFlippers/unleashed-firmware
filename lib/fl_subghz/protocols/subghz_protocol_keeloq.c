#include "subghz_protocol_keeloq.h"

#include <furi.h>

#include <m-string.h>
#include <m-array.h>

/*
 * Keeloq
 * https://ru.wikipedia.org/wiki/KeeLoq
 * https://phreakerclub.com/forum/showthread.php?t=1094
 *
 */

#define KEELOQ_NLF          0x3A5C742E
#define bit(x,n)            (((x)>>(n))&1)
#define g5(x,a,b,c,d,e)     (bit(x,a)+bit(x,b)*2+bit(x,c)*4+bit(x,d)*8+bit(x,e)*16)

/*
 * KeeLoq learning types
 * https://phreakerclub.com/forum/showthread.php?t=67
 */
#define KEELOQ_LEARNING_UNKNOWN    0u
#define KEELOQ_LEARNING_SIMPLE     1u
#define KEELOQ_LEARNING_NORMAL     2u
#define KEELOQ_LEARNING_SECURE     3u

typedef struct {
    string_t name;
    uint64_t key;
    uint16_t type;
} KeeLoqManufactureCode;

ARRAY_DEF(KeeLoqManufactureCodeArray, KeeLoqManufactureCode, M_POD_OPLIST)
#define M_OPL_KeeLoqManufactureCodeArray_t() ARRAY_OPLIST(KeeLoqManufactureCodeArray, M_POD_OPLIST)

struct SubGhzProtocolKeeloq {
    SubGhzProtocolCommon common;
    KeeLoqManufactureCodeArray_t manufacture_codes;
    const char* manufacture_name;
};

/** Simple Learning Encrypt
 * @param data - 0xBSSSCCCC, B(4bit) key, S(10bit) serial&0x3FF, C(16bit) counter
 * @param key - manufacture (64bit)
 * @return keelog encrypt data
 */
inline uint32_t subghz_protocol_keeloq_encrypt(const uint32_t data, const uint64_t key) {
    uint32_t x = data, r;
    for (r = 0; r < 528; r++)
        x = (x>>1)^((bit(x,0)^bit(x,16)^(uint32_t)bit(key,r&63)^bit(KEELOQ_NLF,g5(x,1,9,20,26,31)))<<31);
    return x;
}

/** Simple Learning Decrypt
 * @param data - keelog encrypt data
 * @param key - manufacture (64bit)
 * @return 0xBSSSCCCC, B(4bit) key, S(10bit) serial&0x3FF, C(16bit) counter
 */
inline uint32_t subghz_protocol_keeloq_decrypt(const uint32_t data, const uint64_t key) {
    uint32_t x = data, r;
    for (r = 0; r < 528; r++)
        x = (x<<1)^bit(x,31)^bit(x,15)^(uint32_t)bit(key,(15-r)&63)^bit(KEELOQ_NLF,g5(x,0,8,19,25,30));
    return x;
}

/** Normal Learning
 * @param data - serial number (28bit)
 * @param key - manufacture (64bit)
 * @return manufacture for this serial number (64bit)
 */
inline uint64_t subghz_protocol_keeloq_normal_learning(uint32_t data, const uint64_t key){
    uint32_t k1,k2;

    data&=0x0FFFFFFF;
    data|=0x20000000;
    k1=subghz_protocol_keeloq_decrypt(data, key);

    data&=0x0FFFFFFF;
    data|=0x60000000;
    k2=subghz_protocol_keeloq_decrypt(data, key);

    return ((uint64_t)k2<<32)| k1; // key - shifrovanoya
}

SubGhzProtocolKeeloq* subghz_protocol_keeloq_alloc() {
    SubGhzProtocolKeeloq* instance = furi_alloc(sizeof(SubGhzProtocolKeeloq));

    instance->common.name = "KeeLoq";
    instance->common.code_min_count_bit_for_found = 64;
    instance->common.te_shot = 400;
    instance->common.te_long = 800;
    instance->common.te_delta = 140;
    instance->common.to_string = (SubGhzProtocolCommonToStr)subghz_protocol_keeloq_to_str;

    KeeLoqManufactureCodeArray_init(instance->manufacture_codes);

    return instance;
}

void subghz_protocol_keeloq_free(SubGhzProtocolKeeloq* instance) {
    furi_assert(instance);
    for
        M_EACH(manufacture_code, instance->manufacture_codes, KeeLoqManufactureCodeArray_t) {
            string_clear(manufacture_code->name);
            manufacture_code->key = 0;
    }
    KeeLoqManufactureCodeArray_clear(instance->manufacture_codes);
    free(instance);
}

void subghz_protocol_keeloq_add_manafacture_key(SubGhzProtocolKeeloq* instance, const char* name, uint64_t key, uint16_t type) {
    KeeLoqManufactureCode* manufacture_code = KeeLoqManufactureCodeArray_push_raw(instance->manufacture_codes);
    string_init_set_str(manufacture_code->name, name);
    manufacture_code->key = key;
    manufacture_code->type = type;
}

/** Checking the accepted code against the database manafacture key
 * 
 * @param instance SubGhzProtocolKeeloq instance
 * @param fix fix part of the parcel
 * @param hop hop encrypted part of the parcel
 * @return true on successful search
 */
uint8_t subghz_protocol_keeloq_check_remote_controller_selector(SubGhzProtocolKeeloq* instance, uint32_t fix , uint32_t hop) {
    uint16_t end_serial = (uint16_t)(fix&0x3FF);
    uint8_t btn = (uint8_t)(fix>>28);
    uint32_t decrypt = 0;
    uint64_t man_normal_learning;

    for
        M_EACH(manufacture_code, instance->manufacture_codes, KeeLoqManufactureCodeArray_t) {
            switch (manufacture_code->type){
                case KEELOQ_LEARNING_SIMPLE:
                    //Simple Learning
                    decrypt = subghz_protocol_keeloq_decrypt(hop, manufacture_code->key);
                    if((decrypt>>28 == btn) && ((((uint16_t)(decrypt>>16)) & 0x3FF) == end_serial)){
                        instance->manufacture_name = string_get_cstr(manufacture_code->name);
                        instance->common.cnt = decrypt & 0x0000FFFF;
                        return 1;
                    }
                break;
                case KEELOQ_LEARNING_NORMAL:
                    // Normal_Learning
                    // https://phreakerclub.com/forum/showpost.php?p=43557&postcount=37
                    man_normal_learning = subghz_protocol_keeloq_normal_learning(fix, manufacture_code->key);
                    decrypt=subghz_protocol_keeloq_decrypt(hop, man_normal_learning);
                    if( (decrypt>>28 ==btn)&& ((((uint16_t)(decrypt>>16))&0x3FF)==end_serial)){ 
                        instance->manufacture_name = string_get_cstr(manufacture_code->name);
                        instance->common.cnt = decrypt & 0x0000FFFF;
                        return 1;
                    }
                break;
                case KEELOQ_LEARNING_UNKNOWN:
                    // Simple Learning
                    decrypt=subghz_protocol_keeloq_decrypt(hop, manufacture_code->key);
                    if( (decrypt>>28 ==btn) && ((((uint16_t)(decrypt>>16))&0x3FF)==end_serial)){
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
                    decrypt=subghz_protocol_keeloq_decrypt(hop, man_rev);
                    if( (decrypt>>28 ==btn) && ((((uint16_t)(decrypt>>16))&0x3FF)==end_serial)){
                      instance->manufacture_name = string_get_cstr(manufacture_code->name);
                      instance->common.cnt= decrypt&0x0000FFFF;
                      return 1;
                    }
                    //###########################
                    // Normal_Learning
                    // https://phreakerclub.com/forum/showpost.php?p=43557&postcount=37
                    man_normal_learning = subghz_protocol_keeloq_normal_learning(fix, manufacture_code->key);
                    decrypt=subghz_protocol_keeloq_decrypt(hop, man_normal_learning);
                    if( (decrypt>>28 ==btn)&& ((((uint16_t)(decrypt>>16))&0x3FF)==end_serial)){
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
                    man_normal_learning = subghz_protocol_keeloq_normal_learning(fix, man_rev);
                    decrypt=subghz_protocol_keeloq_decrypt(hop, man_normal_learning);
                    if( (decrypt>>28 ==btn) && ((((uint16_t)(decrypt>>16))&0x3FF)==end_serial)){
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
 * @param instance SubGhzProtocolKeeloq instance
 */
void subghz_protocol_keeloq_check_remote_controller(SubGhzProtocolKeeloq* instance) {
    uint64_t key = subghz_protocol_common_reverse_key(instance->common.code_found, instance->common.code_count_bit);
    uint32_t key_fix = key >> 32;
    uint32_t key_hop = key & 0x00000000ffffffff;
    // Check key AN-Motors
    if((key_hop >> 24) == ((key_hop>>16)&0x00ff) && (key_fix>>28) ==((key_hop>>12)&0x0f) ){
        instance->manufacture_name = "AN-Motors";
        instance->common.cnt = key_hop>>16;
    } else {
        subghz_protocol_keeloq_check_remote_controller_selector(instance, key_fix, key_hop);
    }
    instance ->common.serial= key_fix&0x0FFFFF;
    instance->common.btn = key_fix >> 28;
    if (instance->common.callback) instance->common.callback((SubGhzProtocolCommon*)instance, instance->common.context);
}

/** Send bit 
 * 
 * @param instance - SubGhzProtocolKeeloq instance
 * @param bit - bit
 */
void subghz_protocol_keeloq_send_bit(SubGhzProtocolKeeloq* instance, uint8_t bit) {
    if (bit) {
        // send bit 1
        SUBGHZ_TX_PIN_HIGTH();
        delay_us(instance->common.te_shot);
        SUBGHZ_TX_PIN_LOW();
        delay_us(instance->common.te_long);
    } else {
        // send bit 0
        SUBGHZ_TX_PIN_HIGTH();
        delay_us(instance->common.te_long);
        SUBGHZ_TX_PIN_LOW();
        delay_us(instance->common.te_shot);
    }
}

void subghz_protocol_keeloq_send_key(SubGhzProtocolKeeloq* instance, uint64_t key, uint8_t bit, uint8_t repeat) {
    while (repeat--) {
        // Send header
        for (uint8_t i = 11; i > 0; i--) {
            SUBGHZ_TX_PIN_HIGTH();
            delay_us(instance->common.te_shot);
            SUBGHZ_TX_PIN_LOW();
            delay_us(instance->common.te_shot);
        }
        delay_us(instance->common.te_shot * 9); //+1 up Send header

        for (uint8_t i = bit; i > 0; i--) {
            subghz_protocol_keeloq_send_bit(instance, bit_read(key, i - 1));
        }
        // +send 2 status bit
        subghz_protocol_keeloq_send_bit(instance, 0);
        subghz_protocol_keeloq_send_bit(instance, 0);
        // send end
        subghz_protocol_keeloq_send_bit(instance, 0);
        delay_us(instance->common.te_shot * 2);   //+2 interval END SEND
    }
}

void subghz_protocol_keeloq_reset(SubGhzProtocolKeeloq* instance) {
    instance->common.parser_step = 0;
}

void subghz_protocol_keeloq_parse(SubGhzProtocolKeeloq* instance, bool level, uint32_t duration) {
    switch (instance->common.parser_step) {
    case 0:
        if ((level) && DURATION_DIFF(duration, instance->common.te_shot)< instance->common.te_delta) {
            instance->common.parser_step = 1;
            instance->common.header_count++;
        } else {
            instance->common.parser_step = 0;
        }

        break;
    case 1:
        if ((!level) && (DURATION_DIFF(duration, instance->common.te_shot ) < instance->common.te_delta)) {
            instance->common.parser_step = 0;
            break;
        }
        if ((instance->common.header_count > 2) && ( DURATION_DIFF(duration, instance->common.te_shot * 10)< instance->common.te_delta * 10)) {
            // Found header
            instance->common.parser_step = 2;
            instance->common.code_found = 0;
            instance->common.code_count_bit = 0;
        } else {
            instance->common.parser_step = 0;
            instance->common.header_count = 0;
        }
        break;
    case 2:
        if (level) {
            instance->common.te_last = duration;
            instance->common.parser_step = 3;
        }
        break;
    case 3:
        if (!level) {
            if (duration >= (instance->common.te_shot * 2 + instance->common.te_delta)) {
                // Found end TX
                instance->common.parser_step = 0;
                if (instance->common.code_count_bit >= instance->common.code_min_count_bit_for_found) {
                    instance->common.code_last_found = instance->common.code_found;

                    subghz_protocol_keeloq_check_remote_controller(instance);

                    instance->common.code_found = 0;
                    instance->common.code_count_bit = 0;
                    instance->common.header_count = 0;
                }
                break;
            } else if ((DURATION_DIFF(instance->common.te_last, instance->common.te_shot) < instance->common.te_delta)
                    && (DURATION_DIFF(duration, instance->common.te_long) < instance->common.te_delta)) {
                if (instance->common.code_count_bit < instance->common.code_min_count_bit_for_found) {
                    subghz_protocol_common_add_bit(&instance->common, 1);
                }
                instance->common.parser_step = 2;
            } else if ((DURATION_DIFF(instance->common.te_last, instance->common.te_long) < instance->common.te_delta)
                    && (DURATION_DIFF(duration, instance->common.te_shot) < instance->common.te_delta)) {
                if (instance->common.code_count_bit < instance->common.code_min_count_bit_for_found) {
                    subghz_protocol_common_add_bit(&instance->common, 0);
                }
                instance->common.parser_step = 2;
            } else {
                instance->common.parser_step = 0;
                instance->common.header_count = 0;
            }
        } else {
            instance->common.parser_step = 0;
            instance->common.header_count = 0;
        }
        break;
    }
}

void subghz_protocol_keeloq_to_str(SubGhzProtocolKeeloq* instance, string_t output) {
    uint32_t code_found_hi = instance->common.code_found >> 32;
    uint32_t code_found_lo = instance->common.code_found & 0x00000000ffffffff;

    uint64_t code_found_reverse = subghz_protocol_common_reverse_key(instance->common.code_found, instance->common.code_count_bit);

    uint32_t code_found_reverse_hi = code_found_reverse>>32;
    uint32_t code_found_reverse_lo = code_found_reverse&0x00000000ffffffff;
    string_cat_printf(
        output,
        "Protocol %s, %d Bit\r\n"
        "KEY:0x%lX%lX\r\n"
        "FIX:%lX MF:%s \r\n"
        "HOP:%lX \r\n"
        //"CNT:%04X BTN:%02lX\r\n",
        "SN:%05lX CNT:%04X BTN:%02lX\r\n",
        //"YEK:0x%lX%lX\r\n",
        instance->common.name,
        instance->common.code_count_bit,
        code_found_hi,
        code_found_lo,
        //code_found_reverse_hi,
        //code_found_reverse_lo
        code_found_reverse_hi,
        instance->manufacture_name,
        code_found_reverse_lo,
        instance->common.serial,
        instance->common.cnt, 
        instance->common.btn
    );
}