/**
 * @file thread.h
 * @brief Furi: Furi Thread API
 */

#pragma once

#include "base.h"
#include "common_defines.h"

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration of possible FuriThread states.
 *
 * Many of the FuriThread functions MUST ONLY be called when the thread is STOPPED.
 */
typedef enum {
    FuriThreadStateStopped, /**< Thread is stopped */
    FuriThreadStateStarting, /**< Thread is starting */
    FuriThreadStateRunning, /**< Thread is running */
} FuriThreadState;

/**
 * @brief Enumeration of possible FuriThread priorities.
 */
typedef enum {
    FuriThreadPriorityNone = 0, /**< Uninitialized, choose system default */
    FuriThreadPriorityIdle = 1, /**< Idle priority */
    FuriThreadPriorityLowest = 14, /**< Lowest */
    FuriThreadPriorityLow = 15, /**< Low */
    FuriThreadPriorityNormal = 16, /**< Normal */
    FuriThreadPriorityHigh = 17, /**< High */
    FuriThreadPriorityHighest = 18, /**< Highest */
    FuriThreadPriorityIsr =
        (FURI_CONFIG_THREAD_MAX_PRIORITIES - 1), /**< Deferred ISR (highest possible) */
} FuriThreadPriority;

/**
 * @brief FuriThread opaque type.
 */
typedef struct FuriThread FuriThread;

/**
 * @brief Unique thread identifier type (used by the OS kernel).
 */
typedef void* FuriThreadId;

/**
 * @brief Thread callback function pointer type.
 *
 * The function to be used as a thread callback MUST follow this signature.
 *
 * @param[in,out] context pointer to a user-specified object
 * @return value to be used as the thread return code
 */
typedef int32_t (*FuriThreadCallback)(void* context);

/**
 * @brief Standard output callback function pointer type.
 *
 * The function to be used as a standard output callback MUST follow this signature.
 *
 * @warning The handler MUST process ALL of the provided data before returning.
 *
 * @param[in] data pointer to the data to be written to the standard out
 * @param[in] size size of the data in bytes
 */
typedef void (*FuriThreadStdoutWriteCallback)(const char* data, size_t size);

/**
 * @brief State change callback function pointer type.
 *
 * The function to be used as a state callback MUST follow this signature.
 *
 * @param[in] state identifier of the state the thread has transitioned to
 * @param[in,out] context pointer to a user-specified object
 */
typedef void (*FuriThreadStateCallback)(FuriThreadState state, void* context);

/**
 * @brief Create a FuriThread instance.
 *
 * @return pointer to the created FuriThread instance
 */
FuriThread* furi_thread_alloc(void);

/**
 * @brief Create a FuriThread instance (service mode).
 *
 * Service threads are more memory efficient, but have
 * the following limitations:
 *
 * - Cannot return from the callback
 * - Cannot be joined or freed
 * - Stack size cannot be altered
 *
 * @param[in] name human-readable thread name (can be NULL)
 * @param[in] stack_size stack size in bytes (cannot be changed later)
 * @param[in] callback pointer to a function to be executed in this thread
 * @param[in] context pointer to a user-specified object (will be passed to the callback)
 * @return pointer to the created FuriThread instance
 */
FuriThread* furi_thread_alloc_service(
    const char* name,
    uint32_t stack_size,
    FuriThreadCallback callback,
    void* context);

/**
 * @brief Create a FuriThread instance w/ extra parameters.
 * 
 * @param[in] name human-readable thread name (can be NULL)
 * @param[in] stack_size stack size in bytes (can be changed later)
 * @param[in] callback pointer to a function to be executed in this thread
 * @param[in] context pointer to a user-specified object (will be passed to the callback)
 * @return pointer to the created FuriThread instance
 */
FuriThread* furi_thread_alloc_ex(
    const char* name,
    uint32_t stack_size,
    FuriThreadCallback callback,
    void* context);

/**
 * @brief Delete a FuriThread instance.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @warning see furi_thread_join for caveats on stopping a thread.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be deleted
 */
void furi_thread_free(FuriThread* thread);

/**
 * @brief Set the name of a FuriThread instance.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 * @param[in] name human-readable thread name (can be NULL)
 */
void furi_thread_set_name(FuriThread* thread, const char* name);

/**
 * @brief Set the application ID of a FuriThread instance.
 *
 * The thread MUST be stopped when calling this function.
 *
 * Technically, it is like a "process id", but it is not a system-wide unique identifier.
 * All threads spawned by the same app will have the same appid.
 * 
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 * @param[in] appid thread application ID (can be NULL)
 */
void furi_thread_set_appid(FuriThread* thread, const char* appid);

/**
 * @brief Set the stack size of a FuriThread instance.
 *
 * The thread MUST be stopped when calling this function. Additionally, it is NOT possible
 * to change the stack size of a service thread under any circumstances.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 * @param[in] stack_size stack size in bytes
 */
void furi_thread_set_stack_size(FuriThread* thread, size_t stack_size);

/**
 * @brief Set the user callback function to be executed in a FuriThread.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 * @param[in] callback pointer to a user-specified function to be executed in this thread
 */
void furi_thread_set_callback(FuriThread* thread, FuriThreadCallback callback);

/**
 * @brief Set the callback function context.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 * @param[in] context pointer to a user-specified object (will be passed to the callback, can be NULL)
 */
void furi_thread_set_context(FuriThread* thread, void* context);

/**
 * @brief Set the priority of a FuriThread.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 * @param[in] priority priority level value
 */
void furi_thread_set_priority(FuriThread* thread, FuriThreadPriority priority);

/**
 * @brief Get the priority of a FuriThread.
 *
 * @param[in] thread pointer to the FuriThread instance to be queried
 * @return priority level value
 */
FuriThreadPriority furi_thread_get_priority(FuriThread* thread);

/**
 * @brief Set the priority of the current FuriThread.
 *
 * @param priority priority level value
 */
void furi_thread_set_current_priority(FuriThreadPriority priority);

/**
 * @brief Get the priority of the current FuriThread.
 *
 * @return priority level value
 */
FuriThreadPriority furi_thread_get_current_priority(void);

/**
 * Set the callback function to be executed upon a state thransition of a FuriThread.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 * @param[in] callback pointer to a user-specified callback function
 */
void furi_thread_set_state_callback(FuriThread* thread, FuriThreadStateCallback callback);

/**
 * @brief Set the state change callback context.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 * @param[in] context pointer to a user-specified object (will be passed to the callback, can be NULL)
 */
void furi_thread_set_state_context(FuriThread* thread, void* context);

/**
 * @brief Get the state of a FuriThread isntance.
 *
 * @param[in] thread pointer to the FuriThread instance to be queried
 * @return thread state value
 */
FuriThreadState furi_thread_get_state(FuriThread* thread);

/**
 * @brief Start a FuriThread instance.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be started
 */
void furi_thread_start(FuriThread* thread);

/**
 * @brief Wait for a FuriThread to exit.
 *
 * The thread callback function must return in order for the FuriThread instance to become joinable.
 *
 * @warning Use this method only when the CPU is not busy (i.e. when the
 *          Idle task receives control), otherwise it will wait forever.
 *
 * @param[in] thread pointer to the FuriThread instance to be joined
 * @return always true
 */
bool furi_thread_join(FuriThread* thread);

/**
 * @brief Get the unique identifier of a FuriThread instance.
 *
 * @param[in] thread pointer to the FuriThread instance to be queried
 * @return unique identifier value or NULL if thread is not running
 */
FuriThreadId furi_thread_get_id(FuriThread* thread);

/**
 * @brief Enable heap usage tracing for a FuriThread.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 */
void furi_thread_enable_heap_trace(FuriThread* thread);

/**
 * @brief Disable heap usage tracing for a FuriThread.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @param[in,out] thread pointer to the FuriThread instance to be modified
 */
void furi_thread_disable_heap_trace(FuriThread* thread);

/**
 * @brief Get heap usage by a FuriThread instance.
 *
 * The heap trace MUST be enabled before callgin this function.
 *
 * @param[in] thread pointer to the FuriThread instance to be queried
 * @return heap usage in bytes
 */
size_t furi_thread_get_heap_size(FuriThread* thread);

/**
 * @brief Get the return code of a FuriThread instance.
 *
 * This value is equal to the return value of the thread callback function.
 *
 * The thread MUST be stopped when calling this function.
 *
 * @param[in] thread pointer to the FuriThread instance to be queried
 * @return return code value
 */
int32_t furi_thread_get_return_code(FuriThread* thread);

/**
 * @brief Get the unique identifier of the current FuriThread.
 *
 * @return unique identifier value
 */
FuriThreadId furi_thread_get_current_id(void);

/**
 * @brief Get the FuriThread instance associated with the current thread.
 * 
 * @return pointer to a FuriThread instance or NULL if this thread does not belong to Furi
 */
FuriThread* furi_thread_get_current(void);

/**
 * @brief Return control to the scheduler.
 */
void furi_thread_yield(void);

/**
 * @brief Set the thread flags of a FuriThread.
 *
 * Can be used as a simple inter-thread communication mechanism.
 *
 * @param[in] thread_id unique identifier of the thread to be notified
 * @param[in] flags bitmask of thread flags to set
 * @return bitmask combination of previous and newly set flags
 */
uint32_t furi_thread_flags_set(FuriThreadId thread_id, uint32_t flags);

/**
 * @brief Clear the thread flags of the current FuriThread.
 *
 * @param[in] flags bitmask of thread flags to clear
 * @return bitmask of thread flags before clearing
 */
uint32_t furi_thread_flags_clear(uint32_t flags);

/**
 * @brief Get the thread flags of the current FuriThread.
 * @return current bitmask of thread flags
 */
uint32_t furi_thread_flags_get(void);

/**
 * @brief Wait for some thread flags to be set.
 *
 * @see FuriFlag for option and error flags.
 *
 * @param[in] flags bitmask of thread flags to wait for
 * @param[in] options combination of option flags determining the behavior of the function
 * @param[in] timeout maximum time to wait in milliseconds (use FuriWaitForever to wait forever)
 * @return bitmask combination of received thread and error flags
 */
uint32_t furi_thread_flags_wait(uint32_t flags, uint32_t options, uint32_t timeout);

/**
 * @brief Enumerate all threads.
 * 
 * @param[out] thread_array pointer to the output array (must be properly allocated)
 * @param[in] array_item_count output array capacity in elements (NOT bytes)
 * @return total thread count (array_item_count or less)
 */
uint32_t furi_thread_enumerate(FuriThreadId* thread_array, uint32_t array_item_count);

/**
 * @brief Get the name of a thread based on its unique identifier.
 * 
 * @param[in] thread_id unique identifier of the thread to be queried
 * @return pointer to a zero-terminated string or NULL
 */
const char* furi_thread_get_name(FuriThreadId thread_id);

/**
 * @brief Get the application id of a thread based on its unique identifier.
 * 
 * @param[in] thread_id unique identifier of the thread to be queried
 * @return pointer to a zero-terminated string
 */
const char* furi_thread_get_appid(FuriThreadId thread_id);

/**
 * @brief Get thread stack watermark.
 * 
 * @param[in] thread_id unique identifier of the thread to be queried
 * @return stack watermark value
 */
uint32_t furi_thread_get_stack_space(FuriThreadId thread_id);

/**
 * @brief Get the standard output callback for the current thead.
 *
 * @return pointer to the standard out callback function
 */
FuriThreadStdoutWriteCallback furi_thread_get_stdout_callback(void);

/** Set standard output callback for the current thread.
 *
 * @param[in] callback pointer to the callback function or NULL to clear
 */
void furi_thread_set_stdout_callback(FuriThreadStdoutWriteCallback callback);

/** Write data to buffered standard output.
 * 
 * @param[in] data pointer to the data to be written
 * @param[in] size data size in bytes
 * @return number of bytes that was actually written
 */
size_t furi_thread_stdout_write(const char* data, size_t size);

/**
 * @brief Flush buffered data to standard output.
 * 
 * @return error code value
 */
int32_t furi_thread_stdout_flush(void);

/**
 * @brief Suspend a thread.
 *
 * Suspended threads are no more receiving any of the processor time.
 * 
 * @param[in] thread_id unique identifier of the thread to be suspended
 */
void furi_thread_suspend(FuriThreadId thread_id);

/**
 * @brief Resume a thread.
 * 
 * @param[in] thread_id unique identifier of the thread to be resumed
 */
void furi_thread_resume(FuriThreadId thread_id);

/**
 * @brief Test if a thread is suspended.
 * 
 * @param[in] thread_id unique identifier of the thread to be queried
 * @return true if thread is suspended, false otherwise
 */
bool furi_thread_is_suspended(FuriThreadId thread_id);

#ifdef __cplusplus
}
#endif
