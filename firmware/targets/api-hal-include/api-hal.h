#pragma once

#ifdef __cplusplus
template <unsigned int N> struct STOP_EXTERNING_ME {};
#endif

#include "api-hal-boot.h"
#include "api-hal-os.h"
#include "api-hal-i2c.h"
#include "api-hal-resources.h"
#include "api-hal-gpio.h"
#include "api-hal-light.h"
#include "api-hal-delay.h"
#include "api-hal-pwm.h"
#include "api-hal-task.h"
#include "api-hal-tim.h"
#include "api-hal-power.h"
#include "api-hal-vcp.h"
#include "api-hal-version.h"
#include "api-hal-uid.h"
#include "api-hal-bt.h"
#include "api-hal-spi.h"
#include "api-hal-flash.h"
#include "api-hal-subghz.h"
#include "api-hal-vibro.h"
#include "api-hal-ibutton.h"
#include "api-hal-rfid.h"

/** Init api-hal */
void api_hal_init();
