#include "../infrared_i.h"
#include "infrared_protocol_defs_i.h"

static const InfraredProtocolSpecification infrared_samsung32_protocol_specification = {
    .name = "Samsung32",
    .address_length = 8,
    .command_length = 8,
    .frequency = INFRARED_COMMON_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_COMMON_DUTY_CYCLE,
};

const InfraredProtocolSpecification* infrared_samsung32_get_spec(InfraredProtocol protocol) {
    if(protocol == InfraredProtocolSamsung32)
        return &infrared_samsung32_protocol_specification;
    else
        return NULL;
}
