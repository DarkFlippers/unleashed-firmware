#include "subghz_protocol_nice_flor_s.h"

#include <furi.h>
#include "file-worker.h"
/*
 * https://phreakerclub.com/1615
 * https://phreakerclub.com/forum/showthread.php?t=2360
 * https://vrtp.ru/index.php?showtopic=27867
 */

struct SubGhzProtocolNiceFlorS {
    SubGhzProtocolCommon common;
    const char* rainbow_table_file_name;
};

SubGhzProtocolNiceFlorS* subghz_protocol_nice_flor_s_alloc() {
    SubGhzProtocolNiceFlorS* instance = furi_alloc(sizeof(SubGhzProtocolNiceFlorS));

    instance->common.name = "Nice FloR-S";
    instance->common.code_min_count_bit_for_found = 52;
    instance->common.te_shot = 500;
    instance->common.te_long = 1000;
    instance->common.te_delta = 300;
    instance->common.to_string = (SubGhzProtocolCommonToStr)subghz_protocol_nice_flor_s_to_str;

    return instance;
}

void subghz_protocol_nice_flor_s_free(SubGhzProtocolNiceFlorS* instance) {
    furi_assert(instance);
    free(instance);
}

void subghz_protocol_nice_flor_s_name_file(SubGhzProtocolNiceFlorS* instance, const char* name) {
    instance->rainbow_table_file_name = name;
    printf("Loading Nice FloR S rainbow table %s\r\n", name);
}

/** Send bit 
 * 
 * @param instance - SubGhzProtocolNiceFlorS instance
 * @param bit - bit
 */
void subghz_protocol_nice_flor_s_send_bit(SubGhzProtocolNiceFlorS* instance, uint8_t bit) {
    if(bit) {
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

void subghz_protocol_nice_flor_s_send_key(
    SubGhzProtocolNiceFlorS* instance,
    uint64_t key,
    uint8_t bit,
    uint8_t repeat) {
    while(repeat--) {
        //Send header
        SUBGHZ_TX_PIN_LOW();
        delay_us(instance->common.te_shot * 34);
        //Send Start Bit
        SUBGHZ_TX_PIN_HIGTH();
        delay_us(instance->common.te_shot * 3);
        SUBGHZ_TX_PIN_LOW();
        delay_us(instance->common.te_shot * 3);
        //Send key data
        for(uint8_t i = bit; i > 0; i--) {
            subghz_protocol_nice_flor_s_send_bit(instance, bit_read(key, i - 1));
        }
        //Send Stop Bit
        SUBGHZ_TX_PIN_HIGTH();
        delay_us(instance->common.te_shot * 3);
        SUBGHZ_TX_PIN_LOW();
        delay_us(instance->common.te_shot * 3);
    }
}

/** Read bytes from rainbow table
 * 
 * @param instance - SubGhzProtocolNiceFlorS* instance
 * @param address  - address byte
 * @return byte data
 */
uint8_t subghz_nice_flor_s_get_byte_in_file(SubGhzProtocolNiceFlorS* instance, uint32_t address) {
    if(!instance->rainbow_table_file_name) 
        return 0;

    uint8_t buffer = 0;
    FileWorker* file_worker = file_worker_alloc(true);
    if(file_worker_open(file_worker, instance->rainbow_table_file_name, FSAM_READ, FSOM_OPEN_EXISTING)) {
        file_worker_seek(file_worker, address, true);
        file_worker_read(file_worker, &buffer, 1);
        // bool res = file_worker_read(file_worker, &buffer, 1);
        // furi_assert(res== true);
    }
    file_worker_close(file_worker);
    file_worker_free(file_worker);

    return buffer;
}

/** Decrypt protocol Nice Flor S
 * 
 * @param instance - SubGhzProtocolNiceFlorS* instance
 */
void subghz_nice_flor_s_decoder_decrypt(SubGhzProtocolNiceFlorS* instance) {
    /*
    * Packet format Nice Flor-s: START-P0-P1-P2-P3-P4-P5-P6-P7-STOP
    * P0 (4-bit)    - button positional code - 1:0x1, 2:0x2, 3:0x4, 4:0x8;
    * P1 (4-bit)    - batch repetition number, calculated by the formula:
    * P1 = 0xF ^ P0 ^ n; where n changes from 1 to 15, then 0, and then in a circle
    * key 1: {0xF,0xC,0xD,0xA,0xB,0x8,0x9,0x6,0x7,0x4,0x5,0x2,0x3,0x0,0x1,0xE};
    * key 2: {0xC,0xF,0xE,0x9,0x8,0xB,0xA,0x5,0x4,0x7,0x6,0x1,0x0,0x3,0x2,0xD};
    * key 3: {0xA,0x9,0x8,0xF,0xE,0xD,0xC,0x3,0x2,0x1,0x0,0x7,0x6,0x5,0x4,0xB};
    * P2 (4-bit)    - part of the serial number, P2 = (K ^ S3) & 0xF;
    * P3 (byte)     - the major part of the encrypted index
    * P4 (byte)     - the low-order part of the encrypted index
    * P5 (byte)     - part of the serial number, P5 = K ^ S2;
    * P6 (byte)     - part of the serial number, P6 = K ^ S1;
    * P7 (byte)     - part of the serial number, P7 = K ^ S0;
    * K (byte)      - depends on P3 and P4, K = Fk(P3, P4);
    * S3,S2,S1,S0   - serial number of the console 28 bit.
    */

    uint16_t p3p4 = (uint16_t)(instance->common.code_found >> 24);
    instance->common.cnt = subghz_nice_flor_s_get_byte_in_file(instance,p3p4*2) << 8 | subghz_nice_flor_s_get_byte_in_file(instance,p3p4*2+1); //nice_flor_srainbow_table_for_search[p3p4]; тут надо считать поле с файла причем адрес надо у множить на 2
    uint8_t k =(uint8_t)(p3p4 & 0x00FF) ^subghz_nice_flor_s_get_byte_in_file(instance,(0x20000 |(instance->common.cnt &0x00ff))); //nice_flor_srainbow_table_for_search[0x10000|subghz_protocol_nice_flor_s.cnt & 0x00ff];

    uint8_t s3 = ((uint8_t)(instance->common.code_found >> 40) ^ k) & 0x0f;
    uint8_t s2 = ((uint8_t)(instance->common.code_found >> 16) ^ k);
    uint8_t s1 = ((uint8_t)(instance->common.code_found >> 8) ^ k);
    uint8_t s0 = ((uint8_t)(instance->common.code_found) ^ k);
    instance->common.serial = s3 << 24 | s2 << 16 | s1 << 8 | s0;

    instance->common.btn = (instance->common.code_found >> 48) & 0x0f;
    if(instance->common.callback) instance->common.callback((SubGhzProtocolCommon*)instance, instance->common.context);
}

void subghz_protocol_nice_flor_s_parse(SubGhzProtocolNiceFlorS* instance, LevelPair data) {
    switch(instance->common.parser_step) {
    case 0:
        if((data.level == ApiHalSubGhzCaptureLevelLow) 
            && (DURATION_DIFF(data.duration, instance->common.te_shot * 38) < instance->common.te_delta * 38)) {
            //Found start header Nice Flor-S
            instance->common.parser_step = 1;
        } else {
            instance->common.parser_step = 0;
        }
        break;
    case 1:
        if((data.level == ApiHalSubGhzCaptureLevelHigh) 
            && (DURATION_DIFF(data.duration, instance->common.te_shot * 3) < instance->common.te_delta * 3)) {
            //Found next header Nice Flor-S
            instance->common.parser_step = 2;
        } else {
            instance->common.parser_step = 0;
        }
        break;
    case 2:
        if((data.level == ApiHalSubGhzCaptureLevelLow) 
            && (DURATION_DIFF(data.duration, instance->common.te_shot * 3) < instance->common.te_delta * 3)) {
            //Found header Nice Flor-S
            instance->common.parser_step = 3;
            instance->common.code_found = 0;
            instance->common.code_count_bit = 0;
        } else {
            instance->common.parser_step = 0;
        }
        break;
    case 3:
        if(data.level == ApiHalSubGhzCaptureLevelHigh) {
            if(DURATION_DIFF(data.duration, instance->common.te_shot * 3) < instance->common.te_delta) {
                //Found STOP bit
                instance->common.parser_step = 0;
                if(instance->common.code_count_bit >=instance->common.code_min_count_bit_for_found) {
                    //ToDo out data display
                    subghz_nice_flor_s_decoder_decrypt(instance);
                }
                break;
            } else {
                //save interval
                instance->common.te_last = data.duration;
                instance->common.parser_step = 4;
            }
        }
        break;
    case 4:
        if(data.level == ApiHalSubGhzCaptureLevelLow) {
            if((DURATION_DIFF(instance->common.te_last, instance->common.te_shot) < instance->common.te_delta) 
                &&(DURATION_DIFF(data.duration, instance->common.te_long) < instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 0);
                instance->common.parser_step = 3;
            } else if(
                (DURATION_DIFF(instance->common.te_last, instance->common.te_long) < instance->common.te_delta) 
                    &&(DURATION_DIFF(data.duration, instance->common.te_shot) < instance->common.te_delta)) {
                subghz_protocol_common_add_bit(&instance->common, 1);
                instance->common.parser_step = 3;
            } else
                instance->common.parser_step = 0;
        } else {
            instance->common.parser_step = 0;
        }
        break;
    }
}

void subghz_protocol_nice_flor_s_to_str(SubGhzProtocolNiceFlorS* instance, string_t output) {
    uint32_t code_found_hi = instance->common.code_found >> 32;
    uint32_t code_found_lo = instance->common.code_found & 0x00000000ffffffff;

    string_cat_printf(
        output,
        "Protocol %s, %d Bit\r\n"
        " KEY:0x%lX%08lX\r\n"
        " SN:%05lX\r\n"
        " CNT:%04X BTN:%02lX\r\n",
        instance->common.name,
        instance->common.code_count_bit,
        code_found_hi,
        code_found_lo,
        instance->common.serial,
        instance->common.cnt,
        instance->common.btn
    );
}