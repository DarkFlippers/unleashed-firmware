#include "../infrared_i.h"
#include "infrared_protocol_defs_i.h"

static const InfraredProtocolSpecification infrared_kaseikyo_protocol_specification = {
    .name = "Kaseikyo",
    .address_length = 26,
    .command_length = 10,
    .frequency = INFRARED_COMMON_CARRIER_FREQUENCY,
    .duty_cycle = INFRARED_COMMON_DUTY_CYCLE,
};

const InfraredProtocolSpecification* infrared_kaseikyo_get_spec(InfraredProtocol protocol) {
    if(protocol == InfraredProtocolKaseikyo)
        return &infrared_kaseikyo_protocol_specification;
    else
        return NULL;
}
