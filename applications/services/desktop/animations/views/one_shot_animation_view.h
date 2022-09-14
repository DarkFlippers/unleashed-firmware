#pragma once

#include <furi.h>
#include <gui/view.h>
#include <stdint.h>

typedef void (*OneShotInteractCallback)(void*);
typedef struct OneShotView OneShotView;

OneShotView* one_shot_view_alloc(void);
void one_shot_view_free(OneShotView* view);
void one_shot_view_set_interact_callback(
    OneShotView* view,
    OneShotInteractCallback callback,
    void* context);
void one_shot_view_start_animation(OneShotView* view, const Icon* icon);
View* one_shot_view_get_view(OneShotView* view);
