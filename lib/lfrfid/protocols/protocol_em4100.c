#include <furi.h>
#include <toolbox/protocols/protocol.h>
#include <toolbox/manchester_decoder.h>
#include "lfrfid_protocols.h"

typedef uint64_t EM4100DecodedData;

#define EM_HEADER_POS (55)
#define EM_HEADER_MASK (0x1FFLLU << EM_HEADER_POS)

#define EM_FIRST_ROW_POS (50)

#define EM_ROW_COUNT (10)
#define EM_COLUMN_COUNT (4)
#define EM_BITS_PER_ROW_COUNT (EM_COLUMN_COUNT + 1)

#define EM_COLUMN_POS (4)
#define EM_STOP_POS (0)
#define EM_STOP_MASK (0x1LLU << EM_STOP_POS)

#define EM_HEADER_AND_STOP_MASK (EM_HEADER_MASK | EM_STOP_MASK)
#define EM_HEADER_AND_STOP_DATA (EM_HEADER_MASK)

#define EM4100_DECODED_DATA_SIZE (5)
#define EM4100_ENCODED_DATA_SIZE (sizeof(EM4100DecodedData))

#define EM4100_CLOCK_PER_BIT (64)

#define EM_READ_SHORT_TIME (256)
#define EM_READ_LONG_TIME (512)
#define EM_READ_JITTER_TIME (100)

#define EM_READ_SHORT_TIME_LOW (EM_READ_SHORT_TIME - EM_READ_JITTER_TIME)
#define EM_READ_SHORT_TIME_HIGH (EM_READ_SHORT_TIME + EM_READ_JITTER_TIME)
#define EM_READ_LONG_TIME_LOW (EM_READ_LONG_TIME - EM_READ_JITTER_TIME)
#define EM_READ_LONG_TIME_HIGH (EM_READ_LONG_TIME + EM_READ_JITTER_TIME)

typedef struct {
    uint8_t data[EM4100_DECODED_DATA_SIZE];

    EM4100DecodedData encoded_data;
    uint8_t encoded_data_index;
    bool encoded_polarity;

    ManchesterState decoder_manchester_state;
} ProtocolEM4100;

ProtocolEM4100* protocol_em4100_alloc(void) {
    ProtocolEM4100* proto = malloc(sizeof(ProtocolEM4100));
    return (void*)proto;
};

void protocol_em4100_free(ProtocolEM4100* proto) {
    free(proto);
};

uint8_t* protocol_em4100_get_data(ProtocolEM4100* proto) {
    return proto->data;
};

static void em4100_decode(
    const uint8_t* encoded_data,
    const uint8_t encoded_data_size,
    uint8_t* decoded_data,
    const uint8_t decoded_data_size) {
    furi_check(decoded_data_size >= EM4100_DECODED_DATA_SIZE);
    furi_check(encoded_data_size >= EM4100_ENCODED_DATA_SIZE);

    uint8_t decoded_data_index = 0;
    EM4100DecodedData card_data = *((EM4100DecodedData*)(encoded_data));

    // clean result
    memset(decoded_data, 0, decoded_data_size);

    // header
    for(uint8_t i = 0; i < 9; i++) {
        card_data = card_data << 1;
    }

    // nibbles
    uint8_t value = 0;
    for(uint8_t r = 0; r < EM_ROW_COUNT; r++) {
        uint8_t nibble = 0;
        for(uint8_t i = 0; i < 5; i++) {
            if(i < 4) nibble = (nibble << 1) | (card_data & (1LLU << 63) ? 1 : 0);
            card_data = card_data << 1;
        }
        value = (value << 4) | nibble;
        if(r % 2) {
            decoded_data[decoded_data_index] |= value;
            decoded_data_index++;
            value = 0;
        }
    }
}

static bool em4100_can_be_decoded(const uint8_t* encoded_data, const uint8_t encoded_data_size) {
    furi_check(encoded_data_size >= EM4100_ENCODED_DATA_SIZE);
    const EM4100DecodedData* card_data = (EM4100DecodedData*)encoded_data;

    // check header and stop bit
    if((*card_data & EM_HEADER_AND_STOP_MASK) != EM_HEADER_AND_STOP_DATA) return false;

    // check row parity
    for(uint8_t i = 0; i < EM_ROW_COUNT; i++) {
        uint8_t parity_sum = 0;

        for(uint8_t j = 0; j < EM_BITS_PER_ROW_COUNT; j++) {
            parity_sum += (*card_data >> (EM_FIRST_ROW_POS - i * EM_BITS_PER_ROW_COUNT + j)) & 1;
        }

        if((parity_sum % 2)) {
            return false;
        }
    }

    // check columns parity
    for(uint8_t i = 0; i < EM_COLUMN_COUNT; i++) {
        uint8_t parity_sum = 0;

        for(uint8_t j = 0; j < EM_ROW_COUNT + 1; j++) {
            parity_sum += (*card_data >> (EM_COLUMN_POS - i + j * EM_BITS_PER_ROW_COUNT)) & 1;
        }

        if((parity_sum % 2)) {
            return false;
        }
    }

    return true;
}

void protocol_em4100_decoder_start(ProtocolEM4100* proto) {
    memset(proto->data, 0, EM4100_DECODED_DATA_SIZE);
    proto->encoded_data = 0;
    manchester_advance(
        proto->decoder_manchester_state,
        ManchesterEventReset,
        &proto->decoder_manchester_state,
        NULL);
};

bool protocol_em4100_decoder_feed(ProtocolEM4100* proto, bool level, uint32_t duration) {
    bool result = false;

    ManchesterEvent event = ManchesterEventReset;

    if(duration > EM_READ_SHORT_TIME_LOW && duration < EM_READ_SHORT_TIME_HIGH) {
        if(!level) {
            event = ManchesterEventShortHigh;
        } else {
            event = ManchesterEventShortLow;
        }
    } else if(duration > EM_READ_LONG_TIME_LOW && duration < EM_READ_LONG_TIME_HIGH) {
        if(!level) {
            event = ManchesterEventLongHigh;
        } else {
            event = ManchesterEventLongLow;
        }
    }

    if(event != ManchesterEventReset) {
        bool data;
        bool data_ok = manchester_advance(
            proto->decoder_manchester_state, event, &proto->decoder_manchester_state, &data);

        if(data_ok) {
            proto->encoded_data = (proto->encoded_data << 1) | data;

            if(em4100_can_be_decoded((uint8_t*)&proto->encoded_data, sizeof(EM4100DecodedData))) {
                em4100_decode(
                    (uint8_t*)&proto->encoded_data,
                    sizeof(EM4100DecodedData),
                    proto->data,
                    EM4100_DECODED_DATA_SIZE);
                result = true;
            }
        }
    }

    return result;
};

static void em4100_write_nibble(bool low_nibble, uint8_t data, EM4100DecodedData* encoded_data) {
    uint8_t parity_sum = 0;
    uint8_t start = 0;
    if(!low_nibble) start = 4;

    for(int8_t i = (start + 3); i >= start; i--) {
        parity_sum += (data >> i) & 1;
        *encoded_data = (*encoded_data << 1) | ((data >> i) & 1);
    }

    *encoded_data = (*encoded_data << 1) | ((parity_sum % 2) & 1);
}

bool protocol_em4100_encoder_start(ProtocolEM4100* proto) {
    // header
    proto->encoded_data = 0b111111111;

    // data
    for(uint8_t i = 0; i < EM4100_DECODED_DATA_SIZE; i++) {
        em4100_write_nibble(false, proto->data[i], &proto->encoded_data);
        em4100_write_nibble(true, proto->data[i], &proto->encoded_data);
    }

    // column parity and stop bit
    uint8_t parity_sum;

    for(uint8_t c = 0; c < EM_COLUMN_COUNT; c++) {
        parity_sum = 0;
        for(uint8_t i = 1; i <= EM_ROW_COUNT; i++) {
            uint8_t parity_bit = (proto->encoded_data >> (i * EM_BITS_PER_ROW_COUNT - 1)) & 1;
            parity_sum += parity_bit;
        }
        proto->encoded_data = (proto->encoded_data << 1) | ((parity_sum % 2) & 1);
    }

    // stop bit
    proto->encoded_data = (proto->encoded_data << 1) | 0;

    proto->encoded_data_index = 0;
    proto->encoded_polarity = true;

    return true;
};

LevelDuration protocol_em4100_encoder_yield(ProtocolEM4100* proto) {
    bool level = (proto->encoded_data >> (63 - proto->encoded_data_index)) & 1;
    uint32_t duration = EM4100_CLOCK_PER_BIT / 2;

    if(proto->encoded_polarity) {
        proto->encoded_polarity = false;
    } else {
        level = !level;

        proto->encoded_polarity = true;
        proto->encoded_data_index++;
        if(proto->encoded_data_index >= 64) {
            proto->encoded_data_index = 0;
        }
    }

    return level_duration_make(level, duration);
};

bool protocol_em4100_write_data(ProtocolEM4100* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    protocol_em4100_encoder_start(protocol);

    if(request->write_type == LFRFIDWriteTypeT5577) {
        request->t5577.block[0] =
            (LFRFID_T5577_MODULATION_MANCHESTER | LFRFID_T5577_BITRATE_RF_64 |
             (2 << LFRFID_T5577_MAXBLOCK_SHIFT));
        request->t5577.block[1] = protocol->encoded_data;
        request->t5577.block[2] = protocol->encoded_data >> 32;
        request->t5577.blocks_to_write = 3;
        result = true;
    }
    return result;
};

void protocol_em4100_render_data(ProtocolEM4100* protocol, string_t result) {
    uint8_t* data = protocol->data;
    string_printf(result, "FC: %03u, Card: %05u", data[2], (uint16_t)((data[3] << 8) | (data[4])));
};

const ProtocolBase protocol_em4100 = {
    .name = "EM4100",
    .manufacturer = "EM-Micro",
    .data_size = EM4100_DECODED_DATA_SIZE,
    .features = LFRFIDFeatureASK | LFRFIDFeaturePSK,
    .validate_count = 3,
    .alloc = (ProtocolAlloc)protocol_em4100_alloc,
    .free = (ProtocolFree)protocol_em4100_free,
    .get_data = (ProtocolGetData)protocol_em4100_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_em4100_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_em4100_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_em4100_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_em4100_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_em4100_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_em4100_render_data,
    .write_data = (ProtocolWriteData)protocol_em4100_write_data,
};