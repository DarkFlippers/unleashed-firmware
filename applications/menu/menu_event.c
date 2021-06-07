#include "menu_event.h"

#include <string.h>
#include <stdlib.h>

#include <furi.h>

#define MENU_MESSAGE_MQUEUE_SIZE 8

struct MenuEvent {
    osMessageQueueId_t mqueue;
};

void MenuEventimeout_callback(void* arg) {
    MenuEvent* menu_event = arg;
    MenuMessage message;
    message.type = MenuMessageTypeIdle;
    osMessageQueuePut(menu_event->mqueue, &message, 0, osWaitForever);
}

MenuEvent* menu_event_alloc() {
    MenuEvent* menu_event = furi_alloc(sizeof(MenuEvent));
    menu_event->mqueue = osMessageQueueNew(MENU_MESSAGE_MQUEUE_SIZE, sizeof(MenuMessage), NULL);
    furi_check(menu_event->mqueue);
    return menu_event;
}

void menu_event_free(MenuEvent* menu_event) {
    furi_assert(menu_event);
    furi_check(osMessageQueueDelete(menu_event->mqueue) == osOK);
    free(menu_event);
}

void menu_event_activity_notify(MenuEvent* menu_event) {
    furi_assert(menu_event);
}

MenuMessage menu_event_next(MenuEvent* menu_event) {
    furi_assert(menu_event);
    MenuMessage message;
    while(osMessageQueueGet(menu_event->mqueue, &message, NULL, osWaitForever) != osOK) {
    };
    return message;
}

void menu_event_input_callback(InputEvent* input_event, void* context) {
    MenuEvent* menu_event = context;
    MenuMessage message;

    if(input_event->type != InputTypeShort) return;

    if(input_event->key == InputKeyUp) {
        message.type = MenuMessageTypeUp;
    } else if(input_event->key == InputKeyDown) {
        message.type = MenuMessageTypeDown;
    } else if(input_event->key == InputKeyOk) {
        message.type = MenuMessageTypeOk;
    } else if(input_event->key == InputKeyBack) {
        message.type = MenuMessageTypeBack;
    } else {
        message.type = MenuMessageTypeUnknown;
    }

    osMessageQueuePut(menu_event->mqueue, &message, 0, osWaitForever);
}