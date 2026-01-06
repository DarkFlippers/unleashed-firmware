#include "iso14443_4_layer.h"

#include <furi.h>

// EMV Specific masks
#define ISO14443_4_BLOCK_PCB_I_        (0U << 6)
#define ISO14443_4_BLOCK_PCB_R_        (2U << 6)
#define ISO14443_4_BLOCK_PCB_TYPE_MASK (3U << 6)
#define ISO14443_4_BLOCK_PCB_S_WTX     (3U << 4)
#define ISO14443_4_BLOCK_PCB_S         (3U << 6)
//

#define ISO14443_4_BLOCK_PCB      (1U << 1)
#define ISO14443_4_BLOCK_PCB_MASK (0x03)

#define ISO14443_4_BLOCK_PCB_I              (0U)
#define ISO14443_4_BLOCK_PCB_I_MASK         (1U << 1)
#define ISO14443_4_BLOCK_PCB_I_ZERO_MASK    (7U << 5)
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

#define ISO14443_4_BLOCK_PPS_START      (0xD0)
#define ISO14443_4_BLOCK_PPS_START_MASK (0xF0)

#define ISO14443_4_BLOCK_PPS_0_HAS_PPS1 (1U << 4)

#define ISO14443_4_BLOCK_PPS_1_DSI_MASK (3U << 2)
#define ISO14443_4_BLOCK_PPS_1_DRI_MASK (3U << 0)

#define ISO14443_4_BLOCK_CID_MASK (0x0F)

#define ISO14443_4_BLOCK_PCB_BITS_ACTIVE(pcb, mask) (((pcb) & (mask)) == (mask))

#define ISO14443_4_BLOCK_PCB_IS_I_BLOCK(pcb)                               \
    (ISO14443_4_BLOCK_PCB_BITS_ACTIVE(pcb, ISO14443_4_BLOCK_PCB_I_MASK) && \
     (((pcb) & ISO14443_4_BLOCK_PCB_I_ZERO_MASK) == 0))

#define ISO14443_4_BLOCK_PCB_IS_R_BLOCK(pcb) \
    ISO14443_4_BLOCK_PCB_BITS_ACTIVE(pcb, ISO14443_4_BLOCK_PCB_R_MASK)

#define ISO14443_4_BLOCK_PCB_IS_S_BLOCK(pcb) \
    ISO14443_4_BLOCK_PCB_BITS_ACTIVE(pcb, ISO14443_4_BLOCK_PCB_S_MASK)

#define ISO14443_4_BLOCK_PCB_IS_CHAIN_ACTIVE(pcb) \
    ISO14443_4_BLOCK_PCB_BITS_ACTIVE(pcb, ISO14443_4_BLOCK_PCB_I_CHAIN_MASK)

#define ISO14443_4_BLOCK_PCB_R_NACK_ACTIVE(pcb) \
    ISO14443_4_BLOCK_PCB_BITS_ACTIVE(pcb, ISO14443_4_BLOCK_PCB_R_NACK_MASK)

#define ISO14443_4_LAYER_NAD_NOT_SUPPORTED ((uint8_t) - 1)
#define ISO14443_4_LAYER_NAD_NOT_SET       ((uint8_t) - 2)

#define ISO14443_4_BLOCK_PPS_IS_START(pps) \
    ((pps & ISO14443_4_BLOCK_PPS_START_MASK) == ISO14443_4_BLOCK_PPS_START)

struct Iso14443_4Layer {
    uint8_t pcb;
    uint8_t pcb_prev;

    // Listener specific
    uint8_t cid;
    uint8_t nad;
    bool can_pps;
};

static inline void iso14443_4_layer_update_pcb(Iso14443_4Layer* instance, bool toggle_num) {
    instance->pcb_prev = instance->pcb;
    if(toggle_num) {
        instance->pcb ^= (uint8_t)0x01;
    }
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

    instance->cid = ISO14443_4_LAYER_CID_NOT_SUPPORTED;
    instance->nad = ISO14443_4_LAYER_NAD_NOT_SUPPORTED;
    instance->can_pps = true;
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

void iso14443_4_layer_encode_command(
    Iso14443_4Layer* instance,
    const BitBuffer* input_data,
    BitBuffer* block_data) {
    furi_assert(instance);

    bit_buffer_append_byte(block_data, instance->pcb);
    bit_buffer_append(block_data, input_data);

    iso14443_4_layer_update_pcb(instance, true);
}

static inline uint8_t iso14443_4_layer_get_response_pcb(const BitBuffer* block_data) {
    const uint8_t* data = bit_buffer_get_data(block_data);
    return data[0];
}

bool iso14443_4_layer_decode_response(
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
            iso14443_4_layer_update_pcb(instance, true);
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

Iso14443_4aError iso14443_4_layer_decode_response_pwt_ext(
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

void iso14443_4_layer_set_cid(Iso14443_4Layer* instance, uint8_t cid) {
    instance->cid = cid;
}

void iso14443_4_layer_set_nad_supported(Iso14443_4Layer* instance, bool nad) {
    instance->nad = nad ? 0 : ISO14443_4_LAYER_NAD_NOT_SUPPORTED;
}

Iso14443_4LayerResult iso14443_4_layer_decode_command(
    Iso14443_4Layer* instance,
    const BitBuffer* input_data,
    BitBuffer* block_data) {
    furi_assert(instance);

    uint8_t ppss = bit_buffer_get_byte(input_data, 0);
    if(ISO14443_4_BLOCK_PPS_IS_START(ppss)) {
        if(instance->can_pps) {
            const uint8_t cid = ppss & ISO14443_4_BLOCK_CID_MASK;
            if(instance->cid != ISO14443_4_LAYER_CID_NOT_SUPPORTED && cid != instance->cid) {
                return Iso14443_4LayerResultSkip;
            }
            instance->can_pps = false;
            uint8_t pps0 = bit_buffer_get_byte(input_data, 1);
            if(pps0 & ISO14443_4_BLOCK_PPS_0_HAS_PPS1) {
                uint8_t pps1 = bit_buffer_get_byte(input_data, 2);
                uint8_t dsi = pps1 & ISO14443_4_BLOCK_PPS_1_DSI_MASK;
                uint8_t dri = pps1 & ISO14443_4_BLOCK_PPS_1_DRI_MASK;
                // TODO: do we need to change bit timings somehow? DRI and DSI mean different bit timing divisors
                UNUSED(dsi);
                UNUSED(dri);
            }
            bit_buffer_reset(block_data);
            bit_buffer_append_byte(block_data, ppss);
            return Iso14443_4LayerResultSend;
        } else {
            return Iso14443_4LayerResultSkip;
        }
    }
    instance->can_pps = false;

    uint8_t prologue_len = 0;
    instance->pcb = bit_buffer_get_byte(input_data, prologue_len++);

    if(ISO14443_4_BLOCK_PCB_IS_I_BLOCK(instance->pcb)) {
        if(instance->pcb & ISO14443_4_BLOCK_PCB_I_CID_MASK) {
            const uint8_t cid = bit_buffer_get_byte(input_data, prologue_len++) &
                                ISO14443_4_BLOCK_CID_MASK;
            if(instance->cid == ISO14443_4_LAYER_CID_NOT_SUPPORTED || cid != instance->cid) {
                return Iso14443_4LayerResultSkip;
            }
        } else if(instance->cid != ISO14443_4_LAYER_CID_NOT_SUPPORTED && instance->cid != 0) {
            return Iso14443_4LayerResultSkip;
        }
        // TODO: properly handle block chaining
        if(instance->pcb & ISO14443_4_BLOCK_PCB_I_NAD_MASK) {
            if(instance->nad == ISO14443_4_LAYER_NAD_NOT_SUPPORTED) {
                return Iso14443_4LayerResultSkip;
            }
            instance->nad = bit_buffer_get_byte(input_data, prologue_len++);
        }
        bit_buffer_copy_right(block_data, input_data, prologue_len);
        iso14443_4_layer_update_pcb(instance, false);
        return Iso14443_4LayerResultData;

    } else if(ISO14443_4_BLOCK_PCB_IS_S_BLOCK(instance->pcb)) {
        if(instance->pcb & ISO14443_4_BLOCK_PCB_S_CID_MASK) {
            const uint8_t cid = bit_buffer_get_byte(input_data, prologue_len++) &
                                ISO14443_4_BLOCK_CID_MASK;
            if(instance->cid == ISO14443_4_LAYER_CID_NOT_SUPPORTED || cid != instance->cid) {
                return Iso14443_4LayerResultSkip;
            }
        } else if(instance->cid != ISO14443_4_LAYER_CID_NOT_SUPPORTED && instance->cid != 0) {
            return Iso14443_4LayerResultSkip;
        }
        if((instance->pcb & ISO14443_4_BLOCK_PCB_S_WTX_DESELECT_MASK) == 0) {
            // DESELECT
            bit_buffer_copy(block_data, input_data);
            return Iso14443_4LayerResultSend | Iso14443_4LayerResultHalt;
        } else {
            // WTX ACK or wrong value
            return Iso14443_4LayerResultSkip;
        }

    } else if(ISO14443_4_BLOCK_PCB_IS_R_BLOCK(instance->pcb)) {
        // TODO: properly handle R blocks while chaining
        iso14443_4_layer_update_pcb(instance, true);
        instance->pcb |= ISO14443_4_BLOCK_PCB_R_NACK_MASK;
        bit_buffer_reset(block_data);
        bit_buffer_append_byte(block_data, instance->pcb);
        iso14443_4_layer_update_pcb(instance, false);
        return Iso14443_4LayerResultSend;
    }
    return Iso14443_4LayerResultSkip;
}

bool iso14443_4_layer_encode_response(
    Iso14443_4Layer* instance,
    const BitBuffer* input_data,
    BitBuffer* block_data) {
    furi_assert(instance);

    if(ISO14443_4_BLOCK_PCB_IS_I_BLOCK(instance->pcb_prev)) {
        bit_buffer_append_byte(block_data, 0x00);
        if(instance->pcb_prev & ISO14443_4_BLOCK_PCB_I_CID_MASK) {
            bit_buffer_append_byte(block_data, instance->cid);
        }
        // TODO: properly handle block chaining and related R block responses
        if(instance->pcb_prev & ISO14443_4_BLOCK_PCB_I_NAD_MASK &&
           instance->nad != ISO14443_4_LAYER_NAD_NOT_SET) {
            bit_buffer_append_byte(block_data, instance->nad);
            instance->nad = ISO14443_4_LAYER_NAD_NOT_SET;
        } else {
            instance->pcb &= ~ISO14443_4_BLOCK_PCB_I_NAD_MASK;
        }
        instance->pcb &= ~ISO14443_4_BLOCK_PCB_I_CHAIN_MASK;
        bit_buffer_set_byte(block_data, 0, instance->pcb);
        bit_buffer_append(block_data, input_data);
        iso14443_4_layer_update_pcb(instance, false);
        return true;
    }
    return false;
}
