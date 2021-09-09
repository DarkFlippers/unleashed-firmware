#include "../irda_i.h"
#include "irda_protocol_defs_i.h"

static const IrdaProtocolSpecification irda_rc5_protocol_specification = {
      .name = "RC5",
      .address_length = 5,
      .command_length = 6,
      .frequency = IRDA_RC5_CARRIER_FREQUENCY,
      .duty_cycle = IRDA_RC5_DUTY_CYCLE,
};

static const IrdaProtocolSpecification irda_rc5x_protocol_specification = {
      .name = "RC5X",
      .address_length = 5,
      .command_length = 7,
      .frequency = IRDA_RC5_CARRIER_FREQUENCY,
      .duty_cycle = IRDA_RC5_DUTY_CYCLE,
};

const IrdaProtocolSpecification* irda_rc5_get_spec(IrdaProtocol protocol) {
    if (protocol == IrdaProtocolRC5)
        return &irda_rc5_protocol_specification;
    else if (protocol == IrdaProtocolRC5X)
        return &irda_rc5x_protocol_specification;
    else
        return NULL;
}

