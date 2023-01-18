/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license. */

#include <inttypes.h>
#include <lib/flipper_format/flipper_format_i.h>
#include <furi/core/string.h>
#include <lib/subghz/registry.h>
#include <lib/subghz/protocols/base.h>
#include "app_buffer.h"

#define TAG "PROTOVIEW-protocol"

const SubGhzProtocol subghz_protocol_protoview;

/* The feed() method puts data in the RawSamples global (protected by
 * a mutex). */
extern RawSamplesBuffer *RawSamples;

/* This is totally dummy: we just define the decoder base for the async
 * system to work but we don't really use it if not to collect raw
 * data via the feed() method. */
typedef struct SubGhzProtocolDecoderprotoview {
    SubGhzProtocolDecoderBase base;
} SubGhzProtocolDecoderprotoview;

void* subghz_protocol_decoder_protoview_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);

    SubGhzProtocolDecoderprotoview* instance =
        malloc(sizeof(SubGhzProtocolDecoderprotoview));
    instance->base.protocol = &subghz_protocol_protoview;
    return instance;
}

void subghz_protocol_decoder_protoview_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderprotoview* instance = context;
    free(instance);
}

void subghz_protocol_decoder_protoview_reset(void* context) {
    furi_assert(context);
}

/* That's the only thig we really use of the protocol decoder
 * implementation. We avoid the subghz provided abstractions and put
 * the data in our simple abstraction: the RawSamples circular buffer. */
void subghz_protocol_decoder_protoview_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    UNUSED(context);

    /* Add data to the circular buffer. */
    raw_samples_add(RawSamples, level, duration);
    // FURI_LOG_E(TAG, "FEED: %d %d", (int)level, (int)duration);
    return;
}

/* The only scope of this method is to avoid duplicated messages in the
 * Subghz history, which we don't use. */
uint8_t subghz_protocol_decoder_protoview_get_hash_data(void* context) {
    furi_assert(context);
    return 123;
}

/* Not used. */
bool subghz_protocol_decoder_protoview_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset)
{
    UNUSED(context);
    UNUSED(flipper_format);
    UNUSED(preset);
    return false;
}

/* Not used. */
bool subghz_protocol_decoder_protoview_deserialize(void* context, FlipperFormat* flipper_format)
{
    UNUSED(context);
    UNUSED(flipper_format);
    return false;
}

void subhz_protocol_decoder_protoview_get_string(void* context, FuriString* output)
{
    furi_assert(context);
    furi_string_cat_printf(output, "Protoview");
}

const SubGhzProtocolDecoder subghz_protocol_protoview_decoder = {
    .alloc = subghz_protocol_decoder_protoview_alloc,
    .free = subghz_protocol_decoder_protoview_free,
    .reset = subghz_protocol_decoder_protoview_reset,
    .feed = subghz_protocol_decoder_protoview_feed,
    .get_hash_data = subghz_protocol_decoder_protoview_get_hash_data,
    .serialize = subghz_protocol_decoder_protoview_serialize,
    .deserialize = subghz_protocol_decoder_protoview_deserialize,
    .get_string = subhz_protocol_decoder_protoview_get_string,
};

/* Well, we don't really target a specific protocol. So let's put flags
 * that make sense. */
const SubGhzProtocol subghz_protocol_protoview = {
    .name = "Protoview",
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_AM | SubGhzProtocolFlag_FM | SubGhzProtocolFlag_Decodable,
    .decoder = &subghz_protocol_protoview_decoder,
};

/* Our table has just the single dummy protocol we defined for the
 * sake of data collection. */
const SubGhzProtocol* protoview_protocol_registry_items[] = {
    &subghz_protocol_protoview,
};

const SubGhzProtocolRegistry protoview_protocol_registry = {
    .items = protoview_protocol_registry_items,
    .size = COUNT_OF(protoview_protocol_registry_items)
};
