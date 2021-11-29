#include "dolphin/dolphin.h"
#include "desktop/desktop.h"
#include "dolphin/helpers/dolphin_state.h"
#include "dolphin_i.h"
#include "furi/pubsub.h"
#include "sys/_stdint.h"
#include <furi.h>
#define DOLPHIN_TIMEGATE 86400 // one day
#define DOLPHIN_LOCK_EVENT_FLAG (0x1)

void dolphin_deed(Dolphin* dolphin, DolphinDeed deed) {
    furi_assert(dolphin);
    DolphinEvent event;
    event.type = DolphinEventTypeDeed;
    event.deed = deed;
    dolphin_event_send_async(dolphin, &event);
}

DolphinStats dolphin_stats(Dolphin* dolphin) {
    furi_assert(dolphin);

    DolphinStats stats;
    DolphinEvent event;

    event.type = DolphinEventTypeStats;
    event.stats = &stats;

    dolphin_event_send_wait(dolphin, &event);

    return stats;
}

void dolphin_flush(Dolphin* dolphin) {
    furi_assert(dolphin);

    DolphinEvent event;
    event.type = DolphinEventTypeFlush;

    dolphin_event_send_wait(dolphin, &event);
}

Dolphin* dolphin_alloc() {
    Dolphin* dolphin = furi_alloc(sizeof(Dolphin));

    dolphin->state = dolphin_state_alloc();
    dolphin->event_queue = osMessageQueueNew(8, sizeof(DolphinEvent), NULL);
    dolphin->pubsub = furi_pubsub_alloc();

    return dolphin;
}

void dolphin_free(Dolphin* dolphin) {
    furi_assert(dolphin);

    dolphin_state_free(dolphin->state);
    osMessageQueueDelete(dolphin->event_queue);

    free(dolphin);
}

void dolphin_event_send_async(Dolphin* dolphin, DolphinEvent* event) {
    furi_assert(dolphin);
    furi_assert(event);
    event->flag = NULL;
    furi_check(osMessageQueuePut(dolphin->event_queue, event, 0, osWaitForever) == osOK);
}

void dolphin_event_send_wait(Dolphin* dolphin, DolphinEvent* event) {
    furi_assert(dolphin);
    furi_assert(event);
    event->flag = osEventFlagsNew(NULL);
    furi_check(event->flag);
    furi_check(osMessageQueuePut(dolphin->event_queue, event, 0, osWaitForever) == osOK);
    furi_check(
        osEventFlagsWait(event->flag, DOLPHIN_LOCK_EVENT_FLAG, osFlagsWaitAny, osWaitForever) ==
        DOLPHIN_LOCK_EVENT_FLAG);
    furi_check(osEventFlagsDelete(event->flag) == osOK);
}

void dolphin_event_release(Dolphin* dolphin, DolphinEvent* event) {
    if(event->flag) {
        osEventFlagsSet(event->flag, DOLPHIN_LOCK_EVENT_FLAG);
    }
}

static void dolphin_check_butthurt(DolphinState* state) {
    furi_assert(state);
    float diff_time = difftime(state->data.timestamp, dolphin_state_timestamp());

    if((fabs(diff_time)) > DOLPHIN_TIMEGATE) {
        dolphin_state_butthurted(state);
    }
}

FuriPubSub* dolphin_get_pubsub(Dolphin* dolphin) {
    return dolphin->pubsub;
}

int32_t dolphin_srv(void* p) {
    Dolphin* dolphin = dolphin_alloc();
    furi_record_create("dolphin", dolphin);

    dolphin_state_load(dolphin->state);

    DolphinEvent event;
    while(1) {
        if(osMessageQueueGet(dolphin->event_queue, &event, NULL, 60000) == osOK) {
            if(event.type == DolphinEventTypeDeed) {
                if(dolphin_state_on_deed(dolphin->state, event.deed)) {
                    DolphinPubsubEvent event = DolphinPubsubEventUpdate;
                    furi_pubsub_publish(dolphin->pubsub, &event);
                }
            } else if(event.type == DolphinEventTypeStats) {
                event.stats->icounter = dolphin->state->data.icounter;
                event.stats->butthurt = dolphin->state->data.butthurt;
                event.stats->timestamp = dolphin->state->data.timestamp;
                event.stats->level = dolphin_get_level(dolphin->state->data.icounter);
                event.stats->level_up_is_pending =
                    !dolphin_state_xp_to_levelup(dolphin->state->data.icounter);
            } else if(event.type == DolphinEventTypeFlush) {
                dolphin_state_save(dolphin->state);
            }
            dolphin_event_release(dolphin, &event);
        } else {
            dolphin_check_butthurt(dolphin->state);
            dolphin_state_save(dolphin->state);
        }
    }

    dolphin_free(dolphin);

    return 0;
}

void dolphin_upgrade_level(Dolphin* dolphin) {
    dolphin_state_increase_level(dolphin->state);
    dolphin_flush(dolphin);
}
