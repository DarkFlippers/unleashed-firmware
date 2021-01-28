#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*InterruptCallback)(void*, void*);

typedef enum {
    InterruptTypeComparatorTrigger,
    InterruptTypeTimerCapture,
    InterruptTypeTimerOutputCompare,
    InterruptTypeTimerUpdate,
    InterruptTypeExternalInterrupt,
} InterruptType;

typedef struct {
    InterruptCallback callback;
    InterruptType type;
    void* context;
    bool ready;
} InterruptCallbackItem;

bool api_interrupt_init();
void api_interrupt_add(InterruptCallback callback, InterruptType type, void* context);
void api_interrupt_remove(InterruptCallback callback, InterruptType type);
void api_interrupt_enable(InterruptCallback callback, InterruptType type);
void api_interrupt_disable(InterruptCallback callback, InterruptType type);
void api_interrupt_call(InterruptType type, void* hw);

#ifdef __cplusplus
}
#endif
