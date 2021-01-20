#pragma once

#ifdef __cplusplus
template <unsigned int N> struct STOP_EXTERNING_ME {};
#endif

#include "api-hal-resources.h"
#include "api-hal-timebase.h"
#include "api-hal-boot.h"
#include "api-hal-gpio.h"
#include "api-hal-delay.h"
#include "api-hal-pwm.h"
#include "api-hal-task.h"
#include "api-hal-tim.h"
#include "api-hal-power.h"
#include "api-hal-vcp.h"
#include "api-hal-uid.h"
#include "api-hal-bt.h"
#include "api-hal-spi.h"
#include "api-hal-flash.h"

void api_hal_init();
