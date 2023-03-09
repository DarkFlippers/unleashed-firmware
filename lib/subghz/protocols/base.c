#include "base.h"
#include "registry.h"

void subghz_protocol_decoder_base_set_decoder_callback(
    SubGhzProtocolDecoderBase* decoder_base,
    SubGhzProtocolDecoderBaseRxCallback callback,
    void* context) {
    decoder_base->callback = callback;
    decoder_base->context = context;
}

bool subghz_protocol_decoder_base_get_string(
    SubGhzProtocolDecoderBase* decoder_base,
    FuriString* output) {
    bool status = false;

    if(decoder_base->protocol && decoder_base->protocol->decoder &&
       decoder_base->protocol->decoder->get_string) {
        decoder_base->protocol->decoder->get_string(decoder_base, output);
        status = true;
    }

    return status;
}

SubGhzProtocolStatus subghz_protocol_decoder_base_serialize(
    SubGhzProtocolDecoderBase* decoder_base,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    SubGhzProtocolStatus status = SubGhzProtocolStatusError;

    if(decoder_base->protocol && decoder_base->protocol->decoder &&
       decoder_base->protocol->decoder->serialize) {
        status = decoder_base->protocol->decoder->serialize(decoder_base, flipper_format, preset);
    }

    return status;
}

SubGhzProtocolStatus subghz_protocol_decoder_base_deserialize(
    SubGhzProtocolDecoderBase* decoder_base,
    FlipperFormat* flipper_format) {
    SubGhzProtocolStatus status = SubGhzProtocolStatusError;

    if(decoder_base->protocol && decoder_base->protocol->decoder &&
       decoder_base->protocol->decoder->deserialize) {
        status = decoder_base->protocol->decoder->deserialize(decoder_base, flipper_format);
    }

    return status;
}

uint8_t subghz_protocol_decoder_base_get_hash_data(SubGhzProtocolDecoderBase* decoder_base) {
    uint8_t hash = 0;

    if(decoder_base->protocol && decoder_base->protocol->decoder &&
       decoder_base->protocol->decoder->get_hash_data) {
        hash = decoder_base->protocol->decoder->get_hash_data(decoder_base);
    }

    return hash;
}
