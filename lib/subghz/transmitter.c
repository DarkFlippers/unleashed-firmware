#include "transmitter.h"

#include "protocols/base.h"
#include "protocols/registry.h"

struct SubGhzTransmitter {
    const SubGhzProtocol* protocol;
    SubGhzProtocolEncoderBase* protocol_instance;
};

SubGhzTransmitter*
    subghz_transmitter_alloc_init(SubGhzEnvironment* environment, const char* protocol_name) {
    SubGhzTransmitter* instance = NULL;
    const SubGhzProtocol* protocol = subghz_protocol_registry_get_by_name(protocol_name);

    if(protocol && protocol->encoder && protocol->encoder->alloc) {
        instance = malloc(sizeof(SubGhzTransmitter));
        instance->protocol = protocol;
        instance->protocol_instance = instance->protocol->encoder->alloc(environment);
    }

    return instance;
}

void subghz_transmitter_free(SubGhzTransmitter* instance) {
    furi_assert(instance);
    instance->protocol->encoder->free(instance->protocol_instance);
    free(instance);
}

SubGhzProtocolEncoderBase* subghz_transmitter_get_protocol_instance(SubGhzTransmitter* instance) {
    furi_assert(instance);
    return instance->protocol_instance;
}

bool subghz_transmitter_stop(SubGhzTransmitter* instance) {
    furi_assert(instance);
    bool ret = false;
    if(instance->protocol && instance->protocol->encoder && instance->protocol->encoder->stop) {
        instance->protocol->encoder->stop(instance->protocol_instance);
        ret = true;
    }
    return ret;
}

bool subghz_transmitter_deserialize(SubGhzTransmitter* instance, FlipperFormat* flipper_format) {
    furi_assert(instance);
    bool ret = false;
    if(instance->protocol && instance->protocol->encoder &&
       instance->protocol->encoder->deserialize) {
        ret =
            instance->protocol->encoder->deserialize(instance->protocol_instance, flipper_format);
    }
    return ret;
}

LevelDuration subghz_transmitter_yield(void* context) {
    SubGhzTransmitter* instance = context;
    return instance->protocol->encoder->yield(instance->protocol_instance);
}
