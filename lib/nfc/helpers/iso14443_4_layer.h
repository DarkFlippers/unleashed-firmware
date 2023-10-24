#pragma once

#include <toolbox/bit_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Iso14443_4Layer Iso14443_4Layer;

Iso14443_4Layer* iso14443_4_layer_alloc();

void iso14443_4_layer_free(Iso14443_4Layer* instance);

void iso14443_4_layer_reset(Iso14443_4Layer* instance);

void iso14443_4_layer_encode_block(
    Iso14443_4Layer* instance,
    const BitBuffer* input_data,
    BitBuffer* block_data);

bool iso14443_4_layer_decode_block(
    Iso14443_4Layer* instance,
    BitBuffer* output_data,
    const BitBuffer* block_data);

#ifdef __cplusplus
}
#endif
