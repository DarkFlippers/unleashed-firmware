#include "iso14443_4_layer.h"

#include <furi.h>

#define ISO14443_4_BLOCK_PCB   (1U << 1)
#define ISO14443_4_BLOCK_PCB_I (0U)
#define ISO14443_4_BLOCK_PCB_R (5U << 5)
#define ISO14443_4_BLOCK_PCB_S (3U << 6)

#define ISO14443_4_BLOCK_PCB_I_        (0U << 6)
#define ISO14443_4_BLOCK_PCB_R_        (2U << 6)
#define ISO14443_4_BLOCK_PCB_TYPE_MASK (3U << 6)

#define ISO14443_4_BLOCK_PCB_S_DESELECT   (0U << 4)
#define ISO14443_4_BLOCK_PCB_S_WTX        (3U << 4)
#define ISO14443_4_BLOCK_PCB_BLOCK_NUMBER (1U << 0)

#define ISO14443_4_BLOCK_PCB_NAD      (1U << 2)
#define ISO14443_4_BLOCK_PCB_CID      (1U << 3)
#define ISO14443_4_BLOCK_PCB_CHAINING (1U << 4)

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
        // TODO: Fix crash
        bit_buffer_copy_right(output_data, block_data, 1);
        ret = true;
    } while(false);

    return ret;
}

Iso14443_4aError iso14443_4_layer_decode_block_pwt_ext(
    Iso14443_4Layer* instance,
    BitBuffer* output_data,
    const BitBuffer* block_data) {
    furi_assert(instance);

    Iso14443_4aError ret = Iso14443_4aErrorProtocol;
    bit_buffer_reset(output_data);

    do {
        const uint8_t pcb_field = bit_buffer_get_byte(block_data, 0);
        const uint8_t block_type = pcb_field & ISO14443_4_BLOCK_PCB_TYPE_MASK;
        switch(block_type) {
        case ISO14443_4_BLOCK_PCB_I_:
            if(pcb_field == instance->pcb_prev) {
                bit_buffer_copy_right(output_data, block_data, 1);
                ret = Iso14443_4aErrorNone;
            } else {
                // send original request again
                ret = Iso14443_4aErrorSendExtra;
            }
            break;
        case ISO14443_4_BLOCK_PCB_R_:
            // TODO
            break;
        case ISO14443_4_BLOCK_PCB_S:
            if((pcb_field & ISO14443_4_BLOCK_PCB_S_WTX) == ISO14443_4_BLOCK_PCB_S_WTX) {
                const uint8_t inf_field = bit_buffer_get_byte(block_data, 1);
                //const uint8_t power_level = inf_field >> 6;
                const uint8_t wtxm = inf_field & 0b111111;
                //uint32_t fwt_temp = MIN((fwt * wtxm), fwt_max);

                bit_buffer_append_byte(
                    output_data,
                    ISO14443_4_BLOCK_PCB_S | ISO14443_4_BLOCK_PCB_S_WTX | ISO14443_4_BLOCK_PCB);
                bit_buffer_append_byte(output_data, wtxm);
                ret = Iso14443_4aErrorSendExtra;
            }
            break;
        }
    } while(false);

    if(ret != Iso14443_4aErrorNone) {
        FURI_LOG_RAW_T("RAW RX:");
        for(size_t x = 0; x < bit_buffer_get_size_bytes(block_data); x++) {
            FURI_LOG_RAW_T("%02X ", bit_buffer_get_byte(block_data, x));
        }
        FURI_LOG_RAW_T("\r\n");
    }

    return ret;
}
