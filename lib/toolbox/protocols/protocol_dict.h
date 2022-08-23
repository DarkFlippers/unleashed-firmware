#pragma once
#include "protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ProtocolDict ProtocolDict;

typedef int32_t ProtocolId;

#define PROTOCOL_NO (-1)
#define PROTOCOL_ALL_FEATURES (0xFFFFFFFF)

ProtocolDict* protocol_dict_alloc(const ProtocolBase** protocols, size_t protocol_count);

void protocol_dict_free(ProtocolDict* dict);

void protocol_dict_set_data(
    ProtocolDict* dict,
    size_t protocol_index,
    const uint8_t* data,
    size_t data_size);

void protocol_dict_get_data(
    ProtocolDict* dict,
    size_t protocol_index,
    uint8_t* data,
    size_t data_size);

size_t protocol_dict_get_data_size(ProtocolDict* dict, size_t protocol_index);

size_t protocol_dict_get_max_data_size(ProtocolDict* dict);

const char* protocol_dict_get_name(ProtocolDict* dict, size_t protocol_index);

const char* protocol_dict_get_manufacturer(ProtocolDict* dict, size_t protocol_index);

void protocol_dict_decoders_start(ProtocolDict* dict);

uint32_t protocol_dict_get_features(ProtocolDict* dict, size_t protocol_index);

ProtocolId protocol_dict_decoders_feed(ProtocolDict* dict, bool level, uint32_t duration);

ProtocolId protocol_dict_decoders_feed_by_feature(
    ProtocolDict* dict,
    uint32_t feature,
    bool level,
    uint32_t duration);

ProtocolId protocol_dict_decoders_feed_by_id(
    ProtocolDict* dict,
    size_t protocol_index,
    bool level,
    uint32_t duration);

bool protocol_dict_encoder_start(ProtocolDict* dict, size_t protocol_index);

LevelDuration protocol_dict_encoder_yield(ProtocolDict* dict, size_t protocol_index);

void protocol_dict_render_data(ProtocolDict* dict, string_t result, size_t protocol_index);

void protocol_dict_render_brief_data(ProtocolDict* dict, string_t result, size_t protocol_index);

uint32_t protocol_dict_get_validate_count(ProtocolDict* dict, size_t protocol_index);

ProtocolId protocol_dict_get_protocol_by_name(ProtocolDict* dict, const char* name);

bool protocol_dict_get_write_data(ProtocolDict* dict, size_t protocol_index, void* data);

#ifdef __cplusplus
}
#endif
