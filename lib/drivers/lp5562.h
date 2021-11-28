#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <furi-hal-i2c.h>

/** Channel types */
typedef enum {
    LP5562ChannelRed,
    LP5562ChannelGreen,
    LP5562ChannelBlue,
    LP5562ChannelWhite,
} LP5562Channel;

/** Initialize Driver */
void lp5562_reset(FuriHalI2cBusHandle* handle);

/** Configure Driver */
void lp5562_configure(FuriHalI2cBusHandle* handle);

/** Enable Driver */
void lp5562_enable(FuriHalI2cBusHandle* handle);

/** Set channel current */
void lp5562_set_channel_current(FuriHalI2cBusHandle* handle, LP5562Channel channel, uint8_t value);

/** Set channel current */
void lp5562_set_channel_value(FuriHalI2cBusHandle* handle, LP5562Channel channel, uint8_t value);
