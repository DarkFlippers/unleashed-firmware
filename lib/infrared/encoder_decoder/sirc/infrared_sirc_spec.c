#include "../infrared_i.h"
#include "infrared_protocol_defs_i.h"

static const InfraredProtocolSpecification infrared_sirc_protocol_specification = {
    .name = "SIRC",
    .address_length = 5,
    .command_length = 7,
    .frequency = INFRARED_SIRC_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_SIRC_DUTY_CYCLE,
};

static const InfraredProtocolSpecification infrared_sirc15_protocol_specification = {
    .name = "SIRC15",
    .address_length = 8,
    .command_length = 7,
    .frequency = INFRARED_SIRC_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_SIRC_DUTY_CYCLE,
};

static const InfraredProtocolSpecification infrared_sirc20_protocol_specification = {
    .name = "SIRC20",
    .address_length = 13,
    .command_length = 7,
    .frequency = INFRARED_SIRC_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_SIRC_DUTY_CYCLE,
};

const InfraredProtocolSpecification* infrared_sirc_get_spec(InfraredProtocol protocol) {
    if(protocol == InfraredProtocolSIRC)
        return &infrared_sirc_protocol_specification;
    else if(protocol == InfraredProtocolSIRC15)
        return &infrared_sirc15_protocol_specification;
    else if(protocol == InfraredProtocolSIRC20)
        return &infrared_sirc20_protocol_specification;
    else
        return NULL;
}
