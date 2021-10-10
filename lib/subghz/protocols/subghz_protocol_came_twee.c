#include "subghz_protocol_came_twee.h"
#include "subghz_protocol_common.h"
#include <lib/toolbox/manchester-decoder.h>
#include <lib/toolbox/manchester-encoder.h>

/*
 * Help
 * https://phreakerclub.com/forum/showthread.php?t=635&highlight=came+twin
 *
 */

#define DIP_PATTERN "%c%c%c%c%c%c%c%c%c%c"
#define CNT_TO_DIP(dip)                                                                     \
    (dip & 0x0200 ? '1' : '0'), (dip & 0x0100 ? '1' : '0'), (dip & 0x0080 ? '1' : '0'),     \
        (dip & 0x0040 ? '1' : '0'), (dip & 0x0020 ? '1' : '0'), (dip & 0x0010 ? '1' : '0'), \
        (dip & 0x0008 ? '1' : '0'), (dip & 0x0004 ? '1' : '0'), (dip & 0x0002 ? '1' : '0'), \
        (dip & 0x0001 ? '1' : '0')

struct SubGhzProtocolCameTwee {
    SubGhzProtocolCommon common;
    ManchesterState manchester_saved_state;
};

typedef enum {
    CameTweeDecoderStepReset = 0,
    CameTweeDecoderStepDecoderData,
} CameTweeDecoderStep;

SubGhzProtocolCameTwee* subghz_protocol_came_twee_alloc() {
    SubGhzProtocolCameTwee* instance = furi_alloc(sizeof(SubGhzProtocolCameTwee));

    instance->common.name = "CAME TWEE";
    instance->common.code_min_count_bit_for_found = 54;
    instance->common.te_short = 500;
    instance->common.te_long = 1000;
    instance->common.te_delta = 250;
    instance->common.type_protocol = SubGhzProtocolCommonTypeStatic;
    instance->common.to_string = (SubGhzProtocolCommonToStr)subghz_protocol_came_twee_to_str;
    instance->common.to_save_string =
        (SubGhzProtocolCommonGetStrSave)subghz_protocol_came_twee_to_save_str;
    instance->common.to_load_protocol_from_file =
        (SubGhzProtocolCommonLoadFromFile)subghz_protocol_came_twee_to_load_protocol_from_file;
    instance->common.to_load_protocol =
        (SubGhzProtocolCommonLoadFromRAW)subghz_decoder_came_twee_to_load_protocol;
    instance->common.get_upload_protocol =
        (SubGhzProtocolCommonEncoderGetUpLoad)subghz_protocol_came_twee_send_key;

    return instance;
}

void subghz_protocol_came_twee_free(SubGhzProtocolCameTwee* instance) {
    furi_assert(instance);
    free(instance);
}

LevelDuration subghz_protocol_came_twee_add_duration_to_upload(
    SubGhzProtocolCameTwee* instance,
    ManchesterEncoderResult result) {
    LevelDuration data;
    switch(result) {
    case ManchesterEncoderResultShortLow:
        data.duration = instance->common.te_short;
        data.level = false;
        break;
    case ManchesterEncoderResultLongLow:
        data.duration = instance->common.te_long;
        data.level = false;
        break;
    case ManchesterEncoderResultLongHigh:
        data.duration = instance->common.te_long;
        data.level = true;
        break;
    case ManchesterEncoderResultShortHigh:
        data.duration = instance->common.te_short;
        data.level = true;
        break;

    default:
        printf("DO CRASH HERE\r\n");
        // furi_crash
        break;
    }
    return level_duration_make(data.level, data.duration);
}

bool subghz_protocol_came_twee_send_key(
    SubGhzProtocolCameTwee* instance,
    SubGhzProtocolCommonEncoder* encoder) {
    furi_assert(instance);
    furi_assert(encoder);
    const uint32_t magic_numbers_xor[15] = {
        0x0E0E0E00,
        0x1D1D1D11,
        0x2C2C2C22,
        0x3B3B3B33,
        0x4A4A4A44,
        0x59595955,
        0x68686866,
        0x77777777,
        0x86868688,
        0x95959599,
        0xA4A4A4AA,
        0xB3B3B3BB,
        0xC2C2C2CC,
        0xD1D1D1DD,
        0xE0E0E0EE,
    };

    size_t index = 0;
    ManchesterEncoderState enc_state;
    manchester_encoder_reset(&enc_state);
    ManchesterEncoderResult result;

    // encoder->size_upload = (instance->common.code_last_count_bit * 2) + 2;
    // if(encoder->size_upload > SUBGHZ_ENCODER_UPLOAD_MAX_SIZE) return false;

    uint64_t temp_parcel = 0x003FFF7200000000; //parcel mask

    for(int i = 14; i >= 0; i--) {
        temp_parcel = (temp_parcel & 0xFFFFFFFF00000000) |
                      (instance->common.serial ^ magic_numbers_xor[i]);

        for(uint8_t i = instance->common.code_last_count_bit; i > 0; i--) {
            if(!manchester_encoder_advance(&enc_state, !bit_read(temp_parcel, i - 1), &result)) {
                encoder->upload[index++] =
                    subghz_protocol_came_twee_add_duration_to_upload(instance, result);
                manchester_encoder_advance(&enc_state, !bit_read(temp_parcel, i - 1), &result);
            }
            encoder->upload[index++] =
                subghz_protocol_came_twee_add_duration_to_upload(instance, result);
        }
        encoder->upload[index] = subghz_protocol_came_twee_add_duration_to_upload(
            instance, manchester_encoder_finish(&enc_state));
        if(level_duration_get_level(encoder->upload[index])) {
            index++;
        }
        encoder->upload[index++] =
            level_duration_make(false, (uint32_t)instance->common.te_long * 51);
    }
    encoder->size_upload = index;
    return true;
}

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolCameTwee instance
 */
void subghz_protocol_came_twee_remote_controller(SubGhzProtocolCameTwee* instance) {
    /*      Came Twee 54 bit, rolling code 15 parcels with
    *       a decreasing counter from 0xE to 0x0
    *       with originally coded dip switches on the console 10 bit code
    * 
    *  0x003FFF72E04A6FEE
    *  0x003FFF72D17B5EDD
    *  0x003FFF72C2684DCC
    *  0x003FFF72B3193CBB
    *  0x003FFF72A40E2BAA
    *  0x003FFF72953F1A99
    *  0x003FFF72862C0988
    *  0x003FFF7277DDF877
    *  0x003FFF7268C2E766
    *  0x003FFF7259F3D655
    *  0x003FFF724AE0C544
    *  0x003FFF723B91B433
    *  0x003FFF722C86A322
    *  0x003FFF721DB79211
    *  0x003FFF720EA48100
    * 
    *   decryption
    * the last 32 bits, do XOR by the desired number, divide the result by 4,
    * convert the first 16 bits of the resulting 32-bit number to bin and do
    * bit-by-bit mirroring, adding up to 10 bits
    * 
    * Example
    * Step 1. 0x003FFF721DB79211        => 0x1DB79211
    * Step 4. 0x1DB79211 xor 0x1D1D1D11 => 0x00AA8F00
    * Step 4. 0x00AA8F00 / 4            => 0x002AA3C0
    * Step 5. 0x002AA3C0                => 0x002A
    * Step 6. 0x002A    bin             => b101010
    * Step 7. b101010                   => b0101010000
    * Step 8. b0101010000               => (Dip) Off ON Off ON Off ON Off Off Off Off
    */

    const uint32_t magic_numbers_xor[15] = {
        0x0E0E0E00,
        0x1D1D1D11,
        0x2C2C2C22,
        0x3B3B3B33,
        0x4A4A4A44,
        0x59595955,
        0x68686866,
        0x77777777,
        0x86868688,
        0x95959599,
        0xA4A4A4AA,
        0xB3B3B3BB,
        0xC2C2C2CC,
        0xD1D1D1DD,
        0xE0E0E0EE,
    };
    uint8_t cnt_parcel = (uint8_t)(instance->common.code_last_found & 0xF);
    uint32_t data = (uint32_t)(instance->common.code_last_found & 0x0FFFFFFFF);

    data = (data ^ magic_numbers_xor[cnt_parcel]);
    instance->common.serial = data;
    data /= 4;
    instance->common.btn = (data >> 4) & 0x0F;
    data >>= 16;
    data = (uint16_t)subghz_protocol_common_reverse_key(data, 16);
    instance->common.cnt = data >> 6;
}

void subghz_protocol_came_twee_reset(SubGhzProtocolCameTwee* instance) {
    instance->common.parser_step = CameTweeDecoderStepReset;
    manchester_advance(
        instance->manchester_saved_state,
        ManchesterEventReset,
        &instance->manchester_saved_state,
        NULL);
}

void subghz_protocol_came_twee_parse(
    SubGhzProtocolCameTwee* instance,
    bool level,
    uint32_t duration) {
    ManchesterEvent event = ManchesterEventReset;
    switch(instance->common.parser_step) {
    case CameTweeDecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, instance->common.te_long * 51) <
                        instance->common.te_delta * 20)) {
            //Found header CAME
            instance->common.parser_step = CameTweeDecoderStepDecoderData;
            instance->common.code_found = 0;
            instance->common.code_count_bit = 0;
            manchester_advance(
                instance->manchester_saved_state,
                ManchesterEventLongLow,
                &instance->manchester_saved_state,
                NULL);
            manchester_advance(
                instance->manchester_saved_state,
                ManchesterEventLongHigh,
                &instance->manchester_saved_state,
                NULL);
            manchester_advance(
                instance->manchester_saved_state,
                ManchesterEventShortLow,
                &instance->manchester_saved_state,
                NULL);
        } else {
            instance->common.parser_step = CameTweeDecoderStepReset;
        }
        break;
    case CameTweeDecoderStepDecoderData:
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

                    if(instance->common.callback)
                        instance->common.callback(
                            (SubGhzProtocolCommon*)instance, instance->common.context);
                }
                instance->common.code_found = 0;
                instance->common.code_count_bit = 0;
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventLongLow,
                    &instance->manchester_saved_state,
                    NULL);
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventLongHigh,
                    &instance->manchester_saved_state,
                    NULL);
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventShortLow,
                    &instance->manchester_saved_state,
                    NULL);
            } else {
                instance->common.parser_step = CameTweeDecoderStepReset;
            }
        } else {
            if(DURATION_DIFF(duration, instance->common.te_short) < instance->common.te_delta) {
                event = ManchesterEventShortHigh;
            } else if(DURATION_DIFF(duration, instance->common.te_long) < instance->common.te_delta) {
                event = ManchesterEventLongHigh;
            } else {
                instance->common.parser_step = CameTweeDecoderStepReset;
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
void subghz_protocol_came_twee_to_str(SubGhzProtocolCameTwee* instance, string_t output) {
    uint32_t code_found_hi = instance->common.code_last_found >> 32;
    uint32_t code_found_lo = instance->common.code_last_found & 0x00000000ffffffff;

    string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%lX%08lX\r\n"
        "Btn:%lX\r\n"
        "DIP:" DIP_PATTERN,
        instance->common.name,
        instance->common.code_last_count_bit,
        code_found_hi,
        code_found_lo,
        instance->common.btn,
        CNT_TO_DIP(instance->common.cnt));
}

void subghz_protocol_came_twee_to_save_str(SubGhzProtocolCameTwee* instance, string_t output) {
    string_printf(
        output,
        "Protocol: %s\n"
        "Bit: %d\n"
        "Key: %08lX%08lX\r\n",
        instance->common.name,
        instance->common.code_last_count_bit,
        (uint32_t)(instance->common.code_last_found >> 32),
        (uint32_t)(instance->common.code_last_found & 0xFFFFFFFF));
}

bool subghz_protocol_came_twee_to_load_protocol_from_file(
    FileWorker* file_worker,
    SubGhzProtocolCameTwee* instance) {
    bool loaded = false;
    string_t temp_str;
    string_init(temp_str);
    int res = 0;
    int data = 0;

    do {
        // Read and parse bit data from 2nd line
        if(!file_worker_read_until(file_worker, temp_str, '\n')) {
            break;
        }
        res = sscanf(string_get_cstr(temp_str), "Bit: %d\n", &data);
        if(res != 1) {
            break;
        }
        instance->common.code_last_count_bit = (uint8_t)data;

        // Read and parse key data from 3nd line
        if(!file_worker_read_until(file_worker, temp_str, '\n')) {
            break;
        }
        // strlen("Key: ") = 5
        string_right(temp_str, 5);

        uint8_t buf_key[8] = {0};
        if(!subghz_protocol_common_read_hex(temp_str, buf_key, 8)) {
            break;
        }

        for(uint8_t i = 0; i < 8; i++) {
            instance->common.code_last_found = instance->common.code_last_found << 8 | buf_key[i];
        }

        loaded = true;
    } while(0);

    string_clear(temp_str);

    subghz_protocol_came_twee_remote_controller(instance);
    return loaded;
}

void subghz_decoder_came_twee_to_load_protocol(SubGhzProtocolCameTwee* instance, void* context) {
    furi_assert(context);
    furi_assert(instance);
    SubGhzProtocolCommonLoad* data = context;
    instance->common.code_last_found = data->code_found;
    instance->common.code_last_count_bit = data->code_count_bit;
    subghz_protocol_came_twee_remote_controller(instance);
}
