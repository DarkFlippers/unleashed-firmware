#pragma once

#include "view_dispatcher.h"
#include "view_i.h"
#include <furi.h>
#include <m-dict.h>

DICT_DEF2(ViewDict, uint32_t, M_DEFAULT_OPLIST, View*, M_PTR_OPLIST)

struct ViewDispatcher {
    Gui* gui;
    Widget* widget;
    ViewDict_t views;
    View* current_view;
};

/* Widget Draw Callback */
void view_dispatcher_draw_callback(Canvas* canvas, void* context);

/* Widget Input Callback */
void view_dispatcher_input_callback(InputEvent* event, void* context);

/* View to ViewDispatcher update event */
void view_dispatcher_update(ViewDispatcher* view_dispatcher, View* view);
