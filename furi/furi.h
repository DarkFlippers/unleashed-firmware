#pragma once

#include <stdlib.h>

#include <core/check.h>
#include <core/common_defines.h>
#include <core/event_flag.h>
#include <core/kernel.h>
#include <core/log.h>
#include <core/memmgr.h>
#include <core/memmgr_heap.h>
#include <core/message_queue.h>
#include <core/mutex.h>
#include <core/pubsub.h>
#include <core/record.h>
#include <core/semaphore.h>
#include <core/thread.h>
#include <core/timer.h>
#include <core/valuemutex.h>

#include <furi_hal_gpio.h>

// FreeRTOS timer, REMOVE AFTER REFACTORING
#include <timers.h>

#ifdef __cplusplus
extern "C" {
#endif

void furi_init();

void furi_run();

#ifdef __cplusplus
}
#endif
