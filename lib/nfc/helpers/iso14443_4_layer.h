#pragma once

#include <toolbox/bit_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Iso14443_4Layer Iso14443_4Layer;

Iso14443_4Layer* iso14443_4_layer_alloc(void);

void iso14443_4_layer_free(Iso14443_4Layer* instance);

void iso14443_4_layer_reset(Iso14443_4Layer* instance);

void iso14443_4_layer_set_i_block(Iso14443_4Layer* instance, bool chaining, bool CID_present);
void iso14443_4_layer_set_r_block(Iso14443_4Layer* instance, bool acknowledged, bool CID_present);
void iso14443_4_layer_set_s_block(Iso14443_4Layer* instance, bool deselect, bool CID_present);

// Poller mode

void iso14443_4_layer_encode_command(
    Iso14443_4Layer* instance,
    const BitBuffer* input_data,
    BitBuffer* block_data);

bool iso14443_4_layer_decode_response(
    Iso14443_4Layer* instance,
    BitBuffer* output_data,
    const BitBuffer* block_data);

// Listener mode

typedef enum {
    Iso14443_4LayerResultSkip = (0),
    Iso14443_4LayerResultData = (1 << 1),
    Iso14443_4LayerResultSend = (1 << 2),
    Iso14443_4LayerResultHalt = (1 << 3),
} Iso14443_4LayerResult;

Iso14443_4LayerResult iso14443_4_layer_decode_command(
    Iso14443_4Layer* instance,
    const BitBuffer* input_data,
    BitBuffer* block_data);

bool iso14443_4_layer_encode_response(
    Iso14443_4Layer* instance,
    const BitBuffer* input_data,
    BitBuffer* block_data);

#define ISO14443_4_LAYER_CID_NOT_SUPPORTED ((uint8_t) - 1)
void iso14443_4_layer_set_cid(Iso14443_4Layer* instance, uint8_t cid);

void iso14443_4_layer_set_nad_supported(Iso14443_4Layer* instance, bool nad);

#ifdef __cplusplus
}
#endif
