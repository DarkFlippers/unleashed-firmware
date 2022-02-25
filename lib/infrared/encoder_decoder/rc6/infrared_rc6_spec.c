#include "../infrared_i.h"
#include "infrared_protocol_defs_i.h"

static const InfraredProtocolSpecification infrared_rc6_protocol_specification = {
    .name = "RC6",
    .address_length = 8,
    .command_length = 8,
    .frequency = INFRARED_RC6_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_RC6_DUTY_CYCLE,
};

const InfraredProtocolSpecification* infrared_rc6_get_spec(InfraredProtocol protocol) {
    if(protocol == InfraredProtocolRC6)
        return &infrared_rc6_protocol_specification;
    else
        return NULL;
}
