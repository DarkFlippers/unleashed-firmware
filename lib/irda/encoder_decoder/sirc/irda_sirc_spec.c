#include "../irda_i.h"
#include "irda_protocol_defs_i.h"

static const IrdaProtocolSpecification irda_sirc_protocol_specification = {
    .name = "SIRC",
    .address_length = 5,
    .command_length = 7,
    .frequency = IRDA_SIRC_CARRIER_FREQUENCY,
    .duty_cycle = IRDA_SIRC_DUTY_CYCLE,
};

static const IrdaProtocolSpecification irda_sirc15_protocol_specification = {
    .name = "SIRC15",
    .address_length = 8,
    .command_length = 7,
    .frequency = IRDA_SIRC_CARRIER_FREQUENCY,
    .duty_cycle = IRDA_SIRC_DUTY_CYCLE,
};

static const IrdaProtocolSpecification irda_sirc20_protocol_specification = {
    .name = "SIRC20",
    .address_length = 13,
    .command_length = 7,
    .frequency = IRDA_SIRC_CARRIER_FREQUENCY,
    .duty_cycle = IRDA_SIRC_DUTY_CYCLE,
};

const IrdaProtocolSpecification* irda_sirc_get_spec(IrdaProtocol protocol) {
    if(protocol == IrdaProtocolSIRC)
        return &irda_sirc_protocol_specification;
    else if(protocol == IrdaProtocolSIRC15)
        return &irda_sirc15_protocol_specification;
    else if(protocol == IrdaProtocolSIRC20)
        return &irda_sirc20_protocol_specification;
    else
        return NULL;
}
