#include "../infrared_i.h"
#include "infrared_protocol_defs_i.h"

static const InfraredProtocolSpecification infrared_nec_protocol_specification = {
    .name = "NEC",
    .address_length = 8,
    .command_length = 8,
    .frequency = INFRARED_COMMON_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_COMMON_DUTY_CYCLE,
};

static const InfraredProtocolSpecification infrared_necext_protocol_specification = {
    .name = "NECext",
    .address_length = 16,
    .command_length = 16,
    .frequency = INFRARED_COMMON_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_COMMON_DUTY_CYCLE,
};

static const InfraredProtocolSpecification infrared_nec42_protocol_specification = {
    .name = "NEC42",
    .address_length = 13,
    .command_length = 8,
    .frequency = INFRARED_COMMON_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_COMMON_DUTY_CYCLE,
};

static const InfraredProtocolSpecification infrared_nec42ext_protocol_specification = {
    .name = "NEC42ext",
    .address_length = 26,
    .command_length = 16,
    .frequency = INFRARED_COMMON_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_COMMON_DUTY_CYCLE,
};

const InfraredProtocolSpecification* infrared_nec_get_spec(InfraredProtocol protocol) {
    if(protocol == InfraredProtocolNEC)
        return &infrared_nec_protocol_specification;
    else if(protocol == InfraredProtocolNECext)
        return &infrared_necext_protocol_specification;
    else if(protocol == InfraredProtocolNEC42)
        return &infrared_nec42_protocol_specification;
    else if(protocol == InfraredProtocolNEC42ext)
        return &infrared_nec42ext_protocol_specification;
    else
        return NULL;
}
