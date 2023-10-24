#include "iso14443_4_layer.h"

#include <furi.h>

#define ISO14443_4_BLOCK_PCB (1U << 1)
#define ISO14443_4_BLOCK_PCB_I (0U)
#define ISO14443_4_BLOCK_PCB_R (5U << 5)
#define ISO14443_4_BLOCK_PCB_S (3U << 6)

struct Iso14443_4Layer {
    uint8_t pcb;
    uint8_t pcb_prev;
};

static inline void iso14443_4_layer_update_pcb(Iso14443_4Layer* instance) {
    instance->pcb_prev = instance->pcb;
    instance->pcb ^= (uint8_t)0x01;
}

Iso14443_4Layer* iso14443_4_layer_alloc() {
    Iso14443_4Layer* instance = malloc(sizeof(Iso14443_4Layer));

    iso14443_4_layer_reset(instance);
    return instance;
}

void iso14443_4_layer_free(Iso14443_4Layer* instance) {
    furi_assert(instance);
    free(instance);
}

void iso14443_4_layer_reset(Iso14443_4Layer* instance) {
    furi_assert(instance);
    instance->pcb = ISO14443_4_BLOCK_PCB_I | ISO14443_4_BLOCK_PCB;
}

void iso14443_4_layer_encode_block(
    Iso14443_4Layer* instance,
    const BitBuffer* input_data,
    BitBuffer* block_data) {
    furi_assert(instance);

    bit_buffer_append_byte(block_data, instance->pcb);
    bit_buffer_append(block_data, input_data);

    iso14443_4_layer_update_pcb(instance);
}

bool iso14443_4_layer_decode_block(
    Iso14443_4Layer* instance,
    BitBuffer* output_data,
    const BitBuffer* block_data) {
    furi_assert(instance);

    bool ret = false;

    do {
        if(!bit_buffer_starts_with_byte(block_data, instance->pcb_prev)) break;
        bit_buffer_copy_right(output_data, block_data, 1);
        ret = true;
    } while(false);

    return ret;
}
