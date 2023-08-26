/**
 * @file thread.h
 * Furi: Furi Thread API
 */

#pragma once

#include "base.h"
#include "common_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

/** FuriThreadState */
typedef enum {
    FuriThreadStateStopped,
    FuriThreadStateStarting,
    FuriThreadStateRunning,
} FuriThreadState;

/** FuriThreadPriority */
typedef enum {
    FuriThreadPriorityNone = 0, /**< Uninitialized, choose system default */
    FuriThreadPriorityIdle = 1, /**< Idle priority */
    FuriThreadPriorityLowest = 14, /**< Lowest */
    FuriThreadPriorityLow = 15, /**< Low */
    FuriThreadPriorityNormal = 16, /**< Normal */
    FuriThreadPriorityHigh = 17, /**< High */
    FuriThreadPriorityHighest = 18, /**< Highest */
    FuriThreadPriorityIsr = (configMAX_PRIORITIES - 1), /**< Deferred ISR (highest possible) */
} FuriThreadPriority;

/** FuriThread anonymous structure */
typedef struct FuriThread FuriThread;

/** FuriThreadId proxy type to OS low level functions */
typedef void* FuriThreadId;

/** FuriThreadCallback Your callback to run in new thread
 * @warning    never use osThreadExit in FuriThread
 */
typedef int32_t (*FuriThreadCallback)(void* context);

/** Write to stdout callback
 * @param      data     pointer to data
 * @param      size     data size @warning your handler must consume everything
 */
typedef void (*FuriThreadStdoutWriteCallback)(const char* data, size_t size);

/** FuriThread state change callback called upon thread state change
 * @param      state    new thread state
 * @param      context  callback context
 */
typedef void (*FuriThreadStateCallback)(FuriThreadState state, void* context);

/** Allocate FuriThread
 *
 * @return     FuriThread instance
 */
FuriThread* furi_thread_alloc();

/** Allocate FuriThread, shortcut version
 * 
 * @param name 
 * @param stack_size 
 * @param callback 
 * @param context 
 * @return FuriThread* 
 */
FuriThread* furi_thread_alloc_ex(
    const char* name,
    uint32_t stack_size,
    FuriThreadCallback callback,
    void* context);

/** Release FuriThread
 *
 * @warning    see furi_thread_join
 *
 * @param      thread  FuriThread instance
 */
void furi_thread_free(FuriThread* thread);

/** Set FuriThread name
 *
 * @param      thread  FuriThread instance
 * @param      name    string
 */
void furi_thread_set_name(FuriThread* thread, const char* name);

/**
 * @brief Set FuriThread appid
 * Technically, it is like a "process id", but it is not a system-wide unique identifier.
 * All threads spawned by the same app will have the same appid.
 * 
 * @param thread 
 * @param appid 
 */
void furi_thread_set_appid(FuriThread* thread, const char* appid);

/** Mark thread as service
 * The service cannot be stopped or removed, and cannot exit from the thread body
 * 
 * @param thread 
 */
void furi_thread_mark_as_service(FuriThread* thread);

/** Set FuriThread stack size
 *
 * @param      thread      FuriThread instance
 * @param      stack_size  stack size in bytes
 */
void furi_thread_set_stack_size(FuriThread* thread, size_t stack_size);

/** Set FuriThread callback
 *
 * @param      thread    FuriThread instance
 * @param      callback  FuriThreadCallback, called upon thread run
 */
void furi_thread_set_callback(FuriThread* thread, FuriThreadCallback callback);

/** Set FuriThread context
 *
 * @param      thread   FuriThread instance
 * @param      context  pointer to context for thread callback
 */
void furi_thread_set_context(FuriThread* thread, void* context);

/** Set FuriThread priority
 *
 * @param      thread   FuriThread instance
 * @param      priority FuriThreadPriority value
 */
void furi_thread_set_priority(FuriThread* thread, FuriThreadPriority priority);

/** Set current thread priority
 *
 * @param      priority FuriThreadPriority value
 */
void furi_thread_set_current_priority(FuriThreadPriority priority);

/** Get current thread priority
 *
 * @return     FuriThreadPriority value
 */
FuriThreadPriority furi_thread_get_current_priority();

/** Set FuriThread state change callback
 *
 * @param      thread    FuriThread instance
 * @param      callback  state change callback
 */
void furi_thread_set_state_callback(FuriThread* thread, FuriThreadStateCallback callback);

/** Set FuriThread state change context
 *
 * @param      thread   FuriThread instance
 * @param      context  pointer to context
 */
void furi_thread_set_state_context(FuriThread* thread, void* context);

/** Get FuriThread state
 *
 * @param      thread  FuriThread instance
 *
 * @return     thread state from FuriThreadState
 */
FuriThreadState furi_thread_get_state(FuriThread* thread);

/** Start FuriThread
 *
 * @param      thread  FuriThread instance
 */
void furi_thread_start(FuriThread* thread);

/** Join FuriThread
 *
 * @warning    Use this method only when CPU is not busy(Idle task receives
 *             control), otherwise it will wait forever.
 *
 * @param      thread  FuriThread instance
 *
 * @return     bool
 */
bool furi_thread_join(FuriThread* thread);

/** Get FreeRTOS FuriThreadId for FuriThread instance
 *
 * @param      thread  FuriThread instance
 *
 * @return     FuriThreadId or NULL
 */
FuriThreadId furi_thread_get_id(FuriThread* thread);

/** Enable heap tracing
 *
 * @param      thread  FuriThread instance
 */
void furi_thread_enable_heap_trace(FuriThread* thread);

/** Disable heap tracing
 *
 * @param      thread  FuriThread instance
 */
void furi_thread_disable_heap_trace(FuriThread* thread);

/** Get thread heap size
 *
 * @param      thread  FuriThread instance
 *
 * @return     size in bytes
 */
size_t furi_thread_get_heap_size(FuriThread* thread);

/** Get thread return code
 *
 * @param      thread  FuriThread instance
 *
 * @return     return code
 */
int32_t furi_thread_get_return_code(FuriThread* thread);

/** Thread related methods that doesn't involve FuriThread directly */

/** Get FreeRTOS FuriThreadId for current thread
 *
 * @param      thread  FuriThread instance
 *
 * @return     FuriThreadId or NULL
 */
FuriThreadId furi_thread_get_current_id();

/** Get FuriThread instance for current thread
 * 
 * @return pointer to FuriThread or NULL if this thread doesn't belongs to Furi
 */
FuriThread* furi_thread_get_current();

/** Return control to scheduler */
void furi_thread_yield();

uint32_t furi_thread_flags_set(FuriThreadId thread_id, uint32_t flags);

uint32_t furi_thread_flags_clear(uint32_t flags);

uint32_t furi_thread_flags_get(void);

uint32_t furi_thread_flags_wait(uint32_t flags, uint32_t options, uint32_t timeout);

/**
 * @brief Enumerate threads
 * 
 * @param thread_array array of FuriThreadId, where thread ids will be stored
 * @param array_items array size
 * @return uint32_t threads count
 */
uint32_t furi_thread_enumerate(FuriThreadId* thread_array, uint32_t array_items);

/**
 * @brief Get thread name
 * 
 * @param thread_id 
 * @return const char* name or NULL
 */
const char* furi_thread_get_name(FuriThreadId thread_id);

/**
 * @brief Get thread appid
 * 
 * @param thread_id 
 * @return const char* appid
 */
const char* furi_thread_get_appid(FuriThreadId thread_id);

/**
 * @brief Get thread stack watermark
 * 
 * @param thread_id 
 * @return uint32_t 
 */
uint32_t furi_thread_get_stack_space(FuriThreadId thread_id);

/** Get STDOUT callback for thead
 *
 * @return STDOUT callback
 */
FuriThreadStdoutWriteCallback furi_thread_get_stdout_callback();

/** Set STDOUT callback for thread
 *
 * @param      callback  callback or NULL to clear
 */
void furi_thread_set_stdout_callback(FuriThreadStdoutWriteCallback callback);

/** Write data to buffered STDOUT
 * 
 * @param data input data
 * @param size input data size
 * 
 * @return size_t written data size
 */
size_t furi_thread_stdout_write(const char* data, size_t size);

/** Flush data to STDOUT
 * 
 * @return int32_t error code
 */
int32_t furi_thread_stdout_flush();

/** Suspend thread
 * 
 * @param thread_id thread id
 */
void furi_thread_suspend(FuriThreadId thread_id);

/** Resume thread
 * 
 * @param thread_id thread id
 */
void furi_thread_resume(FuriThreadId thread_id);

/** Get thread suspended state
 * 
 * @param thread_id thread id
 * @return true if thread is suspended
 */
bool furi_thread_is_suspended(FuriThreadId thread_id);

#ifdef __cplusplus
}
#endif
