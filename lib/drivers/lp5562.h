#pragma once

#include <stdint.h>
#include <stdbool.h>

/** Channel types */
typedef enum {
    LP5562ChannelRed,
    LP5562ChannelGreen,
    LP5562ChannelBlue,
    LP5562ChannelWhite,
} LP5562Channel;

/** Initialize Driver */
void lp5562_reset();

/** Configure Driver */
void lp5562_configure();

/** Enable Driver */
void lp5562_enable();

/** Set channel current */
void lp5562_set_channel_current(LP5562Channel channel, uint8_t value);

/** Set channel current */
void lp5562_set_channel_value(LP5562Channel channel, uint8_t value);
