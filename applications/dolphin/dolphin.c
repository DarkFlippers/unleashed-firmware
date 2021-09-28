#include "dolphin_i.h"
#include <furi.h>

bool dolphin_load(Dolphin* dolphin) {
    furi_assert(dolphin);
    return dolphin_state_load(dolphin->state);
}

void dolphin_save(Dolphin* dolphin) {
    furi_assert(dolphin);
    DolphinEvent event;
    event.type = DolphinEventTypeSave;
    furi_check(osMessageQueuePut(dolphin->event_queue, &event, 0, osWaitForever) == osOK);
}

void dolphin_deed(Dolphin* dolphin, DolphinDeed deed) {
    furi_assert(dolphin);
    DolphinEvent event;
    event.type = DolphinEventTypeDeed;
    event.deed = deed;
    furi_check(osMessageQueuePut(dolphin->event_queue, &event, 0, osWaitForever) == osOK);
}

DolphinDeedWeight dolphin_stats(Dolphin* dolphin) {
    DolphinDeedWeight stats;
    stats.butthurt = dolphin_state_get_butthurt(dolphin->state);
    stats.icounter = dolphin_state_get_icounter(dolphin->state);

    return stats;
}

Dolphin* dolphin_alloc() {
    Dolphin* dolphin = furi_alloc(sizeof(Dolphin));

    dolphin->state = dolphin_state_alloc();
    dolphin->event_queue = osMessageQueueNew(8, sizeof(DolphinEvent), NULL);

    return dolphin;
}

void dolphin_free(Dolphin* dolphin) {
    furi_assert(dolphin);

    dolphin_state_free(dolphin->state);
    osMessageQueueDelete(dolphin->event_queue);

    free(dolphin);
}

int32_t dolphin_srv(void* p) {
    Dolphin* dolphin = dolphin_alloc();
    furi_record_create("dolphin", dolphin);

    DolphinEvent event;
    while(1) {
        furi_check(osMessageQueueGet(dolphin->event_queue, &event, NULL, osWaitForever) == osOK);
        switch(event.type) {
        case DolphinEventTypeDeed:
            dolphin_state_on_deed(dolphin->state, event.deed);
            break;

        case DolphinEventTypeSave:
            dolphin_state_save(dolphin->state);
            break;

        default:
            break;
        }
    }

    dolphin_free(dolphin);
    return 0;
}
