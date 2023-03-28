#include "pocsag.h"

#include <inttypes.h>
#include <lib/flipper_format/flipper_format_i.h>
#include <furi/core/string.h>

#define TAG "POCSAG"

static const SubGhzBlockConst pocsag_const = {
    .te_short = 833,
    .te_delta = 100,
};

// Minimal amount of sync bits (interleaving zeros and ones)
#define POCSAG_MIN_SYNC_BITS 32
#define POCSAG_CW_BITS 32
#define POCSAG_CW_MASK 0xFFFFFFFF
#define POCSAG_FRAME_SYNC_CODE 0x7CD215D8
#define POCSAG_IDLE_CODE_WORD 0x7A89C197

#define POCSAG_FUNC_NUM 0
#define POCSAG_FUNC_ALERT1 1
#define POCSAG_FUNC_ALERT2 2
#define POCSAG_FUNC_ALPHANUM 3

static const char* func_msg[] = {"\e#Num:\e# ", "\e#Alert\e#", "\e#Alert:\e# ", "\e#Msg:\e# "};
static const char* bcd_chars = "*U -)(";

struct SubGhzProtocolDecoderPocsag {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    PCSGBlockGeneric generic;

    uint8_t codeword_idx;
    uint32_t ric;
    uint8_t func;

    // partially decoded character
    uint8_t char_bits;
    uint8_t char_data;

    // message being decoded
    FuriString* msg;

    // Done messages, ready to be serialized/deserialized
    FuriString* done_msg;
};

typedef struct SubGhzProtocolDecoderPocsag SubGhzProtocolDecoderPocsag;

typedef enum {
    PocsagDecoderStepReset = 0,
    PocsagDecoderStepFoundSync,
    PocsagDecoderStepFoundPreamble,
    PocsagDecoderStepMessage,
} PocsagDecoderStep;

void* subghz_protocol_decoder_pocsag_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);

    SubGhzProtocolDecoderPocsag* instance = malloc(sizeof(SubGhzProtocolDecoderPocsag));
    instance->base.protocol = &subghz_protocol_pocsag;
    instance->generic.protocol_name = instance->base.protocol->name;
    instance->msg = furi_string_alloc();
    instance->done_msg = furi_string_alloc();
    if(instance->generic.result_msg == NULL) {
        instance->generic.result_msg = furi_string_alloc();
    }
    if(instance->generic.result_ric == NULL) {
        instance->generic.result_ric = furi_string_alloc();
    }

    return instance;
}

void subghz_protocol_decoder_pocsag_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderPocsag* instance = context;
    furi_string_free(instance->msg);
    furi_string_free(instance->done_msg);
    free(instance);
}

void subghz_protocol_decoder_pocsag_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderPocsag* instance = context;

    instance->decoder.parser_step = PocsagDecoderStepReset;
    instance->decoder.decode_data = 0UL;
    instance->decoder.decode_count_bit = 0;
    instance->codeword_idx = 0;
    instance->char_bits = 0;
    instance->char_data = 0;
    furi_string_reset(instance->msg);
    furi_string_reset(instance->done_msg);
    furi_string_reset(instance->generic.result_msg);
    furi_string_reset(instance->generic.result_ric);
}

static void pocsag_decode_address_word(SubGhzProtocolDecoderPocsag* instance, uint32_t data) {
    instance->ric = (data >> 13);
    instance->ric = (instance->ric << 3) | (instance->codeword_idx >> 1);
    instance->func = (data >> 11) & 0b11;
}

static bool decode_message_alphanumeric(SubGhzProtocolDecoderPocsag* instance, uint32_t data) {
    for(uint8_t i = 0; i < 20; i++) {
        instance->char_data >>= 1;
        if(data & (1 << 30)) {
            instance->char_data |= 1 << 6;
        }
        instance->char_bits++;
        if(instance->char_bits == 7) {
            if(instance->char_data == 0) return false;
            furi_string_push_back(instance->msg, instance->char_data);
            instance->char_data = 0;
            instance->char_bits = 0;
        }
        data <<= 1;
    }
    return true;
}

static void decode_message_numeric(SubGhzProtocolDecoderPocsag* instance, uint32_t data) {
    // 5 groups with 4 bits each
    uint8_t val;
    for(uint8_t i = 0; i < 5; i++) {
        val = (data >> (27 - i * 4)) & 0b1111;
        // reverse the order of 4 bits
        val = (val & 0x5) << 1 | (val & 0xA) >> 1;
        val = (val & 0x3) << 2 | (val & 0xC) >> 2;

        if(val <= 9)
            val += '0';
        else
            val = bcd_chars[val - 10];
        furi_string_push_back(instance->msg, val);
    }
}

// decode message word, maintaining instance state for partial decoding. Return true if more data
// might follow or false if end of message reached.
static bool pocsag_decode_message_word(SubGhzProtocolDecoderPocsag* instance, uint32_t data) {
    switch(instance->func) {
    case POCSAG_FUNC_ALERT2:
    case POCSAG_FUNC_ALPHANUM:
        return decode_message_alphanumeric(instance, data);

    case POCSAG_FUNC_NUM:
        decode_message_numeric(instance, data);
        return true;
    }
    return false;
}

// Function called when current message got decoded, but other messages might follow
static void pocsag_message_done(SubGhzProtocolDecoderPocsag* instance) {
    // append the message to the long-term storage string
    furi_string_printf(instance->generic.result_ric, "\e#RIC: %" PRIu32 "\e# | ", instance->ric);
    furi_string_cat_str(instance->generic.result_ric, func_msg[instance->func]);
    if(instance->func != POCSAG_FUNC_ALERT1) {
        furi_string_cat(instance->done_msg, instance->msg);
    }
    furi_string_cat_str(instance->done_msg, " ");

    furi_string_cat(instance->generic.result_msg, instance->done_msg);

    // reset the state
    instance->char_bits = 0;
    instance->char_data = 0;
    furi_string_reset(instance->msg);
}

void subghz_protocol_decoder_pocsag_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderPocsag* instance = context;

    // reset state - waiting for 32 bits of interleaving 1s and 0s
    if(instance->decoder.parser_step == PocsagDecoderStepReset) {
        if(DURATION_DIFF(duration, pocsag_const.te_short) < pocsag_const.te_delta) {
            // POCSAG signals are inverted
            subghz_protocol_blocks_add_bit(&instance->decoder, !level);

            if(instance->decoder.decode_count_bit == POCSAG_MIN_SYNC_BITS) {
                instance->decoder.parser_step = PocsagDecoderStepFoundSync;
            }
        } else if(instance->decoder.decode_count_bit > 0) {
            subghz_protocol_decoder_pocsag_reset(context);
        }
        return;
    }

    int bits_count = duration / pocsag_const.te_short;
    uint32_t extra = duration - pocsag_const.te_short * bits_count;

    if(DURATION_DIFF(extra, pocsag_const.te_short) < pocsag_const.te_delta)
        bits_count++;
    else if(extra > pocsag_const.te_delta) {
        // in non-reset state we faced the error signal - we reached the end of the packet, flush data
        if(furi_string_size(instance->done_msg) > 0) {
            if(instance->base.callback)
                instance->base.callback(&instance->base, instance->base.context);
        }
        subghz_protocol_decoder_pocsag_reset(context);
        return;
    }

    uint32_t codeword;

    // handle state machine for every incoming bit
    while(bits_count-- > 0) {
        subghz_protocol_blocks_add_bit(&instance->decoder, !level);

        switch(instance->decoder.parser_step) {
        case PocsagDecoderStepFoundSync:
            if((instance->decoder.decode_data & POCSAG_CW_MASK) == POCSAG_FRAME_SYNC_CODE) {
                instance->decoder.parser_step = PocsagDecoderStepFoundPreamble;
                instance->decoder.decode_count_bit = 0;
                instance->decoder.decode_data = 0UL;
            }
            break;
        case PocsagDecoderStepFoundPreamble:
            // handle codewords
            if(instance->decoder.decode_count_bit == POCSAG_CW_BITS) {
                codeword = (uint32_t)(instance->decoder.decode_data & POCSAG_CW_MASK);
                switch(codeword) {
                case POCSAG_IDLE_CODE_WORD:
                    instance->codeword_idx++;
                    break;
                case POCSAG_FRAME_SYNC_CODE:
                    instance->codeword_idx = 0;
                    break;
                default:
                    // Here we expect only address messages
                    if(codeword >> 31 == 0) {
                        pocsag_decode_address_word(instance, codeword);
                        instance->decoder.parser_step = PocsagDecoderStepMessage;
                    }
                    instance->codeword_idx++;
                }
                instance->decoder.decode_count_bit = 0;
                instance->decoder.decode_data = 0UL;
            }
            break;

        case PocsagDecoderStepMessage:
            if(instance->decoder.decode_count_bit == POCSAG_CW_BITS) {
                codeword = (uint32_t)(instance->decoder.decode_data & POCSAG_CW_MASK);
                switch(codeword) {
                case POCSAG_IDLE_CODE_WORD:
                    // Idle during the message stops the message
                    instance->codeword_idx++;
                    instance->decoder.parser_step = PocsagDecoderStepFoundPreamble;
                    pocsag_message_done(instance);
                    break;
                case POCSAG_FRAME_SYNC_CODE:
                    instance->codeword_idx = 0;
                    break;
                default:
                    // In this state, both address and message words can arrive
                    if(codeword >> 31 == 0) {
                        pocsag_message_done(instance);
                        pocsag_decode_address_word(instance, codeword);
                    } else {
                        if(!pocsag_decode_message_word(instance, codeword)) {
                            instance->decoder.parser_step = PocsagDecoderStepFoundPreamble;
                            pocsag_message_done(instance);
                        }
                    }
                    instance->codeword_idx++;
                }
                instance->decoder.decode_count_bit = 0;
                instance->decoder.decode_data = 0UL;
            }
            break;
        }
    }
}

uint8_t subghz_protocol_decoder_pocsag_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderPocsag* instance = context;
    uint8_t hash = 0;
    for(size_t i = 0; i < furi_string_size(instance->done_msg); i++)
        hash ^= furi_string_get_char(instance->done_msg, i);
    return hash;
}

SubGhzProtocolStatus subghz_protocol_decoder_pocsag_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderPocsag* instance = context;
    uint32_t msg_len;

    if(SubGhzProtocolStatusOk !=
       pcsg_block_generic_serialize(&instance->generic, flipper_format, preset))
        return SubGhzProtocolStatusError;

    msg_len = furi_string_size(instance->done_msg);
    if(!flipper_format_write_uint32(flipper_format, "MsgLen", &msg_len, 1)) {
        FURI_LOG_E(TAG, "Error adding MsgLen");
        return SubGhzProtocolStatusError;
    }

    uint8_t* s = (uint8_t*)furi_string_get_cstr(instance->done_msg);
    if(!flipper_format_write_hex(flipper_format, "Msg", s, msg_len)) {
        FURI_LOG_E(TAG, "Error adding Msg");
        return SubGhzProtocolStatusError;
    }
    return SubGhzProtocolStatusOk;
}

SubGhzProtocolStatus
    subghz_protocol_decoder_pocsag_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderPocsag* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    uint32_t msg_len;
    uint8_t* buf;

    do {
        if(SubGhzProtocolStatusOk !=
           pcsg_block_generic_deserialize(&instance->generic, flipper_format)) {
            break;
        }

        if(!flipper_format_read_uint32(flipper_format, "MsgLen", &msg_len, 1)) {
            FURI_LOG_E(TAG, "Missing MsgLen");
            break;
        }

        buf = malloc(msg_len);
        if(!flipper_format_read_hex(flipper_format, "Msg", buf, msg_len)) {
            FURI_LOG_E(TAG, "Missing Msg");
            free(buf);
            break;
        }
        furi_string_set_strn(instance->done_msg, (const char*)buf, msg_len);
        free(buf);

        ret = SubGhzProtocolStatusOk;
    } while(false);
    return ret;
}

void subhz_protocol_decoder_pocsag_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderPocsag* instance = context;
    furi_string_cat_printf(output, "%s\r\n", instance->generic.protocol_name);
    furi_string_cat(output, instance->done_msg);
}

const SubGhzProtocolDecoder subghz_protocol_pocsag_decoder = {
    .alloc = subghz_protocol_decoder_pocsag_alloc,
    .free = subghz_protocol_decoder_pocsag_free,
    .reset = subghz_protocol_decoder_pocsag_reset,
    .feed = subghz_protocol_decoder_pocsag_feed,
    .get_hash_data = subghz_protocol_decoder_pocsag_get_hash_data,
    .serialize = subghz_protocol_decoder_pocsag_serialize,
    .deserialize = subghz_protocol_decoder_pocsag_deserialize,
    .get_string = subhz_protocol_decoder_pocsag_get_string,
};

const SubGhzProtocol subghz_protocol_pocsag = {
    .name = SUBGHZ_PROTOCOL_POCSAG_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_FM | SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_Save |
            SubGhzProtocolFlag_Load,

    .decoder = &subghz_protocol_pocsag_decoder,
};
