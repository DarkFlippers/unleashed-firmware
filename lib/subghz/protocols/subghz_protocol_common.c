#include "subghz_protocol_common.h"
#include <stdio.h>
#include <lib/toolbox/hex.h>

SubGhzProtocolCommonEncoder* subghz_protocol_encoder_common_alloc() {
    SubGhzProtocolCommonEncoder* instance = furi_alloc(sizeof(SubGhzProtocolCommonEncoder));
    instance->upload = furi_alloc(SUBGHZ_ENCODER_UPLOAD_MAX_SIZE * sizeof(LevelDuration));
    instance->start = true;
    instance->repeat = 10; //default number of repeat
    return instance;
}

void subghz_protocol_encoder_common_free(SubGhzProtocolCommonEncoder* instance) {
    furi_assert(instance);
    if(instance->callback_end) {
        instance->callback_end((SubGhzProtocolCommon*)instance->context_end);
    }
    free(instance->upload);
    free(instance);
}

size_t subghz_encoder_common_get_repeat_left(SubGhzProtocolCommonEncoder* instance) {
    furi_assert(instance);
    return instance->repeat;
}

void subghz_protocol_encoder_common_set_callback(
    SubGhzProtocolCommonEncoder* instance,
    SubGhzProtocolCommonEncoderCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);
    instance->callback = callback;
    instance->context = context;
}

void subghz_protocol_encoder_common_set_callback_end(
    SubGhzProtocolCommonEncoder* instance,
    SubGhzProtocolCommonEncoderCallbackEnd callback_end,
    void* context_end) {
    furi_assert(instance);
    furi_assert(callback_end);
    instance->callback_end = callback_end;
    instance->context_end = context_end;
}

LevelDuration subghz_protocol_encoder_common_yield(void* context) {
    SubGhzProtocolCommonEncoder* instance = context;

    if(instance->callback) {
        return instance->callback((SubGhzProtocolCommon*)instance->context);
    }

    if(instance->repeat == 0) {
        return level_duration_reset();
    }

    LevelDuration ret = instance->upload[instance->front];

    if(++instance->front == instance->size_upload) {
        instance->repeat--;
        instance->front = 0;
    }

    return ret;
}

void subghz_protocol_common_add_bit(SubGhzProtocolCommon* common, uint8_t bit) {
    common->code_found = common->code_found << 1 | bit;
    common->code_count_bit++;
}

bool subghz_protocol_common_check_interval(
    SubGhzProtocolCommon* common,
    uint32_t duration,
    uint16_t duration_check) {
    if((duration_check >= (duration - common->te_delta)) &&
       (duration_check <= (duration + common->te_delta))) {
        return true;
    } else {
        return false;
    }
}

uint64_t subghz_protocol_common_reverse_key(uint64_t key, uint8_t count_bit) {
    uint64_t key_reverse = 0;
    for(uint8_t i = 0; i < count_bit; i++) {
        key_reverse = key_reverse << 1 | bit_read(key, i);
    }
    return key_reverse;
}

void subghz_protocol_common_set_callback(
    SubGhzProtocolCommon* common,
    SubGhzProtocolCommonCallback callback,
    void* context) {
    common->callback = callback;
    common->context = context;
}

void subghz_protocol_common_to_str(SubGhzProtocolCommon* instance, string_t output) {
    if(instance->to_string) {
        instance->to_string(instance, output);
    } else {
        uint32_t code_found_hi = instance->code_found >> 32;
        uint32_t code_found_lo = instance->code_found & 0x00000000ffffffff;

        uint64_t code_found_reverse =
            subghz_protocol_common_reverse_key(instance->code_found, instance->code_count_bit);

        uint32_t code_found_reverse_hi = code_found_reverse >> 32;
        uint32_t code_found_reverse_lo = code_found_reverse & 0x00000000ffffffff;

        if(code_found_hi > 0) {
            string_cat_printf(
                output,
                "Protocol %s, %d Bit\r\n"
                " KEY:0x%lX%08lX\r\n"
                " YEK:0x%lX%08lX\r\n"
                " SN:0x%05lX BTN:%02X\r\n",
                instance->name,
                instance->code_count_bit,
                code_found_hi,
                code_found_lo,
                code_found_reverse_hi,
                code_found_reverse_lo,
                instance->serial,
                instance->btn);
        } else {
            string_cat_printf(
                output,
                "Protocol %s, %d Bit\r\n"
                " KEY:0x%lX%lX\r\n"
                " YEK:0x%lX%lX\r\n"
                " SN:0x%05lX BTN:%02X\r\n",
                instance->name,
                instance->code_count_bit,
                code_found_hi,
                code_found_lo,
                code_found_reverse_hi,
                code_found_reverse_lo,
                instance->serial,
                instance->btn);
        }
    }
}

bool subghz_protocol_common_read_hex(string_t str, uint8_t* buff, uint16_t len) {
    string_strim(str);
    uint8_t nibble_high = 0;
    uint8_t nibble_low = 0;
    bool parsed = true;

    for(uint16_t i = 0; i < len; i++) {
        if(hex_char_to_hex_nibble(string_get_char(str, 0), &nibble_high) &&
           hex_char_to_hex_nibble(string_get_char(str, 1), &nibble_low)) {
            buff[i] = (nibble_high << 4) | nibble_low;
            if(string_size(str) > 2) {
                string_right(str, 2);
            } else if(i < len - 1) {
                parsed = false;
                break;
            };
        } else {
            parsed = false;
            break;
        }
    }
    return parsed;
}
