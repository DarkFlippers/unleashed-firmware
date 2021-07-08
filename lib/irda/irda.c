#include "irda.h"
#include "furi/check.h"
#include "irda_common_i.h"
#include "irda_protocol_defs_i.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <furi.h>
#include "irda_i.h"
#include <api-hal-irda.h>

struct IrdaDecoderHandler {
    void** ctx;
};

typedef struct {
    IrdaAlloc alloc;
    IrdaDecode decode;
    IrdaReset reset;
    IrdaFree free;
} IrdaDecoders;

typedef struct {
    IrdaEncoderReset reset;
    IrdaAlloc alloc;
    IrdaEncode encode;
    IrdaFree free;
} IrdaEncoders;

typedef struct {
    IrdaProtocol protocol;
    const char* name;
    IrdaDecoders decoder;
    IrdaEncoders encoder;
    uint8_t address_length;
    uint8_t command_length;
} IrdaProtocolImplementation;

struct IrdaEncoderHandler {
    void* encoder;
    IrdaProtocol protocol;
};

// TODO: replace with key-value, Now we refer by enum index, which is dangerous.
static const IrdaProtocolImplementation irda_protocols[] = {
    // #0
    { .protocol = IrdaProtocolNEC,
      .name = "NEC",
      .decoder = {
          .alloc = irda_decoder_nec_alloc,
          .decode = irda_decoder_nec_decode,
          .reset = irda_decoder_nec_reset,
          .free = irda_decoder_nec_free},
      .encoder = {
          .alloc = irda_encoder_nec_alloc,
          .encode = irda_encoder_nec_encode,
          .reset = irda_encoder_nec_reset,
          .free = irda_encoder_nec_free},
      .address_length = 2,
      .command_length = 2,
    },
    // #1 - have to be after NEC
    { .protocol = IrdaProtocolNECext,
      .name = "NECext",
      .decoder = {
          .alloc = irda_decoder_necext_alloc,
          .decode = irda_decoder_nec_decode,
          .reset = irda_decoder_nec_reset,
          .free = irda_decoder_nec_free},
      .encoder = {
          .alloc = irda_encoder_necext_alloc,
          .encode = irda_encoder_nec_encode,
          .reset = irda_encoder_necext_reset,
          .free = irda_encoder_nec_free},
      .address_length = 4,
      .command_length = 2,
    },
    // #2
    { .protocol = IrdaProtocolSamsung32,
      .name ="Samsung32",
      .decoder = {
          .alloc = irda_decoder_samsung32_alloc,
          .decode = irda_decoder_samsung32_decode,
          .reset = irda_decoder_samsung32_reset,
          .free = irda_decoder_samsung32_free},
      .encoder = {
          .alloc = irda_encoder_samsung32_alloc,
          .encode = irda_encoder_samsung32_encode,
          .reset = irda_encoder_samsung32_reset,
          .free = irda_encoder_samsung32_free},
      .address_length = 2,
      .command_length = 2,
    },
    // #3
    { .protocol = IrdaProtocolRC6,
      .name = "RC6",
      .decoder = {
          .alloc = irda_decoder_rc6_alloc,
          .decode = irda_decoder_rc6_decode,
          .reset = irda_decoder_rc6_reset,
          .free = irda_decoder_rc6_free},
      .encoder = {
          .alloc = irda_encoder_rc6_alloc,
          .encode = irda_encoder_rc6_encode,
          .reset = irda_encoder_rc6_reset,
          .free = irda_encoder_rc6_free},
      .address_length = 2,
      .command_length = 2,
    },
};

const IrdaMessage* irda_decode(IrdaDecoderHandler* handler, bool level, uint32_t duration) {
    furi_assert(handler);

    IrdaMessage* message = NULL;
    IrdaMessage* result = NULL;

    for (int i = 0; i < COUNT_OF(irda_protocols); ++i) {
        if (irda_protocols[i].decoder.decode) {
            message = irda_protocols[i].decoder.decode(handler->ctx[i], level, duration);
            if (!result && message) {
                message->protocol = irda_protocols[i].protocol;
                result = message;
            }
        }
    }

    return result;
}

IrdaDecoderHandler* irda_alloc_decoder(void) {
    IrdaDecoderHandler* handler = furi_alloc(sizeof(IrdaDecoderHandler));
    handler->ctx = furi_alloc(sizeof(void*) * COUNT_OF(irda_protocols));

    for (int i = 0; i < COUNT_OF(irda_protocols); ++i) {
        handler->ctx[i] = 0;
        if (irda_protocols[i].decoder.alloc)
            handler->ctx[i] = irda_protocols[i].decoder.alloc();
    }

    return handler;
}

void irda_free_decoder(IrdaDecoderHandler* handler) {
    furi_assert(handler);
    furi_assert(handler->ctx);

    for (int i = 0; i < COUNT_OF(irda_protocols); ++i) {
        if (irda_protocols[i].decoder.free)
            irda_protocols[i].decoder.free(handler->ctx[i]);
    }

    free(handler->ctx);
    free(handler);
}

void irda_reset_decoder(IrdaDecoderHandler* handler) {
    for (int i = 0; i < COUNT_OF(irda_protocols); ++i) {
        if (irda_protocols[i].decoder.reset)
            irda_protocols[i].decoder.reset(handler->ctx[i]);
    }
}

IrdaEncoderHandler* irda_alloc_encoder(void) {
    IrdaEncoderHandler* handler = furi_alloc(sizeof(IrdaEncoderHandler));
    handler->encoder = NULL;
    handler->protocol = IrdaProtocolUnknown;
    return handler;
}

void irda_free_encoder(IrdaEncoderHandler* handler) {
    furi_assert(handler);

    if (handler->encoder) {
        furi_assert(irda_is_protocol_valid(handler->protocol));
        furi_assert(irda_protocols[handler->protocol].encoder.free);
        irda_protocols[handler->protocol].encoder.free(handler->encoder);
    }

    free(handler);
}

void irda_reset_encoder(IrdaEncoderHandler* handler, const IrdaMessage* message) {
    furi_assert(handler);
    furi_assert(message);
    furi_assert(irda_is_protocol_valid(message->protocol));
    furi_assert(irda_protocols[message->protocol].encoder.reset);
    furi_assert(irda_protocols[message->protocol].encoder.alloc);

    /* Realloc encoder if different protocol set */
    if (message->protocol != handler->protocol) {
        if (handler->encoder != NULL) {
            furi_assert(handler->protocol != IrdaProtocolUnknown);
            irda_protocols[handler->protocol].encoder.free(handler->encoder);
        }
        handler->encoder = irda_protocols[message->protocol].encoder.alloc();
        handler->protocol = message->protocol;
    }

    irda_protocols[handler->protocol].encoder.reset(handler->encoder, message);
}


IrdaStatus irda_encode(IrdaEncoderHandler* handler, uint32_t* duration, bool* level) {
    furi_assert(handler);
    furi_assert(irda_is_protocol_valid(handler->protocol));
    furi_assert(irda_protocols[handler->protocol].encoder.encode);

    IrdaStatus status = irda_protocols[handler->protocol].encoder.encode(handler->encoder, duration, level);
    furi_assert(status != IrdaStatusError);

    return status;
}


bool irda_is_protocol_valid(IrdaProtocol protocol) {
    return (protocol >= 0) && (protocol < COUNT_OF(irda_protocols));
}

IrdaProtocol irda_get_protocol_by_name(const char* protocol_name) {
    for (int i = 0; i < COUNT_OF(irda_protocols); ++i) {
        if (!strcmp(irda_protocols[i].name, protocol_name))
            return i;
    }
    return IrdaProtocolUnknown;
}

const char* irda_get_protocol_name(IrdaProtocol protocol) {
    if (irda_is_protocol_valid(protocol))
        return irda_protocols[protocol].name;
    else
        return "Invalid";
}

uint8_t irda_get_protocol_address_length(IrdaProtocol protocol) {
    if (irda_is_protocol_valid(protocol))
        return irda_protocols[protocol].address_length;
    else
        return 0;
}

uint8_t irda_get_protocol_command_length(IrdaProtocol protocol) {
    if (irda_is_protocol_valid(protocol))
        return irda_protocols[protocol].command_length;
    else
        return 0;
}

