#pragma once

#include <cmsis_os2.h>

#include <furi/check.h>
#include <furi/memmgr.h>
#include <furi/pubsub.h>
#include <furi/record.h>
#include <furi/stdglue.h>
#include <furi/thread.h>
#include <furi/valuemutex.h>
#include <furi/log.h>
#include <furi/utils.h>

#include <api-hal-gpio.h>
#include <api-hal/api-interrupt-mgr.h>

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void furi_init();

#ifdef __cplusplus
}
#endif
