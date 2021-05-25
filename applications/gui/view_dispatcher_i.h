#pragma once

#include <furi.h>
#include <m-dict.h>

#include "view_dispatcher.h"
#include "view_i.h"
#include "gui_i.h"

DICT_DEF2(ViewDict, uint32_t, M_DEFAULT_OPLIST, View*, M_PTR_OPLIST)

struct ViewDispatcher {
    osMessageQueueId_t queue;
    Gui* gui;
    ViewPort* view_port;
    ViewDict_t views;
    View* current_view;
};

typedef enum {
    ViewDispatcherMessageTypeInput,
    ViewDispatcherMessageTypeStop,
} ViewDispatcherMessageType;

typedef struct {
    ViewDispatcherMessageType type;
    union {
        InputEvent input;
    };
} ViewDispatcherMessage;

/* ViewPort Draw Callback */
void view_dispatcher_draw_callback(Canvas* canvas, void* context);

/* ViewPort Input Callback */
void view_dispatcher_input_callback(InputEvent* event, void* context);

/* Input handler */
void view_dispatcher_handle_input(ViewDispatcher* view_dispatcher, InputEvent* event);

/* Set current view, dispatches view enter and exit */
void view_dispatcher_set_current_view(ViewDispatcher* view_dispatcher, View* view);

/* ViewDispatcher update event */
void view_dispatcher_update(View* view, void* context);
