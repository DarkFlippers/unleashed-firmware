#pragma once

#include "flipper.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "api-basic/furi.h"
//#include "api-basic/flapp.h"
#include "cmsis_os2.h"
#include "api-basic/valuemutex.h"
#include "api-basic/pubsub.h"
#include "api-basic/value-expanders.h"
#include "api-basic/event.h"

#include "api-basic/memmgr.h"
#include "api-basic/check.h"

#include "api-hal/api-gpio.h"
#include "api-hal/api-interrupt-mgr.h"
#include "api-hal-resources.h"

#include "gui/gui.h"

// tmeout for helper functions
#define FLIPPER_HELPER_TIMEOUT 10

bool init_flipper_api(void);

#ifdef __cplusplus
}
#endif