#include "api-interrupt-mgr.h"
#include "mlib/m-dict.h"

#include <m-list.h>
#include <cmsis_os2.h>

LIST_DEF(list_interrupt, InterruptCallbackItem, M_POD_OPLIST);
DICT_DEF2(dict_interrupt, uint32_t, M_DEFAULT_OPLIST, list_interrupt_t, M_A1_OPLIST);

dict_interrupt_t interrupts_dict;
osMutexId_t interrupt_mutex;

bool api_interrupt_init() {
    dict_interrupt_init(interrupts_dict);
    interrupt_mutex = osMutexNew(NULL);
    return (interrupt_mutex != NULL);
}

void api_interrupt_add(InterruptCallback callback, InterruptType type, void* context) {
    if(osMutexAcquire(interrupt_mutex, osWaitForever) == osOK) {
        list_interrupt_t* list = dict_interrupt_get(interrupts_dict, (uint32_t)type);

        if(list == NULL) {
            list_interrupt_t new_list;
            list_interrupt_init(new_list);
            dict_interrupt_set_at(interrupts_dict, (uint32_t)type, new_list);
            list = dict_interrupt_get(interrupts_dict, (uint32_t)type);
        }

        // put uninitialized item to the list
        // M_POD_OPLIST provide memset(&(a), 0, sizeof (a)) constructor
        // so item will not be ready until we set ready flag
        InterruptCallbackItem* item = list_interrupt_push_new(*list);

        // initialize item
        item->callback = callback;
        item->type = type;
        item->context = context;
        item->ready = true;

        // TODO remove on app exit
        //flapp_on_exit(api_interrupt_remove, callback);

        osMutexRelease(interrupt_mutex);
    }
}

void api_interrupt_remove(InterruptCallback callback, InterruptType type) {
    if(osMutexAcquire(interrupt_mutex, osWaitForever) == osOK) {
        list_interrupt_t* list = dict_interrupt_get(interrupts_dict, (uint32_t)type);

        if(list != NULL) {
            // iterate over items
            list_interrupt_it_t it;
            list_interrupt_it(it, *list);
            while(!list_interrupt_end_p(it)) {
                if(it->current->data.callback == callback) {
                    list_interrupt_remove(*list, it);
                } else {
                    list_interrupt_next(it);
                }
            }
        }

        osMutexRelease(interrupt_mutex);
    }
}

void api_interrupt_enable(InterruptCallback callback, InterruptType type) {
    if(osMutexAcquire(interrupt_mutex, osWaitForever) == osOK) {
        list_interrupt_t* list = dict_interrupt_get(interrupts_dict, (uint32_t)type);

        if(list != NULL) {
            // iterate over items
            list_interrupt_it_t it;
            for(list_interrupt_it(it, *list); !list_interrupt_end_p(it); list_interrupt_next(it)) {
                // if the iterator is equal to our element
                if(it->current->data.callback == callback) {
                    it->current->data.ready = true;
                }
            }
        }

        osMutexRelease(interrupt_mutex);
    }
}

void api_interrupt_disable(InterruptCallback callback, InterruptType type) {
    if(osMutexAcquire(interrupt_mutex, osWaitForever) == osOK) {
        list_interrupt_t* list = dict_interrupt_get(interrupts_dict, (uint32_t)type);

        if(list != NULL) {
            // iterate over items
            list_interrupt_it_t it;
            for(list_interrupt_it(it, *list); !list_interrupt_end_p(it); list_interrupt_next(it)) {
                // if the iterator is equal to our element
                if(it->current->data.callback == callback) {
                    it->current->data.ready = false;
                }
            }
        }

        osMutexRelease(interrupt_mutex);
    }
}

void api_interrupt_call(InterruptType type, void* hw) {
    // that executed in interrupt ctx so mutex don't needed
    // but we need to check ready flag

    list_interrupt_t* list = dict_interrupt_get(interrupts_dict, (uint32_t)type);

    if(list != NULL) {
        // iterate over items
        list_interrupt_it_t it;
        for(list_interrupt_it(it, *list); !list_interrupt_end_p(it); list_interrupt_next(it)) {
            // if the iterator is equal to our element
            if(it->current->data.ready) {
                it->current->data.callback(hw, it->current->data.context);
            }
        }
    }
}
