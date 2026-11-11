// Decoder-only port of the Momentum Firmware Hormann BiSecur protocol.
// Original protocol implementation by user890104.

#include "hormann_bisecur.h"

#include <lib/toolbox/manchester_decoder.h>
#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/generic.h>
#include <lib/subghz/blocks/math.h>

#define TAG "RaHormannBiSecur"

#define HORMANN_BISECUR_DATA_SIZE 22

static const SubGhzBlockConst ra_protocol_hormann_bisecur_const = {
    .te_short = 208,
    .te_long = 416,
    .te_delta = 104,
    .min_count_bit_for_found = 176,
};

typedef struct {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;

    uint8_t sync_cnt;
    ManchesterState manchester_saved_state;

    uint8_t type;
    uint8_t data[HORMANN_BISECUR_DATA_SIZE];
    uint8_t crc;
} RaProtocolDecoderHormannBiSecur;

typedef enum {
    HormannBiSecurDecoderStepReset = 0,
    HormannBiSecurDecoderStepFoundPreambleAlternatingShort,
    HormannBiSecurDecoderStepFoundPreambleHighVeryLong,
    HormannBiSecurDecoderStepFoundPreambleAlternatingLong,
    HormannBiSecurDecoderStepFoundData,
} HormannBiSecurDecoderStep;

/* The shared manchester_advance() mis-handles this encoding, so the state machine is spelled
 * out here. Thanks to CodeAllNight (https://github.com/jamisonderek) for the fixed version. */
static const ManchesterState manchester_reset_state = ManchesterStateMid1;

static bool ra_manchester_advance_alt(
    ManchesterState state,
    ManchesterEvent event,
    ManchesterState* next_state,
    bool* data) {
    bool result = false;
    ManchesterState new_state = manchester_reset_state;

    if(event == ManchesterEventReset) {
        new_state = manchester_reset_state;
    } else if(state == ManchesterStateStart1) {
        if(event == ManchesterEventShortLow) {
            new_state = ManchesterStateMid1;
            result = true;
            if(data) *data = false;
        } else if(event == ManchesterEventLongLow) {
            new_state = ManchesterStateStart0;
            result = true;
            if(data) *data = false;
        }
    } else if(state == ManchesterStateMid1) {
        if(event == ManchesterEventShortHigh) {
            new_state = ManchesterStateStart1;
        } else if(event == ManchesterEventShortLow) {
            new_state = ManchesterStateStart0;
        }
    } else if(state == ManchesterStateStart0) {
        if(event == ManchesterEventShortHigh) {
            new_state = ManchesterStateMid0;
            result = true;
            if(data) *data = true;
        } else if(event == ManchesterEventLongHigh) {
            new_state = ManchesterStateStart1;
            result = true;
            if(data) *data = true;
        }
    } else if(state == ManchesterStateMid0) {
        if(event == ManchesterEventShortLow) {
            new_state = ManchesterStateStart0;
        } else if(event == ManchesterEventShortHigh) {
            new_state = ManchesterStateStart1;
        }
    }

    *next_state = new_state;
    return result;
}

static void* ra_protocol_decoder_hormann_bisecur_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    RaProtocolDecoderHormannBiSecur* instance = malloc(sizeof(RaProtocolDecoderHormannBiSecur));
    instance->base.protocol = &ra_protocol_hormann_bisecur;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

static void ra_protocol_decoder_hormann_bisecur_free(void* context) {
    furi_assert(context);
    free(context);
}

static void ra_protocol_decoder_hormann_bisecur_reset(void* context) {
    furi_assert(context);
    RaProtocolDecoderHormannBiSecur* instance = context;
    instance->decoder.parser_step = HormannBiSecurDecoderStepReset;
    memset(instance->data, 0, HORMANN_BISECUR_DATA_SIZE);
    instance->generic.data_count_bit = 0;
    instance->manchester_saved_state = manchester_reset_state;
}

static void ra_protocol_hormann_bisecur_parse_data(RaProtocolDecoderHormannBiSecur* instance) {
    instance->type = instance->data[0];

    instance->generic.serial = 0;
    for(uint8_t i = 1; i < 5; i++) {
        instance->generic.serial = instance->generic.serial << 8 | instance->data[i];
    }

    instance->generic.data = 0;
    for(uint8_t i = 5; i < 13; i++) {
        instance->generic.data = instance->generic.data << 8 | instance->data[i];
    }

    instance->generic.data_2 = 0;
    for(uint8_t i = 13; i < 21; i++) {
        instance->generic.data_2 = instance->generic.data_2 << 8 | instance->data[i];
    }

    instance->crc = instance->data[21];
}

static bool ra_protocol_hormann_bisecur_check_crc(RaProtocolDecoderHormannBiSecur* instance) {
    switch(instance->type) {
    case 0x50:
        return (uint8_t)(~(
                   subghz_protocol_blocks_crc8(instance->data + 1, 20, 0x07, 0x00) ^ 0x55)) ==
               instance->crc;
    case 0x70:
        return subghz_protocol_blocks_crc8le(instance->data, 21, 0x07, 0xFF) == instance->crc;
    default:
        return false;
    }
}

static void ra_protocol_decoder_hormann_bisecur_add_bit(
    RaProtocolDecoderHormannBiSecur* instance,
    bool level) {
    if(instance->generic.data_count_bit >=
       ra_protocol_hormann_bisecur_const.min_count_bit_for_found) {
        return;
    }

    if(level) {
        uint8_t byte_index = instance->generic.data_count_bit / 8;
        uint8_t bit_index = instance->generic.data_count_bit % 8;
        instance->data[byte_index] |= 1 << (7 - bit_index);
    }

    instance->generic.data_count_bit++;

    if(instance->generic.data_count_bit >=
       ra_protocol_hormann_bisecur_const.min_count_bit_for_found) {
        ra_protocol_hormann_bisecur_parse_data(instance);
        if(instance->base.callback) {
            instance->base.callback(&instance->base, instance->base.context);
        } else {
            ra_protocol_decoder_hormann_bisecur_reset(instance);
        }
    }
}

static void
    ra_protocol_decoder_hormann_bisecur_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    RaProtocolDecoderHormannBiSecur* instance = context;

    const uint32_t duration_short = (uint32_t)ra_protocol_hormann_bisecur_const.te_short;
    const uint32_t duration_long = (uint32_t)ra_protocol_hormann_bisecur_const.te_long;
    const uint32_t duration_delta = (uint32_t)ra_protocol_hormann_bisecur_const.te_delta;
    const uint32_t duration_half_short = duration_short / 2;

    ManchesterEvent event = ManchesterEventReset;

    switch(instance->decoder.parser_step) {
    case HormannBiSecurDecoderStepReset:
        if(!level &&
           DURATION_DIFF(duration, duration_short + duration_half_short) < duration_delta) {
            instance->decoder.parser_step = HormannBiSecurDecoderStepFoundPreambleAlternatingShort;
        }
        break;
    case HormannBiSecurDecoderStepFoundPreambleAlternatingShort:
        if(DURATION_DIFF(duration, duration_short) < duration_delta) {
            // stay on the same step, the pattern repeats around 21 times
            break;
        }
        if(level && DURATION_DIFF(duration, duration_long * 4) < duration_delta) {
            instance->decoder.parser_step = HormannBiSecurDecoderStepFoundPreambleHighVeryLong;
            break;
        }
        instance->decoder.parser_step = HormannBiSecurDecoderStepReset;
        break;
    case HormannBiSecurDecoderStepFoundPreambleHighVeryLong:
        if(!level && DURATION_DIFF(duration, duration_long) < duration_delta) {
            instance->sync_cnt = 3;
            instance->decoder.parser_step = HormannBiSecurDecoderStepFoundPreambleAlternatingLong;
            break;
        }
        instance->decoder.parser_step = HormannBiSecurDecoderStepReset;
        break;
    case HormannBiSecurDecoderStepFoundPreambleAlternatingLong:
        if(level == (bool)(instance->sync_cnt-- & 1) &&
           DURATION_DIFF(duration, duration_long) < duration_delta) {
            if(!instance->sync_cnt) {
                ra_manchester_advance_alt(
                    instance->manchester_saved_state,
                    ManchesterEventReset,
                    &instance->manchester_saved_state,
                    NULL);
                instance->decoder.parser_step = HormannBiSecurDecoderStepFoundData;
            }
            // stay on the same step, or advance to the next if enough transitions are found
            break;
        }
        instance->decoder.parser_step = HormannBiSecurDecoderStepReset;
        break;
    case HormannBiSecurDecoderStepFoundData:
        if(DURATION_DIFF(duration, duration_short) < duration_delta ||
           (
               // the last bit can be arbitrarily long, but it is parsed as a short
               instance->generic.data_count_bit ==
                   ra_protocol_hormann_bisecur_const.min_count_bit_for_found - 1 &&
               duration > duration_short)) {
            event = !level ? ManchesterEventShortHigh : ManchesterEventShortLow;
        }

        if(DURATION_DIFF(duration, duration_long) < duration_delta) {
            event = !level ? ManchesterEventLongHigh : ManchesterEventLongLow;
        }

        if(event == ManchesterEventReset) {
            ra_protocol_decoder_hormann_bisecur_reset(instance);
        } else {
            bool new_level;
            if(ra_manchester_advance_alt(
                   instance->manchester_saved_state,
                   event,
                   &instance->manchester_saved_state,
                   &new_level)) {
                ra_protocol_decoder_hormann_bisecur_add_bit(instance, new_level);
            }
        }
        break;
    }
}

static void ra_protocol_decoder_hormann_bisecur_get_string(void* context, FuriString* output) {
    furi_assert(context);
    RaProtocolDecoderHormannBiSecur* instance = context;
    bool valid_crc = ra_protocol_hormann_bisecur_check_crc(instance);

    furi_string_cat_printf(
        output,
        "%s %db\r\n"
        "Sn:0x%08lX T:0x%02X\r\n"
        "Key:%08lX%08lX\r\n"
        "   :%08lX%08lX\r\n"
        "CRC:0x%02X %s",
        instance->generic.protocol_name,
        instance->generic.data_count_bit,
        instance->generic.serial,
        instance->type,
        (uint32_t)(instance->generic.data >> 32),
        (uint32_t)(instance->generic.data & 0xFFFFFFFF),
        (uint32_t)(instance->generic.data_2 >> 32),
        (uint32_t)(instance->generic.data_2 & 0xFFFFFFFF),
        instance->crc,
        valid_crc ? "OK" : "BAD");
}

static const SubGhzProtocolDecoder ra_protocol_hormann_bisecur_decoder = {
    .alloc = ra_protocol_decoder_hormann_bisecur_alloc,
    .free = ra_protocol_decoder_hormann_bisecur_free,

    .feed = ra_protocol_decoder_hormann_bisecur_feed,
    .reset = ra_protocol_decoder_hormann_bisecur_reset,

    .get_hash_data = NULL,
    .serialize = NULL,
    .deserialize = NULL,
    .get_string = ra_protocol_decoder_hormann_bisecur_get_string,
};

const SubGhzProtocol ra_protocol_hormann_bisecur = {
    .name = SUBGHZ_PROTOCOL_HORMANN_BISECUR_NAME,
    .type = SubGhzProtocolTypeDynamic,
    .flag = SubGhzProtocolFlag_868 | SubGhzProtocolFlag_FM | SubGhzProtocolFlag_Decodable,

    .decoder = &ra_protocol_hormann_bisecur_decoder,
    .encoder = NULL,
};
