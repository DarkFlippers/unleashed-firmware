#include "nero_radio.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"
#include "common.h"

#define TAG "SubGhzProtocolNeroRadio"

static const SubGhzBlockConst subghz_protocol_nero_radio_const = {
    .te_short = 200,
    .te_long = 400,
    .te_delta = 80,
    .min_count_bit_for_found = 56,
};

// "Rolling long range" (manual name) format: ~128 ms carrier, 34 x 200/200 preamble, 430/1240 sync,
// then 251 NRZ bits at 4800 baud: 11-bit header, 4 fixed sync words and 11 data words.
// Every data word is one of 64 balanced 16-chip codes, so it carries 6 bits.
// All 66 data bits change on each press (even the same button), so it can only be replayed.
// Stored as Key = symbols 0..9 (6 bits each), Data = symbol 10.
#define NERO_RADIO_DYN_COUNT_BIT   66
#define NERO_RADIO_DYN_TE          208
#define NERO_RADIO_DYN_FRAME_BITS  251
#define NERO_RADIO_DYN_HEADER      0x54C // 10101001100
#define NERO_RADIO_DYN_HEADER_BITS 11
#define NERO_RADIO_DYN_SYNC_WORDS  4
#define NERO_RADIO_DYN_DATA_WORDS  11
#define NERO_RADIO_DYN_PREAMBLE    34
#define NERO_RADIO_DYN_CARRIER     128700
#define NERO_RADIO_DYN_GAP         3000

static const uint16_t nero_radio_dyn_sync[NERO_RADIO_DYN_SYNC_WORDS] = {
    0x896C,
    0x7435,
    0xCF7C,
    0x117B,
};

// Data alphabet (0x7435 only shows up as a sync word). Sorted, so the symbol value is
// just the index; the real value mapping is unknown
static const uint16_t nero_radio_dyn_symbols[64] = {
    0x117B, 0x12BD, 0x13C7, 0x14F6, 0x172E, 0x1D65, 0x1D8B, 0x1E53, 0x1ECC, 0x22DE, 0x236D,
    0x2597, 0x25EA, 0x26F1, 0x29BC, 0x2E1D, 0x2E66, 0x33B2, 0x344F, 0x3754, 0x38D5, 0x3A2B,
    0x3C78, 0x43AB, 0x44DB, 0x453D, 0x4772, 0x4AF4, 0x4DA6, 0x4E69, 0x519E, 0x5791, 0x58B3,
    0x59E8, 0x5B0D, 0x66AC, 0x691B, 0x6A87, 0x6DD0, 0x7127, 0x7259, 0x76C2, 0x85CD, 0x86A7,
    0x871B, 0x8B6A, 0x8DB1, 0x98DA, 0x9AE1, 0x9B94, 0xA176, 0xA9C3, 0xAC8E, 0xB0EC, 0xB11D,
    0xB433, 0xB689, 0xC24F, 0xC89D, 0xCE16, 0xD23A, 0xD364, 0xD455, 0xD54A,
};

// "Normal rolling" (manual name) format, same 4800 baud chips:
// ~128 ms carrier, 8 x 200/200, 400 low, 1250 high, 52 fixed chips (same on every press,
// maybe the remote ID), 1260 low, 47 x 200/200, 830 low, then 251 chips:
// 1110 + 00100100 sync, 16 Hamming(7,4) words Manchester coded (1 = 10, 0 = 01) = 64 bits,
// then 2 more Hamming(7,4) words sent as raw chips with a 0 chip between them.
// The tail always repeats the last key byte. The high nibbles of bytes 1..6 depend only on
// the high nibble of byte 0 (16 entry table, h(n ^ D) = h(n) ^ DDDDDD on one remote), so only
// ~40 of the 64 bits carry the rolling part. The button changes the high nibbles of bytes 5..6.
// The last byte follows a press counter: counter bit 0 flipping always XORs it with 0x5F,
// bits 0..1 with 0x69, bits 0..3 and up with 0x14. The algorithm is still unknown, so it can
// only be replayed. Stored as Key = 64 bits, Data = 52 fixed chips << 8 | tail.
#define NERO_RADIO_NORM_COUNT_BIT  72
#define NERO_RADIO_NORM_HEADER     0xE24 // 111000100100
#define NERO_RADIO_NORM_HEAD_BITS  12
#define NERO_RADIO_NORM_TAIL_START (NERO_RADIO_NORM_HEAD_BITS + 16 * 7 * 2)
#define NERO_RADIO_NORM_ID_BITS    52
#define NERO_RADIO_NORM_ID_PREAM   8
#define NERO_RADIO_NORM_PREAMBLE   47

struct SubGhzProtocolDecoderNeroRadio {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint16_t header_count;
    uint16_t dyn_bit;
    uint16_t dyn_word;
    uint8_t dyn_last;
    bool norm_id_ok;
    uint64_t norm_id;
    uint8_t norm_word;
    uint8_t norm_tail;
};
SUBGHZ_ASSERT_DECODER_COMMON_LAYOUT(SubGhzProtocolDecoderNeroRadio);

struct SubGhzProtocolEncoderNeroRadio {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};
SUBGHZ_ASSERT_ENCODER_GENERIC_LAYOUT(SubGhzProtocolEncoderNeroRadio);

typedef enum {
    NeroRadioDecoderStepReset = 0,
    NeroRadioDecoderStepCheckPreambula,
    NeroRadioDecoderStepSaveDuration,
    NeroRadioDecoderStepCheckDuration,
    NeroRadioDecoderStepDynData,
    NeroRadioDecoderStepNormIdSync,
    NeroRadioDecoderStepNormId,
    NeroRadioDecoderStepNormData,
} NeroRadioDecoderStep;

/**
 * Hamming(7,4) codeword of a nibble, data bits first.
 * @param nibble Data
 * @return 7-bit codeword
 */
static uint8_t subghz_protocol_nero_radio_hamming(uint8_t nibble) {
    static const uint8_t parity[4] = {0x7, 0x6, 0x5, 0x3}; // for data bits 0..3
    uint8_t par = 0;
    for(uint8_t i = 0; i < 4; i++) {
        if(nibble & (1 << i)) par ^= parity[i];
    }
    return (nibble << 3) | par;
}

static uint8_t subghz_protocol_decoder_nero_radio_get_hash_data(void* context);
static SubGhzProtocolStatus subghz_protocol_decoder_nero_radio_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

const SubGhzProtocolDecoder subghz_protocol_nero_radio_decoder = {
    .alloc = subghz_protocol_decoder_nero_radio_alloc,
    .free = subghz_protocol_decoder_common_free,

    .feed = subghz_protocol_decoder_nero_radio_feed,
    .reset = subghz_protocol_decoder_common_reset,

    .get_hash_data = subghz_protocol_decoder_nero_radio_get_hash_data,
    .serialize = subghz_protocol_decoder_nero_radio_serialize,
    .deserialize = subghz_protocol_decoder_nero_radio_deserialize,
    .get_string = subghz_protocol_decoder_nero_radio_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_nero_radio_encoder = {
    .alloc = subghz_protocol_encoder_nero_radio_alloc,
    .free = subghz_protocol_encoder_common_free,

    .deserialize = subghz_protocol_encoder_nero_radio_deserialize,
    .stop = subghz_protocol_encoder_common_stop,
    .yield = subghz_protocol_encoder_common_yield,
};

const SubGhzProtocol subghz_protocol_nero_radio = {
    .name = SUBGHZ_PROTOCOL_NERO_RADIO_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable |
            SubGhzProtocolFlag_Load | SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_nero_radio_decoder,
    .encoder = &subghz_protocol_nero_radio_encoder,
};

void* subghz_protocol_encoder_nero_radio_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    return subghz_protocol_encoder_common_alloc(
        sizeof(SubGhzProtocolEncoderNeroRadio), &subghz_protocol_nero_radio, 3, 440);
}

typedef struct {
    LevelDuration* upload;
    size_t index;
    bool level;
    uint32_t run;
} NeroRadioChipWriter;

static void nero_radio_chip(NeroRadioChipWriter* writer, bool bit) {
    if((bit != writer->level) && writer->run) {
        writer->upload[writer->index++] = level_duration_make(writer->level, writer->run);
        writer->run = 0;
    }
    writer->level = bit;
    writer->run += NERO_RADIO_DYN_TE;
}

static void nero_radio_chip_gap(NeroRadioChipWriter* writer, uint32_t gap) {
    if(writer->level) {
        writer->upload[writer->index++] = level_duration_make(true, writer->run);
        writer->run = 0;
    }
    writer->upload[writer->index++] = level_duration_make(false, writer->run + gap);
    writer->level = false;
    writer->run = 0;
}

static void nero_radio_preamble(NeroRadioChipWriter* writer, uint8_t count, uint32_t last_low) {
    for(uint8_t i = 0; i < count; i++) {
        writer->upload[writer->index++] = level_duration_make(true, NERO_RADIO_DYN_TE);
        writer->upload[writer->index++] =
            level_duration_make(false, (i == count - 1) ? last_low : NERO_RADIO_DYN_TE);
    }
}

/**
 * Generating an upload for the rolling long range format.
 * @param instance Pointer to a SubGhzProtocolEncoderNeroRadio instance
 * @return true On success
 */
static bool
    subghz_protocol_encoder_nero_radio_get_upload_dyn(SubGhzProtocolEncoderNeroRadio* instance) {
    // Carrier + preamble + sync + worst case one level per bit + gap
    size_t size_upload = 2 + NERO_RADIO_DYN_PREAMBLE * 2 + 2 + NERO_RADIO_DYN_FRAME_BITS + 1;
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    }
    NeroRadioChipWriter writer = {.upload = instance->encoder.upload};

    writer.upload[writer.index++] = level_duration_make(true, NERO_RADIO_DYN_CARRIER);
    writer.upload[writer.index++] = level_duration_make(false, 620);
    nero_radio_preamble(&writer, NERO_RADIO_DYN_PREAMBLE, NERO_RADIO_DYN_TE);
    writer.upload[writer.index++] = level_duration_make(true, 430);
    writer.upload[writer.index++] = level_duration_make(false, 1240);

    uint16_t words[NERO_RADIO_DYN_SYNC_WORDS + NERO_RADIO_DYN_DATA_WORDS];
    for(uint8_t i = 0; i < NERO_RADIO_DYN_SYNC_WORDS; i++) {
        words[i] = nero_radio_dyn_sync[i];
    }
    for(uint8_t i = 0; i < NERO_RADIO_DYN_DATA_WORDS - 1; i++) {
        words[NERO_RADIO_DYN_SYNC_WORDS + i] =
            nero_radio_dyn_symbols[(instance->generic.data >> ((9 - i) * 6)) & 0x3F];
    }
    words[NERO_RADIO_DYN_SYNC_WORDS + NERO_RADIO_DYN_DATA_WORDS - 1] =
        nero_radio_dyn_symbols[instance->generic.data_2 & 0x3F];

    for(uint16_t pos = 0; pos < NERO_RADIO_DYN_HEADER_BITS; pos++) {
        nero_radio_chip(
            &writer, (NERO_RADIO_DYN_HEADER >> (NERO_RADIO_DYN_HEADER_BITS - 1 - pos)) & 1);
    }
    for(uint16_t chip = 0; chip < COUNT_OF(words) * 16; chip++) {
        nero_radio_chip(&writer, (words[chip >> 4] >> (15 - (chip & 0xF))) & 1);
    }
    nero_radio_chip_gap(&writer, NERO_RADIO_DYN_GAP);

    instance->encoder.size_upload = writer.index;
    return true;
}

/**
 * Generating an upload for the normal rolling format.
 * @param instance Pointer to a SubGhzProtocolEncoderNeroRadio instance
 * @return true On success
 */
static bool
    subghz_protocol_encoder_nero_radio_get_upload_norm(SubGhzProtocolEncoderNeroRadio* instance) {
    // Carrier, id preamble + sync, id, preamble + start, worst case one level per chip, gap
    size_t size_upload = 2 + NERO_RADIO_NORM_ID_PREAM * 2 + 1 + NERO_RADIO_NORM_ID_BITS + 1 +
                         (NERO_RADIO_NORM_PREAMBLE + 1) * 2 + NERO_RADIO_DYN_FRAME_BITS + 1;
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    }
    NeroRadioChipWriter writer = {.upload = instance->encoder.upload};

    writer.upload[writer.index++] = level_duration_make(true, NERO_RADIO_DYN_CARRIER);
    writer.upload[writer.index++] = level_duration_make(false, 420);
    nero_radio_preamble(&writer, NERO_RADIO_NORM_ID_PREAM, 430);
    writer.upload[writer.index++] = level_duration_make(true, 1250);
    uint64_t id = instance->generic.data_2 >> 8;
    for(uint8_t i = 0; i < NERO_RADIO_NORM_ID_BITS; i++) {
        nero_radio_chip(&writer, (id >> (NERO_RADIO_NORM_ID_BITS - 1 - i)) & 1);
    }
    nero_radio_chip_gap(&writer, 1260);
    nero_radio_preamble(&writer, NERO_RADIO_NORM_PREAMBLE + 1, 830);

    for(uint8_t i = 0; i < NERO_RADIO_NORM_HEAD_BITS; i++) {
        nero_radio_chip(
            &writer, (NERO_RADIO_NORM_HEADER >> (NERO_RADIO_NORM_HEAD_BITS - 1 - i)) & 1);
    }
    for(uint8_t i = 0; i < 16; i++) {
        uint8_t word =
            subghz_protocol_nero_radio_hamming((instance->generic.data >> ((15 - i) * 4)) & 0xF);
        for(uint8_t j = 0; j < 7; j++) {
            bool bit = (word >> (6 - j)) & 1;
            nero_radio_chip(&writer, bit);
            nero_radio_chip(&writer, !bit);
        }
    }
    for(uint8_t i = 0; i < 2; i++) {
        uint8_t word =
            subghz_protocol_nero_radio_hamming((instance->generic.data_2 >> ((1 - i) * 4)) & 0xF);
        if(i) nero_radio_chip(&writer, false);
        for(uint8_t j = 0; j < 7; j++) {
            nero_radio_chip(&writer, (word >> (6 - j)) & 1);
        }
    }
    nero_radio_chip_gap(&writer, NERO_RADIO_DYN_GAP);

    instance->encoder.size_upload = writer.index;
    return true;
}

/**
 * Generating an upload from data.
 * @param instance Pointer to a SubGhzProtocolEncoderNeroRadio instance
 * @return true On success
 */
static bool
    subghz_protocol_encoder_nero_radio_get_upload(SubGhzProtocolEncoderNeroRadio* instance) {
    furi_assert(instance);
    if(instance->generic.data_count_bit == NERO_RADIO_DYN_COUNT_BIT) {
        return subghz_protocol_encoder_nero_radio_get_upload_dyn(instance);
    }
    if(instance->generic.data_count_bit == NERO_RADIO_NORM_COUNT_BIT) {
        return subghz_protocol_encoder_nero_radio_get_upload_norm(instance);
    }
    size_t index = 0;
    size_t size_upload = 49 * 2 + 2 + (instance->generic.data_count_bit * 2);
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }

    //Send header
    for(uint8_t i = 0; i < 49; i++) {
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_nero_radio_const.te_short);
        instance->encoder.upload[index++] =
            level_duration_make(false, (uint32_t)subghz_protocol_nero_radio_const.te_short);
    }

    //Send start bit
    instance->encoder.upload[index++] = level_duration_make(true, (uint32_t)830);
    instance->encoder.upload[index++] =
        level_duration_make(false, (uint32_t)subghz_protocol_nero_radio_const.te_short);

    //Send key data
    for(uint8_t i = instance->generic.data_count_bit; i > 1; i--) {
        if(bit_read(instance->generic.data, i - 1)) {
            //send bit 1
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_nero_radio_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_nero_radio_const.te_short);
        } else {
            //send bit 0
            instance->encoder.upload[index++] =
                level_duration_make(true, (uint32_t)subghz_protocol_nero_radio_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, (uint32_t)subghz_protocol_nero_radio_const.te_long);
        }
    }
    if(bit_read(instance->generic.data, 0)) {
        //send bit 1
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_nero_radio_const.te_long);
        if(instance->generic.data_count_bit == 57) {
            instance->encoder.upload[index++] = level_duration_make(false, (uint32_t)1300);
        } else {
            instance->encoder.upload[index++] = level_duration_make(
                false, (uint32_t)subghz_protocol_nero_radio_const.te_short * 23);
        }
    } else {
        //send bit 0
        instance->encoder.upload[index++] =
            level_duration_make(true, (uint32_t)subghz_protocol_nero_radio_const.te_short);
        if(instance->generic.data_count_bit == 57) {
            instance->encoder.upload[index++] = level_duration_make(false, (uint32_t)1300);
        } else {
            instance->encoder.upload[index++] = level_duration_make(
                false, (uint32_t)subghz_protocol_nero_radio_const.te_short * 23);
        }
    }
    return true;
}

/**
 * Deserialize the key, accepting the 56/57 bit static and the 66 bit dynamic format.
 * @param generic Pointer to a SubGhzBlockGeneric instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
static SubGhzProtocolStatus subghz_protocol_nero_radio_deserialize_key(
    SubGhzBlockGeneric* generic,
    FlipperFormat* flipper_format) {
    SubGhzProtocolStatus ret = subghz_block_generic_deserialize_check_count_bit(
        generic, flipper_format, subghz_protocol_nero_radio_const.min_count_bit_for_found);
    if(ret != SubGhzProtocolStatusErrorValueBitCount) {
        return ret;
    }
    if(generic->data_count_bit == 57) {
        return SubGhzProtocolStatusOk;
    }
    if((generic->data_count_bit != NERO_RADIO_DYN_COUNT_BIT) &&
       (generic->data_count_bit != NERO_RADIO_NORM_COUNT_BIT)) {
        return ret;
    }

    uint8_t key_data[sizeof(uint64_t)] = {0};
    if(!flipper_format_rewind(flipper_format) ||
       !flipper_format_read_hex(flipper_format, "Data", key_data, sizeof(uint64_t))) {
        FURI_LOG_E(TAG, "Missing Data");
        return SubGhzProtocolStatusErrorParserOthers;
    }
    generic->data_2 = 0;
    for(uint8_t i = 0; i < sizeof(uint64_t); i++) {
        generic->data_2 = generic->data_2 << 8 | key_data[i];
    }
    return SubGhzProtocolStatusOk;
}

SubGhzProtocolStatus
    subghz_protocol_encoder_nero_radio_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderNeroRadio* instance = context;
    SubGhzProtocolStatus ret = SubGhzProtocolStatusError;
    do {
        ret = subghz_protocol_nero_radio_deserialize_key(&instance->generic, flipper_format);
        if(ret != SubGhzProtocolStatusOk) {
            break;
        }
        // Optional value
        flipper_format_rewind(flipper_format);
        flipper_format_read_uint32(
            flipper_format, "Repeat", (uint32_t*)&instance->encoder.repeat, 1);

        if(!subghz_protocol_encoder_nero_radio_get_upload(instance)) {
            ret = SubGhzProtocolStatusErrorEncoderGetUpload;
            break;
        }
        instance->encoder.is_running = true;
    } while(false);

    return ret;
}

void* subghz_protocol_decoder_nero_radio_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    return subghz_protocol_decoder_common_alloc(
        sizeof(SubGhzProtocolDecoderNeroRadio), &subghz_protocol_nero_radio);
}

/**
 * Add one NRZ chip of the dynamic format, checking header and sync words.
 * @param instance Pointer to a SubGhzProtocolDecoderNeroRadio instance
 * @param bit Chip value
 * @return false if the frame does not match
 */
static bool
    subghz_protocol_nero_radio_dyn_add_bit(SubGhzProtocolDecoderNeroRadio* instance, bool bit) {
    uint16_t pos = instance->dyn_bit++;
    if(pos >= NERO_RADIO_DYN_FRAME_BITS) return false;
    if(pos < NERO_RADIO_DYN_HEADER_BITS) {
        return bit == ((NERO_RADIO_DYN_HEADER >> (NERO_RADIO_DYN_HEADER_BITS - 1 - pos)) & 1);
    }

    pos -= NERO_RADIO_DYN_HEADER_BITS;
    instance->dyn_word = (instance->dyn_word << 1) | bit;
    if((pos & 0xF) != 0xF) return true;

    uint8_t word = pos >> 4;
    if(word < NERO_RADIO_DYN_SYNC_WORDS) {
        return instance->dyn_word == nero_radio_dyn_sync[word];
    }
    for(uint8_t i = 0; i < COUNT_OF(nero_radio_dyn_symbols); i++) {
        if(nero_radio_dyn_symbols[i] != instance->dyn_word) continue;
        if(word == NERO_RADIO_DYN_SYNC_WORDS + NERO_RADIO_DYN_DATA_WORDS - 1) {
            instance->dyn_last = i;
        } else {
            instance->decoder.decode_data = (instance->decoder.decode_data << 6) | i;
        }
        return true;
    }
    return false;
}

/**
 * Add one chip of the normal rolling format: header, Manchester coded Hamming words, raw tail.
 * @param instance Pointer to a SubGhzProtocolDecoderNeroRadio instance
 * @param bit Chip value
 * @return false if the frame does not match
 */
static bool
    subghz_protocol_nero_radio_norm_add_bit(SubGhzProtocolDecoderNeroRadio* instance, bool bit) {
    uint16_t pos = instance->dyn_bit++;
    if(pos >= NERO_RADIO_DYN_FRAME_BITS) return false;
    if(pos < NERO_RADIO_NORM_HEAD_BITS) {
        return bit == ((NERO_RADIO_NORM_HEADER >> (NERO_RADIO_NORM_HEAD_BITS - 1 - pos)) & 1);
    }

    uint16_t word_end;
    if(pos < NERO_RADIO_NORM_TAIL_START) {
        uint16_t rel = pos - NERO_RADIO_NORM_HEAD_BITS;
        if(!(rel & 1)) {
            instance->dyn_word = bit;
            return true;
        }
        if(instance->dyn_word == bit) return false; // not Manchester
        instance->norm_word = (instance->norm_word << 1) | instance->dyn_word;
        word_end = (rel >> 1) % 7 == 6;
    } else {
        uint16_t rel = pos - NERO_RADIO_NORM_TAIL_START;
        if(rel == 7) return !bit;
        instance->norm_word = (instance->norm_word << 1) | bit;
        word_end = (rel == 6) || (rel == 14);
    }
    if(!word_end) return true;

    for(uint8_t nibble = 0; nibble < 16; nibble++) {
        if(subghz_protocol_nero_radio_hamming(nibble) != (instance->norm_word & 0x7F)) continue;
        if(pos < NERO_RADIO_NORM_TAIL_START) {
            instance->decoder.decode_data = (instance->decoder.decode_data << 4) | nibble;
        } else {
            instance->norm_tail = (instance->norm_tail << 4) | nibble;
        }
        return true;
    }
    return false;
}

void subghz_protocol_decoder_nero_radio_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderNeroRadio* instance = context;

    switch(instance->decoder.parser_step) {
    case NeroRadioDecoderStepReset:
        if((level) && (DURATION_DIFF(duration, subghz_protocol_nero_radio_const.te_short) <
                       subghz_protocol_nero_radio_const.te_delta)) {
            instance->decoder.parser_step = NeroRadioDecoderStepCheckPreambula;
            instance->decoder.te_last = duration;
            instance->header_count = 0;
        }
        break;
    case NeroRadioDecoderStepCheckPreambula:
        if(level) {
            if((DURATION_DIFF(duration, subghz_protocol_nero_radio_const.te_short) <
                subghz_protocol_nero_radio_const.te_delta) ||
               (DURATION_DIFF(duration, subghz_protocol_nero_radio_const.te_short * 4) <
                subghz_protocol_nero_radio_const.te_delta) ||
               (DURATION_DIFF(duration, subghz_protocol_nero_radio_const.te_long) <
                subghz_protocol_nero_radio_const.te_delta)) {
                instance->decoder.te_last = duration;
            } else {
                instance->decoder.parser_step = NeroRadioDecoderStepReset;
            }
        } else if(
            (DURATION_DIFF(duration, 1240) < 200) &&
            (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_nero_radio_const.te_long) <
             subghz_protocol_nero_radio_const.te_delta)) {
            // Found dynamic format sync
            if(instance->header_count > 25) {
                instance->decoder.parser_step = NeroRadioDecoderStepDynData;
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->dyn_bit = 0;
                instance->dyn_word = 0;
            } else {
                instance->decoder.parser_step = NeroRadioDecoderStepReset;
            }
        } else if(
            (DURATION_DIFF(duration, subghz_protocol_nero_radio_const.te_long) <
             subghz_protocol_nero_radio_const.te_delta) &&
            (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_nero_radio_const.te_short) <
             subghz_protocol_nero_radio_const.te_delta) &&
            (instance->header_count >= NERO_RADIO_NORM_ID_PREAM - 3)) {
            // Found normal rolling id block sync, 1250 us high follows
            instance->decoder.parser_step = NeroRadioDecoderStepNormIdSync;
        } else if(
            (DURATION_DIFF(duration, 830) < 120) &&
            (DURATION_DIFF(instance->decoder.te_last, subghz_protocol_nero_radio_const.te_short) <
             subghz_protocol_nero_radio_const.te_delta) &&
            (instance->header_count > 40)) {
            // Found normal rolling start
            instance->decoder.parser_step = NeroRadioDecoderStepNormData;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->dyn_bit = 0;
            instance->norm_word = 0;
            instance->norm_tail = 0;
        } else if(
            DURATION_DIFF(duration, subghz_protocol_nero_radio_const.te_short) <
            subghz_protocol_nero_radio_const.te_delta) {
            if(DURATION_DIFF(instance->decoder.te_last, subghz_protocol_nero_radio_const.te_short) <
               subghz_protocol_nero_radio_const.te_delta) {
                // Found header
                instance->header_count++;
                break;
            } else if(
                DURATION_DIFF(
                    instance->decoder.te_last, subghz_protocol_nero_radio_const.te_short * 4) <
                subghz_protocol_nero_radio_const.te_delta) {
                // Found start bit
                if(instance->header_count > 40) {
                    instance->decoder.parser_step = NeroRadioDecoderStepSaveDuration;
                    instance->decoder.decode_data = 0;
                    instance->decoder.decode_count_bit = 0;
                } else {
                    instance->decoder.parser_step = NeroRadioDecoderStepReset;
                }
            } else {
                instance->decoder.parser_step = NeroRadioDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = NeroRadioDecoderStepReset;
        }
        break;
    case NeroRadioDecoderStepSaveDuration:
        if(level) {
            instance->decoder.te_last = duration;
            instance->decoder.parser_step = NeroRadioDecoderStepCheckDuration;
        } else {
            instance->decoder.parser_step = NeroRadioDecoderStepReset;
        }
        break;
    case NeroRadioDecoderStepCheckDuration:
        if(!level) {
            // Data lows are 200/400 us, frames end with 1.0..1.3 ms or a longer gap
            if(duration >= ((uint32_t)900)) {
                //Found stop bit
                if(DURATION_DIFF(
                       instance->decoder.te_last, subghz_protocol_nero_radio_const.te_short) <
                   subghz_protocol_nero_radio_const.te_delta) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                } else if(
                    DURATION_DIFF(
                        instance->decoder.te_last, subghz_protocol_nero_radio_const.te_long) <
                    subghz_protocol_nero_radio_const.te_delta) {
                    subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                }
                instance->decoder.parser_step = NeroRadioDecoderStepReset;
                if((instance->decoder.decode_count_bit ==
                    subghz_protocol_nero_radio_const.min_count_bit_for_found) ||
                   (instance->decoder.decode_count_bit ==
                    subghz_protocol_nero_radio_const.min_count_bit_for_found + 1)) {
                    instance->generic.data = instance->decoder.decode_data;
                    instance->generic.data_count_bit = instance->decoder.decode_count_bit;

                    if(instance->base.callback)
                        instance->base.callback(&instance->base, instance->base.context);
                }
                instance->decoder.decode_data = 0;
                instance->decoder.decode_count_bit = 0;
                instance->decoder.parser_step = NeroRadioDecoderStepReset; //-V1048
                break;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_nero_radio_const.te_short) <
                 subghz_protocol_nero_radio_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_nero_radio_const.te_long) <
                 subghz_protocol_nero_radio_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 0);
                instance->decoder.parser_step = NeroRadioDecoderStepSaveDuration;
            } else if(
                (DURATION_DIFF(
                     instance->decoder.te_last, subghz_protocol_nero_radio_const.te_long) <
                 subghz_protocol_nero_radio_const.te_delta) &&
                (DURATION_DIFF(duration, subghz_protocol_nero_radio_const.te_short) <
                 subghz_protocol_nero_radio_const.te_delta)) {
                subghz_protocol_blocks_add_bit(&instance->decoder, 1);
                instance->decoder.parser_step = NeroRadioDecoderStepSaveDuration;
            } else {
                instance->decoder.parser_step = NeroRadioDecoderStepReset;
            }
        } else {
            instance->decoder.parser_step = NeroRadioDecoderStepReset;
        }
        break;
    case NeroRadioDecoderStepDynData:
        if(!level && (duration >= 1500)) {
            // Gap: trailing zero chips of the last word merge into it
            bool valid = (NERO_RADIO_DYN_FRAME_BITS - instance->dyn_bit) <= 8;
            while(valid && instance->dyn_bit < NERO_RADIO_DYN_FRAME_BITS) {
                valid = subghz_protocol_nero_radio_dyn_add_bit(instance, false);
            }
            if(valid) {
                instance->generic.data = instance->decoder.decode_data;
                instance->generic.data_2 = instance->dyn_last;
                instance->generic.data_count_bit = NERO_RADIO_DYN_COUNT_BIT;
                instance->decoder.decode_count_bit = NERO_RADIO_DYN_COUNT_BIT;

                if(instance->base.callback)
                    instance->base.callback(&instance->base, instance->base.context);
            }
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = NeroRadioDecoderStepReset;
        } else {
            uint32_t count = (duration + NERO_RADIO_DYN_TE / 2) / NERO_RADIO_DYN_TE;
            if((count == 0) || (count > 7)) {
                instance->decoder.parser_step = NeroRadioDecoderStepReset;
                break;
            }
            for(uint32_t i = 0; i < count; i++) {
                if(!subghz_protocol_nero_radio_dyn_add_bit(instance, level)) {
                    instance->decoder.parser_step = NeroRadioDecoderStepReset;
                    break;
                }
            }
        }
        break;
    case NeroRadioDecoderStepNormIdSync:
        if(level && (DURATION_DIFF(duration, 1250) < 200)) {
            instance->decoder.parser_step = NeroRadioDecoderStepNormId;
            instance->dyn_bit = 0;
            instance->norm_id = 0;
            instance->norm_id_ok = false;
        } else {
            instance->decoder.parser_step = NeroRadioDecoderStepReset;
        }
        break;
    case NeroRadioDecoderStepNormId:
        if(!level && (duration >= 1000)) {
            // Low before the main preamble ends the id block
            instance->norm_id_ok = (instance->dyn_bit == NERO_RADIO_NORM_ID_BITS);
            instance->decoder.parser_step = NeroRadioDecoderStepReset;
        } else {
            uint32_t count = (duration + NERO_RADIO_DYN_TE / 2) / NERO_RADIO_DYN_TE;
            instance->dyn_bit += count;
            if((count == 0) || (count > 7) || (instance->dyn_bit > NERO_RADIO_NORM_ID_BITS)) {
                instance->decoder.parser_step = NeroRadioDecoderStepReset;
                break;
            }
            for(uint32_t i = 0; i < count; i++) {
                instance->norm_id = (instance->norm_id << 1) | level;
            }
        }
        break;
    case NeroRadioDecoderStepNormData:
        if(!level && (duration >= 1500)) {
            // Gap: trailing zero chips of the tail merge into it
            bool valid = instance->norm_id_ok &&
                         (NERO_RADIO_DYN_FRAME_BITS - instance->dyn_bit) <= 16;
            while(valid && instance->dyn_bit < NERO_RADIO_DYN_FRAME_BITS) {
                valid = subghz_protocol_nero_radio_norm_add_bit(instance, false);
            }
            // The raw tail repeats the last key byte
            valid = valid && (instance->norm_tail == (instance->decoder.decode_data & 0xFF));
            if(valid) {
                instance->generic.data = instance->decoder.decode_data;
                instance->generic.data_2 = (instance->norm_id << 8) | instance->norm_tail;
                instance->generic.data_count_bit = NERO_RADIO_NORM_COUNT_BIT;
                instance->decoder.decode_count_bit = NERO_RADIO_NORM_COUNT_BIT;

                if(instance->base.callback)
                    instance->base.callback(&instance->base, instance->base.context);
            }
            instance->norm_id_ok = false;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
            instance->decoder.parser_step = NeroRadioDecoderStepReset;
        } else {
            uint32_t count = (duration + NERO_RADIO_DYN_TE / 2) / NERO_RADIO_DYN_TE;
            if((count == 0) || (count > 16)) {
                instance->decoder.parser_step = NeroRadioDecoderStepReset;
                break;
            }
            for(uint32_t i = 0; i < count; i++) {
                if(!subghz_protocol_nero_radio_norm_add_bit(instance, level)) {
                    instance->decoder.parser_step = NeroRadioDecoderStepReset;
                    break;
                }
            }
        }
        break;
    }
}

SubGhzProtocolStatus
    subghz_protocol_decoder_nero_radio_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderNeroRadio* instance = context;
    return subghz_protocol_nero_radio_deserialize_key(&instance->generic, flipper_format);
}

static SubGhzProtocolStatus subghz_protocol_decoder_nero_radio_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderNeroRadio* instance = context;
    SubGhzProtocolStatus ret =
        subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
    if((instance->generic.data_count_bit == NERO_RADIO_DYN_COUNT_BIT) ||
       (instance->generic.data_count_bit == NERO_RADIO_NORM_COUNT_BIT)) {
        ret = subghz_protocol_common_append_data_2(ret, &instance->generic, flipper_format);
    }
    return ret;
}

static uint8_t subghz_protocol_decoder_nero_radio_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderNeroRadio* instance = context;
    if(instance->decoder.decode_count_bit == NERO_RADIO_DYN_COUNT_BIT) {
        return subghz_protocol_blocks_get_hash_data(&instance->decoder, sizeof(uint64_t)) ^
               instance->dyn_last;
    }
    if(instance->decoder.decode_count_bit == NERO_RADIO_NORM_COUNT_BIT) {
        return subghz_protocol_blocks_get_hash_data(&instance->decoder, sizeof(uint64_t)) ^
               instance->norm_tail;
    }
    return subghz_protocol_decoder_common_get_hash_data(context);
}

/** 
 * Analysis of received data
 * @param instance Pointer to a SubGhzBlockGeneric* instance
 */
static void subghz_protocol_nero_radio_parse_data(SubGhzBlockGeneric* instance) {
    // Key samples from unit tests
    // 57250501049DD3
    // 57250502049D13
    //
    // Samples from remote
    // 36E4E80104A644
    // 36E4E80204A684
    // 36E4E80304A604
    // 36E4E80404A6E4
    //
    // Held button, one remote (n = hold counter, steps every ~8 frames, restarts per press)
    // F6C18102 n1 6r E5   r = n bit reversed: 0160 1168 2164 316C ... D16B F16F
    // F6C18101 n1 6r 25
    // 57 bit remote, extra bit trails: B2230602 n4 Br 96 + 1, r = 2 for n = 1
    // So r = bit reversed n ^ per remote const (0, A, D seen)

    // possible contents
    // serial  button  hold cnt  ?    check
    // 5725050 1       0         49D  D3
    // 5725050 2       0         49D  13
    // 36E4E80 1       0         4A6  44
    // F6C1810 2       D         16B  E5
    // check depends on serial and button only, not a CRC8 or sum

    // 57 bit frames carry one extra trailing bit
    uint64_t data = instance->data >> (instance->data_count_bit - 56);
    instance->serial = data >> 28;
    instance->btn = (data >> 24) & 0xF;
    instance->cnt = (data >> 20) & 0xF;
}

void subghz_protocol_decoder_nero_radio_get_string(void* context, FuriString* output) {
    furi_assert(context);
    SubGhzProtocolDecoderNeroRadio* instance = context;

    uint32_t code_found_hi = instance->generic.data >> 32;
    uint32_t code_found_lo = instance->generic.data & 0x00000000ffffffff;

    if(instance->generic.data_count_bit == NERO_RADIO_DYN_COUNT_BIT) {
        subghz_block_generic_global.btn_is_available = false;
        furi_string_cat_printf(
            output,
            "%s %dbit\r\n"
            "Key:0x%lX%08lX\r\n"
            "Data:%02X\r\n"
            "Rolling long range,\r\nreplay only\r\n",
            instance->generic.protocol_name,
            instance->generic.data_count_bit,
            code_found_hi,
            code_found_lo,
            (uint8_t)instance->generic.data_2);
        return;
    }
    if(instance->generic.data_count_bit == NERO_RADIO_NORM_COUNT_BIT) {
        subghz_block_generic_global.btn_is_available = false;
        uint64_t id = instance->generic.data_2 >> 8;
        furi_string_cat_printf(
            output,
            "%s %dbit\r\n"
            "Key:0x%08lX%08lX\r\n"
            "Id:0x%05lX%08lX Tail:%02X\r\n"
            "Rolling, replay only\r\n",
            instance->generic.protocol_name,
            instance->generic.data_count_bit,
            code_found_hi,
            code_found_lo,
            (uint32_t)(id >> 32),
            (uint32_t)id,
            (uint8_t)instance->generic.data_2);
        return;
    }

    uint64_t code_found_reverse = subghz_protocol_blocks_reverse_key(
        instance->generic.data, instance->generic.data_count_bit);

    uint32_t code_found_reverse_hi = code_found_reverse >> 32;
    uint32_t code_found_reverse_lo = code_found_reverse & 0x00000000ffffffff;

    subghz_protocol_nero_radio_parse_data(&instance->generic);

    // push protocol data to global variable
    subghz_block_generic_global.btn_is_available = false;
    subghz_block_generic_global.current_btn = instance->generic.btn;
    subghz_block_generic_global.btn_length_bit = 4;
    //

    furi_string_cat_printf(
        output,
        "%s %dbit\r\n"
        "Key:0x%lX%08lX\r\n"
        "Yek:0x%lX%08lX\r\n"
        "Sn:0x%07lX Btn:%X\r\n"
        "Hold:%lX Chk:0x%02X\r\n",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        code_found_hi,
        code_found_lo,
        code_found_reverse_hi,
        code_found_reverse_lo,
        instance->generic.serial,
        instance->generic.btn,
        instance->generic.cnt,
        (uint8_t)(instance->generic.data >> (instance->generic.data_count_bit - 56)));
}
