#pragma once

#include "message_queue.h"

#include "kernel.h"
#include "event_loop_i.h"
#include "check.h"

#include <FreeRTOS.h>
#include <queue.h>

extern const FuriEventLoopContract furi_message_queue_event_loop_contract;