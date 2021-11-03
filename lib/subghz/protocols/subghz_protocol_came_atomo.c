#include "subghz_protocol_came_atomo.h"
#include "subghz_protocol_common.h"
#include <lib/toolbox/manchester-decoder.h>
#include "../subghz_keystore.h"

#define SUBGHZ_NO_CAME_ATOMO_RAINBOW_TABLE 0xFFFFFFFFFFFFFFFF

struct SubGhzProtocolCameAtomo {
    SubGhzProtocolCommon common;
    ManchesterState manchester_saved_state;
    const char* rainbow_table_file_name;
};

typedef enum {
    CameAtomoDecoderStepReset = 0,
    CameAtomoDecoderStepDecoderData,
} CameAtomoDecoderStep;

SubGhzProtocolCameAtomo* subghz_protocol_came_atomo_alloc() {
    SubGhzProtocolCameAtomo* instance = furi_alloc(sizeof(SubGhzProtocolCameAtomo));

    instance->common.name = "CAME Atomo";
    instance->common.code_min_count_bit_for_found = 62;
    instance->common.te_short = 600;
    instance->common.te_long = 1200;
    instance->common.te_delta = 250;
    instance->common.type_protocol = SubGhzProtocolCommonTypeStatic;
    instance->common.to_string = (SubGhzProtocolCommonToStr)subghz_protocol_came_atomo_to_str;
    instance->common.to_load_protocol =
        (SubGhzProtocolCommonLoadFromRAW)subghz_decoder_came_atomo_to_load_protocol;

    return instance;
}

void subghz_protocol_came_atomo_free(SubGhzProtocolCameAtomo* instance) {
    furi_assert(instance);
    free(instance);
}

void subghz_protocol_came_atomo_name_file(SubGhzProtocolCameAtomo* instance, const char* name) {
    instance->rainbow_table_file_name = name;
    printf("Loading CAME Atomo rainbow table %s\r\n", name);
}

/** Read bytes from rainbow table
 * 
 * @param instance - SubGhzProtocolCameAtomo* instance
 * @param number_atomo_magic_xor
 * @return atomo_magic_xor
 */
uint64_t subghz_came_atomo_get_atomo_magic_xor_in_file(
    SubGhzProtocolCameAtomo* instance,
    uint8_t number_atomo_magic_xor) {
    if(!strcmp(instance->rainbow_table_file_name, "")) return SUBGHZ_NO_CAME_ATOMO_RAINBOW_TABLE;

    uint8_t buffer[sizeof(uint64_t)] = {0};
    uint32_t address = number_atomo_magic_xor * sizeof(uint64_t);
    uint64_t atomo_magic_xor = 0;

    if(subghz_keystore_raw_get_data(
           instance->rainbow_table_file_name, address, buffer, sizeof(uint64_t))) {
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            atomo_magic_xor = (atomo_magic_xor << 8) | buffer[i];
        }
    } else {
        atomo_magic_xor = SUBGHZ_NO_CAME_ATOMO_RAINBOW_TABLE;
    }
    return atomo_magic_xor;
}

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolCameAtomo instance
 */
void subghz_protocol_came_atomo_remote_controller(SubGhzProtocolCameAtomo* instance) {
    /* 
    * 0x1fafef3ed0f7d9ef
    * 0x185fcc1531ee86e7
    * 0x184fa96912c567ff
    * 0x187f8a42f3dc38f7
    * 0x186f63915492a5cd
    * 0x181f40bab58bfac5
    * 0x180f25c696a01bdd
    * 0x183f06ed77b944d5
    * 0x182ef661d83d21a9
    * 0x18ded54a39247ea1
    * 0x18ceb0361a0f9fb9
    * 0x18fe931dfb16c0b1
    * 0x18ee7ace5c585d8b
    * ........ 
    * transmission consists of 99 parcels with increasing counter while holding down the button
    * with each new press, the counter in the encrypted part increases
    * 
    * 0x1FAFF13ED0F7D9EF
    * 0x1FAFF11ED0F7D9EF
    * 0x1FAFF10ED0F7D9EF
    * 0x1FAFF0FED0F7D9EF
    * 0x1FAFF0EED0F7D9EF
    * 0x1FAFF0DED0F7D9EF
    * 0x1FAFF0CED0F7D9EF
    * 0x1FAFF0BED0F7D9EF
    * 0x1FAFF0AED0F7D9EF 
    * 
    *                   where     0x1FAF - parcel counter, 0хF0A - button press counter,
    *                           0xED0F7D9E - serial number, 0хF -  key
    * 0x1FAF parcel counter - 1 in the parcel queue ^ 0x185F =  0x07F0
    * 0x185f ^ 0x185F = 0x0000
    * 0x184f ^ 0x185F = 0x0010
    * 0x187f ^ 0x185F = 0x0020
    * .....
    * 0x182e ^ 0x185F = 0x0071 
    * 0x18de ^ 0x185F = 0x0081
    * .....
    * 0x1e43 ^ 0x185F = 0x061C
    *                           where the last nibble is incremented every 8 samples
    * 
    * Decode
    * 
    * 0x1cf6931dfb16c0b1 => 0x1cf6
    * 0x1cf6 ^ 0x185F = 0x04A9
    * 0x04A9 => 0x04A = 74 (dec)
    * 74+1 % 32(atomo_magic_xor) = 11
    * GET atomo_magic_xor[11] = 0xXXXXXXXXXXXXXXXX
    * 0x931dfb16c0b1 ^ 0xXXXXXXXXXXXXXXXX =  0xEF3ED0F7D9EF
    * 0xEF3 ED0F7D9E F  => 0xEF3 - CNT, 0xED0F7D9E - SN, 0xF - key
    * 
    * */

    uint16_t parcel_counter = instance->common.code_last_found >> 48;
    parcel_counter = parcel_counter ^ 0x185F;
    parcel_counter >>= 4;
    uint8_t ind = (parcel_counter + 1) % 32;
    uint64_t temp_data = instance->common.code_last_found & 0x0000FFFFFFFFFFFF;
    uint64_t atomo_magic_xor = subghz_came_atomo_get_atomo_magic_xor_in_file(instance, ind);

    if(atomo_magic_xor != SUBGHZ_NO_CAME_ATOMO_RAINBOW_TABLE) {
        temp_data = temp_data ^ atomo_magic_xor;
        instance->common.cnt = temp_data >> 36;
        instance->common.serial = (temp_data >> 4) & 0x000FFFFFFFF;
        instance->common.btn = temp_data & 0xF;
    } else {
        instance->common.cnt = 0;
        instance->common.serial = 0;
        instance->common.btn = 0;
    }
}

void subghz_protocol_came_atomo_reset(SubGhzProtocolCameAtomo* instance) {
    instance->common.parser_step = CameAtomoDecoderStepReset;
    manchester_advance(
        instance->manchester_saved_state,
        ManchesterEventReset,
        &instance->manchester_saved_state,
        NULL);
}

void subghz_protocol_came_atomo_parse(
    SubGhzProtocolCameAtomo* instance,
    bool level,
    uint32_t duration) {
    ManchesterEvent event = ManchesterEventReset;
    switch(instance->common.parser_step) {
    case CameAtomoDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, instance->common.te_long * 65) <
                        instance->common.te_delta * 20)) {
            //Found header CAME
            instance->common.parser_step = CameAtomoDecoderStepDecoderData;
            instance->common.code_found = 0;
            instance->common.code_count_bit = 1;
            manchester_advance(
                instance->manchester_saved_state,
                ManchesterEventReset,
                &instance->manchester_saved_state,
                NULL);
            manchester_advance(
                instance->manchester_saved_state,
                ManchesterEventShortLow,
                &instance->manchester_saved_state,
                NULL);
        } else {
            instance->common.parser_step = CameAtomoDecoderStepReset;
        }
        break;
    case CameAtomoDecoderStepDecoderData:
        if(!level) {
            if(DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta) {
                event = ManchesterEventShortLow;
            } else if(DURATION_DIFF(duration, instance->common.te_long) < instance->common.te_delta) {
                event = ManchesterEventLongLow;
            } else if(duration >= (instance->common.te_long * 2 + instance->common.te_delta)) {
                if(instance->common.code_count_bit ==
                   instance->common.code_min_count_bit_for_found) {
                    instance->common.code_last_found = instance->common.code_found;
                    instance->common.code_last_count_bit = instance->common.code_count_bit;
                    if(instance->common.callback)
                        instance->common.callback(
                            (SubGhzProtocolCommon*)instance, instance->common.context);
                }
                instance->common.code_found = 0;
                instance->common.code_count_bit = 1;
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventReset,
                    &instance->manchester_saved_state,
                    NULL);
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventShortLow,
                    &instance->manchester_saved_state,
                    NULL);
            } else {
                instance->common.parser_step = CameAtomoDecoderStepReset;
            }
        } else {
            if(DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta) {
                event = ManchesterEventShortHigh;
            } else if(DURATION_DIFF(duration, instance->common.te_long) < instance->common.te_delta) {
                event = ManchesterEventLongHigh;
            } else {
                instance->common.parser_step = CameAtomoDecoderStepReset;
            }
        }
        if(event != ManchesterEventReset) {
            bool data;
            bool data_ok = manchester_advance(
                instance->manchester_saved_state, event, &instance->manchester_saved_state, &data);

            if(data_ok) {
                instance->common.code_found = (instance->common.code_found << 1) | !data;
                instance->common.code_count_bit++;
            }
        }
        break;
    }
}
void subghz_protocol_came_atomo_to_str(SubGhzProtocolCameAtomo* instance, string_t output) {
    subghz_protocol_came_atomo_remote_controller(instance);
    uint32_t code_found_hi = instance->common.code_last_found >> 32;
    uint32_t code_found_lo = instance->common.code_last_found & 0x00000000ffffffff;

    string_cat_printf(
        output,
        "%s %db\r\n"
        "Key:0x%lX%08lX\r\n"
        "Sn:0x%08lX  Btn:0x%01X\r\n"
        "Cnt:0x%03X\r\n",

        instance->common.name,
        instance->common.code_last_count_bit,
        code_found_hi,
        code_found_lo,
        instance->common.serial,
        instance->common.btn,
        instance->common.cnt);
}

void subghz_decoder_came_atomo_to_load_protocol(SubGhzProtocolCameAtomo* instance, void* context) {
    furi_assert(context);
    furi_assert(instance);
    SubGhzProtocolCommonLoad* data = context;
    instance->common.code_last_found = data->code_found;
    instance->common.code_last_count_bit = data->code_count_bit;
    subghz_protocol_came_atomo_remote_controller(instance);
}
