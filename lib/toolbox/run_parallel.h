#pragma once

#include <core/thread.h>

/**
 * @brief Run function in thread, then automatically clean up thread.
 * 
 * @param[in] callback pointer to a function to be executed in parallel
 * @param[in] context pointer to a user-specified object (will be passed to the callback)
 * @param[in] stack_size stack size in bytes
 */
void run_parallel(FuriThreadCallback callback, void* context, uint32_t stack_size);
