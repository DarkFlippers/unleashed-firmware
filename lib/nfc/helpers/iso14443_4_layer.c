#include "iso14443_4_layer.h"

#include <furi.h>

#define ISO14443_4_BLOCK_PCB      (1U << 1)
#define ISO14443_4_BLOCK_PCB_MASK (0x03)

#define ISO14443_4_BLOCK_PCB_I              (0U)
#define ISO14443_4_BLOCK_PCB_I_NAD_OFFSET   (2)
#define ISO14443_4_BLOCK_PCB_I_CID_OFFSET   (3)
#define ISO14443_4_BLOCK_PCB_I_CHAIN_OFFSET (4)
#define ISO14443_4_BLOCK_PCB_I_NAD_MASK     (1U << ISO14443_4_BLOCK_PCB_I_NAD_OFFSET)
#define ISO14443_4_BLOCK_PCB_I_CID_MASK     (1U << ISO14443_4_BLOCK_PCB_I_CID_OFFSET)
#define ISO14443_4_BLOCK_PCB_I_CHAIN_MASK   (1U << ISO14443_4_BLOCK_PCB_I_CHAIN_OFFSET)

#define ISO14443_4_BLOCK_PCB_R_MASK        (5U << 5)
#define ISO14443_4_BLOCK_PCB_R_NACK_OFFSET (4)
#define ISO14443_4_BLOCK_PCB_R_CID_OFFSET  (3)
#define ISO14443_4_BLOCK_PCB_R_CID_MASK    (1U << ISO14443_4_BLOCK_PCB_R_CID_OFFSET)
#define ISO14443_4_BLOCK_PCB_R_NACK_MASK   (1U << ISO14443_4_BLOCK_PCB_R_NACK_OFFSET)

#define ISO14443_4_BLOCK_PCB_S_MASK                (3U << 6)
#define ISO14443_4_BLOCK_PCB_S_CID_OFFSET          (3)
#define ISO14443_4_BLOCK_PCB_S_WTX_DESELECT_OFFSET (4)
#define ISO14443_4_BLOCK_PCB_S_CID_MASK            (1U << ISO14443_4_BLOCK_PCB_R_CID_OFFSET)
#define ISO14443_4_BLOCK_PCB_S_WTX_DESELECT_MASK   (3U << ISO14443_4_BLOCK_PCB_S_WTX_DESELECT_OFFSET)

#define ISO14443_4_BLOCK_PCB_BITS_ACTIVE(pcb, mask) (((pcb) & (mask)) == (mask))

#define ISO14443_4_BLOCK_PCB_IS_R_BLOCK(pcb) \
    ISO14443_4_BLOCK_PCB_BITS_ACTIVE(pcb, ISO14443_4_BLOCK_PCB_R_MASK)

#define ISO14443_4_BLOCK_PCB_IS_S_BLOCK(pcb) \
    ISO14443_4_BLOCK_PCB_BITS_ACTIVE(pcb, ISO14443_4_BLOCK_PCB_S_MASK)

#define ISO14443_4_BLOCK_PCB_IS_CHAIN_ACTIVE(pcb) \
    ISO14443_4_BLOCK_PCB_BITS_ACTIVE(pcb, ISO14443_4_BLOCK_PCB_I_CHAIN_MASK)

#define ISO14443_4_BLOCK_PCB_R_NACK_ACTIVE(pcb) \
    ISO14443_4_BLOCK_PCB_BITS_ACTIVE(pcb, ISO14443_4_BLOCK_PCB_R_NACK_MASK)

struct Iso14443_4Layer {
    uint8_t pcb;
    uint8_t pcb_prev;
};

static inline void iso14443_4_layer_update_pcb(Iso14443_4Layer* instance) {
    instance->pcb_prev = instance->pcb;
    instance->pcb ^= (uint8_t)0x01;
}

Iso14443_4Layer* iso14443_4_layer_alloc(void) {
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
    instance->pcb_prev = 0;
    instance->pcb = ISO14443_4_BLOCK_PCB_I | ISO14443_4_BLOCK_PCB;
}

void iso14443_4_layer_set_i_block(Iso14443_4Layer* instance, bool chaining, bool CID_present) {
    uint8_t block_pcb = instance->pcb & ISO14443_4_BLOCK_PCB_MASK;
    instance->pcb = ISO14443_4_BLOCK_PCB_I | (chaining << ISO14443_4_BLOCK_PCB_I_CHAIN_OFFSET) |
                    (CID_present << ISO14443_4_BLOCK_PCB_I_CID_OFFSET) | block_pcb;
}

void iso14443_4_layer_set_r_block(Iso14443_4Layer* instance, bool acknowledged, bool CID_present) {
    furi_assert(instance);
    uint8_t block_pcb = instance->pcb & ISO14443_4_BLOCK_PCB_MASK;
    instance->pcb = ISO14443_4_BLOCK_PCB_R_MASK |
                    (!acknowledged << ISO14443_4_BLOCK_PCB_R_NACK_OFFSET) |
                    (CID_present << ISO14443_4_BLOCK_PCB_R_CID_OFFSET) | block_pcb;
}

void iso14443_4_layer_set_s_block(Iso14443_4Layer* instance, bool deselect, bool CID_present) {
    furi_assert(instance);
    uint8_t des_wtx = !deselect ? (ISO14443_4_BLOCK_PCB_S_WTX_DESELECT_MASK) : 0;
    instance->pcb = ISO14443_4_BLOCK_PCB_S_MASK | des_wtx |
                    (CID_present << ISO14443_4_BLOCK_PCB_S_CID_OFFSET) | ISO14443_4_BLOCK_PCB;
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

static inline uint8_t iso14443_4_layer_get_response_pcb(const BitBuffer* block_data) {
    const uint8_t* data = bit_buffer_get_data(block_data);
    return data[0];
}

bool iso14443_4_layer_decode_block(
    Iso14443_4Layer* instance,
    BitBuffer* output_data,
    const BitBuffer* block_data) {
    furi_assert(instance);

    bool ret = false;

    do {
        if(ISO14443_4_BLOCK_PCB_IS_R_BLOCK(instance->pcb_prev)) {
            const uint8_t response_pcb = iso14443_4_layer_get_response_pcb(block_data);
            ret = (ISO14443_4_BLOCK_PCB_IS_R_BLOCK(response_pcb)) &&
                  (!ISO14443_4_BLOCK_PCB_R_NACK_ACTIVE(response_pcb));
            instance->pcb &= ISO14443_4_BLOCK_PCB_MASK;
            iso14443_4_layer_update_pcb(instance);
        } else if(ISO14443_4_BLOCK_PCB_IS_CHAIN_ACTIVE(instance->pcb_prev)) {
            const uint8_t response_pcb = iso14443_4_layer_get_response_pcb(block_data);
            ret = (ISO14443_4_BLOCK_PCB_IS_R_BLOCK(response_pcb)) &&
                  (!ISO14443_4_BLOCK_PCB_R_NACK_ACTIVE(response_pcb));
            instance->pcb &= ~(ISO14443_4_BLOCK_PCB_I_CHAIN_MASK);
        } else if(ISO14443_4_BLOCK_PCB_IS_S_BLOCK(instance->pcb_prev)) {
            ret = bit_buffer_starts_with_byte(block_data, instance->pcb_prev);
            if(bit_buffer_get_size_bytes(block_data) > 1)
                bit_buffer_copy_right(output_data, block_data, 1);
        } else {
            if(!bit_buffer_starts_with_byte(block_data, instance->pcb_prev)) break;
            bit_buffer_copy_right(output_data, block_data, 1);
            ret = true;
        }
    } while(false);

    return ret;
}
