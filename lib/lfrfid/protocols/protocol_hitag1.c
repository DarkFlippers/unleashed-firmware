#include <furi.h>
#include <toolbox/protocols/protocol.h>
#include "lfrfid_protocols.h"

#define HITAG1_PAGES 64
#define HITAG1_DATA_SIZE HITAG1_PAGES * 4 + HITAG1_PAGES

typedef struct {
    uint8_t tagData[HITAG1_DATA_SIZE];
} ProtocolHitag1;

ProtocolHitag1* protocol_hitag1_alloc(void) {
    ProtocolHitag1* protocol = malloc(sizeof(ProtocolHitag1));

    return protocol;
};

void protocol_hitag1_free(ProtocolHitag1* protocol) {
    free(protocol);
};

uint8_t* protocol_hitag1_get_data(ProtocolHitag1* protocol) {
    return protocol->tagData;
};

void protocol_hitag1_decoder_start(ProtocolHitag1* protocol) {
    UNUSED(protocol);
    // Not applicalble, encoding & decoding is handled in lfrfid_hitag_worker...
};

bool protocol_hitag1_decoder_feed(ProtocolHitag1* protocol, bool level, uint32_t duration) {
    UNUSED(protocol);
    UNUSED(level);
    UNUSED(duration);
    // Not applicalble, encoding & decoding is handled in lfrfid_hitag_worker...

    bool result = false;
    return result;
};

bool protocol_hitag1_encoder_start(ProtocolHitag1* protocol) {
    UNUSED(protocol);
    // Not applicalble, encoding & decoding is handled in lfrfid_hitag_worker...

    return false;
};

LevelDuration protocol_hitag1_encoder_yield(ProtocolHitag1* protocol) {
    UNUSED(protocol);
    // Not applicalble, encoding & decoding is handled in lfrfid_hitag_worker...

    bool level = 0;
    uint32_t duration = 0;
    return level_duration_make(level, duration);
};

bool protocol_hitag1_write_data(ProtocolHitag1* protocol, void* data) {
    UNUSED(protocol);
    UNUSED(data);

    //this protocol cannot be simply written to card --> don't do anything, just return false

    return false;
};

void protocol_hitag1_render_data(ProtocolHitag1* protocol, FuriString* result) {
    uint8_t pages = 0;
    for(uint8_t p = 0; p < HITAG1_PAGES; p++) {
        pages += protocol->tagData[HITAG1_PAGES * 4 + p];
    }
    furi_string_printf(
        result,
        "SN: %02X %02X %02X %02X\r\n"
        "Pages read: %u / 64",
        protocol->tagData[0],
        protocol->tagData[1],
        protocol->tagData[2],
        protocol->tagData[3],
        pages);
};

const ProtocolBase protocol_hitag1 = {
    .name = "Hitag1",
    .manufacturer = "Philips",
    .data_size = HITAG1_DATA_SIZE,
    .features = LFRFIDFeatureRTF,
    .validate_count = 1,
    .alloc = (ProtocolAlloc)protocol_hitag1_alloc,
    .free = (ProtocolFree)protocol_hitag1_free,
    .get_data = (ProtocolGetData)protocol_hitag1_get_data,
    .decoder =
        {
            .start = (ProtocolDecoderStart)protocol_hitag1_decoder_start,
            .feed = (ProtocolDecoderFeed)protocol_hitag1_decoder_feed,
        },
    .encoder =
        {
            .start = (ProtocolEncoderStart)protocol_hitag1_encoder_start,
            .yield = (ProtocolEncoderYield)protocol_hitag1_encoder_yield,
        },
    .render_data = (ProtocolRenderData)protocol_hitag1_render_data,
    .render_brief_data = (ProtocolRenderData)protocol_hitag1_render_data,
    .write_data = (ProtocolWriteData)protocol_hitag1_write_data,
};
