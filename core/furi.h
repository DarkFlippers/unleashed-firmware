#pragma once

#include <cmsis_os2.h>

#include <furi/check.h>
#include <furi/event.h>
#include <furi/memmgr.h>
#include <furi/pubsub.h>
#include <furi/record.h>
#include <furi/stdglue.h>
#include <furi/value-expanders.h>
#include <furi/valuemutex.h>

#include <api-hal/api-gpio.h>
#include <api-hal/api-interrupt-mgr.h>

#include <api-hal.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define furiac_exit(ptr) osThreadExit()

#ifdef __cplusplus
}
#endif
