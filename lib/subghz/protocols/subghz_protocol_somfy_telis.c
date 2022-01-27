#include "subghz_protocol_somfy_telis.h"
#include "subghz_protocol_common.h"
#include <lib/toolbox/manchester_decoder.h>

#define TAG "SubGhzSomfyTelis"

struct SubGhzProtocolSomfyTelis {
    SubGhzProtocolCommon common;
    ManchesterState manchester_saved_state;
};

typedef enum {
    SomfyTelisDecoderStepReset = 0,
    SomfyTelisDecoderStepCheckPreambula,
    SomfyTelisDecoderStepFoundPreambula,
    SomfyTelisDecoderStepStartDecode,
    SomfyTelisDecoderStepDecoderData,
} SomfyTelisDecoderStep;

SubGhzProtocolSomfyTelis* subghz_protocol_somfy_telis_alloc() {
    SubGhzProtocolSomfyTelis* instance = furi_alloc(sizeof(SubGhzProtocolSomfyTelis));

    instance->common.name = "Somfy Telis";
    instance->common.code_min_count_bit_for_found = 56;
    instance->common.te_short = 640;
    instance->common.te_long = 1280;
    instance->common.te_delta = 250;
    instance->common.type_protocol = SubGhzProtocolCommonTypeDynamic;
    instance->common.to_string = (SubGhzProtocolCommonToStr)subghz_protocol_somfy_telis_to_str;
    instance->common.to_load_protocol =
        (SubGhzProtocolCommonLoadFromRAW)subghz_decoder_somfy_telis_to_load_protocol;

    return instance;
}

void subghz_protocol_somfy_telis_free(SubGhzProtocolSomfyTelis* instance) {
    furi_assert(instance);
    free(instance);
}

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolSomfyTelis instance
 */
void subghz_protocol_somfy_telis_remote_controller(SubGhzProtocolSomfyTelis* instance) {
    //https://pushstack.wordpress.com/somfy-rts-protocol/
    /*
 *                                                  604 us
 *                                                  /
 *  | 2416us | 2416us | 2416us | 2416us | 4550 us |  | 67648 us |  30415 us  |
 *  
 *  +--------+        +--------+        +---...---+
 *  +        +--------+        +--------+         +--+XXXX...XXX+-----...-----
 *  
 *  |              hw. sync.            |   soft.    |          | Inter-frame
 *  |                                   |   sync.    |   data   |     gap
 *  
 * 
 *     encrypt              |           decrypt
 *  
 *  package 56 bit    cnt    key  btn|crc    cnt     serial
 *  0xA7232323312222 - 0   => A7    8 0   | 00 00 | 12 13 00
 *  0xA7222223312222 - 1   => A7    8 5   | 00 01 | 12 13 00
 *  0xA7212123312222 - 2   => A7    8 6   | 00 02 | 12 13 00
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
 *      Btn
 *
 *     Value  Button(s)       Description
 *      0x1     My          Stop or move to favourite position
 *      0x2     Up          Move up
 *      0x3     My + Up     Set upper motor limit in initial programming mode
 *      0x4     Down        Move down
 *      0x5     My + Down   Set lower motor limit in initial programming mode
 *      0x6     Up + Down   Change motor limit and initial programming mode
 *      0x8     Prog        Used for (de-)registering remotes, see below
 *      0x9     Sun + Flag  Enable sun and wind detector (SUN and FLAG symbol on the Telis Soliris RC)
 *      0xA     Flag        Disable sun detector (FLAG symbol on the Telis Soliris RC)
 * 
 *      CRC
 * 
 *      uint8_t frame[7];
 *      for (i=0; i < 7; i++) {
 *          cksum = cksum ^ frame[i] ^ (frame[i] >> 4);
 *      }
 *      cksum = cksum & 0xf;
 *
 */

    uint64_t data = instance->common.code_last_found ^ (instance->common.code_last_found >> 8);
    instance->common.btn = (data >> 44) & 0xF;
    instance->common.cnt = (data >> 24) & 0xFFFF;
    instance->common.serial = data & 0xFFFFFF;
}

uint8_t subghz_protocol_somfy_telis_crc(uint64_t data) {
    uint8_t crc = 0;
    data &= 0xFFF0FFFFFFFFFF;
    for(uint8_t i = 0; i < 56; i += 8) {
        crc = crc ^ data >> i ^ (data >> (i + 4));
    }
    return crc & 0xf;
}

const char* subghz_protocol_somfy_telis_get_name_button(uint8_t btn) {
    const char* name_btn[0x10] = {
        "Unknown",
        "My",
        "Up",
        "My+Up",
        "Down",
        "My+Down",
        "Up+Down",
        "0x07",
        "Prog",
        "Sun+Flag",
        "Flag",
        "0x0B",
        "0x0C",
        "0x0D",
        "0x0E",
        "0x0F"};
    return btn <= 0xf ? name_btn[btn] : name_btn[0];
}

void subghz_protocol_somfy_telis_reset(SubGhzProtocolSomfyTelis* instance) {
    instance->common.parser_step = SomfyTelisDecoderStepReset;
    manchester_advance(
        instance->manchester_saved_state,
        ManchesterEventReset,
        &instance->manchester_saved_state,
        NULL);
}

void subghz_protocol_somfy_telis_parse(
    SubGhzProtocolSomfyTelis* instance,
    bool level,
    uint32_t duration) {
    ManchesterEvent event = ManchesterEventReset;
    switch(instance->common.parser_step) {
    case SomfyTelisDecoderStepReset:
        if((level) && DURATION_DIFF(duration, instance->common.te_short * 4) <
                          instance->common.te_delta * 4) {
            instance->common.parser_step = SomfyTelisDecoderStepFoundPreambula;
            instance->common.header_count++;
        }
        break;
    case SomfyTelisDecoderStepFoundPreambula:
        if((!level) && (DURATION_DIFF(duration, instance->common.te_short * 4) <
                        instance->common.te_delta * 4)) {
            instance->common.parser_step = SomfyTelisDecoderStepCheckPreambula;
        } else {
            instance->common.header_count = 0;
            instance->common.parser_step = SomfyTelisDecoderStepReset;
        }
        break;
    case SomfyTelisDecoderStepCheckPreambula:
        if(level) {
            if(DURATION_DIFF(duration, instance->common.te_short * 4) <
               instance->common.te_delta * 4) {
                instance->common.parser_step = SomfyTelisDecoderStepFoundPreambula;
                instance->common.header_count++;
            } else if(
                (instance->common.header_count > 1) &&
                (DURATION_DIFF(duration, instance->common.te_short * 7) <
                 instance->common.te_delta * 4)) {
                instance->common.parser_step = SomfyTelisDecoderStepDecoderData;
                instance->common.code_found = 0;
                instance->common.code_count_bit = 0;
                instance->common.header_count = 0;
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

    case SomfyTelisDecoderStepDecoderData:
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
                    if(((data_tmp >> 40) & 0xF) == subghz_protocol_somfy_telis_crc(data_tmp)) {
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
                instance->common.parser_step = SomfyTelisDecoderStepReset;
            } else {
                instance->common.parser_step = SomfyTelisDecoderStepReset;
            }
        } else {
            if(DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta) {
                event = ManchesterEventShortHigh;
            } else if(DURATION_DIFF(duration, instance->common.te_long) < instance->common.te_delta) {
                event = ManchesterEventLongHigh;
            } else {
                instance->common.parser_step = SomfyTelisDecoderStepReset;
            }
        }
        if(event != ManchesterEventReset) {
            bool data;
            bool data_ok = manchester_advance(
                instance->manchester_saved_state, event, &instance->manchester_saved_state, &data);

            if(data_ok) {
                instance->common.code_found = (instance->common.code_found << 1) | data;
                instance->common.code_count_bit++;
            }
        }
        break;
    }
}

void subghz_protocol_somfy_telis_to_str(SubGhzProtocolSomfyTelis* instance, string_t output) {
    subghz_protocol_somfy_telis_remote_controller(instance);
    uint32_t code_found_hi = instance->common.code_last_found >> 32;
    uint32_t code_found_lo = instance->common.code_last_found & 0x00000000ffffffff;

    string_cat_printf(
        output,
        "%s %db\r\n"
        "Key:0x%lX%08lX\r\n"
        "Sn:0x%06lX \r\n"
        "Cnt:0x%04X\r\n"
        "Btn:%s\r\n",

        instance->common.name,
        instance->common.code_last_count_bit,
        code_found_hi,
        code_found_lo,
        instance->common.serial,
        instance->common.cnt,
        subghz_protocol_somfy_telis_get_name_button(instance->common.btn));
}

void subghz_decoder_somfy_telis_to_load_protocol(SubGhzProtocolSomfyTelis* instance, void* context) {
    furi_assert(context);
    furi_assert(instance);
    SubGhzProtocolCommonLoad* data = context;
    instance->common.code_last_found = data->code_found;
    instance->common.code_last_count_bit = data->code_count_bit;
    subghz_protocol_somfy_telis_remote_controller(instance);
}
