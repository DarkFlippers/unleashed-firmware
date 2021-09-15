#include "furi/memmgr.h"
#include "irda.h"
#include "common/irda_common_i.h"
#include "irda_protocol_defs_i.h"
#include <stdint.h>
#include "../irda_i.h"

typedef struct IrdaEncoderRC5 {
    IrdaCommonEncoder* common_encoder;
    bool toggle_bit;
} IrdaEncoderRC5;

void irda_encoder_rc5_reset(void* encoder_ptr, const IrdaMessage* message) {
    furi_assert(encoder_ptr);

    IrdaEncoderRC5* encoder = encoder_ptr;
    IrdaCommonEncoder* common_encoder = encoder->common_encoder;
    irda_common_encoder_reset(common_encoder);

    uint32_t* data = (void*) common_encoder->data;
    /* RC5 */
    *data |= 0x01;    // start bit
    if (message->protocol == IrdaProtocolRC5) {
        *data |= 0x02;    // start bit
    }
    *data |= encoder->toggle_bit ? 0x04 : 0;
    *data |= (reverse(message->address) >> 3) << 3; /* address 5 bit */
    *data |= (reverse(message->command) >> 2) << 8; /* command 6 bit */

    common_encoder->data[0] = ~common_encoder->data[0];
    common_encoder->data[1] = ~common_encoder->data[1];

    common_encoder->bits_to_encode = common_encoder->protocol->databit_len[0];
    encoder->toggle_bit ^= 1;
}

IrdaStatus irda_encoder_rc5_encode(void* encoder_ptr, uint32_t* duration, bool* level) {
    IrdaEncoderRC5* encoder = encoder_ptr;
    return irda_common_encode(encoder->common_encoder, duration, level);
}

void* irda_encoder_rc5_alloc(void) {
    IrdaEncoderRC5* encoder = furi_alloc(sizeof(IrdaEncoderRC5));
    encoder->common_encoder = irda_common_encoder_alloc(&protocol_rc5);
    encoder->toggle_bit = false;
    return encoder;
}

void irda_encoder_rc5_free(void* encoder_ptr) {
    furi_assert(encoder_ptr);

    IrdaEncoderRC5* encoder = encoder_ptr;
    free(encoder->common_encoder);
    free(encoder);
}

