#include "../irda_i.h"
#include "irda_protocol_defs_i.h"

static const IrdaProtocolSpecification irda_rc6_protocol_specification = {
    .name = "RC6",
    .address_length = 8,
    .command_length = 8,
    .frequency = IRDA_RC6_CARRIER_FREQUENCY,
    .duty_cycle = IRDA_RC6_DUTY_CYCLE,
};

const IrdaProtocolSpecification* irda_rc6_get_spec(IrdaProtocol protocol) {
    if(protocol == IrdaProtocolRC6)
        return &irda_rc6_protocol_specification;
    else
        return NULL;
}
