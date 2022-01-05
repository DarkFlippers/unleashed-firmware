#include "../irda_i.h"
#include "irda_protocol_defs_i.h"

static const IrdaProtocolSpecification irda_samsung32_protocol_specification = {
    .name = "Samsung32",
    .address_length = 8,
    .command_length = 8,
    .frequency = IRDA_COMMON_CARRIER_FREQUENCY,
    .duty_cycle = IRDA_COMMON_DUTY_CYCLE,
};

const IrdaProtocolSpecification* irda_samsung32_get_spec(IrdaProtocol protocol) {
    if(protocol == IrdaProtocolSamsung32)
        return &irda_samsung32_protocol_specification;
    else
        return NULL;
}
