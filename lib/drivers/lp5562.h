#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum {
	LP5562ChannelRed,
	LP5562ChannelGreen,
	LP5562ChannelBlue,
	LP5562ChannelWhite,
} LP5562Channel;

/* Initialize Driver */
void lp5562_reset();

void lp5562_configure();

void lp5562_enable();

void lp5562_set_channel_current(LP5562Channel channel, uint8_t value);

void lp5562_set_channel_value(LP5562Channel channel, uint8_t value);
