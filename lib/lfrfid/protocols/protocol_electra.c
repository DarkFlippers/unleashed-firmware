/*
 * Electra intercom rfid protocol (Romania)
 *
 * Based on EM4100 protocol implementation from https://github.com/flipperdevices/flipperzero-firmware/blob/dev/lib/lfrfid/protocols/protocol_em4100.c
 *
 * Copyright 2024 Leptoptilos <leptoptilos@icloud.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ------------------------------------------------------------------------------------------------------------------------------
 *                                                      PROTOCOL DESCRIPTION:
 * ------------------------------------------------------------------------------------------------------------------------------
 * Electra intercom 125 kHz protocol based on 64-bit clock EM4100, but includes some extra data after base EM4100 data (epilogue)
 *
 * Epilogue size is 64 bits, but only first 16 bits matter. Rest 6 bytes - some filler data, 
 * that arbitrary change is not validated by the Electra intercoms
 *
 * There are curently three known types of epilogue:
 * - 0x7E71AAAAAAAAAAAA (AA filler)
 * - 0x7E71000000000000 (00 filler)
 * - 0x0030AAAAAAAAAAAA
 *
 * First two epilogue bytes may be interpreted as EM4100 data continuation
 * Nevertheless, these bytes have correct row parity bits, but have not correct collumn parity
 
 * For example: 0x7E71AAAAAAAAAAAA epilogue:
 *
 * In binary: | 0b01111110 | 01110001 | 10101010 | 10101010 | 10101010 | 10101010 | 10101010 | 10101010 |
 * In hex:    |   0x7E     |    71    |    AA    |    AA    |    AA    |    AA    |    AA    |    AA    |
 * 
 * As EM4100 data:
 * 0111 1 // 7
 * 1100 0 // C
 * 0111 1 // 7
 * 1101 0 // here epilogue filler starts (from second bit)
 * 1010 1 // there is no correct raw parity bits anymore
 * 0101 0
 * 1010 1
 * 0101 0
 * 1010 // and no correct column parity 
 */

#include "bit_lib/bit_lib.h"
#include <furi.h>
#include <stdlib.h>
#include <toolbox/protocols/protocol.h>
#include <toolbox/manchester_decoder.h>
#include "lfrfid_protocols.h"

#define TAG "ELECTRA"

typedef uint64_t ElectraDecodedData;

#define EM_HEADER_POS  (55)
#define EM_HEADER_MASK (0x1FFLLU << EM_HEADER_POS)

#define EM_FIRST_ROW_POS (50)

#define EM_ROW_COUNT          (10)
#define EM_COLUMN_COUNT       (4)
#define EM_BITS_PER_ROW_COUNT (EM_COLUMN_COUNT + 1)

#define EM_COLUMN_POS     (4)
#define ELECTRA_STOP_POS  (0)
#define ELECTRA_STOP_MASK (0x1LLU << ELECTRA_STOP_POS)

#define EM_HEADER_AND_STOP_MASK (EM_HEADER_MASK | ELECTRA_STOP_MASK)
#define EM_HEADER_AND_STOP_DATA (EM_HEADER_MASK)

#define ELECTRA_DECODED_BASE_DATA_SIZE (5)
#define ELECTRA_ENCODED_BASE_DATA_SIZE (sizeof(ElectraDecodedData))

#define ELECTRA_DECODED_EPILOGUE_SIZE (3)
#define ELECTRA_ENCODED_EPILOGUE_SIZE (sizeof(ElectraDecodedData))

#define ELECTRA_DECODED_DATA_SIZE (ELECTRA_DECODED_BASE_DATA_SIZE + ELECTRA_DECODED_EPILOGUE_SIZE)
#define ELECTRA_ENCODED_DATA_SIZE (ELECTRA_ENCODED_BASE_DATA_SIZE + ELECTRA_ENCODED_EPILOGUE_SIZE)

#define ELECTRA_DECODED_DATA_EPILOGUE_START_POS (ELECTRA_DECODED_BASE_DATA_SIZE)

#define ELECTRA_CLOCK_PER_BIT (64)

#define ELECTRA_READ_SHORT_TIME  (256)
#define ELECTRA_READ_LONG_TIME   (512)
#define ELECTRA_READ_JITTER_TIME (100)

#define ELECTRA_READ_SHORT_TIME_LOW  (ELECTRA_READ_SHORT_TIME - ELECTRA_READ_JITTER_TIME)
#define ELECTRA_READ_SHORT_TIME_HIGH (ELECTRA_READ_SHORT_TIME + ELECTRA_READ_JITTER_TIME)
#define ELECTRA_READ_LONG_TIME_LOW   (ELECTRA_READ_LONG_TIME - ELECTRA_READ_JITTER_TIME)
#define ELECTRA_READ_LONG_TIME_HIGH  (ELECTRA_READ_LONG_TIME + ELECTRA_READ_JITTER_TIME)

#define EM_ENCODED_DATA_HEADER (0xFF80000000000000ULL)

typedef struct {
    uint8_t data[ELECTRA_DECODED_DATA_SIZE];

    ElectraDecodedData encoded_base_data;
    ElectraDecodedData encoded_epilogue;

    uint8_t encoded_data_index;
    bool encoded_polarity;

    ManchesterState decoder_manchester_state;
} ProtocolElectra;

ProtocolElectra* protocol_electra_alloc(void) {
    ProtocolElectra* proto = malloc(sizeof(ProtocolElectra));
    return (void*)proto;
}

void protocol_electra_free(ProtocolElectra* proto) {
    free(proto);
}

uint8_t* protocol_electra_get_data(ProtocolElectra* proto) {
    return proto->data;
}

static void electra_decode(
    const uint8_t* encoded_base_data,
    const uint8_t encoded_base_data_size,
    const uint8_t* encoded_epilogue,
    const uint8_t encoded_epilogue_size,
    uint8_t* decoded_data,
    const uint8_t decoded_data_size) {
    furi_check(decoded_data_size >= ELECTRA_DECODED_DATA_SIZE);
    furi_check(encoded_base_data_size >= ELECTRA_ENCODED_BASE_DATA_SIZE);
    furi_check(encoded_epilogue_size >= ELECTRA_ENCODED_EPILOGUE_SIZE);

    uint8_t decoded_data_index = 0;
    ElectraDecodedData base_data = *((ElectraDecodedData*)(encoded_base_data));
    //ElectraDecodedData epilogue = *((ElectraDecodedData*)(encoded_epilogue));

    // clean result
    memset(decoded_data, 0, decoded_data_size);

    // header
    for(uint8_t i = 0; i < 9; i++) {
        base_data = base_data << 1;
    }

    // nibbles
    uint8_t value = 0;
    for(uint8_t r = 0; r < EM_ROW_COUNT; r++) {
        uint8_t nibble = 0;
        for(uint8_t i = 0; i < 5; i++) {
            if(i < 4) nibble = (nibble << 1) | (base_data & (1LLU << 63) ? 1 : 0);
            base_data = base_data << 1;
        }
        value = (value << 4) | nibble;
        if(r % 2) {
            decoded_data[decoded_data_index] |= value;
            decoded_data_index++;
            value = 0;
        }
    }

    // copy first 3 bytes of encoded epilogue to decoded data
    decoded_data[ELECTRA_DECODED_DATA_EPILOGUE_START_POS] =
        encoded_epilogue[ELECTRA_ENCODED_EPILOGUE_SIZE - 1];
    decoded_data[ELECTRA_DECODED_DATA_EPILOGUE_START_POS + 1] =
        encoded_epilogue[ELECTRA_ENCODED_EPILOGUE_SIZE - 2];
    decoded_data[ELECTRA_DECODED_DATA_EPILOGUE_START_POS + 2] =
        encoded_epilogue[ELECTRA_ENCODED_EPILOGUE_SIZE - 3];
}

static bool electra_can_be_decoded(
    const uint8_t* encoded_base_data,
    const uint8_t encoded_base_data_size,
    const uint8_t* encoded_epilogue_data,
    const uint8_t encoded_epilogue_data_size) {
    furi_check(encoded_base_data_size >= ELECTRA_ENCODED_BASE_DATA_SIZE);
    furi_check(encoded_epilogue_data_size >= ELECTRA_ENCODED_EPILOGUE_SIZE);
    const ElectraDecodedData* base_data = (ElectraDecodedData*)encoded_base_data;
    const ElectraDecodedData* epilogue = (ElectraDecodedData*)encoded_epilogue_data;

    // check electra epilogue. if em4100 header - break
    if((*epilogue & EM_ENCODED_DATA_HEADER) == EM_ENCODED_DATA_HEADER) return false;

    // check header and stop bit
    if((*base_data & EM_HEADER_AND_STOP_MASK) != EM_HEADER_AND_STOP_DATA) return false;

    // check row parity
    for(uint8_t i = 0; i < EM_ROW_COUNT; i++) {
        uint8_t parity_sum = 0;

        for(uint8_t j = 0; j < EM_BITS_PER_ROW_COUNT; j++) {
            parity_sum += (*base_data >> (EM_FIRST_ROW_POS - i * EM_BITS_PER_ROW_COUNT + j)) & 1;
        }

        if(parity_sum % 2) {
            return false;
        }
    }

    // check columns parity
    for(uint8_t i = 0; i < EM_COLUMN_COUNT; i++) {
        uint8_t parity_sum = 0;

        for(uint8_t j = 0; j < EM_ROW_COUNT + 1; j++) {
            parity_sum += (*base_data >> (EM_COLUMN_POS - i + j * EM_BITS_PER_ROW_COUNT)) & 1;
        }

        if(parity_sum % 2) {
            FURI_LOG_D(
                TAG,
                "Unexpected column parity found. EM4100 data: %016llX",
                bit_lib_bytes_to_num_be(encoded_base_data, encoded_base_data_size));
            return false;
        }
    }

    // encoded_epilogue_data lsb encoded
    uint8_t epilogue_filler = encoded_epilogue_data[(ELECTRA_ENCODED_EPILOGUE_SIZE - 1) - 2];

    for(uint8_t i = 0; i < ((ELECTRA_ENCODED_EPILOGUE_SIZE - 1) - 2); i++)
        if(encoded_epilogue_data[i] != epilogue_filler) {
            FURI_LOG_D(TAG, "Unexpected epilogue filler found: %016llX", *epilogue);
            return false;
        }

    return true;
}

void protocol_electra_decoder_start(ProtocolElectra* proto) {
    memset(proto->data, 0, ELECTRA_DECODED_DATA_SIZE);
    proto->encoded_base_data = 0;
    proto->encoded_epilogue = 0;

    manchester_advance(
        proto->decoder_manchester_state,
        ManchesterEventReset,
        &proto->decoder_manchester_state,
        NULL);
}

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
            /*
                EM 4100 BASE DATA (64 bit)         ELECTRA EPILOGUE (64 bit)
            _________________________________  _________________________________
            | | | | | | | | | | | | | | | | |  | | | | | | | | | | | | | | | | |    <- new data bit
            ---------------------------------  --------------------------------- 
                                             <- epilogue msb is carry bit to base data  
            */
            bool carry = proto->encoded_epilogue >> 63 & 0b1;

            proto->encoded_base_data = (proto->encoded_base_data << 1) | carry;
            proto->encoded_epilogue = (proto->encoded_epilogue << 1) | data;

            if(electra_can_be_decoded(
                   (uint8_t*)&proto->encoded_base_data,
                   ELECTRA_ENCODED_BASE_DATA_SIZE,
                   (uint8_t*)&proto->encoded_epilogue,
                   ELECTRA_ENCODED_EPILOGUE_SIZE)) {
                electra_decode(
                    (uint8_t*)&proto->encoded_base_data,
                    ELECTRA_ENCODED_BASE_DATA_SIZE,
                    (uint8_t*)&proto->encoded_epilogue,
                    ELECTRA_ENCODED_EPILOGUE_SIZE,
                    proto->data,
                    ELECTRA_DECODED_DATA_SIZE);
                result = true;
            }
        }
    }

    return result;
}

static void em_write_nibble(bool low_nibble, uint8_t data, ElectraDecodedData* encoded_base_data) {
    uint8_t parity_sum = 0;
    uint8_t start = 0;
    if(!low_nibble) start = 4;

    for(int8_t i = (start + 3); i >= start; i--) {
        parity_sum += (data >> i) & 1;
        *encoded_base_data = (*encoded_base_data << 1) | ((data >> i) & 1);
    }

    *encoded_base_data = (*encoded_base_data << 1) | ((parity_sum % 2) & 1);
}

bool protocol_electra_encoder_start(ProtocolElectra* proto) {
    // header
    proto->encoded_base_data = 0b111111111;

    // data
    for(uint8_t i = 0; i < ELECTRA_DECODED_BASE_DATA_SIZE; i++) {
        em_write_nibble(false, proto->data[i], &proto->encoded_base_data);
        em_write_nibble(true, proto->data[i], &proto->encoded_base_data);
    }

    // column parity and stop bit
    uint8_t parity_sum;

    for(uint8_t c = 0; c < EM_COLUMN_COUNT; c++) {
        parity_sum = 0;
        for(uint8_t i = 1; i <= EM_ROW_COUNT; i++) {
            uint8_t parity_bit = (proto->encoded_base_data >> (i * EM_BITS_PER_ROW_COUNT - 1)) & 1;
            parity_sum += parity_bit;
        }
        proto->encoded_base_data = (proto->encoded_base_data << 1) | ((parity_sum % 2) & 1);
    }

    // stop bit
    proto->encoded_base_data = (proto->encoded_base_data << 1) | 0;

    proto->encoded_data_index = 0;
    proto->encoded_polarity = true;

    // epilogue
    proto->encoded_epilogue = (proto->data[ELECTRA_DECODED_DATA_EPILOGUE_START_POS]);
    proto->encoded_epilogue <<= 8;
    proto->encoded_epilogue |= (proto->data[ELECTRA_DECODED_DATA_EPILOGUE_START_POS + 1]);

    //fill bytes 2-7 by epilogue filler
    for(uint8_t i = 2; i < ELECTRA_ENCODED_EPILOGUE_SIZE; i++) {
        proto->encoded_epilogue <<= 8;
        proto->encoded_epilogue |= proto->data[ELECTRA_DECODED_DATA_EPILOGUE_START_POS + 2];
    }

    return true;
}

LevelDuration protocol_electra_encoder_yield(ProtocolElectra* proto) {
    bool level;
    if(proto->encoded_data_index < 64)
        level = (proto->encoded_base_data >> (63 - proto->encoded_data_index)) & 1;
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
}

bool protocol_electra_write_data(ProtocolElectra* protocol, void* data) {
    LFRFIDWriteRequest* request = (LFRFIDWriteRequest*)data;
    bool result = false;

    // Correct protocol data by redecoding
    protocol_electra_encoder_start(protocol);
    electra_decode(
        (uint8_t*)&protocol->encoded_base_data,
        sizeof(ElectraDecodedData),
        (uint8_t*)&protocol->encoded_epilogue,
        sizeof(ElectraDecodedData),
        protocol->data,
        ELECTRA_DECODED_DATA_SIZE);

    protocol_electra_encoder_start(protocol);

    if(request->write_type == LFRFIDWriteTypeT5577) {
        request->t5577.block[0] =
            (LFRFID_T5577_MODULATION_MANCHESTER | LFRFID_T5577_BITRATE_RF_64 |
             (4 << LFRFID_T5577_MAXBLOCK_SHIFT));
        request->t5577.block[1] = protocol->encoded_base_data >> 32;
        request->t5577.block[2] = protocol->encoded_base_data & 0xFFFFFFFF;
        request->t5577.block[3] = protocol->encoded_epilogue >> 32;
        request->t5577.block[4] = protocol->encoded_epilogue & 0xFFFFFFFF;
        request->t5577.blocks_to_write = 5;
        result = true;
    }
    return result;
}

void protocol_electra_render_data(ProtocolElectra* protocol, FuriString* result) {
    protocol_electra_encoder_start(protocol);
    furi_string_printf(result, "Epilogue: %016llX", protocol->encoded_epilogue);
}

const ProtocolBase protocol_electra = {
    .name = "Electra",
    .manufacturer = "Electra",
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
