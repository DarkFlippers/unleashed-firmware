#pragma once

#include "thread.h"

/**
 * @brief Notify `FuriEventLoop` that `furi_thread_flags_set` has been called
 * 
 * @param thread_id Thread id
 */
extern void furi_event_loop_thread_flag_callback(FuriThreadId thread_id);
