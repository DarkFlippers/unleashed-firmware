#include "infrared.h"

#include <stdlib.h>
#include <string.h>
#include <core/check.h>
#include <core/common_defines.h>

#include "nec/infrared_protocol_nec.h"
#include "samsung/infrared_protocol_samsung.h"
#include "rc5/infrared_protocol_rc5.h"
#include "rc6/infrared_protocol_rc6.h"
#include "sirc/infrared_protocol_sirc.h"
#include "kaseikyo/infrared_protocol_kaseikyo.h"
#include "rca/infrared_protocol_rca.h"
#include "pioneer/infrared_protocol_pioneer.h"

typedef struct {
    InfraredAlloc alloc;
    InfraredDecode decode;
    InfraredDecoderReset reset;
    InfraredFree free;
    InfraredDecoderCheckReady check_ready;
} InfraredDecoders;

typedef struct {
    InfraredAlloc alloc;
    InfraredEncode encode;
    InfraredEncoderReset reset;
    InfraredFree free;
} InfraredEncoders;

struct InfraredDecoderHandler {
    void** ctx;
};

struct InfraredEncoderHandler {
    void* handler;
    const InfraredEncoders* encoder;
};

typedef struct {
    InfraredEncoders encoder;
    InfraredDecoders decoder;
    InfraredGetProtocolVariant get_protocol_variant;
} InfraredEncoderDecoder;

static const InfraredEncoderDecoder infrared_encoder_decoder[] = {
    {
        .decoder =
            {.alloc = infrared_decoder_nec_alloc,
             .decode = infrared_decoder_nec_decode,
             .reset = infrared_decoder_nec_reset,
             .check_ready = infrared_decoder_nec_check_ready,
             .free = infrared_decoder_nec_free},
        .encoder =
            {.alloc = infrared_encoder_nec_alloc,
             .encode = infrared_encoder_nec_encode,
             .reset = infrared_encoder_nec_reset,
             .free = infrared_encoder_nec_free},
        .get_protocol_variant = infrared_protocol_nec_get_variant,
    },
    {
        .decoder =
            {.alloc = infrared_decoder_samsung32_alloc,
             .decode = infrared_decoder_samsung32_decode,
             .reset = infrared_decoder_samsung32_reset,
             .check_ready = infrared_decoder_samsung32_check_ready,
             .free = infrared_decoder_samsung32_free},
        .encoder =
            {.alloc = infrared_encoder_samsung32_alloc,
             .encode = infrared_encoder_samsung32_encode,
             .reset = infrared_encoder_samsung32_reset,
             .free = infrared_encoder_samsung32_free},
        .get_protocol_variant = infrared_protocol_samsung32_get_variant,
    },
    {
        .decoder =
            {.alloc = infrared_decoder_rc5_alloc,
             .decode = infrared_decoder_rc5_decode,
             .reset = infrared_decoder_rc5_reset,
             .check_ready = infrared_decoder_rc5_check_ready,
             .free = infrared_decoder_rc5_free},
        .encoder =
            {.alloc = infrared_encoder_rc5_alloc,
             .encode = infrared_encoder_rc5_encode,
             .reset = infrared_encoder_rc5_reset,
             .free = infrared_encoder_rc5_free},
        .get_protocol_variant = infrared_protocol_rc5_get_variant,
    },
    {
        .decoder =
            {.alloc = infrared_decoder_rc6_alloc,
             .decode = infrared_decoder_rc6_decode,
             .reset = infrared_decoder_rc6_reset,
             .check_ready = infrared_decoder_rc6_check_ready,
             .free = infrared_decoder_rc6_free},
        .encoder =
            {.alloc = infrared_encoder_rc6_alloc,
             .encode = infrared_encoder_rc6_encode,
             .reset = infrared_encoder_rc6_reset,
             .free = infrared_encoder_rc6_free},
        .get_protocol_variant = infrared_protocol_rc6_get_variant,
    },
    {
        .decoder =
            {.alloc = infrared_decoder_sirc_alloc,
             .decode = infrared_decoder_sirc_decode,
             .reset = infrared_decoder_sirc_reset,
             .check_ready = infrared_decoder_sirc_check_ready,
             .free = infrared_decoder_sirc_free},
        .encoder =
            {.alloc = infrared_encoder_sirc_alloc,
             .encode = infrared_encoder_sirc_encode,
             .reset = infrared_encoder_sirc_reset,
             .free = infrared_encoder_sirc_free},
        .get_protocol_variant = infrared_protocol_sirc_get_variant,
    },
    {
        .decoder =
            {.alloc = infrared_decoder_pioneer_alloc,
             .decode = infrared_decoder_pioneer_decode,
             .reset = infrared_decoder_pioneer_reset,
             .check_ready = infrared_decoder_pioneer_check_ready,
             .free = infrared_decoder_pioneer_free},
        .encoder =
            {.alloc = infrared_encoder_pioneer_alloc,
             .encode = infrared_encoder_pioneer_encode,
             .reset = infrared_encoder_pioneer_reset,
             .free = infrared_encoder_pioneer_free},
        .get_protocol_variant = infrared_protocol_pioneer_get_variant,
    },
    {
        .decoder =
            {.alloc = infrared_decoder_kaseikyo_alloc,
             .decode = infrared_decoder_kaseikyo_decode,
             .reset = infrared_decoder_kaseikyo_reset,
             .check_ready = infrared_decoder_kaseikyo_check_ready,
             .free = infrared_decoder_kaseikyo_free},
        .encoder =
            {.alloc = infrared_encoder_kaseikyo_alloc,
             .encode = infrared_encoder_kaseikyo_encode,
             .reset = infrared_encoder_kaseikyo_reset,
             .free = infrared_encoder_kaseikyo_free},
        .get_protocol_variant = infrared_protocol_kaseikyo_get_variant,
    },
    {
        .decoder =
            {.alloc = infrared_decoder_rca_alloc,
             .decode = infrared_decoder_rca_decode,
             .reset = infrared_decoder_rca_reset,
             .check_ready = infrared_decoder_rca_check_ready,
             .free = infrared_decoder_rca_free},
        .encoder =
            {.alloc = infrared_encoder_rca_alloc,
             .encode = infrared_encoder_rca_encode,
             .reset = infrared_encoder_rca_reset,
             .free = infrared_encoder_rca_free},
        .get_protocol_variant = infrared_protocol_rca_get_variant,
    },
};

static int infrared_find_index_by_protocol(InfraredProtocol protocol);
static const InfraredProtocolVariant* infrared_get_variant_by_protocol(InfraredProtocol protocol);

const InfraredMessage*
    infrared_decode(InfraredDecoderHandler* handler, bool level, uint32_t duration) {
    furi_check(handler);

    InfraredMessage* message = NULL;
    InfraredMessage* result = NULL;

    for(size_t i = 0; i < COUNT_OF(infrared_encoder_decoder); ++i) {
        if(infrared_encoder_decoder[i].decoder.decode) {
            message = infrared_encoder_decoder[i].decoder.decode(handler->ctx[i], level, duration);
            if(!result && message) {
                result = message;
            }
        }
    }

    return result;
}

InfraredDecoderHandler* infrared_alloc_decoder(void) {
    InfraredDecoderHandler* handler = malloc(sizeof(InfraredDecoderHandler));
    handler->ctx = malloc(sizeof(void*) * COUNT_OF(infrared_encoder_decoder));

    for(size_t i = 0; i < COUNT_OF(infrared_encoder_decoder); ++i) {
        handler->ctx[i] = 0;
        if(infrared_encoder_decoder[i].decoder.alloc)
            handler->ctx[i] = infrared_encoder_decoder[i].decoder.alloc();
    }

    infrared_reset_decoder(handler);
    return handler;
}

void infrared_free_decoder(InfraredDecoderHandler* handler) {
    furi_check(handler);
    furi_check(handler->ctx);

    for(size_t i = 0; i < COUNT_OF(infrared_encoder_decoder); ++i) {
        if(infrared_encoder_decoder[i].decoder.free)
            infrared_encoder_decoder[i].decoder.free(handler->ctx[i]);
    }

    free(handler->ctx);
    free(handler);
}

void infrared_reset_decoder(InfraredDecoderHandler* handler) {
    furi_check(handler);

    for(size_t i = 0; i < COUNT_OF(infrared_encoder_decoder); ++i) {
        if(infrared_encoder_decoder[i].decoder.reset)
            infrared_encoder_decoder[i].decoder.reset(handler->ctx[i]);
    }
}

const InfraredMessage* infrared_check_decoder_ready(InfraredDecoderHandler* handler) {
    furi_check(handler);

    InfraredMessage* message = NULL;
    InfraredMessage* result = NULL;

    for(size_t i = 0; i < COUNT_OF(infrared_encoder_decoder); ++i) {
        if(infrared_encoder_decoder[i].decoder.check_ready) {
            message = infrared_encoder_decoder[i].decoder.check_ready(handler->ctx[i]);
            if(!result && message) {
                result = message;
            }
        }
    }

    return result;
}

InfraredEncoderHandler* infrared_alloc_encoder(void) {
    InfraredEncoderHandler* handler = malloc(sizeof(InfraredEncoderHandler));
    handler->handler = NULL;
    handler->encoder = NULL;
    return handler;
}

void infrared_free_encoder(InfraredEncoderHandler* handler) {
    furi_check(handler);
    const InfraredEncoders* encoder = handler->encoder;

    if(encoder || handler->handler) {
        furi_check(encoder);
        furi_check(handler->handler);
        furi_check(encoder->free);
        encoder->free(handler->handler);
    }

    free(handler);
}

static int infrared_find_index_by_protocol(InfraredProtocol protocol) {
    for(size_t i = 0; i < COUNT_OF(infrared_encoder_decoder); ++i) {
        if(infrared_encoder_decoder[i].get_protocol_variant(protocol)) {
            return i;
        }
    }

    return -1;
}

void infrared_reset_encoder(InfraredEncoderHandler* handler, const InfraredMessage* message) {
    furi_check(handler);
    furi_check(message);
    int index = infrared_find_index_by_protocol(message->protocol);
    furi_check(index >= 0);

    const InfraredEncoders* required_encoder = &infrared_encoder_decoder[index].encoder;
    furi_check(required_encoder);
    furi_check(required_encoder->reset);
    furi_check(required_encoder->alloc);

    /* Realloc encoder if different protocol set */
    if(required_encoder != handler->encoder) {
        if(handler->handler != NULL) {
            furi_check(handler->encoder->free);
            handler->encoder->free(handler->handler);
        }
        handler->encoder = required_encoder;
        handler->handler = handler->encoder->alloc();
    }

    handler->encoder->reset(handler->handler, message);
}

InfraredStatus infrared_encode(InfraredEncoderHandler* handler, uint32_t* duration, bool* level) {
    furi_check(handler);
    furi_check(duration);
    furi_check(level);

    const InfraredEncoders* encoder = handler->encoder;
    furi_check(encoder);
    furi_check(encoder->encode);

    InfraredStatus status = encoder->encode(handler->handler, duration, level);
    furi_check(status != InfraredStatusError);

    return status;
}

bool infrared_is_protocol_valid(InfraredProtocol protocol) {
    return infrared_find_index_by_protocol(protocol) >= 0;
}

InfraredProtocol infrared_get_protocol_by_name(const char* protocol_name) {
    furi_check(protocol_name);

    for(InfraredProtocol protocol = 0; protocol < InfraredProtocolMAX; ++protocol) {
        const char* name = infrared_get_protocol_name(protocol);
        if(!strcmp(name, protocol_name)) return protocol;
    }
    return InfraredProtocolUnknown;
}

static const InfraredProtocolVariant* infrared_get_variant_by_protocol(InfraredProtocol protocol) {
    int index = infrared_find_index_by_protocol(protocol);
    const InfraredProtocolVariant* variant = NULL;
    if(index >= 0) {
        variant = infrared_encoder_decoder[index].get_protocol_variant(protocol);
    }

    furi_check(variant);
    return variant;
}

const char* infrared_get_protocol_name(InfraredProtocol protocol) {
    return infrared_get_variant_by_protocol(protocol)->name;
}

uint8_t infrared_get_protocol_address_length(InfraredProtocol protocol) {
    return infrared_get_variant_by_protocol(protocol)->address_length;
}

uint8_t infrared_get_protocol_command_length(InfraredProtocol protocol) {
    return infrared_get_variant_by_protocol(protocol)->command_length;
}

uint32_t infrared_get_protocol_frequency(InfraredProtocol protocol) {
    return infrared_get_variant_by_protocol(protocol)->frequency;
}

float infrared_get_protocol_duty_cycle(InfraredProtocol protocol) {
    return infrared_get_variant_by_protocol(protocol)->duty_cycle;
}

size_t infrared_get_protocol_min_repeat_count(InfraredProtocol protocol) {
    return infrared_get_variant_by_protocol(protocol)->repeat_count;
}
