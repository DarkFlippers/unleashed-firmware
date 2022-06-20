#pragma once

#include <FreeRTOS.h>
#include <timers.h>
#include <task.h>

#include <cmsis_os2.h>

#include <furi/check.h>
#include <furi/common_defines.h>
#include <furi/log.h>
#include <furi/event_flags.h>
#include <furi/memmgr.h>
#include <furi/memmgr_heap.h>
#include <furi/mutex.h>
#include <furi/pubsub.h>
#include <furi/record.h>
#include <furi/semaphore.h>
#include <furi/stdglue.h>
#include <furi/thread.h>
#include <furi/valuemutex.h>

#include <furi_hal_gpio.h>

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void furi_init();

void furi_run();

#ifdef __cplusplus
}
#endif
