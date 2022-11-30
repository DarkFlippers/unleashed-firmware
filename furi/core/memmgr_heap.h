/**
 * @file memmgr_heap.h
 * Furi: heap memory management API and allocator
 */

#pragma once

#include <stdint.h>
#include <core/thread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MEMMGR_HEAP_UNKNOWN 0xFFFFFFFF

/** Memmgr heap enable thread allocation tracking
 *
 * @param      thread_id  - thread id to track
 */
void memmgr_heap_enable_thread_trace(FuriThreadId taks_handle);

/** Memmgr heap disable thread allocation tracking
 *
 * @param      thread_id  - thread id to track
 */
void memmgr_heap_disable_thread_trace(FuriThreadId taks_handle);

/** Memmgr heap get allocatred thread memory
 *
 * @param      thread_id  - thread id to track
 *
 * @return     bytes allocated right now
 */
size_t memmgr_heap_get_thread_memory(FuriThreadId taks_handle);

/** Memmgr heap get the max contiguous block size on the heap
 *
 * @return     size_t max contiguous block size
 */
size_t memmgr_heap_get_max_free_block();

/** Print the address and size of all free blocks to stdout
 */
void memmgr_heap_printf_free_blocks();

#ifdef __cplusplus
}
#endif
