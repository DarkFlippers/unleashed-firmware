/**
 * @file view_dispatcher_i.h
 * GUI: ViewDispatcher API
 */

#pragma once

#include <m-dict.h>

#include "view_dispatcher.h"
#include "view_i.h"
#include "gui_i.h"

DICT_DEF2(ViewDict, uint32_t, M_DEFAULT_OPLIST, View*, M_PTR_OPLIST)

struct ViewDispatcher {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* event_queue;

    Gui* gui;
    ViewPort* view_port;
    ViewDict_t views;

    View* current_view;

    View* ongoing_input_view;
    uint8_t ongoing_input;

    ViewDispatcherCustomEventCallback custom_event_callback;
    ViewDispatcherNavigationEventCallback navigation_event_callback;
    ViewDispatcherTickEventCallback tick_event_callback;
    uint32_t tick_period;
    void* event_context;
};

/** ViewPort Draw Callback */
void view_dispatcher_draw_callback(Canvas* canvas, void* context);

/** ViewPort Input Callback */
void view_dispatcher_input_callback(InputEvent* event, void* context);

/** Input handler */
void view_dispatcher_handle_input(ViewDispatcher* view_dispatcher, InputEvent* event);

/** Tick handler */
void view_dispatcher_handle_tick_event(void* context);

/** Custom event handler */
void view_dispatcher_handle_custom_event(ViewDispatcher* view_dispatcher, uint32_t event);

/** Set current view, dispatches view enter and exit */
void view_dispatcher_set_current_view(ViewDispatcher* view_dispatcher, View* view);

/** ViewDispatcher update event */
void view_dispatcher_update(View* view, void* context);

/** ViewDispatcher run event loop event callback */
bool view_dispatcher_run_event_callback(FuriMessageQueue* queue, void* context);

/** ViewDispatcher run event loop input callback */
bool view_dispatcher_run_input_callback(FuriMessageQueue* queue, void* context);
