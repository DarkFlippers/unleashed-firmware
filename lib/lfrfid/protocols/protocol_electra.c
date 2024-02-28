#include <furi.h>
#include <toolbox/protocols/protocol.h>
#include <toolbox/manchester_decoder.h>
#include "lfrfid_protocols.h"

typedef uint64_t ElectraDecodedData;

#define ELECTRA_HEADER_POS (55)
#define ELECTRA_HEADER_MASK (0x1FFLLU << ELECTRA_HEADER_POS)

#define ELECTRA_FIRST_ROW_POS (50)

#define ELECTRA_ROW_COUNT (10)
#define ELECTRA_COLUMN_COUNT (4)
#define ELECTRA_BITS_PER_ROW_COUNT (ELECTRA_COLUMN_COUNT + 1)

#define ELECTRA_COLUMN_POS (4)
#define ELECTRA_STOP_POS (0)
#define ELECTRA_STOP_MASK (0x1LLU << ELECTRA_STOP_POS)

#define ELECTRA_HEADER_AND_STOP_MASK (ELECTRA_HEADER_MASK | ELECTRA_STOP_MASK)
#define ELECTRA_HEADER_AND_STOP_DATA (ELECTRA_HEADER_MASK)

#define ELECTRA_DECODED_DATA_SIZE (5)
#define ELECTRA_ENCODED_DATA_SIZE (sizeof(ElectraDecodedData))

#define ELECTRA_CLOCK_PER_BIT (64)

#define ELECTRA_READ_SHORT_TIME (256)
#define ELECTRA_READ_LONG_TIME (512)
#define ELECTRA_READ_JITTER_TIME (100)

#define ELECTRA_READ_SHORT_TIME_LOW (ELECTRA_READ_SHORT_TIME - ELECTRA_READ_JITTER_TIME)
#define ELECTRA_READ_SHORT_TIME_HIGH (ELECTRA_READ_SHORT_TIME + ELECTRA_READ_JITTER_TIME)
#define ELECTRA_READ_LONG_TIME_LOW (ELECTRA_READ_LONG_TIME - ELECTRA_READ_JITTER_TIME)
#define ELECTRA_READ_LONG_TIME_HIGH (ELECTRA_READ_LONG_TIME + ELECTRA_READ_JITTER_TIME)

#define EM_ENCODED_DATA_HEADER (0xFF80000000000000ULL)

#define ELECTRA_EPILOGUE (0x7E1E000000000000ULL)
// #define ELECTRA_EPILOGUE_2 (0x0030AAAAAAAAAAAAULL)

typedef struct {
    uint8_t data[ELECTRA_DECODED_DATA_SIZE];

    ElectraDecodedData encoded_data;
    ElectraDecodedData encoded_epilogue;

    uint8_t encoded_data_index;
    bool encoded_polarity;

    ManchesterState decoder_manchester_state;
} ProtocolElectra;

ProtocolElectra* protocol_electra_alloc(void) {
    ProtocolElectra* proto = malloc(sizeof(ProtocolElectra));
    return (void*)proto;
};

void protocol_electra_free(ProtocolElectra* proto) {
    free(proto);
};

uint8_t* protocol_electra_get_data(ProtocolElectra* proto) {
    return proto->data;
};

static void electra_decode(
    const uint8_t* encoded_data,
    const uint8_t encoded_data_size,
    uint8_t* decoded_data,
    const uint8_t decoded_data_size) {
    furi_check(decoded_data_size >= ELECTRA_DECODED_DATA_SIZE);
    furi_check(encoded_data_size >= ELECTRA_ENCODED_DATA_SIZE);

    uint8_t decoded_data_index = 0;
    ElectraDecodedData card_data = *((ElectraDecodedData*)(encoded_data));

    // clean result
    memset(decoded_data, 0, decoded_data_size);

    // header
    for(uint8_t i = 0; i < 9; i++) {
        card_data = card_data << 1;
    }

    // nibbles
    uint8_t value = 0;
    for(uint8_t r = 0; r < ELECTRA_ROW_COUNT; r++) {
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

static bool electra_can_be_decoded(
    const uint8_t* encoded_data,
    const uint8_t encoded_data_size,
    const uint8_t* epilogue_data) {
    furi_check(encoded_data_size >= ELECTRA_ENCODED_DATA_SIZE);
    const ElectraDecodedData* card_data = (ElectraDecodedData*)encoded_data;
    const ElectraDecodedData* epilogue = (ElectraDecodedData*)epilogue_data;

    bool decoded = false;

    do {
        // check electra epilogue
        if((*epilogue & EM_ENCODED_DATA_HEADER) == EM_ENCODED_DATA_HEADER) break;

        // check header and stop bit
        if((*card_data & ELECTRA_HEADER_AND_STOP_MASK) != ELECTRA_HEADER_AND_STOP_DATA) break;

        // check row parity
        for(uint8_t i = 0; i < ELECTRA_ROW_COUNT; i++) {
            uint8_t parity_sum = 0;

            for(uint8_t j = 0; j < ELECTRA_BITS_PER_ROW_COUNT; j++) {
                parity_sum +=
                    (*card_data >> (ELECTRA_FIRST_ROW_POS - i * ELECTRA_BITS_PER_ROW_COUNT + j)) &
                    1;
            }

            if((parity_sum % 2)) {
                break;
            }
        }

        // check columns parity
        for(uint8_t i = 0; i < ELECTRA_COLUMN_COUNT; i++) {
            uint8_t parity_sum = 0;

            for(uint8_t j = 0; j < ELECTRA_ROW_COUNT + 1; j++) {
                parity_sum +=
                    (*card_data >> (ELECTRA_COLUMN_POS - i + j * ELECTRA_BITS_PER_ROW_COUNT)) & 1;
            }

            if((parity_sum % 2)) {
                break;
            }
        }

        decoded = true;
    } while(false);

    return decoded;
}

void protocol_electra_decoder_start(ProtocolElectra* proto) {
    memset(proto->data, 0, ELECTRA_DECODED_DATA_SIZE);
    proto->encoded_data = 0;
    proto->encoded_epilogue = 0;

    manchester_advance(
        proto->decoder_manchester_state,
        ManchesterEventReset,
        &proto->decoder_manchester_state,
        NULL);
};

bool protocol_electra_decoder_feed(ProtocolElectra* proto, bool level, uint32_t duration) {
    bool result = false;

    ManchesterEvent event = ManchesterEventReset;

    if(duration > ELECTRA_READ_SHORT_TIME_LOW && duration < ELECTRA_READ_SHORT_TIME_HIGH) {
        if(!level) {
            event = ManchesterEventShortHigh;
        } else {
            event = ManchesterEventShortLow;
        }
    } else if(duration > ELECTRA_READ_LONG_TIME_LOW && duration < ELECTRA_READ_LONG_TIME_HIGH) {
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
            bool carry = proto->encoded_epilogue >> 63 & 0b1;

            proto->encoded_data = (proto->encoded_data << 1) | carry;
            proto->encoded_epilogue = (proto->encoded_epilogue << 1) | data;

            if(electra_can_be_decoded(
                   (uint8_t*)&proto->encoded_data,
                   sizeof(ElectraDecodedData),
                   (uint8_t*)&proto->encoded_epilogue)) {
                electra_decode(
                    (uint8_t*)&proto->encoded_data,
                    sizeof(ElectraDecodedData),
                    proto->data,
                    ELECTRA_DECODED_DATA_SIZE);
                result = true;
            }
        }
    }

    return result;
};

static void electra_write_nibble(bool low_nibble, uint8_t data, ElectraDecodedData* encoded_data) {
    uint8_t parity_sum = 0;
    uint8_t start = 0;
    if(!low_nibble) start = 4;

    for(int8_t i = (start + 3); i >= start; i--) {
        parity_sum += (data >> i) & 1;
        *encoded_data = (*encoded_data << 1) | ((data >> i) & 1);
    }

    *encoded_data = (*encoded_data << 1) | ((parity_sum % 2) & 1);
}

bool protocol_electra_encoder_start(ProtocolElectra* proto) {
    // header
    proto->encoded_data = 0b111111111;

    // data
    for(uint8_t i = 0; i < ELECTRA_DECODED_DATA_SIZE; i++) {
        electra_write_nibble(false, proto->data[i], &proto->encoded_data);
        electra_write_nibble(true, proto->data[i], &proto->encoded_data);
    }

    // column parity and stop bit
    uint8_t parity_sum;

    for(uint8_t c = 0; c < ELECTRA_COLUMN_COUNT; c++) {
        parity_sum = 0;
        for(uint8_t i = 1; i <= ELECTRA_ROW_COUNT; i++) {
            uint8_t parity_bit = (proto->encoded_data >> (i * ELECTRA_BITS_PER_ROW_COUNT - 1)) & 1;
            parity_sum += parity_bit;
        }
        proto->encoded_data = (proto->encoded_data << 1) | ((parity_sum % 2) & 1);
    }

    // stop bit
    proto->encoded_data = (proto->encoded_data << 1) | 0;

    proto->encoded_data_index = 0;
    proto->encoded_polarity = true;

    // epilogue
    proto->encoded_epilogue = ELECTRA_EPILOGUE;

    return true;
};

LevelDuration protocol_electra_encoder_yield(ProtocolElectra* proto) {
    bool level;
    if(proto->encoded_data_index < 64)
        level = (proto->encoded_data >> (63 - proto->encoded_data_index)) & 1;
    else
        level = (proto->encoded_epilogue >> (63 - (proto->encoded_data_index - 64))) & 1;

    uint32_t duration = ELECTRA_CLOCK_PER_BIT / 2;

    if(proto->encoded_polarity) {
        proto->encoded_polarity = false;
    } else {
        level = !level;

        proto->encoded_polarity = true;
        proto->encoded_data_index++;
        if(proto->encoded_data_index >= 128) {
            proto->encoded_data_index = 0;
        }
    }

    return level_duration_make(level, duration);
};

bool protocol_electra_write_data(ProtocolElectra* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    // Correct protocol data by redecoding
    protocol_electra_encoder_start(protocol);
    electra_decode(
        (uint8_t*)&protocol->encoded_data,
        sizeof(ElectraDecodedData),
        protocol->data,
        ELECTRA_DECODED_DATA_SIZE);

    protocol_electra_encoder_start(protocol);

    if(request->write_type == LFRFIDWriteTypeT5577) {
        request->t5577.block[0] =
            (LFRFID_T5577_MODULATION_MANCHESTER | LFRFID_T5577_BITRATE_RF_64 |
             (4 << LFRFID_T5577_MAXBLOCK_SHIFT));
        request->t5577.block[1] = protocol->encoded_data >> 32;
        request->t5577.block[2] = protocol->encoded_data & 0xFFFFFFFF;
        request->t5577.block[3] = ELECTRA_EPILOGUE >> 32;
        request->t5577.block[4] = ELECTRA_EPILOGUE & 0xFFFFFFFF;
        request->t5577.blocks_to_write = 5;
        result = true;
    }
    return result;
};

void protocol_electra_render_data(ProtocolElectra* protocol, FuriString* result) {
    furi_string_printf(result, "Epilogue: %016llX", protocol->encoded_epilogue);
};

const ProtocolBase protocol_electra = {
    .name = "Electra",
    .manufacturer = "ELECTRA",
    .data_size = ELECTRA_DECODED_DATA_SIZE,
    .features = LFRFIDFeatureASK | LFRFIDFeaturePSK,
    .validate_count = 3,
    .alloc = (ProtocolAlloc)protocol_electra_alloc,
    .free = (ProtocolFree)protocol_electra_free,
    .get_data = (ProtocolGetData)protocol_electra_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_electra_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_electra_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_electra_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_electra_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_electra_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_electra_render_data,
    .write_data = (ProtocolWriteData)protocol_electra_write_data,
};