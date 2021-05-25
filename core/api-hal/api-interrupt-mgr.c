#include "api-interrupt-mgr.h"
#include <cmsis_os2.h>
#include <furi.h>

static volatile InterruptCallbackItem callback_list[InterruptTypeLast];

bool api_interrupt_init() {
    for(uint8_t i = 0; i < InterruptTypeLast; i++) {
        callback_list[i].callback = NULL;
        callback_list[i].context = NULL;
        callback_list[i].ready = false;
    }

    return true;
}

void api_interrupt_add(InterruptCallback callback, InterruptType type, void* context) {
    furi_assert(type < InterruptTypeLast);
    furi_check(callback_list[type].callback == NULL);

    callback_list[type].callback = callback;
    callback_list[type].context = context;
    __DMB();
    callback_list[type].ready = true;
}

void api_interrupt_remove(InterruptCallback callback, InterruptType type) {
    furi_assert(type < InterruptTypeLast);
    if(callback_list[type].callback != NULL) {
        furi_check(callback_list[type].callback == callback);
    }

    callback_list[type].ready = false;
    __DMB();
    callback_list[type].callback = NULL;
    callback_list[type].context = NULL;
}

void api_interrupt_enable(InterruptCallback callback, InterruptType type) {
    furi_assert(type < InterruptTypeLast);
    furi_check(callback_list[type].callback == callback);

    callback_list[type].ready = true;
    __DMB();
}

void api_interrupt_disable(InterruptCallback callback, InterruptType type) {
    furi_assert(type < InterruptTypeLast);
    furi_check(callback_list[type].callback == callback);

    callback_list[type].ready = false;
    __DMB();
}

void api_interrupt_call(InterruptType type, void* hw) {
    // that executed in interrupt ctx so mutex don't needed
    // but we need to check ready flag
    furi_assert(type < InterruptTypeLast);

    if(callback_list[type].callback != NULL) {
        if(callback_list[type].ready) {
            callback_list[type].callback(hw, callback_list[type].context);
        }
    }
}
