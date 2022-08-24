#include "raw_em4100.h"
#include <lib/lfrfid/protocols/protocol_em4100.c>

bool protocol_em4100_raw_encoder_start(ProtocolEM4100* proto) {
    FURI_LOG_D("RAW_EM4100", "encoder_start : CLEAN");
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

bool protocol_em4100_wrong_crc_encoder_start(ProtocolEM4100* proto) {
    FURI_LOG_D("RAW_EM4100", "encoder_start : WRONG CRC");
    // header
    proto->encoded_data = 0b111111111;

    // data
    for(uint8_t i = 0; i < EM4100_DECODED_DATA_SIZE; i++) {
        em4100_write_nibble(false, proto->data[i], &proto->encoded_data);
        em4100_write_nibble(true, proto->data[i], &proto->encoded_data);
    }

    for(uint8_t c = 0; c < EM_COLUMN_COUNT; c++) {
        proto->encoded_data = (proto->encoded_data << 1) | ((0 % 2) & 1);
    }

    // stop bit
    proto->encoded_data = (proto->encoded_data << 1) | 1;

    proto->encoded_data_index = 0;
    proto->encoded_polarity = true;

    return true;
};

const ProtocolBase protocol_raw_em4100 = {
    .name = "RawEM4100",
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
            .start = (ProtocolEncoderStart)protocol_em4100_raw_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_em4100_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_em4100_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_em4100_render_data,
    .write_data = (ProtocolWriteData)protocol_em4100_write_data,
};

const ProtocolBase protocol_raw_wrong_crc_em4100 = {
    .name = "RawEM4100",
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
            .start = (ProtocolEncoderStart)protocol_em4100_wrong_crc_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_em4100_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_em4100_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_em4100_render_data,
    .write_data = (ProtocolWriteData)protocol_em4100_write_data,
};