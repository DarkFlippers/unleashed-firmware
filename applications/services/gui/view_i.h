/**
 * @file view_i.h
 * GUI: internal View API
 */

#pragma once

#include "view.h"
#include <furi.h>

typedef struct {
    void* data;
    FuriMutex* mutex;
} ViewModelLocking;

struct View {
    ViewDrawCallback draw_callback;
    ViewInputCallback input_callback;
    ViewCustomCallback custom_callback;

    ViewModelType model_type;
    ViewNavigationCallback previous_callback;
    ViewCallback enter_callback;
    ViewCallback exit_callback;
    ViewOrientation orientation;

    ViewUpdateCallback update_callback;
    void* update_callback_context;

    void* model;
    void* context;
};

/** IconAnimation tie callback */
void view_icon_animation_callback(IconAnimation* instance, void* context);

/** Unlock model */
void view_unlock_model(View* view);

/** Draw Callback for View dispatcher */
void view_draw(View* view, Canvas* canvas);

/** Input Callback for View dispatcher */
bool view_input(View* view, InputEvent* event);

/** Custom Callback for View dispatcher */
bool view_custom(View* view, uint32_t event);

/** Previous Callback for View dispatcher */
uint32_t view_previous(View* view);

/** Enter Callback for View dispatcher */
void view_enter(View* view);

/** Exit Callback for View dispatcher */
void view_exit(View* view);
