#include "api-interrupt-mgr.h"

LIST_DEF(list_interrupt, InterruptCallbackItem, M_POD_OPLIST);
list_interrupt_t interrupts;
osMutexId_t interrupt_list_mutex;

bool api_interrupt_init() {
    interrupt_list_mutex = osMutexNew(NULL);
    return (interrupt_list_mutex != NULL);
}

void api_interrupt_add(InterruptCallback callback, InterruptType type, void* context) {
    if(osMutexAcquire(interrupt_list_mutex, osWaitForever) == osOK) {
        // put uninitialized item to the list
        // M_POD_OPLIST provide memset(&(a), 0, sizeof (a)) constructor
        // so item will not be ready until we set ready flag
        InterruptCallbackItem* item = list_interrupt_push_new(interrupts);

        // initialize item
        item->callback = callback;
        item->type = type;
        item->context = context;
        item->ready = true;

        // TODO remove on app exit
        //flapp_on_exit(api_interrupt_remove, callback);

        osMutexRelease(interrupt_list_mutex);
    }
}

void api_interrupt_remove(InterruptCallback callback) {
    if(osMutexAcquire(interrupt_list_mutex, osWaitForever) == osOK) {
        // iterate over items
        list_interrupt_it_t it;
        for(list_interrupt_it(it, interrupts); !list_interrupt_end_p(it);
            list_interrupt_next(it)) {
            const InterruptCallbackItem* item = list_interrupt_cref(it);

            // if the iterator is equal to our element
            if(item->callback == callback) {
                list_interrupt_remove(interrupts, it);
                break;
            }
        }

        osMutexRelease(interrupt_list_mutex);
    }
}

void api_interrupt_call(InterruptType type, void* hw) {
    // that executed in interrupt ctx so mutex don't needed
    // but we need to check ready flag

    // iterate over items
    list_interrupt_it_t it;
    for(list_interrupt_it(it, interrupts); !list_interrupt_end_p(it); list_interrupt_next(it)) {
        const InterruptCallbackItem* item = list_interrupt_cref(it);

        // if the iterator is equal to our element
        if(item->type == type && item->ready) {
            item->callback(hw, item->context);
        }
    }
}