#pragma once

#include <stdint.h>
#include <input/input.h>

typedef enum {
    GuiMessageTypeRedraw = 0x00,
    GuiMessageTypeInput = 0x01,
} GuiMessageType;

typedef struct {
    GuiMessageType type;
    InputEvent input;
    void* data;
} GuiMessage;

typedef struct GuiEvent GuiEvent;

GuiEvent* gui_event_alloc();

void gui_event_free(GuiEvent* gui_event);

void gui_event_lock(GuiEvent* gui_event);

void gui_event_unlock(GuiEvent* gui_event);

void gui_event_messsage_send(GuiEvent* gui_event, GuiMessage* message);

GuiMessage gui_event_message_next(GuiEvent* gui_event);
