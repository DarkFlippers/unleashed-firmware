#pragma once

#include <stdint.h>
#include <cmsis_os2.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Memmgr heap enable thread allocation tracking
 * @param thread_id - thread id to track
 */
void memmgr_heap_enable_thread_trace(osThreadId_t thread_id);

/** Memmgr heap disable thread allocation tracking
 * @param thread_id - thread id to track
 */
void memmgr_heap_disable_thread_trace(osThreadId_t thread_id);

/** Memmgr heap get allocatred thread memory
 * @param thread_id - thread id to track
 * @return bytes allocated right now
 */
size_t memmgr_heap_get_thread_memory(osThreadId_t thread_id);

#ifdef __cplusplus
}
#endif
