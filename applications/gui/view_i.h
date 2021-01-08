#pragma once

#include "view.h"
#include "view_dispatcher_i.h"
#include <flipper_v2.h>

typedef struct {
    void* data;
    osMutexId_t mutex;
} ViewModelLocking;

struct View {
    ViewDispatcher* dispatcher;
    ViewDrawCallback draw_callback;
    ViewInputCallback input_callback;
    ViewModelType model_type;
    ViewNavigationCallback previous_callback;
    ViewNavigationCallback next_callback;
    void* model;
    void* context;
};

/* Set View dispatcher */
void view_set_dispatcher(View* view, ViewDispatcher* view_dispatcher);

/* Draw Callback for View dispatcher */
void view_draw(View* view, Canvas* canvas);

/* Input Callback for View dispatcher */
bool view_input(View* view, InputEvent* event);

/* Previous Callback for View dispatcher */
uint32_t view_previous(View* view);

/* Next Callback for View dispatcher */
uint32_t view_next(View* view);
