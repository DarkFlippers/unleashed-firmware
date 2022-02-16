#include "irda.h"
#include "furi/check.h"
#include "common/irda_common_i.h"
#include "irda_protocol_defs_i.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <furi.h>
#include "irda_i.h"
#include <furi_hal_irda.h>

typedef struct {
    IrdaAlloc alloc;
    IrdaDecode decode;
    IrdaDecoderReset reset;
    IrdaFree free;
    IrdaDecoderCheckReady check_ready;
} IrdaDecoders;

typedef struct {
    IrdaAlloc alloc;
    IrdaEncode encode;
    IrdaEncoderReset reset;
    IrdaFree free;
} IrdaEncoders;

struct IrdaDecoderHandler {
    void** ctx;
};

struct IrdaEncoderHandler {
    void* handler;
    const IrdaEncoders* encoder;
};

typedef struct {
    IrdaEncoders encoder;
    IrdaDecoders decoder;
    IrdaGetProtocolSpec get_protocol_spec;
} IrdaEncoderDecoder;

static const IrdaEncoderDecoder irda_encoder_decoder[] = {
    {
        .decoder =
            {.alloc = irda_decoder_nec_alloc,
             .decode = irda_decoder_nec_decode,
             .reset = irda_decoder_nec_reset,
             .check_ready = irda_decoder_nec_check_ready,
             .free = irda_decoder_nec_free},
        .encoder =
            {.alloc = irda_encoder_nec_alloc,
             .encode = irda_encoder_nec_encode,
             .reset = irda_encoder_nec_reset,
             .free = irda_encoder_nec_free},
        .get_protocol_spec = irda_nec_get_spec,
    },
    {
        .decoder =
            {.alloc = irda_decoder_samsung32_alloc,
             .decode = irda_decoder_samsung32_decode,
             .reset = irda_decoder_samsung32_reset,
             .check_ready = irda_decoder_samsung32_check_ready,
             .free = irda_decoder_samsung32_free},
        .encoder =
            {.alloc = irda_encoder_samsung32_alloc,
             .encode = irda_encoder_samsung32_encode,
             .reset = irda_encoder_samsung32_reset,
             .free = irda_encoder_samsung32_free},
        .get_protocol_spec = irda_samsung32_get_spec,
    },
    {
        .decoder =
            {.alloc = irda_decoder_rc5_alloc,
             .decode = irda_decoder_rc5_decode,
             .reset = irda_decoder_rc5_reset,
             .check_ready = irda_decoder_rc5_check_ready,
             .free = irda_decoder_rc5_free},
        .encoder =
            {.alloc = irda_encoder_rc5_alloc,
             .encode = irda_encoder_rc5_encode,
             .reset = irda_encoder_rc5_reset,
             .free = irda_encoder_rc5_free},
        .get_protocol_spec = irda_rc5_get_spec,
    },
    {
        .decoder =
            {.alloc = irda_decoder_rc6_alloc,
             .decode = irda_decoder_rc6_decode,
             .reset = irda_decoder_rc6_reset,
             .check_ready = irda_decoder_rc6_check_ready,
             .free = irda_decoder_rc6_free},
        .encoder =
            {.alloc = irda_encoder_rc6_alloc,
             .encode = irda_encoder_rc6_encode,
             .reset = irda_encoder_rc6_reset,
             .free = irda_encoder_rc6_free},
        .get_protocol_spec = irda_rc6_get_spec,
    },
    {
        .decoder =
            {.alloc = irda_decoder_sirc_alloc,
             .decode = irda_decoder_sirc_decode,
             .reset = irda_decoder_sirc_reset,
             .check_ready = irda_decoder_sirc_check_ready,
             .free = irda_decoder_sirc_free},
        .encoder =
            {.alloc = irda_encoder_sirc_alloc,
             .encode = irda_encoder_sirc_encode,
             .reset = irda_encoder_sirc_reset,
             .free = irda_encoder_sirc_free},
        .get_protocol_spec = irda_sirc_get_spec,
    },
};

static int irda_find_index_by_protocol(IrdaProtocol protocol);
static const IrdaProtocolSpecification* irda_get_spec_by_protocol(IrdaProtocol protocol);

const IrdaMessage* irda_decode(IrdaDecoderHandler* handler, bool level, uint32_t duration) {
    furi_assert(handler);

    IrdaMessage* message = NULL;
    IrdaMessage* result = NULL;

    for(int i = 0; i < COUNT_OF(irda_encoder_decoder); ++i) {
        if(irda_encoder_decoder[i].decoder.decode) {
            message = irda_encoder_decoder[i].decoder.decode(handler->ctx[i], level, duration);
            if(!result && message) {
                result = message;
            }
        }
    }

    return result;
}

IrdaDecoderHandler* irda_alloc_decoder(void) {
    IrdaDecoderHandler* handler = furi_alloc(sizeof(IrdaDecoderHandler));
    handler->ctx = furi_alloc(sizeof(void*) * COUNT_OF(irda_encoder_decoder));

    for(int i = 0; i < COUNT_OF(irda_encoder_decoder); ++i) {
        handler->ctx[i] = 0;
        if(irda_encoder_decoder[i].decoder.alloc)
            handler->ctx[i] = irda_encoder_decoder[i].decoder.alloc();
    }

    irda_reset_decoder(handler);
    return handler;
}

void irda_free_decoder(IrdaDecoderHandler* handler) {
    furi_assert(handler);
    furi_assert(handler->ctx);

    for(int i = 0; i < COUNT_OF(irda_encoder_decoder); ++i) {
        if(irda_encoder_decoder[i].decoder.free)
            irda_encoder_decoder[i].decoder.free(handler->ctx[i]);
    }

    free(handler->ctx);
    free(handler);
}

void irda_reset_decoder(IrdaDecoderHandler* handler) {
    for(int i = 0; i < COUNT_OF(irda_encoder_decoder); ++i) {
        if(irda_encoder_decoder[i].decoder.reset)
            irda_encoder_decoder[i].decoder.reset(handler->ctx[i]);
    }
}

const IrdaMessage* irda_check_decoder_ready(IrdaDecoderHandler* handler) {
    furi_assert(handler);

    IrdaMessage* message = NULL;
    IrdaMessage* result = NULL;

    for(int i = 0; i < COUNT_OF(irda_encoder_decoder); ++i) {
        if(irda_encoder_decoder[i].decoder.check_ready) {
            message = irda_encoder_decoder[i].decoder.check_ready(handler->ctx[i]);
            if(!result && message) {
                result = message;
            }
        }
    }

    return result;
}

IrdaEncoderHandler* irda_alloc_encoder(void) {
    IrdaEncoderHandler* handler = furi_alloc(sizeof(IrdaEncoderHandler));
    handler->handler = NULL;
    handler->encoder = NULL;
    return handler;
}

void irda_free_encoder(IrdaEncoderHandler* handler) {
    furi_assert(handler);
    const IrdaEncoders* encoder = handler->encoder;

    if(encoder || handler->handler) {
        furi_assert(encoder);
        furi_assert(handler->handler);
        furi_assert(encoder->free);
        encoder->free(handler->handler);
    }

    free(handler);
}

static int irda_find_index_by_protocol(IrdaProtocol protocol) {
    for(int i = 0; i < COUNT_OF(irda_encoder_decoder); ++i) {
        if(irda_encoder_decoder[i].get_protocol_spec(protocol)) {
            return i;
        }
    }

    return -1;
}

void irda_reset_encoder(IrdaEncoderHandler* handler, const IrdaMessage* message) {
    furi_assert(handler);
    furi_assert(message);
    int index = irda_find_index_by_protocol(message->protocol);
    furi_check(index >= 0);

    const IrdaEncoders* required_encoder = &irda_encoder_decoder[index].encoder;
    furi_assert(required_encoder);
    furi_assert(required_encoder->reset);
    furi_assert(required_encoder->alloc);

    /* Realloc encoder if different protocol set */
    if(required_encoder != handler->encoder) {
        if(handler->handler != NULL) {
            furi_assert(handler->encoder->free);
            handler->encoder->free(handler->handler);
        }
        handler->encoder = required_encoder;
        handler->handler = handler->encoder->alloc();
    }

    handler->encoder->reset(handler->handler, message);
}

IrdaStatus irda_encode(IrdaEncoderHandler* handler, uint32_t* duration, bool* level) {
    furi_assert(handler);
    furi_assert(duration);
    furi_assert(level);
    const IrdaEncoders* encoder = handler->encoder;
    furi_assert(encoder);
    furi_assert(encoder->encode);

    IrdaStatus status = encoder->encode(handler->handler, duration, level);
    furi_assert(status != IrdaStatusError);

    return status;
}

bool irda_is_protocol_valid(IrdaProtocol protocol) {
    return irda_find_index_by_protocol(protocol) >= 0;
}

IrdaProtocol irda_get_protocol_by_name(const char* protocol_name) {
    for(IrdaProtocol protocol = 0; protocol < IrdaProtocolMAX; ++protocol) {
        const char* name = irda_get_protocol_name(protocol);
        if(!strcmp(name, protocol_name)) return protocol;
    }
    return IrdaProtocolUnknown;
}

static const IrdaProtocolSpecification* irda_get_spec_by_protocol(IrdaProtocol protocol) {
    int index = irda_find_index_by_protocol(protocol);
    const IrdaProtocolSpecification* spec = NULL;
    if(index >= 0) {
        spec = irda_encoder_decoder[index].get_protocol_spec(protocol);
    }

    furi_assert(spec);
    return spec;
}

const char* irda_get_protocol_name(IrdaProtocol protocol) {
    return irda_get_spec_by_protocol(protocol)->name;
}

uint8_t irda_get_protocol_address_length(IrdaProtocol protocol) {
    return irda_get_spec_by_protocol(protocol)->address_length;
}

uint8_t irda_get_protocol_command_length(IrdaProtocol protocol) {
    return irda_get_spec_by_protocol(protocol)->command_length;
}

uint32_t irda_get_protocol_frequency(IrdaProtocol protocol) {
    return irda_get_spec_by_protocol(protocol)->frequency;
}

float irda_get_protocol_duty_cycle(IrdaProtocol protocol) {
    return irda_get_spec_by_protocol(protocol)->duty_cycle;
}
