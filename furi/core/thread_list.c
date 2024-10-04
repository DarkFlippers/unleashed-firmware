#include "thread_list.h"
#include "check.h"

#include <furi_hal_interrupt.h>

#include <m-array.h>
#include <m-dict.h>

ARRAY_DEF(FuriThreadListItemArray, FuriThreadListItem*, M_PTR_OPLIST) // NOLINT

#define M_OPL_FuriThreadListItemArray_t() ARRAY_OPLIST(FuriThreadListItemArray, M_PTR_OPLIST)

DICT_DEF2(
    FuriThreadListItemDict,
    uint32_t,
    M_DEFAULT_OPLIST,
    FuriThreadListItem*,
    M_PTR_OPLIST) // NOLINT

#define M_OPL_FuriThreadListItemDict_t() \
    DICT_OPLIST(FuriThreadListItemDict, M_DEFAULT_OPLIST, M_PTR_OPLIST)

struct FuriThreadList {
    FuriThreadListItemArray_t items;
    FuriThreadListItemDict_t search;
    uint32_t runtime_previous;
    uint32_t runtime_current;
    uint32_t isr_previous;
    uint32_t isr_current;
};

FuriThreadList* furi_thread_list_alloc(void) {
    FuriThreadList* instance = malloc(sizeof(FuriThreadList));

    FuriThreadListItemArray_init(instance->items);
    FuriThreadListItemDict_init(instance->search);

    return instance;
}

void furi_thread_list_free(FuriThreadList* instance) {
    furi_check(instance);

    FuriThreadListItemArray_it_t it;
    FuriThreadListItemArray_it(it, instance->items);
    while(!FuriThreadListItemArray_end_p(it)) {
        FuriThreadListItem* item = *FuriThreadListItemArray_cref(it);
        free(item);
        FuriThreadListItemArray_next(it);
    }

    FuriThreadListItemDict_clear(instance->search);
    FuriThreadListItemArray_clear(instance->items);

    free(instance);
}

size_t furi_thread_list_size(FuriThreadList* instance) {
    furi_check(instance);
    return FuriThreadListItemArray_size(instance->items);
}

FuriThreadListItem* furi_thread_list_get_at(FuriThreadList* instance, size_t position) {
    furi_check(instance);
    furi_check(position < furi_thread_list_size(instance));

    return *FuriThreadListItemArray_get(instance->items, position);
}

FuriThreadListItem* furi_thread_list_get_or_insert(FuriThreadList* instance, FuriThread* thread) {
    furi_check(instance);

    FuriThreadListItem** item_ptr = FuriThreadListItemDict_get(instance->search, (uint32_t)thread);
    if(item_ptr) {
        return *item_ptr;
    }

    FuriThreadListItem* item = malloc(sizeof(FuriThreadListItem));

    FuriThreadListItemArray_push_back(instance->items, item);
    FuriThreadListItemDict_set_at(instance->search, (uint32_t)thread, item);

    return item;
}

void furi_thread_list_process(FuriThreadList* instance, uint32_t runtime, uint32_t tick) {
    furi_assert(instance);

    instance->runtime_previous = instance->runtime_current;
    instance->runtime_current = runtime;

    instance->isr_previous = instance->isr_current;
    instance->isr_current = furi_hal_interrupt_get_time_in_isr_total();

    const uint32_t runtime_counter = instance->runtime_current - instance->runtime_previous;

    FuriThreadListItemArray_it_t it;
    FuriThreadListItemArray_it(it, instance->items);
    while(!FuriThreadListItemArray_end_p(it)) {
        FuriThreadListItem* item = *FuriThreadListItemArray_cref(it);
        if(item->tick != tick) {
            FuriThreadListItemArray_remove(instance->items, it);
            (void)FuriThreadListItemDict_erase(instance->search, (uint32_t)item->thread);
            free(item);
        } else {
            uint32_t item_counter = item->counter_current - item->counter_previous;
            if(item_counter && item->counter_previous && item->counter_current) {
                item->cpu = (float)item_counter / (float)runtime_counter * 100.0f;
                if(item->cpu > 200.0f) item->cpu = 0.0f;
            } else {
                item->cpu = 0.0f;
            }

            FuriThreadListItemArray_next(it);
        }
    }
}

float furi_thread_list_get_isr_time(FuriThreadList* instance) {
    const uint32_t runtime_counter = instance->runtime_current - instance->runtime_previous;
    const uint32_t isr_counter = instance->isr_current - instance->isr_previous;

    return (float)isr_counter / (float)runtime_counter;
}
