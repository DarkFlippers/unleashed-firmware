#include "subghz_protocol_came_atomo.h"
#include "subghz_protocol_common.h"
#include <lib/toolbox/manchester-decoder.h>

struct SubGhzProtocolCameAtomo {
    SubGhzProtocolCommon common;
    ManchesterState manchester_saved_state;
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
    // instance->common.to_save_string =
    //     (SubGhzProtocolCommonGetStrSave)subghz_protocol_came_atomo_to_save_str;
    //instance->common.to_load_protocol_from_file =
    //    (SubGhzProtocolCommonLoadFromFile)subghz_protocol_came_atomo_to_load_protocol_from_file;
    instance->common.to_load_protocol =
        (SubGhzProtocolCommonLoadFromRAW)subghz_decoder_came_atomo_to_load_protocol;
    // instance->common.get_upload_protocol =
    //     (SubGhzProtocolCommonEncoderGetUpLoad)subghz_protocol_came_atomo_send_key;

    return instance;
}

void subghz_protocol_came_atomo_free(SubGhzProtocolCameAtomo* instance) {
    furi_assert(instance);
    free(instance);
}

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolCameAtomo instance
 */
void subghz_protocol_came_atomo_remote_controller(SubGhzProtocolCameAtomo* instance) {
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
                if(instance->common.code_count_bit >=
                   instance->common.code_min_count_bit_for_found) {
                    instance->common.code_last_found = instance->common.code_found;
                    instance->common.code_last_count_bit = instance->common.code_count_bit;
                    // uint32_t code_found_hi = instance->common.code_last_found >> 32;
                    // uint32_t code_found_lo = instance->common.code_last_found & 0x00000000ffffffff;

                    // uint64_t code_found_reverse = subghz_protocol_common_reverse_key(
                    //     instance->common.code_last_found, instance->common.code_last_count_bit);

                    // uint32_t code_found_reverse_hi = code_found_reverse >> 32;
                    // uint32_t code_found_reverse_lo = code_found_reverse & 0x00000000ffffffff;
                    // FURI_LOG_I(
                    //     "ATOMO",
                    //     "%08lX%08lX  %08lX%08lX  %d",
                    //     code_found_hi,
                    //     code_found_lo,
                    //     code_found_reverse_hi,
                    //     code_found_reverse_lo,
                    //     instance->common.code_last_count_bit);
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
    uint32_t code_found_hi = instance->common.code_last_found >> 32;
    uint32_t code_found_lo = instance->common.code_last_found & 0x00000000ffffffff;

    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%lX%08lX\r\n",
        instance->common.name,
        instance->common.code_last_count_bit,
        code_found_hi,
        code_found_lo);
}

// void subghz_protocol_came_atomo_to_save_str(SubGhzProtocolCameAtomo* instance, string_t output) {
//     string_printf(
//         output,
//         "Protocol: %s\n"
//         "Bit: %d\n"
//         "Key: %08lX%08lX\r\n",
//         instance->common.name,
//         instance->common.code_last_count_bit,
//         (uint32_t)(instance->common.code_last_found >> 32),
//         (uint32_t)(instance->common.code_last_found & 0xFFFFFFFF));
// }

// bool subghz_protocol_came_atomo_to_load_protocol_from_file(
//     FileWorker* file_worker,
//     SubGhzProtocolCameAtomo* instance) {
//     bool loaded = false;
//     string_t temp_str;
//     string_init(temp_str);
//     int res = 0;
//     int data = 0;

//     do {
//         // Read and parse bit data from 2nd line
//         if(!file_worker_read_until(file_worker, temp_str, '\n')) {
//             break;
//         }
//         res = sscanf(string_get_cstr(temp_str), "Bit: %d\n", &data);
//         if(res != 1) {
//             break;
//         }
//         instance->common.code_last_count_bit = (uint8_t)data;

//         // Read and parse key data from 3nd line
//         if(!file_worker_read_until(file_worker, temp_str, '\n')) {
//             break;
//         }
//         // strlen("Key: ") = 5
//         string_right(temp_str, 5);

//         uint8_t buf_key[8] = {0};
//         if(!subghz_protocol_common_read_hex(temp_str, buf_key, 8)) {
//             break;
//         }

//         for(uint8_t i = 0; i < 8; i++) {
//             instance->common.code_last_found = instance->common.code_last_found << 8 | buf_key[i];
//         }

//         loaded = true;
//     } while(0);

//     string_clear(temp_str);

//     subghz_protocol_came_atomo_remote_controller(instance);
//     return loaded;
// }

void subghz_decoder_came_atomo_to_load_protocol(SubGhzProtocolCameAtomo* instance, void* context) {
    furi_assert(context);
    furi_assert(instance);
    SubGhzProtocolCommonLoad* data = context;
    instance->common.code_last_found = data->code_found;
    instance->common.code_last_count_bit = data->code_count_bit;
    subghz_protocol_came_atomo_remote_controller(instance);
}
