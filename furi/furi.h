#pragma once

#include <stdlib.h>

#include "core/check.h"
#include "core/common_defines.h"
#include "core/event_flag.h"
#include "core/kernel.h"
#include "core/log.h"
#include "core/memmgr.h"
#include "core/memmgr_heap.h"
#include "core/message_queue.h"
#include "core/mutex.h"
#include "core/pubsub.h"
#include "core/record.h"
#include "core/semaphore.h"
#include "core/thread.h"
#include "core/timer.h"
#include "core/string.h"
#include "core/stream_buffer.h"

#include <furi_hal_gpio.h>

// Workaround for math.h leaking through HAL in older versions
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

void furi_init();

void furi_run();

#ifdef __cplusplus
}
#endif
