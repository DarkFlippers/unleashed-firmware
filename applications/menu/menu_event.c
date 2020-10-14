#include "menu_event.h"

#include <cmsis_os.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <flipper.h>

#define MENU_MESSAGE_MQUEUE_SIZE 8

struct MenuEvent {
    osMessageQueueId_t mqueue;
    osTimerId_t timeout_timer;
    osMutexId_t lock_mutex;
};

void MenuEventimeout_callback(void* arg) {
    MenuEvent* menu_event = arg;
    MenuMessage message;
    message.type = MenuMessageTypeIdle;
    osMessageQueuePut(menu_event->mqueue, &message, 0, 0);
}

MenuEvent* menu_event_alloc() {
    MenuEvent* menu_event = furi_alloc(sizeof(MenuEvent));

    menu_event->mqueue = osMessageQueueNew(MENU_MESSAGE_MQUEUE_SIZE, sizeof(MenuMessage), NULL);
    assert(menu_event->mqueue);

    menu_event->timeout_timer =
        osTimerNew(MenuEventimeout_callback, osTimerOnce, menu_event, NULL);
    assert(menu_event->timeout_timer);

    menu_event->lock_mutex = osMutexNew(NULL);
    assert(menu_event->lock_mutex);

    menu_event_lock(menu_event);

    return menu_event;
}

void menu_event_free(MenuEvent* menu_event) {
    assert(menu_event);
    menu_event_unlock(menu_event);
    assert(osMessageQueueDelete(menu_event->mqueue) == osOK);
    free(menu_event);
}

void menu_event_lock(MenuEvent* menu_event) {
    assert(osMutexAcquire(menu_event->lock_mutex, osWaitForever) == osOK);
}

void menu_event_unlock(MenuEvent* menu_event) {
    assert(osMutexRelease(menu_event->lock_mutex) == osOK);
}

void menu_event_activity_notify(MenuEvent* menu_event) {
    assert(menu_event);
    osTimerStart(menu_event->timeout_timer, 60000U); // 1m timeout, return to main
}

MenuMessage menu_event_next(MenuEvent* menu_event) {
    assert(menu_event);
    MenuMessage message;
    menu_event_unlock(menu_event);
    while(osMessageQueueGet(menu_event->mqueue, &message, NULL, osWaitForever) != osOK) {
    };
    menu_event_lock(menu_event);
    return message;
}

void menu_event_input_callback(InputEvent* input_event, void* context) {
    MenuEvent* menu_event = context;
    MenuMessage message;

    if(!input_event->state) return;

    if(input_event->input == InputUp) {
        message.type = MenuMessageTypeUp;
    } else if(input_event->input == InputDown) {
        message.type = MenuMessageTypeDown;
    } else if(input_event->input == InputRight) {
        message.type = MenuMessageTypeRight;
    } else if(input_event->input == InputLeft) {
        message.type = MenuMessageTypeLeft;
    } else if(input_event->input == InputOk) {
        message.type = MenuMessageTypeOk;
    } else if(input_event->input == InputBack) {
        message.type = MenuMessageTypeBack;
    } else {
        message.type = MenuMessageTypeUnknown;
    }

    osMessageQueuePut(menu_event->mqueue, &message, 0, 0);
}
