#pragma once
#include "callback-connector.h"
#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>

// simple app class with template variables <state, events>
template <class TState, class TEvent> class AppTemplate {
public:
    Widget* widget;
    osMessageQueueId_t event_queue;
    TState state;
    ValueMutex state_mutex;
    Gui* gui;

    AppTemplate();
    ~AppTemplate();
    void input_callback(InputEvent* input_event, void* ctx);
    void draw_callback(Canvas* canvas, void* ctx);
    virtual void render(Canvas* canvas) = 0;
    void acquire_state(void);
    void release_state(void);
    bool get_event(TEvent* event, uint32_t timeout);
    void app_ready(void);
    void exit(void);
    void update_gui(void);
};

template <class TState, class TEvent> AppTemplate<TState, TEvent>::AppTemplate() {
    // allocate events queue
    event_queue = osMessageQueueNew(10, sizeof(TEvent), NULL);

    // allocate valuemutex
    // TODO: use plain os mutex?
    if(!init_mutex(&state_mutex, &state, sizeof(TState))) {
        printf("cannot create mutex\n");
        furiac_exit();
    }

    // open gui
    gui = (Gui*)furi_record_open("gui");

    // allocate widget
    widget = widget_alloc();
}

template <class TState, class TEvent> AppTemplate<TState, TEvent>::~AppTemplate() {
}

// generic input callback
template <class TState, class TEvent>
void AppTemplate<TState, TEvent>::input_callback(InputEvent* input_event, void* ctx) {
    AppTemplate* app = static_cast<AppTemplate*>(ctx);

    TEvent event;
    event.type = TEvent::EventTypeKey;
    event.value.input = *input_event;
    osMessageQueuePut(app->event_queue, &event, 0, 0);
}

// generic draw callback
template <class TState, class TEvent>
void AppTemplate<TState, TEvent>::draw_callback(Canvas* canvas, void* ctx) {
    AppTemplate* app = static_cast<AppTemplate*>(ctx);
    app->acquire_state();

    canvas_clear(canvas);
    app->render(canvas);

    app->release_state();
}

template <class TState, class TEvent> void AppTemplate<TState, TEvent>::acquire_state(void) {
    acquire_mutex(&state_mutex, osWaitForever);
}

template <class TState, class TEvent> void AppTemplate<TState, TEvent>::release_state(void) {
    release_mutex(&state_mutex, &state);
}

template <class TState, class TEvent>
bool AppTemplate<TState, TEvent>::get_event(TEvent* event, uint32_t timeout) {
    osStatus_t event_status = osMessageQueueGet(event_queue, event, NULL, timeout);

    return (event_status == osOK);
}

// signal that app is ready, and we can render something
// also unblock dependent tasks
template <class TState, class TEvent> void AppTemplate<TState, TEvent>::app_ready(void) {
    // connect widget with input callback
    auto input_cb_ref = cbc::obtain_connector(this, &AppTemplate::input_callback);
    widget_input_callback_set(widget, input_cb_ref, this);

    // connect widget with draw callback
    auto draw_cb_ref = cbc::obtain_connector(this, &AppTemplate::draw_callback);
    widget_draw_callback_set(widget, draw_cb_ref, this);

    // add widget
    gui_add_widget(gui, widget, GuiLayerFullscreen);
}

template <class TState, class TEvent> void AppTemplate<TState, TEvent>::exit(void) {
    // TODO remove all widgets create by app
    widget_enabled_set(widget, false);
    osThreadExit();
}

template <class TState, class TEvent> void AppTemplate<TState, TEvent>::update_gui(void) {
    widget_update(widget);
}