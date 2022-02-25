#include "../infrared_i.h"
#include "infrared_protocol_defs_i.h"

static const InfraredProtocolSpecification infrared_rc5_protocol_specification = {
    .name = "RC5",
    .address_length = 5,
    .command_length = 6,
    .frequency = INFRARED_RC5_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_RC5_DUTY_CYCLE,
};

static const InfraredProtocolSpecification infrared_rc5x_protocol_specification = {
    .name = "RC5X",
    .address_length = 5,
    .command_length = 7,
    .frequency = INFRARED_RC5_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_RC5_DUTY_CYCLE,
};

const InfraredProtocolSpecification* infrared_rc5_get_spec(InfraredProtocol protocol) {
    if(protocol == InfraredProtocolRC5)
        return &infrared_rc5_protocol_specification;
    else if(protocol == InfraredProtocolRC5X)
        return &infrared_rc5x_protocol_specification;
    else
        return NULL;
}
