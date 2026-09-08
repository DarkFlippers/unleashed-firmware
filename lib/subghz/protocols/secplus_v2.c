#include "secplus_v2.h"
#include <lib/toolbox/manchester_decoder.h>
#include <lib/toolbox/manchester_encoder.h>
#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"
#include "common.h"

#include "../blocks/custom_btn_i.h"

/*
* Help
* https://github.com/argilo/secplus
* https://github.com/merbanan/rtl_433/blob/master/src/devices/secplus_v2.c
*/

#define TAG "SubGhzProtocoSecPlusV2"

#define SECPLUS_V2_HEADER      0x3C0000000000
#define SECPLUS_V2_HEADER_MASK 0xFFFF3C0000000000
#define SECPLUS_V2_PACKET_1    0x000000000000
#define SECPLUS_V2_PACKET_2    0x010000000000
#define SECPLUS_V2_PACKET_MASK 0x30000000000

/*
 * Security+ 2.0 also has a longer, 86 bit frame used by keypads. It is a 22 bit
 * preamble (0x3C | packet id) followed by 64 bits of payload; only the payload is
 * stored. The three mixing buffers are 18 bits wide instead of 10, the extra low
 * byte of each carrying a 16 bit "data" field. The two halves together make a 32 bit
 * word whose 8 nibbles XORed with the button must be zero, and whose top half is the
 * keypad PIN (byte swapped).
 */
#define SECPLUS_V2_86_COUNT_BIT    86
#define SECPLUS_V2_86_PREAMBLE     0x3CUL
#define SECPLUS_V2_86_PREAMBLE_BIT 22

/*
 * A keypad opens every 86 bit frame with a te_short pulse and a te_long gap, ahead of
 * the manchester preamble. Those two entries are not part of the 86 bits and cannot be
 * produced by the manchester encoder, so they are written out on their own. A 62 bit
 * remote has no such lead in.
 */
#define SECPLUS_V2_86_LEAD_IN 2

/* A keypad puts four pairs on the air per key press, a 62 bit remote three. */
#define SECPLUS_V2_86_REPEAT 4

/*
 * Idle the transmitter leaves behind each half, in units of te_long. A remote holds it
 * tight: 64 gaps measured off the air run 68.6 ms to 69.2 ms, mean 69.0 ms, so 138. A
 * keypad is not that regular, 28 gaps wander between 89.0 ms and 92.1 ms, mean 90.9 ms,
 * so the receiver has to accept at least that spread and 182 sits in the middle of it.
 */
#define SECPLUS_V2_GAP_62 138
#define SECPLUS_V2_GAP_86 182

/*
 * Lead in gap the receiver syncs on, in units of te_long. A 62 bit remote leaves about
 * 130 of them ahead of each half, the 86 bit keypad about 179, so both windows have to
 * be accepted or a keypad never leaves the reset step.
 */
#define SECPLUS_V2_SYNC_62       130
#define SECPLUS_V2_SYNC_62_DELTA 100
#define SECPLUS_V2_SYNC_86       179
#define SECPLUS_V2_SYNC_86_DELTA 72

/*
 * The two halves of one transmission follow each other closely. A first half left over
 * from an earlier press cannot belong to the second half being decoded now, so drop it
 * once the gap between frames grows past te_long * 358.
 */
#define SECPLUS_V2_PAIR_TIMEOUT 358

/* Manchester emits at most two entries per bit, and each of the two packets adds a
 * finish entry, a trailing gap and, for a keypad, the two lead in entries. The 86 bit
 * frame is the longest:
 *   ((22 preamble + 64 payload) * 2 + 2 + 2) * 2 = 352
 * The 62 bit frame needs 252, which is why a 256 entry buffer used to be enough. */
#define SECPLUS_V2_ENCODER_UPLOAD_SIZE 384
#define SECPLUS_V2_P_MASK_62           0x03FFu
#define SECPLUS_V2_P_MASK_86           0x3FFFFu

static const SubGhzBlockConst subghz_protocol_secplus_v2_const = {
    .te_short = 250,
    .te_long = 500,
    .te_delta = 110,
    .min_count_bit_for_found = 62,
};

struct SubGhzProtocolDecoderSecPlus_v2 {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    ManchesterState manchester_saved_state;
    uint64_t secplus_packet_1;
    // width of the half held in secplus_packet_1, so a 62 bit half can never be
    // handed to the 86 bit reconstruction or the other way round
    uint8_t secplus_packet_1_count_bit;
    uint32_t secplus_data; // 86 bit frames only
    uint8_t packet_id;
};
SUBGHZ_ASSERT_DECODER_COMMON_LAYOUT(SubGhzProtocolDecoderSecPlus_v2);

struct SubGhzProtocolEncoderSecPlus_v2 {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
    uint64_t secplus_packet_1;
    uint32_t secplus_data; // 86 bit frames only
};
SUBGHZ_ASSERT_ENCODER_GENERIC_LAYOUT(SubGhzProtocolEncoderSecPlus_v2);

typedef enum {
    SecPlus_v2DecoderStepReset = 0,
    SecPlus_v2DecoderStepDecoderData,
} SecPlus_v2DecoderStep;

const SubGhzProtocolDecoder subghz_protocol_secplus_v2_decoder = {
    .alloc = subghz_protocol_decoder_secplus_v2_alloc,
    .free = subghz_protocol_decoder_common_free,

    .feed = subghz_protocol_decoder_secplus_v2_feed,
    .reset = subghz_protocol_decoder_secplus_v2_reset,

    .get_hash_data = subghz_protocol_decoder_common_get_hash_data,
    .serialize = subghz_protocol_decoder_secplus_v2_serialize,
    .deserialize = subghz_protocol_decoder_secplus_v2_deserialize,
    .get_string = subghz_protocol_decoder_secplus_v2_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_secplus_v2_encoder = {
    .alloc = subghz_protocol_encoder_secplus_v2_alloc,
    .free = subghz_protocol_encoder_common_free,

    .deserialize = subghz_protocol_encoder_secplus_v2_deserialize,
    .stop = subghz_protocol_encoder_secplus_v2_stop,
    .yield = subghz_protocol_encoder_common_yield,
};

const SubGhzProtocol subghz_protocol_secplus_v2 = {
    .name = SUBGHZ_PROTOCOL_SECPLUS_V2_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_315 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_secplus_v2_decoder,
    .encoder = &subghz_protocol_secplus_v2_encoder,
};

void* subghz_protocol_encoder_secplus_v2_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    return subghz_protocol_encoder_common_alloc(
        sizeof(SubGhzProtocolEncoderSecPlus_v2),
        &subghz_protocol_secplus_v2,
        3,
        SECPLUS_V2_ENCODER_UPLOAD_SIZE);
}

static bool subghz_protocol_secplus_v2_mix_invet(uint8_t invert, uint32_t p[], uint32_t mask) {
    // selectively invert buffers
    switch(invert) {
    case 0x00: // 0b0000 (True, True, False),
        p[0] = ~p[0] & mask;
        p[1] = ~p[1] & mask;
        break;
    case 0x01: // 0b0001 (False, True, False),
        p[1] = ~p[1] & mask;
        break;
    case 0x02: // 0b0010 (False, False, True),
        p[2] = ~p[2] & mask;
        break;
    case 0x04: // 0b0100 (True, True, True),
        p[0] = ~p[0] & mask;
        p[1] = ~p[1] & mask;
        p[2] = ~p[2] & mask;
        break;
    case 0x05: // 0b0101 (True, False, True),
    case 0x0a: // 0b1010 (True, False, True),
        p[0] = ~p[0] & mask;
        p[2] = ~p[2] & mask;
        break;
    case 0x06: // 0b0110 (False, True, True),
        p[1] = ~p[1] & mask;
        p[2] = ~p[2] & mask;
        break;
    case 0x08: // 0b1000 (True, False, False),
        p[0] = ~p[0] & mask;
        break;
    case 0x09: // 0b1001 (False, False, False),
        break;
    default:
        FURI_LOG_E(TAG, "Invert FAIL");
        return false;
    }
    return true;
}

static bool subghz_protocol_secplus_v2_mix_order_decode(uint8_t order, uint32_t p[]) {
    uint32_t a = p[0], b = p[1], c = p[2];

    // selectively reorder buffers
    switch(order) {
    case 0x06: // 0b0110  2, 1, 0],
    case 0x09: // 0b1001  2, 1, 0],
        p[2] = a;
        // p[1]: no change
        p[0] = c;
        break;
    case 0x08: // 0b1000  1, 2, 0],
    case 0x04: // 0b0100  1, 2, 0],
        p[1] = a;
        p[2] = b;
        p[0] = c;
        break;
    case 0x01: // 0b0001 2, 0, 1],
        p[2] = a;
        p[0] = b;
        p[1] = c;
        break;
    case 0x00: // 0b0000  0, 2, 1],
        // p[0]: no change
        p[2] = b;
        p[1] = c;
        break;
    case 0x05: // 0b0101 1, 0, 2],
        p[1] = a;
        p[0] = b;
        // p[2]: no change
        break;
    case 0x02: // 0b0010 0, 1, 2],
    case 0x0A: // 0b1010 0, 1, 2],
        // no reordering
        break;
    default:
        FURI_LOG_E(TAG, "Order FAIL");
        return false;
    }
    return true;
}

static bool subghz_protocol_secplus_v2_mix_order_encode(uint8_t order, uint32_t p[]) {
    uint32_t a, b, c;

    // selectively reorder buffers
    switch(order) {
    case 0x06: // 0b0110  2, 1, 0],
    case 0x09: // 0b1001  2, 1, 0],
        a = p[2];
        b = p[1];
        c = p[0];
        break;
    case 0x08: // 0b1000  1, 2, 0],
    case 0x04: // 0b0100  1, 2, 0],
        a = p[1];
        b = p[2];
        c = p[0];
        break;
    case 0x01: // 0b0001 2, 0, 1],
        a = p[2];
        b = p[0];
        c = p[1];
        break;
    case 0x00: // 0b0000  0, 2, 1],
        a = p[0];
        b = p[2];
        c = p[1];
        break;
    case 0x05: // 0b0101 1, 0, 2],
        a = p[1];
        b = p[0];
        c = p[2];
        break;
    case 0x02: // 0b0010 0, 1, 2],
    case 0x0A: // 0b1010 0, 1, 2],
        a = p[0];
        b = p[1];
        c = p[2];
        break;
    default:
        FURI_LOG_E(TAG, "Order FAIL");
        return false;
    }

    p[0] = a;
    p[1] = b;
    p[2] = c;
    return true;
}

/** 
 * Security+ 2.0 half-message decoding
 * @param data data 
 * @param roll_array[] return roll_array part
 * @param fixed[] return fixed part
 * @return true On success
 */

static bool
    subghz_protocol_secplus_v2_decode_half(uint64_t data, uint8_t roll_array[], uint32_t* fixed) {
    uint8_t order = (data >> 34) & 0x0f;
    uint8_t invert = (data >> 30) & 0x0f;
    uint32_t p[3] = {0};

    for(int i = 29; i >= 0; i -= 3) {
        p[0] = p[0] << 1 | bit_read(data, i);
        p[1] = p[1] << 1 | bit_read(data, i - 1);
        p[2] = p[2] << 1 | bit_read(data, i - 2);
    }

    if(!subghz_protocol_secplus_v2_mix_invet(invert, p, SECPLUS_V2_P_MASK_62)) return false;
    if(!subghz_protocol_secplus_v2_mix_order_decode(order, p)) return false;

    data = order << 4 | invert;
    int k = 0;
    for(int i = 6; i >= 0; i -= 2) {
        roll_array[k] = (data >> i) & 0x03;
        if(roll_array[k++] == 3) {
            FURI_LOG_E(TAG, "Roll_Array FAIL");
            return false;
        }
    }

    for(int i = 8; i >= 0; i -= 2) {
        roll_array[k] = (p[2] >> i) & 0x03;
        if(roll_array[k++] == 3) {
            FURI_LOG_E(TAG, "Roll_Array FAIL");
            return false;
        }
    }

    fixed[0] = p[0] << 10 | p[1];
    return true;
}

/**
 * Security+ 2.0 86-bit half-message decoding.
 * Same shape as the 62 bit variant but the three buffers are 18 bits wide: the top
 * 10 bits of p[0]/p[1] carry the fixed part, the low bytes carry a 16 bit data field,
 * and p[2] holds 9 rolling digits of which the last 4 duplicate the first 4.
 * @param data 64 bit payload of the frame (the 22 bit preamble is not included)
 * @param roll_array return roll_array part, 9 digits
 * @param fixed return fixed part, 20 bits
 * @param data_part return the 16 bit data field
 * @return true On success
 */
static bool subghz_protocol_secplus_v2_decode_half_86(
    uint64_t data,
    uint8_t roll_array[9],
    uint32_t* fixed,
    uint16_t* data_part) {
    if((data >> 62) != 1) return false;

    uint8_t invert = (data >> 54) & 0x0F;
    uint8_t order = (data >> 58) & 0x0F;
    uint32_t p[3] = {0};

    for(int i = 53; i >= 0; i -= 3) {
        p[0] = p[0] << 1 | bit_read(data, i);
        p[1] = p[1] << 1 | bit_read(data, i - 1);
        p[2] = p[2] << 1 | bit_read(data, i - 2);
    }

    if(!subghz_protocol_secplus_v2_mix_invet(invert, p, SECPLUS_V2_P_MASK_86)) return false;
    if(!subghz_protocol_secplus_v2_mix_order_decode(order, p)) return false;

    uint8_t digits[13];
    uint32_t hdr = (uint32_t)order << 4 | invert;
    digits[0] = (uint8_t)(order >> 2);
    digits[1] = (uint8_t)((hdr >> 4) & 0x03);
    digits[2] = (uint8_t)((hdr >> 2) & 0x03);
    digits[3] = (uint8_t)(hdr & 0x03);
    for(uint8_t i = 0; i < 4; i++) {
        if(digits[i] == 3) {
            FURI_LOG_E(TAG, "Roll_Array FAIL");
            return false;
        }
    }
    uint8_t k = 4;
    for(int i = 16; i >= 0; i -= 2) {
        digits[k] = (uint8_t)((p[2] >> i) & 0x03);
        if(digits[k] == 3) {
            FURI_LOG_E(TAG, "Roll_Array FAIL");
            return false;
        }
        k++;
    }
    // the tail repeats the head, use it as a consistency check
    for(uint8_t i = 0; i < 4; i++) {
        if(digits[i] != digits[i + 9]) return false;
    }

    memcpy(roll_array, digits, 9);
    *fixed = (uint32_t)(((p[0] << 2) & 0xFFC00) | ((p[1] >> 8) & 0x3FF));
    *data_part = (uint16_t)((p[0] << 8) | (p[1] & 0xFF));
    return true;
}

/**
 * Security+ 2.0 86-bit half-message encoding.
 * @return 64 bit payload, 0 on failure
 */
static uint64_t subghz_protocol_secplus_v2_encode_half_86(
    const uint8_t roll_array[9],
    uint32_t fixed,
    uint16_t data_part) {
    uint32_t p[3];
    p[0] = (((fixed >> 10) & 0x3FF) << 8) | ((data_part >> 8) & 0xFF);
    p[1] = ((fixed & 0x3FF) << 8) | (data_part & 0xFF);
    p[2] = (uint32_t)roll_array[4] << 16 | (uint32_t)roll_array[5] << 14 |
           (uint32_t)roll_array[6] << 12 | (uint32_t)roll_array[7] << 10 |
           (uint32_t)roll_array[8] << 8 | (uint32_t)roll_array[0] << 6 |
           (uint32_t)roll_array[1] << 4 | (uint32_t)roll_array[2] << 2 | (uint32_t)roll_array[3];

    uint8_t order = (uint8_t)(roll_array[0] << 2 | roll_array[1]);
    uint8_t invert = (uint8_t)(roll_array[2] << 2 | roll_array[3]);

    if(!subghz_protocol_secplus_v2_mix_order_encode(order, p)) return 0;
    if(!subghz_protocol_secplus_v2_mix_invet(invert, p, SECPLUS_V2_P_MASK_86)) return 0;

    uint64_t data = 0;
    for(int i = 0; i < 18; i++) {
        data <<= 3;
        data |= (uint64_t)bit_read(p[0], 17 - i) << 2 | (uint64_t)bit_read(p[1], 17 - i) << 1 |
                (uint64_t)bit_read(p[2], 17 - i);
    }
    data |= (uint64_t)order << 58 | (uint64_t)invert << 54 | (1ULL << 62);
    return data;
}

/**
 * Interleave the two 9 digit rolling arrays into the 28 bit counter.
 * @return true if the value is in range
 */
static bool subghz_protocol_secplus_v2_rolling_join(
    const uint8_t roll_1[9],
    const uint8_t roll_2[9],
    uint32_t* rolling) {
    uint8_t d[18];
    d[0] = roll_2[8];
    d[1] = roll_1[8];
    for(uint8_t i = 0; i < 4; i++) {
        d[2 + i] = roll_2[4 + i];
        d[6 + i] = roll_1[4 + i];
        d[10 + i] = roll_2[i];
        d[14 + i] = roll_1[i];
    }
    uint32_t v = 0;
    for(uint8_t i = 0; i < 18; i++) {
        v = (v * 3) + d[i];
    }
    if(v >= 0x10000000) return false;
    *rolling = subghz_protocol_blocks_reverse_key(v, 28);
    return true;
}

/**
 * Analysis of a received 86 bit (keypad) message.
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 * @param packet_1 first part of the message
 * @param data_out the recovered 32 bit data word
 * @return true On success
 */
static bool subghz_protocol_secplus_v2_remote_controller_86(
    SubGhzBlockGeneric* instance,
    uint64_t packet_1,
    uint32_t* data_out) {
    uint8_t roll_1[9] = {0};
    uint8_t roll_2[9] = {0};
    uint32_t fixed_1 = 0;
    uint32_t fixed_2 = 0;
    uint16_t data_1 = 0;
    uint16_t data_2 = 0;
    uint32_t rolling = 0;

    if(!subghz_protocol_secplus_v2_decode_half_86(packet_1, roll_1, &fixed_1, &data_1))
        return false;
    if(!subghz_protocol_secplus_v2_decode_half_86(instance->data, roll_2, &fixed_2, &data_2))
        return false;
    if(!subghz_protocol_secplus_v2_rolling_join(roll_1, roll_2, &rolling)) {
        FURI_LOG_E(TAG, "Rolling FAIL");
        return false;
    }

    const uint32_t data_word = ((uint32_t)data_1 << 16) | data_2;
    /*
     * The whole byte at bits 19..12 of fixed_1 is the button field, exactly as in the
     * 62 bit frame. It has to be kept whole: generic.serial is 32 bit while the serial
     * built below is 40, so those eight bits are the ones the store drops, and
     * encode_86() is what puts them back. Taking only a nibble here loses fixed_1
     * bits 19..16 for good and the rebuilt packet 1 no longer matches the keypad.
     * Only the low nibble takes part in the data checksum.
     */
    const uint8_t btn = (uint8_t)(fixed_1 >> 12);

    // the 8 nibbles of the data word XORed with the button must cancel out
    uint8_t check = btn & 0x0F;
    for(uint8_t i = 0; i < 32; i += 4) {
        check ^= (data_word >> i) & 0x0F;
    }
    if(check != 0) {
        FURI_LOG_E(TAG, "Data checksum FAIL");
        return false;
    }

    instance->cnt = rolling;
    instance->btn = btn;
    instance->serial = fixed_1 << 20 | fixed_2;
    *data_out = data_word;
    return true;
}

/*
 * The keypad PIN of an 86 bit frame lives in the top half of the data word, byte
 * swapped, so a captured signal already carries it and it is saved along with the rest.
 * The "Pin" key in the file stays authoritative: setting it, by hand or with the
 * Security+ PIN app, makes the encoder rebuild the data word around the new value.
 */
#define SECPLUS_V2_PIN_UNSET 0xFFFFu
#define SECPLUS_V2_PIN_MAX   9999u

/*
 * The PIN of the signal the decoder currently holds, when it came from a file rather
 * than off the air. Owned by the decoder alone: set in its deserialize(), cleared in
 * its reset(), read only by get_string(). The encoder keeps its own local copy so a
 * transmission can never leave a PIN behind for an unrelated signal to display.
 */
static uint16_t secplus_v2_pin = SECPLUS_V2_PIN_UNSET;

/** The PIN a frame carries, or SECPLUS_V2_PIN_UNSET if it is not a plain 4 digit one. */
static uint16_t subghz_protocol_secplus_v2_pin_extract(uint32_t data_word) {
    uint16_t high = (uint16_t)(data_word >> 16);
    uint16_t pin = (uint16_t)((high << 8) | (high >> 8));
    return (pin <= SECPLUS_V2_PIN_MAX) ? pin : SECPLUS_V2_PIN_UNSET;
}

/** Splice a PIN into the data word; the checksum nibble is fixed up by encode_86(). */
static uint32_t subghz_protocol_secplus_v2_pin_apply(uint32_t data_word, uint16_t pin) {
    uint16_t high = (uint16_t)((pin << 8) | (pin >> 8));
    return ((uint32_t)high << 16) | (data_word & 0xFFFF);
}

/** Read the optional "Pin" key. An absent or empty value means "not known yet". */
static uint16_t subghz_protocol_secplus_v2_pin_read(FlipperFormat* flipper_format) {
    uint16_t pin = SECPLUS_V2_PIN_UNSET;
    FuriString* tmp = furi_string_alloc();
    if(flipper_format_rewind(flipper_format) &&
       flipper_format_read_string(flipper_format, "Pin", tmp)) {
        furi_string_trim(tmp);
        if(!furi_string_empty(tmp)) {
            int value = atoi(furi_string_get_cstr(tmp));
            if((value >= 0) && (value <= (int)SECPLUS_V2_PIN_MAX)) pin = (uint16_t)value;
        }
    }
    furi_string_free(tmp);
    return pin;
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 * @param packet_1 first part of the message
 */
static void
    subghz_protocol_secplus_v2_remote_controller(SubGhzBlockGeneric* instance, uint64_t packet_1) {
    uint32_t fixed_1[1];
    uint8_t roll_1[9] = {0};
    uint32_t fixed_2[1];
    uint8_t roll_2[9] = {0};
    uint8_t rolling_digits[18] = {0};

    if(subghz_protocol_secplus_v2_decode_half(packet_1, roll_1, fixed_1) &&
       subghz_protocol_secplus_v2_decode_half(instance->data, roll_2, fixed_2)) {
        rolling_digits[0] = roll_2[8];
        rolling_digits[1] = roll_1[8];

        rolling_digits[2] = roll_2[4];
        rolling_digits[3] = roll_2[5];
        rolling_digits[4] = roll_2[6];
        rolling_digits[5] = roll_2[7];

        rolling_digits[6] = roll_1[4];
        rolling_digits[7] = roll_1[5];
        rolling_digits[8] = roll_1[6];
        rolling_digits[9] = roll_1[7];

        rolling_digits[10] = roll_2[0];
        rolling_digits[11] = roll_2[1];
        rolling_digits[12] = roll_2[2];
        rolling_digits[13] = roll_2[3];

        rolling_digits[14] = roll_1[0];
        rolling_digits[15] = roll_1[1];
        rolling_digits[16] = roll_1[2];
        rolling_digits[17] = roll_1[3];

        uint32_t rolling = 0;
        for(int i = 0; i < 18; i++) {
            rolling = (rolling * 3) + rolling_digits[i];
        }
        // Max value = 2^28 (268435456)
        if(rolling >= 0x10000000) {
            FURI_LOG_E(TAG, "Rolling FAIL");
            instance->cnt = 0;
            instance->btn = 0;
            instance->serial = 0;
        } else {
            instance->cnt = subghz_protocol_blocks_reverse_key(rolling, 28);
            instance->btn = fixed_1[0] >> 12;
            instance->serial = fixed_1[0] << 20 | fixed_2[0];
        }
    } else {
        instance->cnt = 0;
        instance->btn = 0;
        instance->serial = 0;
    }

    // Save original button for later use
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(instance->btn);
    }
    subghz_custom_btn_set_max(4);
}

/** 
 * Security+ 2.0 half-message encoding
 * @param roll_array[] roll_array part
 * @param fixed[] fixed part
 * @return return data 
 */

static uint64_t subghz_protocol_secplus_v2_encode_half(uint8_t roll_array[], uint32_t fixed) {
    uint64_t data = 0;
    uint32_t p[3] = {(fixed >> 10) & 0x3FF, fixed & 0x3FF, 0};
    uint8_t order = roll_array[0] << 2 | roll_array[1];
    uint8_t invert = roll_array[2] << 2 | roll_array[3];
    p[2] = (uint32_t)roll_array[4] << 8 | roll_array[5] << 6 | roll_array[6] << 4 |
           roll_array[7] << 2 | roll_array[8];

    if(!subghz_protocol_secplus_v2_mix_order_encode(order, p)) return 0;
    if(!subghz_protocol_secplus_v2_mix_invet(invert, p, SECPLUS_V2_P_MASK_62)) return 0;

    for(int i = 0; i < 10; i++) {
        data <<= 3;
        data |= bit_read(p[0], 9 - i) << 2 | bit_read(p[1], 9 - i) << 1 | bit_read(p[2], 9 - i);
    }
    data |= ((uint64_t)order) << 34 | ((uint64_t)invert) << 30;

    return data;
}

/**
 * Defines the button value for the current btn_id
 * Basic set | 0x68 | 0x80 | 0x81 | 0xE2 | 0x78
 * @return Button code
 */
static uint8_t subghz_protocol_secplus_v2_get_btn_code(void);

/** 
 * Security+ 2.0 message encoding
 * @param instance SubGhzProtocolEncoderSecPlus_v2* 
 */

/**
 * Security+ 2.0 86 bit (keypad) message encoding. Keeps the PIN that was decoded from
 * the loaded signal and only advances the counter.
 */
static void subghz_protocol_secplus_v2_encode_86(SubGhzProtocolEncoderSecPlus_v2* instance) {
    uint32_t rolling = subghz_protocol_blocks_reverse_key(instance->generic.cnt, 28);
    uint8_t rolling_digits[18] = {0};
    uint8_t roll_1[9] = {0};
    uint8_t roll_2[9] = {0};

    for(int8_t i = 17; i > -1; i--) {
        rolling_digits[i] = rolling % 3;
        rolling /= 3;
    }
    roll_2[8] = rolling_digits[0];
    roll_1[8] = rolling_digits[1];
    for(uint8_t i = 0; i < 4; i++) {
        roll_2[4 + i] = rolling_digits[2 + i];
        roll_1[4 + i] = rolling_digits[6 + i];
        roll_2[i] = rolling_digits[10 + i];
        roll_1[i] = rolling_digits[14 + i];
    }

    uint32_t fixed_1 = instance->generic.btn << 12 | instance->generic.serial >> 20;
    uint32_t fixed_2 = instance->generic.serial & 0xFFFFF;

    // recompute the nibble checksum that lives in bits 15..12 of the data word
    uint32_t data_word = instance->secplus_data & ~0xF000UL;
    uint8_t check = instance->generic.btn & 0x0F;
    for(uint8_t i = 0; i < 32; i += 4) {
        check ^= (data_word >> i) & 0x0F;
    }
    data_word |= (uint32_t)(check & 0x0F) << 12;
    instance->secplus_data = data_word;

    instance->secplus_packet_1 =
        subghz_protocol_secplus_v2_encode_half_86(roll_1, fixed_1, (uint16_t)(data_word >> 16));
    instance->generic.data =
        subghz_protocol_secplus_v2_encode_half_86(roll_2, fixed_2, (uint16_t)data_word);
}

static void subghz_protocol_secplus_v2_encode(SubGhzProtocolEncoderSecPlus_v2* instance) {
    // Save original button for later use
    if(subghz_custom_btn_get_original() == 0) {
        subghz_custom_btn_set_original(instance->generic.btn);
    }

    instance->generic.btn = subghz_protocol_secplus_v2_get_btn_code();

    // override button if we change it with signal settings button editor
    if(subghz_block_generic_global_button_override_get(&instance->generic.btn)) {
        FURI_LOG_D(TAG, "Button sucessfully changed to 0x%X", instance->generic.btn);
    }

    uint32_t fixed_1[1] = {instance->generic.btn << 12 | instance->generic.serial >> 20};
    uint32_t fixed_2[1] = {instance->generic.serial & 0xFFFFF};
    uint8_t rolling_digits[18] = {0};
    uint8_t roll_1[9] = {0};
    uint8_t roll_2[9] = {0};

    // Experemental case - we dont know counter size exactly, so just will be think that it is in range of 0xE500000 - 0xFFFFFFF

    // Check for OFEX (overflow experimental) mode
    if(furi_hal_subghz_get_rolling_counter_mult() != -0x7FFFFFFF) {
        // standart counter mode. PULL data from subghz_block_generic_global variables
        if(!subghz_block_generic_global_counter_override_get(&instance->generic.cnt)) {
            // if counter_override_get return FALSE then counter was not changed and we increase counter by standart mult value
            if((instance->generic.cnt + furi_hal_subghz_get_rolling_counter_mult()) > 0xFFFFFFF) {
                instance->generic.cnt = 0xE500000;
            } else {
                instance->generic.cnt += furi_hal_subghz_get_rolling_counter_mult();
            }
        }
        if(instance->generic.cnt < 0xE500000) instance->generic.cnt = 0xE500000;
    } else {
        // OFEX (overflow experimental) mode
        if((instance->generic.cnt + 0x1) > 0xFFFFFFF) {
            instance->generic.cnt = 0xE500000;
        } else if(instance->generic.cnt >= 0xE500000 && instance->generic.cnt != 0xFFFFFFE) {
            instance->generic.cnt = 0xFFFFFFE;
        } else {
            instance->generic.cnt++;
        }
    }

    uint32_t rolling = subghz_protocol_blocks_reverse_key(instance->generic.cnt, 28);

    for(int8_t i = 17; i > -1; i--) {
        rolling_digits[i] = rolling % 3;
        rolling /= 3;
    }

    roll_2[8] = rolling_digits[0];
    roll_1[8] = rolling_digits[1];

    roll_2[4] = rolling_digits[2];
    roll_2[5] = rolling_digits[3];
    roll_2[6] = rolling_digits[4];
    roll_2[7] = rolling_digits[5];

    roll_1[4] = rolling_digits[6];
    roll_1[5] = rolling_digits[7];
    roll_1[6] = rolling_digits[8];
    roll_1[7] = rolling_digits[9];

    roll_2[0] = rolling_digits[10];
    roll_2[1] = rolling_digits[11];
    roll_2[2] = rolling_digits[12];
    roll_2[3] = rolling_digits[13];

    roll_1[0] = rolling_digits[14];
    roll_1[1] = rolling_digits[15];
    roll_1[2] = rolling_digits[16];
    roll_1[3] = rolling_digits[17];

    instance->secplus_packet_1 = SECPLUS_V2_HEADER | SECPLUS_V2_PACKET_1 |
                                 subghz_protocol_secplus_v2_encode_half(roll_1, fixed_1[0]);
    instance->generic.data = SECPLUS_V2_HEADER | SECPLUS_V2_PACKET_2 |
                             subghz_protocol_secplus_v2_encode_half(roll_2, fixed_2[0]);
}

static LevelDuration
    subghz_protocol_encoder_secplus_v2_add_duration_to_upload(ManchesterEncoderResult result) {
    LevelDuration data = {.duration = 0, .level = 0};
    switch(result) {
    case ManchesterEncoderResultShortLow:
        data.duration = subghz_protocol_secplus_v2_const.te_short;
        data.level = false;
        break;
    case ManchesterEncoderResultLongLow:
        data.duration = subghz_protocol_secplus_v2_const.te_long;
        data.level = false;
        break;
    case ManchesterEncoderResultLongHigh:
        data.duration = subghz_protocol_secplus_v2_const.te_long;
        data.level = true;
        break;
    case ManchesterEncoderResultShortHigh:
        data.duration = subghz_protocol_secplus_v2_const.te_short;
        data.level = true;
        break;

    default:
        furi_crash("SubGhz: ManchesterEncoderResult is incorrect.");
        break;
    }
    return level_duration_make(data.level, data.duration);
}

/**
 * Emit one half packet into the upload: for an 86 bit frame the keypad lead in and the
 * 22 bit preamble, then the payload, the manchester finish and the trailing gap.
 * @param instance Pointer to a SubGhzProtocolEncoderSecPlus_v2 instance
 * @param index write cursor into instance->encoder.upload, advanced in place
 * @param payload the half to send, low @p payload_bits bits
 * @param packet_id id carried in the preamble, 86 bit frames only
 */
static void subghz_protocol_encoder_secplus_v2_emit_packet(
    SubGhzProtocolEncoderSecPlus_v2* instance,
    size_t* index,
    uint64_t payload,
    uint8_t payload_bits,
    uint8_t packet_id,
    bool long_frame) {
    ManchesterEncoderState enc_state;
    manchester_encoder_reset(&enc_state);
    ManchesterEncoderResult result;

    if(long_frame) {
        instance->encoder.upload[(*index)++] =
            level_duration_make(true, subghz_protocol_secplus_v2_const.te_short);
        instance->encoder.upload[(*index)++] =
            level_duration_make(false, subghz_protocol_secplus_v2_const.te_long);

        const uint64_t preamble = SECPLUS_V2_86_PREAMBLE | packet_id;
        for(uint8_t i = SECPLUS_V2_86_PREAMBLE_BIT; i > 0; i--) {
            if(!manchester_encoder_advance(&enc_state, bit_read(preamble, i - 1), &result)) {
                instance->encoder.upload[(*index)++] =
                    subghz_protocol_encoder_secplus_v2_add_duration_to_upload(result);
                manchester_encoder_advance(&enc_state, bit_read(preamble, i - 1), &result);
            }
            instance->encoder.upload[(*index)++] =
                subghz_protocol_encoder_secplus_v2_add_duration_to_upload(result);
        }
    }

    for(uint8_t i = payload_bits; i > 0; i--) {
        if(!manchester_encoder_advance(&enc_state, bit_read(payload, i - 1), &result)) {
            instance->encoder.upload[(*index)++] =
                subghz_protocol_encoder_secplus_v2_add_duration_to_upload(result);
            manchester_encoder_advance(&enc_state, bit_read(payload, i - 1), &result);
        }
        instance->encoder.upload[(*index)++] =
            subghz_protocol_encoder_secplus_v2_add_duration_to_upload(result);
    }

    instance->encoder.upload[*index] = subghz_protocol_encoder_secplus_v2_add_duration_to_upload(
        manchester_encoder_finish(&enc_state));
    if(level_duration_get_level(instance->encoder.upload[*index])) {
        (*index)++;
    }
    instance->encoder.upload[(*index)++] = level_duration_make(
        false,
        (uint32_t)subghz_protocol_secplus_v2_const.te_long *
            (long_frame ? SECPLUS_V2_GAP_86 : SECPLUS_V2_GAP_62));
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderSecPlus_v2 instance
 */
static bool
    subghz_protocol_encoder_secplus_v2_get_upload(SubGhzProtocolEncoderSecPlus_v2* instance) {
    furi_assert(instance);
    size_t index = 0;

    const bool long_frame = (instance->generic.data_count_bit == SECPLUS_V2_86_COUNT_BIT);
    // 86 bit frames carry a 22 bit preamble that is not part of the stored payload
    const uint8_t payload_bits = long_frame ? 64 : instance->generic.data_count_bit;
    const uint8_t frame_bits =
        (uint8_t)(payload_bits + (long_frame ? SECPLUS_V2_86_PREAMBLE_BIT : 0));

    const size_t size_upload =
        ((size_t)frame_bits * 2u + 2u + (long_frame ? SECPLUS_V2_86_LEAD_IN : 0u)) * 2u;
    // Not encoder.size_upload: that holds the capacity only until the first upload
    // replaces it with the length actually emitted, and an 86 bit frame that followed a
    // 62 bit one on the same encoder would then be refused a buffer it does fit in.
    if(size_upload > SECPLUS_V2_ENCODER_UPLOAD_SIZE) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    }

    /* Both a keypad and a remote put packet id 1 on the air first, then id 0, and
     * repeat that pair. The decoder latches id 0 and completes on id 1, so it needs one
     * frame more than it used to before a transmission of ours pairs up; every repeat
     * count in use here is well above that. */
    subghz_protocol_encoder_secplus_v2_emit_packet(
        instance, &index, instance->generic.data, payload_bits, 1, long_frame);
    subghz_protocol_encoder_secplus_v2_emit_packet(
        instance, &index, instance->secplus_packet_1, payload_bits, 0, long_frame);

    instance->encoder.size_upload = index;
    return true;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_secplus_v2_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderSecPlus_v2* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_block_generic_deserialize(&instance->generic, flipper_format);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        if((instance->generic.data_count_bit !=
            subghz_protocol_secplus_v2_const.min_count_bit_for_found) &&
           (instance->generic.data_count_bit != SECPLUS_V2_86_COUNT_BIT)) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            ret = SubGhzProtocolStatusErrorValueBitCount;
            break;
        }
        uint8_t key_data[sizeof(uint64_t)] = {0};
        if(!flipper_format_read_hex(
               flipper_format, "Secplus_packet_1", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Secplus_packet_1");
            ret = SubGhzProtocolStatusErrorParserOthers;
            break;
        }
        for(uint8_t i = 0; i < sizeof(uint64_t); i++) {
            instance->secplus_packet_1 = instance->secplus_packet_1 << 8 | key_data[i];
        }

        if(instance->generic.data_count_bit == SECPLUS_V2_86_COUNT_BIT) {
            uint32_t data_word = 0;
            if(!subghz_protocol_secplus_v2_remote_controller_86(
                   &instance->generic, instance->secplus_packet_1, &data_word)) {
                ret = SubGhzProtocolStatusErrorParserOthers;
                break;
            }
            instance->secplus_data = data_word;

            uint16_t pin = subghz_protocol_secplus_v2_pin_read(flipper_format);
            if(pin <= SECPLUS_V2_PIN_MAX) {
                instance->secplus_data =
                    subghz_protocol_secplus_v2_pin_apply(instance->secplus_data, pin);
            }
            if(!flipper_format_rewind(flipper_format)) {
                FURI_LOG_E(TAG, "Rewind error");
                ret = SubGhzProtocolStatusErrorParserOthers;
                break;
            }

            // Save original button for later use
            if(subghz_custom_btn_get_original() == 0) {
                subghz_custom_btn_set_original(instance->generic.btn);
            }
            instance->generic.btn = subghz_protocol_secplus_v2_get_btn_code();
            if(subghz_block_generic_global_button_override_get(&instance->generic.btn)) {
                FURI_LOG_D(TAG, "Button sucessfully changed to 0x%X", instance->generic.btn);
            }

            if(!subghz_block_generic_global_counter_override_get(&instance->generic.cnt)) {
                int32_t mult = furi_hal_subghz_get_rolling_counter_mult();
                if(mult == -0x7FFFFFFF) mult = 1;
                if((instance->generic.cnt + mult) > 0xFFFFFFF) {
                    instance->generic.cnt = 0xE500000;
                } else {
                    instance->generic.cnt += mult;
                }
                if(instance->generic.cnt < 0xE500000) instance->generic.cnt = 0xE500000;
            }

            subghz_protocol_secplus_v2_encode_86(instance);
        } else {
            subghz_protocol_secplus_v2_remote_controller(
                &instance->generic, instance->secplus_packet_1);
            subghz_protocol_secplus_v2_encode(instance);
        }
        if(instance->generic.data_count_bit == SECPLUS_V2_86_COUNT_BIT) {
            instance->encoder.repeat = SECPLUS_V2_86_REPEAT;
        }
        // Optional value
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);
        if(!subghz_protocol_encoder_secplus_v2_get_upload(instance)) {
            ret = SubGhzProtocolStatusErrorEncoderGetUpload;
            break;
        }

        //update data
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (instance->generic.data >> (i * 8)) & 0xFF;
        }
        if(!flipper_format_update_hex(flipper_format, "Key", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to add Key");
            ret = SubGhzProtocolStatusErrorParserKey;
            break;
        }

        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (instance->secplus_packet_1 >> (i * 8)) & 0xFF;
        }
        if(!flipper_format_update_hex(
               flipper_format, "Secplus_packet_1", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to add Secplus_packet_1");
            ret = SubGhzProtocolStatusErrorParserOthers;
            break;
        }

        instance->encoder.front = 0; // reset before start
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void subghz_protocol_encoder_secplus_v2_stop(void* context) {
    SubGhzProtocolEncoderSecPlus_v2* instance = context;
    instance->encoder.is_running = false;
    instance->encoder.front = 0; // reset position
}

bool subghz_protocol_secplus_v2_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint32_t cnt,
    SubGhzRadioPreset* preset) {
    furi_check(context);

    SubGhzProtocolEncoderSecPlus_v2* instance = context;
    instance->generic.serial = serial;
    instance->generic.cnt = cnt;
    instance->generic.btn = btn;
    instance->generic.data_count_bit =
        (uint8_t)subghz_protocol_secplus_v2_const.min_count_bit_for_found;
    subghz_protocol_secplus_v2_encode(instance);
    SubGhzProtocolStatus res =
        subghz_block_generic_serialize(&instance->generic, flipper_format, preset);

    uint8_t key_data[sizeof(uint64_t)] = {0};
    for(size_t i = 0; i < sizeof(uint64_t); i++) {
        key_data[sizeof(uint64_t) - i - 1] = (instance->secplus_packet_1 >> (i * 8)) & 0xFF;
    }

    if((res == SubGhzProtocolStatusOk) &&
       !flipper_format_write_hex(flipper_format, "Secplus_packet_1", key_data, sizeof(uint64_t))) {
        FURI_LOG_E(TAG, "Unable to add Secplus_packet_1");
        res = SubGhzProtocolStatusErrorParserOthers;
    }
    return res == SubGhzProtocolStatusOk;
}

void* subghz_protocol_decoder_secplus_v2_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    return subghz_protocol_decoder_common_alloc(
        sizeof(SubGhzProtocolDecoderSecPlus_v2), &subghz_protocol_secplus_v2);
}

void subghz_protocol_decoder_secplus_v2_reset(void* context) {
    furi_assert(context);
    // SubGhzProtocolDecoderSecPlus_v2* instance = context;
    // does not reset the decoder because you need to get 2 parts of the package

    // a PIN left over from a loaded file does not belong to what is about to arrive
    secplus_v2_pin = SECPLUS_V2_PIN_UNSET;
}

static bool subghz_protocol_secplus_v2_check_packet(SubGhzProtocolDecoderSecPlus_v2* instance) {
    if(instance->decoder.decode_count_bit == SECPLUS_V2_86_COUNT_BIT) {
        /* 86 bit frame: 22 bits of preamble (0x3C | packet id) then 64 bits of payload.
         * The decoder only ever keeps the last 64 bits, the packet id was latched
         * separately once 22 bits had been shifted in. */
        if((instance->decoder.decode_data >> 62) != 1) return false;
        if(instance->packet_id == 0) {
            instance->secplus_packet_1 = instance->decoder.decode_data;
            instance->secplus_packet_1_count_bit = SECPLUS_V2_86_COUNT_BIT;
            return false;
        }
        return (instance->packet_id == 1) && (instance->secplus_packet_1 != 0) &&
               (instance->secplus_packet_1_count_bit == SECPLUS_V2_86_COUNT_BIT);
    }
    if((instance->decoder.decode_data & SECPLUS_V2_HEADER_MASK) == SECPLUS_V2_HEADER) {
        if((instance->decoder.decode_data & SECPLUS_V2_PACKET_MASK) == SECPLUS_V2_PACKET_1) {
            instance->secplus_packet_1 = instance->decoder.decode_data;
            instance->secplus_packet_1_count_bit =
                subghz_protocol_secplus_v2_const.min_count_bit_for_found;
        } else if(
            ((instance->decoder.decode_data & SECPLUS_V2_PACKET_MASK) == SECPLUS_V2_PACKET_2) &&
            (instance->secplus_packet_1) &&
            (instance->secplus_packet_1_count_bit ==
             subghz_protocol_secplus_v2_const.min_count_bit_for_found)) {
            return true;
        }
    }
    return false;
}

void subghz_protocol_decoder_secplus_v2_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderSecPlus_v2* instance = context;

    ManchesterEvent event = ManchesterEventReset;
    switch(instance->decoder.parser_step) {
    case SecPlus_v2DecoderStepReset:
        if((!level) &&
           ((DURATION_DIFF(
                 duration, subghz_protocol_secplus_v2_const.te_long * SECPLUS_V2_SYNC_62) <
             subghz_protocol_secplus_v2_const.te_delta * SECPLUS_V2_SYNC_62_DELTA) ||
            (DURATION_DIFF(
                 duration, subghz_protocol_secplus_v2_const.te_long * SECPLUS_V2_SYNC_86) <
             subghz_protocol_secplus_v2_const.te_delta * SECPLUS_V2_SYNC_86_DELTA))) {
            //Found header Security+ 2.0
            instance->decoder.parser_step = SecPlus_v2DecoderStepDecoderData;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->secplus_packet_1 = 0;
            instance->secplus_packet_1_count_bit = 0;
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
            manchester_advance(
                instance->manchester_saved_state,
                ManchesterEventShortLow,
                &instance->manchester_saved_state,
                NULL);
        }
        break;
    case SecPlus_v2DecoderStepDecoderData:
        if(!level) {
            if(DURATION_DIFF(duration, subghz_protocol_secplus_v2_const.te_short) <
               subghz_protocol_secplus_v2_const.te_delta) {
                event = ManchesterEventShortLow;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_secplus_v2_const.te_long) <
                subghz_protocol_secplus_v2_const.te_delta) {
                event = ManchesterEventLongLow;
            } else if(
                duration >= (subghz_protocol_secplus_v2_const.te_long * 2UL +
                             subghz_protocol_secplus_v2_const.te_delta)) {
                if((instance->decoder.decode_count_bit ==
                    subghz_protocol_secplus_v2_const.min_count_bit_for_found) ||
                   (instance->decoder.decode_count_bit == SECPLUS_V2_86_COUNT_BIT)) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;
                    if(subghz_protocol_secplus_v2_check_packet(instance)) {
                        if(instance->base.callback)
                            instance->base.callback(&instance->base, instance->base.context);
                        instance->decoder.parser_step = SecPlus_v2DecoderStepReset;
                    }
                }
                if((instance->secplus_packet_1_count_bit == SECPLUS_V2_86_COUNT_BIT) &&
                   (duration >
                    (subghz_protocol_secplus_v2_const.te_long * 1UL * SECPLUS_V2_PAIR_TIMEOUT))) {
                    // a keypad half this old cannot belong to whatever arrives next. 62 bit
                    // halves are left alone: they are routinely paired across repeats of the
                    // same transmission, which is what carries a weak signal.
                    instance->secplus_packet_1 = 0;
                    instance->secplus_packet_1_count_bit = 0;
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
                manchester_advance(
                    instance->manchester_saved_state,
                    ManchesterEventShortLow,
                    &instance->manchester_saved_state,
                    NULL);
            } else {
                instance->decoder.parser_step = SecPlus_v2DecoderStepReset;
            }
        } else {
            if(DURATION_DIFF(duration, subghz_protocol_secplus_v2_const.te_short) <
               subghz_protocol_secplus_v2_const.te_delta) {
                event = ManchesterEventShortHigh;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_secplus_v2_const.te_long) <
                subghz_protocol_secplus_v2_const.te_delta) {
                event = ManchesterEventLongHigh;
            } else {
                instance->decoder.parser_step = SecPlus_v2DecoderStepReset;
            }
        }
        if(event != ManchesterEventReset) {
            bool data;
            bool data_ok = manchester_advance(
                instance->manchester_saved_state, event, &instance->manchester_saved_state, &data);

            if(data_ok) {
                instance->decoder.decode_data = (instance->decoder.decode_data << 1) | data;
                instance->decoder.decode_count_bit++;
                // the 86 bit frame drops its preamble off the top of the accumulator,
                // so remember the packet id while it is still there
                if(instance->decoder.decode_count_bit == SECPLUS_V2_86_PREAMBLE_BIT) {
                    instance->packet_id = (uint8_t)(instance->decoder.decode_data & 0x03);
                }
            }
        }
        break;
    }
}

SubGhzProtocolStatus subghz_protocol_decoder_secplus_v2_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderSecPlus_v2* instance = context;

    /* Recover the PIN before the generic block goes out. remote_controller_86() is also
     * what fills in serial, counter and button, so doing it here means a Save that never
     * passed through the info screen still stores them. */
    uint16_t pin = SECPLUS_V2_PIN_UNSET;
    if(instance->generic.data_count_bit == SECPLUS_V2_86_COUNT_BIT) {
        uint32_t data_word = 0;
        if(subghz_protocol_secplus_v2_remote_controller_86(
               &instance->generic, instance->secplus_packet_1, &data_word)) {
            instance->secplus_data = data_word;
            pin = subghz_protocol_secplus_v2_pin_extract(data_word);
        }
    }

    SubGhzProtocolStatus ret =
        subghz_block_generic_serialize(&instance->generic, flipper_format, preset);

    uint8_t key_data[sizeof(uint64_t)] = {0};
    for(size_t i = 0; i < sizeof(uint64_t); i++) {
        key_data[sizeof(uint64_t) - i - 1] = (instance->secplus_packet_1 >> (i * 8)) & 0xFF;
    }

    if((ret == SubGhzProtocolStatusOk) &&
       !flipper_format_write_hex(flipper_format, "Secplus_packet_1", key_data, sizeof(uint64_t))) {
        FURI_LOG_E(TAG, "Unable to add Secplus_packet_1");
        ret = SubGhzProtocolStatusErrorParserOthers;
    }

    if((ret == SubGhzProtocolStatusOk) &&
       (instance->generic.data_count_bit == SECPLUS_V2_86_COUNT_BIT)) {
        /* Written last so it stays on the last line of the file: it is the one field a
         * user edits by hand, and the PIN app updates it in place. A PIN that cannot be
         * read stays blank, for the user or the PIN app to fill in. */
        FuriString* pin_str = furi_string_alloc();
        if(pin <= SECPLUS_V2_PIN_MAX) furi_string_printf(pin_str, "%04u", pin);
        if(!flipper_format_write_string(flipper_format, "Pin", pin_str)) {
            FURI_LOG_E(TAG, "Unable to add Pin");
            ret = SubGhzProtocolStatusErrorParserOthers;
        }
        furi_string_free(pin_str);
    }

    return ret;
}

SubGhzProtocolStatus
    subghz_protocol_decoder_secplus_v2_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderSecPlus_v2* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    // drop the previous signal's PIN first, so no exit from here can leave it standing
    secplus_v2_pin = SECPLUS_V2_PIN_UNSET;
    do {
        ret = subghz_block_generic_deserialize(&instance->generic, flipper_format);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        if((instance->generic.data_count_bit !=
            subghz_protocol_secplus_v2_const.min_count_bit_for_found) &&
           (instance->generic.data_count_bit != SECPLUS_V2_86_COUNT_BIT)) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            ret = SubGhzProtocolStatusErrorValueBitCount;
            break;
        }
        if(instance->generic.data_count_bit == SECPLUS_V2_86_COUNT_BIT) {
            secplus_v2_pin = subghz_protocol_secplus_v2_pin_read(flipper_format);
        }
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            ret = SubGhzProtocolStatusErrorParserOthers;
            break;
        }
        uint8_t key_data[sizeof(uint64_t)] = {0};
        if(!flipper_format_read_hex(
               flipper_format, "Secplus_packet_1", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Missing Secplus_packet_1");
            ret = SubGhzProtocolStatusErrorParserOthers;
            break;
        }
        for(uint8_t i = 0; i < sizeof(uint64_t); i++) {
            instance->secplus_packet_1 = instance->secplus_packet_1 << 8 | key_data[i];
        }
    } while(false);

    return ret;
}

static uint8_t subghz_protocol_secplus_v2_get_btn_code(void) {
    uint8_t custom_btn_id = subghz_custom_btn_get();
    uint8_t original_btn_code = subghz_custom_btn_get_original();
    uint8_t btn = original_btn_code;

    // Set custom button
    if((custom_btn_id == SUBGHZ_CUSTOM_BTN_OK) && (original_btn_code != 0)) {
        // Restore original button code
        btn = original_btn_code;
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_UP) {
        switch(original_btn_code) {
        case 0x68:
            btn = 0x80;
            break;
        case 0x80:
            btn = 0x68;
            break;
        case 0x81:
            btn = 0x80;
            break;
        case 0xE2:
            btn = 0x80;
            break;
        case 0x78:
            btn = 0x80;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_DOWN) {
        switch(original_btn_code) {
        case 0x68:
            btn = 0x81;
            break;
        case 0x80:
            btn = 0x81;
            break;
        case 0x81:
            btn = 0x68;
            break;
        case 0xE2:
            btn = 0x81;
            break;
        case 0x78:
            btn = 0x81;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_LEFT) {
        switch(original_btn_code) {
        case 0x68:
            btn = 0xE2;
            break;
        case 0x80:
            btn = 0xE2;
            break;
        case 0x81:
            btn = 0xE2;
            break;
        case 0xE2:
            btn = 0x68;
            break;
        case 0x78:
            btn = 0xE2;
            break;

        default:
            break;
        }
    } else if(custom_btn_id == SUBGHZ_CUSTOM_BTN_RIGHT) {
        switch(original_btn_code) {
        case 0x68:
            btn = 0x78;
            break;
        case 0x80:
            btn = 0x78;
            break;
        case 0x81:
            btn = 0x78;
            break;
        case 0xE2:
            btn = 0x78;
            break;
        case 0x78:
            btn = 0x68;
            break;

        default:
            break;
        }
    }

    return btn;
}

void subghz_protocol_decoder_secplus_v2_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderSecPlus_v2* instance = context;

    if(instance->generic.data_count_bit == SECPLUS_V2_86_COUNT_BIT) {
        uint32_t data_word = 0;
        bool ok = subghz_protocol_secplus_v2_remote_controller_86(
            &instance->generic, instance->secplus_packet_1, &data_word);
        instance->secplus_data = data_word;

        subghz_block_generic_global.cnt_is_available = ok;
        subghz_block_generic_global.cnt_length_bit = 28;
        subghz_block_generic_global.current_cnt = instance->generic.cnt;
        subghz_block_generic_global.btn_is_available = ok;
        subghz_block_generic_global.current_btn = instance->generic.btn;
        subghz_block_generic_global.btn_length_bit = 8;

        // a loaded file's "Pin" wins, otherwise use the one the frame itself carries
        uint16_t pin = secplus_v2_pin;
        if(pin > SECPLUS_V2_PIN_MAX) pin = subghz_protocol_secplus_v2_pin_extract(data_word);
        if(ok && (pin <= SECPLUS_V2_PIN_MAX)) {
            furi_string_cat_printf(
                output,
                /* The Save button occupies the bottom right 12 rows, so the last line
                 * has to stay short. Cnt moves up next to Sn to make room for Pin. */
                "%s %db\r\n"
                "Pk1:%lX%08lX\r\n"
                "Pk2:%lX%08lX\r\n"
                "Sn:%08lX Cnt:%07lX\r\n"
                "Btn:%01X  Pin:%04u\r\n",
                instance->generic.protocol_name,
                instance->generic.data_count_bit,
                (uint32_t)(instance->secplus_packet_1 >> 32),
                (uint32_t)instance->secplus_packet_1,
                (uint32_t)(instance->generic.data >> 32),
                (uint32_t)instance->generic.data,
                instance->generic.serial,
                instance->generic.cnt,
                instance->generic.btn,
                pin);
        } else {
            furi_string_cat_printf(
                output,
                "%s %db\r\n"
                "Pk1:%lX%08lX\r\n"
                "Pk2:%lX%08lX\r\n"
                "Sn:%08lX Cnt:%07lX\r\n"
                "Btn:%01X  Pin:----\r\n",
                instance->generic.protocol_name,
                instance->generic.data_count_bit,
                (uint32_t)(instance->secplus_packet_1 >> 32),
                (uint32_t)instance->secplus_packet_1,
                (uint32_t)(instance->generic.data >> 32),
                (uint32_t)instance->generic.data,
                instance->generic.serial,
                instance->generic.cnt,
                instance->generic.btn);
        }
        return;
    }

    subghz_protocol_secplus_v2_remote_controller(&instance->generic, instance->secplus_packet_1);

    // need to research or practice check how much bits in counter
    // push protocol data to global variable
    subghz_block_generic_global.cnt_is_available = true;
    subghz_block_generic_global.cnt_length_bit = 28;
    subghz_block_generic_global.current_cnt = instance->generic.cnt;

    subghz_block_generic_global.btn_is_available = true;
    subghz_block_generic_global.current_btn = instance->generic.btn;
    subghz_block_generic_global.btn_length_bit = 8;
    //

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Pk1:0x%lX%08lX\r\n"
        "Pk2:0x%lX%08lX\r\n"
        "Sn:0x%08lX  Btn:0x%01X\r\n"
        "Cnt:%07lX\r\n",

        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        (uint32_t)(instance->secplus_packet_1 >> 32),
        (uint32_t)instance->secplus_packet_1,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)instance->generic.data,
        instance->generic.serial,
        instance->generic.btn,
        instance->generic.cnt);
}
