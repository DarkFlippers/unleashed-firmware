#include "somfy_keytis.h"
#include <lib/toolbox/manchester_decoder.h>

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolSomfyKeytis"

static const SubGhzBlockConst subghz_protocol_somfy_keytis_const = {
    .te_short = 640,
    .te_long = 1280,
    .te_delta = 250,
    .min_count_bit_for_found = 80,
};

struct SubGhzProtocolDecoderSomfyKeytis {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint16_t header_count;
    ManchesterState manchester_saved_state;
    uint32_t press_duration_counter;
};

struct SubGhzProtocolEncoderSomfyKeytis {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    SomfyKeytisDecoderStepReset = 0,
    SomfyKeytisDecoderStepCheckPreambula,
    SomfyKeytisDecoderStepFoundPreambula,
    SomfyKeytisDecoderStepStartDecode,
    SomfyKeytisDecoderStepDecoderData,
} SomfyKeytisDecoderStep;

const SubGhzProtocolDecoder subghz_protocol_somfy_keytis_decoder = {
    .alloc = subghz_protocol_decoder_somfy_keytis_alloc,
    .free = subghz_protocol_decoder_somfy_keytis_free,

    .feed = subghz_protocol_decoder_somfy_keytis_feed,
    .reset = subghz_protocol_decoder_somfy_keytis_reset,

    .get_hash_data = subghz_protocol_decoder_somfy_keytis_get_hash_data,
    .serialize = subghz_protocol_decoder_somfy_keytis_serialize,
    .deserialize = subghz_protocol_decoder_somfy_keytis_deserialize,
    .get_string = subghz_protocol_decoder_somfy_keytis_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_somfy_keytis_encoder = {
    .alloc = NULL,
    .free = NULL,

    .deserialize = NULL,
    .stop = NULL,
    .yield = NULL,
};

const SubGhzProtocol subghz_protocol_somfy_keytis = {
    .name = SUBGHZ_PROTOCOL_SOMFY_KEYTIS_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_868 | SubGhzProtocolFlag_AM |
            SubGhzProtocolFlag_Decodable,

    .decoder = &subghz_protocol_somfy_keytis_decoder,
    .encoder = &subghz_protocol_somfy_keytis_encoder,
};

void* subghz_protocol_decoder_somfy_keytis_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderSomfyKeytis* instance = malloc(sizeof(SubGhzProtocolDecoderSomfyKeytis));
    instance->base.protocol = &subghz_protocol_somfy_keytis;
    instance->generic.protocol_name = instance->base.protocol->name;

    return instance;
}

void subghz_protocol_decoder_somfy_keytis_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyKeytis* instance = context;
    free(instance);
}

void subghz_protocol_decoder_somfy_keytis_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyKeytis* instance = context;
    instance->decoder.parser_step = SomfyKeytisDecoderStepReset;
    manchester_advance(
        instance->manchester_saved_state,
        ManchesterEventReset,
        &instance->manchester_saved_state,
        NULL);
}

/** 
 * Сhecksum calculation.
 * @param data Вata for checksum calculation
 * @return CRC
 */
static uint8_t subghz_protocol_somfy_keytis_crc(uint64_t data) {
    uint8_t crc = 0;
    data &= 0xFFF0FFFFFFFFFF;
    for(uint8_t i = 0; i < 56; i += 8) {
        crc = crc ^ data >> i ^ (data >> (i + 4));
    }
    return crc & 0xf;
}

void subghz_protocol_decoder_somfy_keytis_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyKeytis* instance = context;

    ManchesterEvent event = ManchesterEventReset;
    switch(instance->decoder.parser_step) {
    case SomfyKeytisDecoderStepReset:
        if((level) && DURATION_DIFF(duration, subghz_protocol_somfy_keytis_const.te_short * 4) <
                          subghz_protocol_somfy_keytis_const.te_delta * 4) {
            instance->decoder.parser_step = SomfyKeytisDecoderStepFoundPreambula;
            instance->header_count++;
        }
        break;
    case SomfyKeytisDecoderStepFoundPreambula:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_somfy_keytis_const.te_short * 4) <
                        subghz_protocol_somfy_keytis_const.te_delta * 4)) {
            instance->decoder.parser_step = SomfyKeytisDecoderStepCheckPreambula;
        } else {
            instance->header_count = 0;
            instance->decoder.parser_step = SomfyKeytisDecoderStepReset;
        }
        break;
    case SomfyKeytisDecoderStepCheckPreambula:
        if(level) {
            if(DURATION_DIFF(duration, subghz_protocol_somfy_keytis_const.te_short * 4) <
               subghz_protocol_somfy_keytis_const.te_delta * 4) {
                instance->decoder.parser_step = SomfyKeytisDecoderStepFoundPreambula;
                instance->header_count++;
            } else if(
                (instance->header_count > 1) &&
                (DURATION_DIFF(duration, subghz_protocol_somfy_keytis_const.te_short * 7) <
                 subghz_protocol_somfy_keytis_const.te_delta * 4)) {
                instance->decoder.parser_step = SomfyKeytisDecoderStepDecoderData;
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
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
            if(DURATION_DIFF(duration, subghz_protocol_somfy_keytis_const.te_short) <
               subghz_protocol_somfy_keytis_const.te_delta) {
                event = ManchesterEventShortLow;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_somfy_keytis_const.te_long) <
                subghz_protocol_somfy_keytis_const.te_delta) {
                event = ManchesterEventLongLow;
            } else if(
                duration >= (subghz_protocol_somfy_keytis_const.te_long +
                             subghz_protocol_somfy_keytis_const.te_delta)) {
                if(instance->decoder.decode_count_bit ==
                   subghz_protocol_somfy_keytis_const.min_count_bit_for_found) {
                    //check crc
                    uint64_t data_tmp = instance->generic.data ^ (instance->generic.data >> 8);
                    if(((data_tmp >> 40) & 0xF) == subghz_protocol_somfy_keytis_crc(data_tmp)) {
                        instance->generic.data = instance->decoder.decode_data;
                        instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                    }
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
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
                instance->decoder.parser_step = SomfyKeytisDecoderStepReset;
            } else {
                instance->decoder.parser_step = SomfyKeytisDecoderStepReset;
            }
        } else {
            if(DURATION_DIFF(duration, subghz_protocol_somfy_keytis_const.te_short) <
               subghz_protocol_somfy_keytis_const.te_delta) {
                event = ManchesterEventShortHigh;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_somfy_keytis_const.te_long) <
                subghz_protocol_somfy_keytis_const.te_delta) {
                event = ManchesterEventLongHigh;
            } else {
                instance->decoder.parser_step = SomfyKeytisDecoderStepReset;
            }
        }
        if(event != ManchesterEventReset) {
            bool data;
            bool data_ok = manchester_advance(
                instance->manchester_saved_state, event, &instance->manchester_saved_state, &data);

            if(data_ok) {
                if(instance->decoder.decode_count_bit < 56) {
                    instance->decoder.decode_data = (instance->decoder.decode_data << 1) | data;
                } else {
                    instance->press_duration_counter = (instance->press_duration_counter << 1) |
                                                       data;
                }

                instance->decoder.decode_count_bit++;
            }
        }
        break;
    }
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_somfy_keytis_check_remote_controller(SubGhzBlockGeneric* instance) {
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

    uint64_t data = instance->data ^ (instance->data >> 8);
    instance->btn = (data >> 48) & 0xF;
    instance->cnt = (data >> 24) & 0xFFFF;
    instance->serial = data & 0xFFFFFF;
}

/** 
 * Get button name.
 * @param btn Button number, 4 bit
 */
static const char* subghz_protocol_somfy_keytis_get_name_button(uint8_t btn) {
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

uint8_t subghz_protocol_decoder_somfy_keytis_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyKeytis* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit / 8) + 1);
}

bool subghz_protocol_decoder_somfy_keytis_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyKeytis* instance = context;
    bool res = subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
    if(res && !flipper_format_write_uint32(
                  flipper_format, "Duration_Counter", &instance->press_duration_counter, 1)) {
        FURI_LOG_E(TAG, "Unable to add Duration_Counter");
        res = false;
    }
    return res;
}

bool subghz_protocol_decoder_somfy_keytis_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyKeytis* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        if(instance->generic.data_count_bit !=
           subghz_protocol_somfy_keytis_const.min_count_bit_for_found) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        if(!flipper_format_read_uint32(
               flipper_format,
               "Duration_Counter",
               (uint32_t*)&instance->press_duration_counter,
               1)) {
            FURI_LOG_E(TAG, "Missing Duration_Counter");
            break;
        }
        res = true;
    } while(false);

    return res;
}

void subghz_protocol_decoder_somfy_keytis_get_string(void* context, string_t output) {
    furi_assert(context);
    SubGhzProtocolDecoderSomfyKeytis* instance = context;

    subghz_protocol_somfy_keytis_check_remote_controller(&instance->generic);

    string_cat_printf(
        output,
        "%s %db\r\n"
        "%lX%08lX%06lX\r\n"
        "Sn:0x%06lX \r\n"
        "Cnt:0x%04X\r\n"
        "Btn:%s\r\n",

        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)instance->generic.data,
        instance->press_duration_counter,
        instance->generic.serial,
        instance->generic.cnt,
        subghz_protocol_somfy_keytis_get_name_button(instance->generic.btn));
}
