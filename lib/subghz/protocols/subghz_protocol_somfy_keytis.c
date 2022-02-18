#include "subghz_protocol_somfy_keytis.h"
#include "subghz_protocol_common.h"
#include <lib/toolbox/manchester_decoder.h>

#define TAG "SubGhzSomfyKeytis"

struct SubGhzProtocolSomfyKeytis {
    SubGhzProtocolCommon common;
    ManchesterState manchester_saved_state;
    uint32_t press_duration_counter;
};

typedef enum {
    SomfyKeytisDecoderStepReset = 0,
    SomfyKeytisDecoderStepCheckPreambula,
    SomfyKeytisDecoderStepFoundPreambula,
    SomfyKeytisDecoderStepStartDecode,
    SomfyKeytisDecoderStepDecoderData,
} SomfyKeytisDecoderStep;

SubGhzProtocolSomfyKeytis* subghz_protocol_somfy_keytis_alloc() {
    SubGhzProtocolSomfyKeytis* instance = malloc(sizeof(SubGhzProtocolSomfyKeytis));

    instance->common.name = "Somfy Keytis";
    instance->common.code_min_count_bit_for_found = 80;
    instance->common.te_short = 640;
    instance->common.te_long = 1280;
    instance->common.te_delta = 250;
    instance->common.type_protocol = SubGhzProtocolCommonTypeDynamic;
    instance->common.to_string = (SubGhzProtocolCommonToStr)subghz_protocol_somfy_keytis_to_str;
    instance->common.to_load_protocol =
        (SubGhzProtocolCommonLoadFromRAW)subghz_decoder_somfy_keytis_to_load_protocol;

    return instance;
}

void subghz_protocol_somfy_keytis_free(SubGhzProtocolSomfyKeytis* instance) {
    furi_assert(instance);
    free(instance);
}

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolSomfyKeytis instance
 */
void subghz_protocol_somfy_keytis_remote_controller(SubGhzProtocolSomfyKeytis* instance) {
    //https://pushstack.wordpress.com/somfy-rts-protocol/
    /*
 *                                                  604 us
 *                                                  /
 *  | 2416us | 2416us | 2416us | 2416us | 4550 us |  | 
 *  
 *  +--------+        +--------+        +---...---+
 *  +        +--------+        +--------+         +--+XXXX...XXX+
 *  
 *  |              hw. sync.            |   soft.    |          |
 *  |                                   |   sync.    |   data   |
 *  
 * 
 *     encrypt              |           decrypt
 *  
 *  package 80 bit   pdc      key btn   crc cnt serial
 * 
 * 0xA453537C4B9855 C40019 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 C80026 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 CC0033 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 D00049 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 D4005C => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 D80063 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 DC0076 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 E00086 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 E40093 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 E800AC => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 EC00B9 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 F000C3 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 F400D6 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 F800E9 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 FC00FC => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 FC0102 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 FC0113 => 0xA  4  F  7 002F 37D3CD
 * 0xA453537C4B9855 FC0120 => 0xA  4  F  7 002F 37D3CD
 * ..........
 * 0xA453537C4B9855 FC048F => 0xA  4  F  7 002F 37D3CD
 *
 * Pdc: "Press Duration Counter" the total delay of the button is sent 72 parcels,
 *            pdc          cnt4b           cnt8b  pdc_crc
 *           C40019	 =>	 11 0001 00 0000 00000001 1001
 *           C80026  =>  11 0010 00 0000 00000010 0110
 *           CC0033  =>  11 0011 00 0000 00000011 0011
 *           D00049  =>  11 0100 00 0000 00000100 1001
 *           D4005C  =>  11 0101 00 0000 00000101 1100
 *           D80063  =>  11 0110 00 0000 00000110 0011
 *           DC0076  =>  11 0111 00 0000 00000111 0110
 *           E00086  =>  11 1000 00 0000 00001000 0110
 *           E40093  =>  11 1001 00 0000 00001001 0011
 *           E800AC  =>  11 1010 00 0000 00001010 1100
 *           EC00B9  =>  11 1011 00 0000 00001011 1001
 *           F000C3  =>  11 1100 00 0000 00001100 0011
 *           F400D6  =>  11 1101 00 0000 00001101 0110
 *           F800E9  =>  11 1110 00 0000 00001110 1001
 *           FC00FC  =>  11 1111 00 0000 00001111 1100
 *           FC0102  =>  11 1111 00 0000 00010000 0010
 *           FC0113  =>  11 1111 00 0000 00010001 0011
 *           FC0120  =>  11 1111 00 0000 00010010 0000
 * 
 *           Cnt4b: 4-bit counter changes from 1 to 15 then always equals 15
 *           Cnt8b: 8-bit counter changes from 1 to 72 (0x48)
 *           Ppdc_crc: 
 *                  uint8_t crc=0;
 *                  for(i=4; i<24; i+=4){
 *                      crc ^=(pdc>>i);
 *                  }
 *                  return crc;
 *               example: crc = 1^0^0^4^C = 9
 *           11, 00, 0000: const
 *          
 * Key: “Encryption Key”, Most significant 4-bit are always 0xA, Least Significant bits is 
 *      a linear counter. In the Smoove Origin this counter is increased together with the 
 *      rolling code. But leaving this on a constant value also works. Gerardwr notes that 
 *      for some other types of remotes the MSB is not constant.
 * Btn: 4-bit Control codes, this indicates the button that is pressed
 * CRC: 4-bit Checksum.
 * Ctn: 16-bit rolling code (big-endian) increased with every button press.
 * Serial: 24-bit identifier of sending device (little-endian)
 * 
 * 
 *      Decrypt
 *  
 *      uint8_t frame[7];
 *      for (i=1; i < 7; i++) {
 *          frame[i] = frame[i] ^ frame[i-1];
 *      }
 *      or
 *      uint64 Decrypt = frame ^ (frame>>8);
 * 
 *      CRC
 * 
 *      uint8_t frame[7];
 *      for (i=0; i < 7; i++) {
 *          crc = crc ^ frame[i] ^ (frame[i] >> 4);
 *      }
 *      crc = crc & 0xf;
 *
 */

    uint64_t data = instance->common.code_last_found ^ (instance->common.code_last_found >> 8);
    instance->common.btn = (data >> 48) & 0xF;
    instance->common.cnt = (data >> 24) & 0xFFFF;
    instance->common.serial = data & 0xFFFFFF;
}

uint8_t subghz_protocol_somfy_keytis_crc(uint64_t data) {
    uint8_t crc = 0;
    data &= 0xFFF0FFFFFFFFFF;
    for(uint8_t i = 0; i < 56; i += 8) {
        crc = crc ^ data >> i ^ (data >> (i + 4));
    }
    return crc & 0xf;
}
const char* subghz_protocol_somfy_keytis_get_name_button(uint8_t btn) {
    const char* name_btn[0x10] = {
        "Unknown",
        "0x01",
        "0x02",
        "Prog",
        "Key_1",
        "0x05",
        "0x06",
        "0x07",
        "0x08",
        "0x09",
        "0x0A",
        "0x0B",
        "0x0C",
        "0x0D",
        "0x0E",
        "0x0F"};
    return btn <= 0xf ? name_btn[btn] : name_btn[0];
}

uint32_t subghz_protocol_somfy_keytis_get_press_duration(void* context) {
    SubGhzProtocolSomfyKeytis* instance = context;
    return instance->press_duration_counter;
}

void subghz_protocol_somfy_keytis_reset(SubGhzProtocolSomfyKeytis* instance) {
    instance->common.parser_step = SomfyKeytisDecoderStepReset;
    manchester_advance(
        instance->manchester_saved_state,
        ManchesterEventReset,
        &instance->manchester_saved_state,
        NULL);
}

void subghz_protocol_somfy_keytis_parse(
    SubGhzProtocolSomfyKeytis* instance,
    bool level,
    uint32_t duration) {
    ManchesterEvent event = ManchesterEventReset;
    switch(instance->common.parser_step) {
    case SomfyKeytisDecoderStepReset:
        if((level) && DURATION_DIFF(duration, instance->common.te_short * 4) <
                          instance->common.te_delta * 4) {
            instance->common.parser_step = SomfyKeytisDecoderStepFoundPreambula;
            instance->common.header_count++;
        }
        break;
    case SomfyKeytisDecoderStepFoundPreambula:
        if((!level) && (DURATION_DIFF(duration, instance->common.te_short * 4) <
                        instance->common.te_delta * 4)) {
            instance->common.parser_step = SomfyKeytisDecoderStepCheckPreambula;
        } else {
            instance->common.header_count = 0;
            instance->common.parser_step = SomfyKeytisDecoderStepReset;
        }
        break;
    case SomfyKeytisDecoderStepCheckPreambula:
        if(level) {
            if(DURATION_DIFF(duration, instance->common.te_short * 4) <
               instance->common.te_delta * 4) {
                instance->common.parser_step = SomfyKeytisDecoderStepFoundPreambula;
                instance->common.header_count++;
            } else if(
                (instance->common.header_count > 1) &&
                (DURATION_DIFF(duration, instance->common.te_short * 7) <
                 instance->common.te_delta * 4)) {
                instance->common.parser_step = SomfyKeytisDecoderStepDecoderData;
                instance->common.code_found = 0;
                instance->common.code_count_bit = 0;
                instance->press_duration_counter = 0;
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventReset,
                    &instance->manchester_saved_state,
                    NULL);
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventLongHigh,
                    &instance->manchester_saved_state,
                    NULL);
            }
        }

        break;

    case SomfyKeytisDecoderStepDecoderData:
        if(!level) {
            if(DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta) {
                event = ManchesterEventShortLow;
            } else if(DURATION_DIFF(duration, instance->common.te_long) < instance->common.te_delta) {
                event = ManchesterEventLongLow;
            } else if(duration >= (instance->common.te_long + instance->common.te_delta)) {
                if(instance->common.code_count_bit ==
                   instance->common.code_min_count_bit_for_found) {
                    instance->common.code_last_found = instance->common.code_found;
                    instance->common.code_last_count_bit = instance->common.code_count_bit;

                    //check crc
                    uint64_t data_tmp = instance->common.code_last_found ^
                                        (instance->common.code_last_found >> 8);
                    if(((data_tmp >> 40) & 0xF) == subghz_protocol_somfy_keytis_crc(data_tmp)) {
                        if(instance->common.callback)
                            instance->common.callback(
                                (SubGhzProtocolCommon*)instance, instance->common.context);
                    }
                }
                instance->common.code_found = 0;
                instance->common.code_count_bit = 0;
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventReset,
                    &instance->manchester_saved_state,
                    NULL);
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventLongHigh,
                    &instance->manchester_saved_state,
                    NULL);
                instance->common.parser_step = SomfyKeytisDecoderStepReset;
            } else {
                instance->common.parser_step = SomfyKeytisDecoderStepReset;
            }
        } else {
            if(DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta) {
                event = ManchesterEventShortHigh;
            } else if(DURATION_DIFF(duration, instance->common.te_long) < instance->common.te_delta) {
                event = ManchesterEventLongHigh;
            } else {
                instance->common.parser_step = SomfyKeytisDecoderStepReset;
            }
        }
        if(event != ManchesterEventReset) {
            bool data;
            bool data_ok = manchester_advance(
                instance->manchester_saved_state, event, &instance->manchester_saved_state, &data);

            if(data_ok) {
                if(instance->common.code_count_bit < 56) {
                    instance->common.code_found = (instance->common.code_found << 1) | data;
                } else {
                    instance->press_duration_counter = (instance->press_duration_counter << 1) |
                                                       data;
                }

                instance->common.code_count_bit++;
            }
        }
        break;
    }
}

void subghz_protocol_somfy_keytis_to_str(SubGhzProtocolSomfyKeytis* instance, string_t output) {
    subghz_protocol_somfy_keytis_remote_controller(instance);
    uint32_t code_found_hi = instance->common.code_last_found >> 32;
    uint32_t code_found_lo = instance->common.code_last_found & 0x00000000ffffffff;

    string_cat_printf(
        output,
        "%s %db\r\n"
        "%lX%08lX%06lX\r\n"
        "Sn:0x%06lX \r\n"
        "Cnt:0x%04X\r\n"
        "Btn:%s\r\n",

        instance->common.name,
        instance->common.code_last_count_bit,
        code_found_hi,
        code_found_lo,
        instance->press_duration_counter,
        instance->common.serial,
        instance->common.cnt,
        subghz_protocol_somfy_keytis_get_name_button(instance->common.btn));
}

void subghz_decoder_somfy_keytis_to_load_protocol(
    SubGhzProtocolSomfyKeytis* instance,
    void* context) {
    furi_assert(context);
    furi_assert(instance);
    SubGhzProtocolCommonLoad* data = context;
    instance->common.code_last_found = data->code_found;
    instance->common.code_last_count_bit = data->code_count_bit;
    instance->press_duration_counter = data->param1;
    subghz_protocol_somfy_keytis_remote_controller(instance);
}
