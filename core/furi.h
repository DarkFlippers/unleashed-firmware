#pragma once

#include <cmsis_os2.h>

#include <furi/common_defines.h>
#include <furi/check.h>
#include <furi/memmgr.h>
#include <furi/memmgr_heap.h>
#include <furi/pubsub.h>
#include <furi/record.h>
#include <furi/stdglue.h>
#include <furi/thread.h>
#include <furi/valuemutex.h>
#include <furi/log.h>

#include <furi-hal-gpio.h>
#include <furi-hal/api-interrupt-mgr.h>

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void furi_init();

#ifdef __cplusplus
}
#endif
