#include "../irda_i.h"
#include "irda_protocol_defs_i.h"

static const IrdaProtocolSpecification irda_nec_protocol_specification = {
    .name = "NEC",
    .address_length = 8,
    .command_length = 8,
    .frequency = IRDA_COMMON_CARRIER_FREQUENCY,
    .duty_cycle = IRDA_COMMON_DUTY_CYCLE,
};

static const IrdaProtocolSpecification irda_necext_protocol_specification = {
    .name = "NECext",
    .address_length = 16,
    .command_length = 16,
    .frequency = IRDA_COMMON_CARRIER_FREQUENCY,
    .duty_cycle = IRDA_COMMON_DUTY_CYCLE,
};

static const IrdaProtocolSpecification irda_nec42_protocol_specification = {
    .name = "NEC42",
    .address_length = 13,
    .command_length = 8,
    .frequency = IRDA_COMMON_CARRIER_FREQUENCY,
    .duty_cycle = IRDA_COMMON_DUTY_CYCLE,
};

static const IrdaProtocolSpecification irda_nec42ext_protocol_specification = {
    .name = "NEC42ext",
    .address_length = 26,
    .command_length = 16,
    .frequency = IRDA_COMMON_CARRIER_FREQUENCY,
    .duty_cycle = IRDA_COMMON_DUTY_CYCLE,
};

const IrdaProtocolSpecification* irda_nec_get_spec(IrdaProtocol protocol) {
    if(protocol == IrdaProtocolNEC)
        return &irda_nec_protocol_specification;
    else if(protocol == IrdaProtocolNECext)
        return &irda_necext_protocol_specification;
    else if(protocol == IrdaProtocolNEC42)
        return &irda_nec42_protocol_specification;
    else if(protocol == IrdaProtocolNEC42ext)
        return &irda_nec42ext_protocol_specification;
    else
        return NULL;
}
