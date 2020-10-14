#pragma once

#include <stdint.h>
#include <input/input.h>

typedef enum {
    GUIMessageTypeRedraw = 0x00,
    GUIMessageTypeInput = 0x01,
} GUIMessageType;

typedef struct {
    GUIMessageType type;
    InputEvent input;
    void* data;
} GUIMessage;

typedef struct GUIEvent GUIEvent;

GUIEvent* gui_event_alloc();

void gui_event_free(GUIEvent* gui_event);

void gui_event_lock(GUIEvent* gui_event);

void gui_event_unlock(GUIEvent* gui_event);

void gui_event_messsage_send(GUIEvent* gui_event, GUIMessage* message);

GUIMessage gui_event_message_next(GUIEvent* gui_event);
