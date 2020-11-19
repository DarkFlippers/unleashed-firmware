#pragma once
#include "callback-connector.h"
#include "flipper.h"
#include "flipper_v2.h"

// simple app class with template variables <state, events>
template <class TState, class TEvent> class AppTemplate {
public:
    Widget* widget;
    osMessageQueueId_t event_queue;
    TState state;
    ValueMutex state_mutex;
    GuiApi* gui;

    AppTemplate();
    ~AppTemplate();
    void input_callback(InputEvent* input_event, void* ctx);
    void draw_callback(CanvasApi* canvas, void* ctx);
    virtual void render(CanvasApi* canvas) = 0;
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
        furiac_exit(NULL);
    }

    // open gui
    gui = (GuiApi*)furi_open("gui");
    if(gui == NULL) {
        printf("gui is not available\n");
        furiac_exit(NULL);
    }

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
void AppTemplate<TState, TEvent>::draw_callback(CanvasApi* canvas, void* ctx) {
    AppTemplate* app = static_cast<AppTemplate*>(ctx);
    app->acquire_state();

    canvas->clear(canvas);
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
    gui->add_widget(gui, widget, GuiLayerFullscreen);

    // signal that our app ready to work
    furiac_ready();
}

template <class TState, class TEvent> void AppTemplate<TState, TEvent>::exit(void) {
    // TODO remove all widgets create by app
    widget_enabled_set(widget, false);
    furiac_exit(NULL);
}

template <class TState, class TEvent> void AppTemplate<TState, TEvent>::update_gui(void) {
    widget_update(widget);
}