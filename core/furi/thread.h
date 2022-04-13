/**
 * @file thread.h
 * Furi: Furi Thread API
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <cmsis_os2.h>

#ifdef __cplusplus
extern "C" {
#endif

/** FuriThreadState */
typedef enum {
    FuriThreadStateStopped,
    FuriThreadStateStarting,
    FuriThreadStateRunning,
} FuriThreadState;

/** FuriThread anonymous structure */
typedef struct FuriThread FuriThread;

/** FuriThreadCallback Your callback to run in new thread
 * @warning    never use osThreadExit in FuriThread
 */
typedef int32_t (*FuriThreadCallback)(void* context);

/** FuriThread state change calback called upon thread state change
 * @param      state    new thread state
 * @param      context  callback context
 */
typedef void (*FuriThreadStateCallback)(FuriThreadState state, void* context);

/** Allocate FuriThread
 *
 * @return     FuriThread instance
 */
FuriThread* furi_thread_alloc();

/** Release FuriThread
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
 *
 * @return     true on success
 */
bool furi_thread_start(FuriThread* thread);

/** Treminate FuriThread
 *
 * @param      thread  FuriThread instance
 *
 * @return     osStatus_t
 * @warning    terminating statefull thread is dangerous use only if you know
 *             what you doing
 */
osStatus_t furi_thread_terminate(FuriThread* thread);

/** Join FuriThread
 *
 * @param      thread  FuriThread instance
 *
 * @return     osStatus_t
 */
osStatus_t furi_thread_join(FuriThread* thread);

/** Get CMSIS Thread ID
 *
 * @param      thread  FuriThread instance
 *
 * @return     osThreadId_t or NULL
 */
osThreadId_t furi_thread_get_thread_id(FuriThread* thread);

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

#ifdef __cplusplus
}
#endif