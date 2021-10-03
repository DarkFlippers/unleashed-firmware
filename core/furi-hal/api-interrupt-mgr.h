/**
 * @file api-interrupt-mgr.h
 * Furi: interrupt API
 */

#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Interrupt callback prototype */
typedef void (*InterruptCallback)(void*, void*);

/** Interupt type */
typedef enum {
    InterruptTypeComparatorTrigger,
    InterruptTypeTimerUpdate,
    InterruptTypeLast,
} InterruptType;

/** Interrupt callback type */
typedef struct {
    InterruptCallback callback;
    void* context;
    bool ready;
} InterruptCallbackItem;

/** Init interrupt
 *
 * @return     true on succsessful initialization, false otherwise
 */
bool api_interrupt_init();

/** Add interrupt
 *
 * @param      callback  InterruptCallback
 * @param      type      InterruptType
 * @param      context   context for callback
 */
void api_interrupt_add(InterruptCallback callback, InterruptType type, void* context);

/** Remove interrupt
 *
 * @param      callback  InterruptCallback
 * @param      type      InterruptType
 */
void api_interrupt_remove(InterruptCallback callback, InterruptType type);

/** Enable interrupt
 *
 * @param      callback  InterruptCallback
 * @param      type      InterruptType
 */
void api_interrupt_enable(InterruptCallback callback, InterruptType type);

/** Disable interrupt
 *
 * @param      callback  InterruptCallback
 * @param      type      InterruptType
 */
void api_interrupt_disable(InterruptCallback callback, InterruptType type);

/** Call interrupt
 *
 * @param      type  InterruptType
 * @param      hw    pointer to hardware peripheral
 */
void api_interrupt_call(InterruptType type, void* hw);

#ifdef __cplusplus
}
#endif
