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

typedef struct GuiEvent GuiEvent;

GuiEvent* gui_event_alloc();

void gui_event_free(GuiEvent* gui_event);

void gui_event_lock(GuiEvent* gui_event);

void gui_event_unlock(GuiEvent* gui_event);

void gui_event_messsage_send(GuiEvent* gui_event, GUIMessage* message);

GUIMessage gui_event_message_next(GuiEvent* gui_event);
