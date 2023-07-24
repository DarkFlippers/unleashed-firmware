#pragma once

#include <inttypes.h>
#include <stdbool.h>

typedef struct IdleTimeoutContext IdleTimeoutContext;

typedef bool (*IDLE_TIMEOUT_CALLBACK)(void* context);

/**
 * @brief Initializes a new instance of IDLE timeout
 * @param timeout_sec IDLE timeout in seconds
 * @param on_idle_callback callback function to trigger when IDLE timeout happened
 * @param on_idle_callback_context callback function context
 * @return IDLE timeout context
 */
IdleTimeoutContext* idle_timeout_alloc(
    uint16_t timeout_sec,
    IDLE_TIMEOUT_CALLBACK on_idle_callback,
    void* on_idle_callback_context);

/**
 * @brief Starts IDLE timeout
 * @param context IDLE timeout context
 */
void idle_timeout_start(IdleTimeoutContext* context);

/**
 * @brief Stops IDLE timeout
 * @param context IDLE timeout context
 */
void idle_timeout_stop(IdleTimeoutContext* context);

/**
 * @brief Reports activity to IDLE timeout
 * @param context IDLE timeout context
 */
void idle_timeout_report_activity(IdleTimeoutContext* context);

/**
 * @brief Disposes IDLE timeout and releases all the resources
 * @param context IDLE timeout context
 */
void idle_timeout_free(IdleTimeoutContext* context);