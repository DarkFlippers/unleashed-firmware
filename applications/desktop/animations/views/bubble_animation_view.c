
#include "cmsis_os2.h"
#include "../animation_manager.h"
#include "../animation_storage.h"
#include "furi_hal_delay.h"
#include "furi_hal_resources.h"
#include "furi/check.h"
#include "furi/memmgr.h"
#include "gui/canvas.h"
#include "gui/elements.h"
#include "gui/view.h"
#include "input/input.h"
#include <furi.h>
#include "portmacro.h"
#include <gui/icon.h>
#include <stdint.h>
#include <FreeRTOS.h>
#include <timers.h>
#include "bubble_animation_view.h"
#include <gui/icon_i.h>

#define ACTIVE_SHIFT 2

typedef struct {
    const BubbleAnimation* current;
    const FrameBubble* current_bubble;
    uint8_t current_frame;
    uint8_t active_cycle;
    uint8_t active_bubbles;
    uint8_t passive_bubbles;
    uint8_t active_shift;
    TickType_t active_ended_at;
    Icon* freeze_frame;
} BubbleAnimationViewModel;

struct BubbleAnimationView {
    View* view;
    osTimerId_t timer;
    BubbleAnimationInteractCallback interact_callback;
    void* interact_callback_context;
};

static void bubble_animation_activate(BubbleAnimationView* view, bool force);
static void bubble_animation_activate_right_now(BubbleAnimationView* view);

static uint8_t bubble_animation_get_icon_index(BubbleAnimationViewModel* model) {
    furi_assert(model);
    uint8_t icon_index = 0;
    const BubbleAnimation* animation = model->current;

    if(model->current_frame < animation->passive_frames) {
        icon_index = model->current_frame;
    } else {
        icon_index =
            (model->current_frame - animation->passive_frames) % animation->active_frames +
            animation->passive_frames;
    }
    furi_assert(icon_index < (animation->passive_frames + animation->active_frames));

    return icon_index;
}

static void bubble_animation_draw_callback(Canvas* canvas, void* model_) {
    furi_assert(model_);
    furi_assert(canvas);

    BubbleAnimationViewModel* model = model_;
    const BubbleAnimation* animation = model->current;

    if(model->freeze_frame) {
        uint8_t y_offset = canvas_height(canvas) - icon_get_height(model->freeze_frame);
        canvas_draw_icon(canvas, 0, y_offset, model->freeze_frame);
        return;
    }

    if(!animation) {
        return;
    }

    furi_assert(model->current_frame < 255);

    const Icon* icon = animation->icons[bubble_animation_get_icon_index(model)];
    furi_assert(icon);
    uint8_t y_offset = canvas_height(canvas) - icon_get_height(icon);
    canvas_draw_icon(canvas, 0, y_offset, icon);

    const FrameBubble* bubble = model->current_bubble;
    if(bubble) {
        if((model->current_frame >= bubble->starts_at_frame) &&
           (model->current_frame <= bubble->ends_at_frame)) {
            const Bubble* b = &bubble->bubble;
            elements_bubble_str(canvas, b->x, b->y, b->str, b->horizontal, b->vertical);
        }
    }
}

static FrameBubble* bubble_animation_pick_bubble(BubbleAnimationViewModel* model, bool active) {
    FrameBubble* bubble = NULL;

    if((model->active_bubbles == 0) && (model->passive_bubbles == 0)) {
        return NULL;
    }

    uint8_t index = random() % (active ? model->active_bubbles : model->passive_bubbles);
    const BubbleAnimation* animation = model->current;

    for(int i = 0; i < animation->frame_bubbles_count; ++i) {
        if((animation->frame_bubbles[i]->starts_at_frame < animation->passive_frames) ^ active) {
            if(!index) {
                bubble = animation->frame_bubbles[i];
            }
            --index;
        }
    }

    return bubble;
}

static bool bubble_animation_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    furi_assert(event);

    BubbleAnimationView* animation_view = context;
    bool consumed = false;

    if(event->type == InputTypePress) {
        bubble_animation_activate(animation_view, false);
    }

    if(event->key == InputKeyRight) {
        /* Right button reserved for animation activation, so consume */
        consumed = true;
        if(event->type == InputTypeShort) {
            if(animation_view->interact_callback) {
                animation_view->interact_callback(animation_view->interact_callback_context);
            }
        }
    } else if(event->key == InputKeyBack) {
        /* Prevent back button to fall down to common handler - leaving
         * application, so consume */
        consumed = true;
    }

    return consumed;
}

static void bubble_animation_activate(BubbleAnimationView* view, bool force) {
    furi_assert(view);
    bool activate = true;
    BubbleAnimationViewModel* model = view_get_model(view->view);
    if(!model->current) {
        activate = false;
    } else if(model->freeze_frame) {
        activate = false;
    } else if(model->current->active_frames == 0) {
        activate = false;
    }

    if(!force) {
        if((model->active_ended_at + model->current->active_cooldown * 1000) >
           xTaskGetTickCount()) {
            activate = false;
        } else if(model->active_shift) {
            activate = false;
        } else if(model->current_frame >= model->current->passive_frames) {
            activate = false;
        }
    }
    view_commit_model(view->view, false);

    if(!activate && !force) {
        return;
    }

    if(ACTIVE_SHIFT > 0) {
        BubbleAnimationViewModel* model = view_get_model(view->view);
        model->active_shift = ACTIVE_SHIFT;
        view_commit_model(view->view, false);
    } else {
        bubble_animation_activate_right_now(view);
    }
}

static void bubble_animation_activate_right_now(BubbleAnimationView* view) {
    furi_assert(view);

    uint8_t frame_rate = 0;

    BubbleAnimationViewModel* model = view_get_model(view->view);
    if(model->current && (model->current->active_frames > 0) && (!model->freeze_frame)) {
        model->current_frame = model->current->passive_frames;
        model->current_bubble = bubble_animation_pick_bubble(model, true);
        frame_rate = model->current->frame_rate;
    }
    view_commit_model(view->view, true);

    if(frame_rate) {
        osTimerStart(view->timer, 1000 / frame_rate);
    }
}

static void bubble_animation_next_frame(BubbleAnimationViewModel* model) {
    furi_assert(model);

    if(!model->current) {
        return;
    }

    if(model->current_frame < model->current->passive_frames) {
        model->current_frame = (model->current_frame + 1) % model->current->passive_frames;
    } else {
        ++model->current_frame;
        model->active_cycle +=
            !((model->current_frame - model->current->passive_frames) %
              model->current->active_frames);
        if(model->active_cycle >= model->current->active_cycles) {
            // switch to passive
            model->active_cycle = 0;
            model->current_frame = 0;
            model->current_bubble = bubble_animation_pick_bubble(model, false);
            model->active_ended_at = xTaskGetTickCount();
        }

        if(model->current_bubble) {
            if(model->current_frame > model->current_bubble->ends_at_frame) {
                model->current_bubble = model->current_bubble->next_bubble;
            }
        }
    }
}

static void bubble_animation_timer_callback(void* context) {
    furi_assert(context);
    BubbleAnimationView* view = context;
    bool activate = false;

    BubbleAnimationViewModel* model = view_get_model(view->view);

    if(model->active_shift > 0) {
        activate = (--model->active_shift == 0);
    }

    if(!model->freeze_frame && !activate) {
        bubble_animation_next_frame(model);
    }

    view_commit_model(view->view, !activate);

    if(activate) {
        bubble_animation_activate_right_now(view);
    }
}

static Icon* bubble_animation_clone_frame(const Icon* icon_orig) {
    furi_assert(icon_orig);
    furi_assert(icon_orig->frames);
    furi_assert(icon_orig->frames[0]);

    Icon* icon_clone = furi_alloc(sizeof(Icon));
    memcpy(icon_clone, icon_orig, sizeof(Icon));

    icon_clone->frames = furi_alloc(sizeof(uint8_t*));
    /* icon bitmap can be either compressed or not. It is compressed if
     * compressed size is less than original, so max size for bitmap is
     * uncompressed (width * height) + 1 byte (in uncompressed case)
     * for compressed header
     */
    size_t max_bitmap_size = ROUND_UP_TO(icon_orig->width, 8) * icon_orig->height + 1;
    icon_clone->frames[0] = furi_alloc(max_bitmap_size);
    memcpy((void*)icon_clone->frames[0], icon_orig->frames[0], max_bitmap_size);

    return icon_clone;
}

static void bubble_animation_release_frame(Icon** icon) {
    furi_assert(icon);
    furi_assert(*icon);

    free((void*)(*icon)->frames[0]);
    free((*icon)->frames);
    free(*icon);
    *icon = NULL;
}

static void bubble_animation_enter(void* context) {
    furi_assert(context);
    BubbleAnimationView* view = context;
    bubble_animation_activate(view, false);

    BubbleAnimationViewModel* model = view_get_model(view->view);
    uint8_t frame_rate = model->current->frame_rate;
    view_commit_model(view->view, false);

    if(frame_rate) {
        osTimerStart(view->timer, 1000 / frame_rate);
    }
}

static void bubble_animation_exit(void* context) {
    furi_assert(context);
    BubbleAnimationView* view = context;
    osTimerStop(view->timer);
}

BubbleAnimationView* bubble_animation_view_alloc(void) {
    BubbleAnimationView* view = furi_alloc(sizeof(BubbleAnimationView));
    view->view = view_alloc();
    view->interact_callback = NULL;
    view->timer = osTimerNew(bubble_animation_timer_callback, osTimerPeriodic, view, NULL);

    view_allocate_model(view->view, ViewModelTypeLocking, sizeof(BubbleAnimationViewModel));
    view_set_context(view->view, view);
    view_set_draw_callback(view->view, bubble_animation_draw_callback);
    view_set_input_callback(view->view, bubble_animation_input_callback);
    view_set_enter_callback(view->view, bubble_animation_enter);
    view_set_exit_callback(view->view, bubble_animation_exit);

    return view;
}

void bubble_animation_view_free(BubbleAnimationView* view) {
    furi_assert(view);

    view_set_draw_callback(view->view, NULL);
    view_set_input_callback(view->view, NULL);
    view_set_context(view->view, NULL);

    view_free(view->view);
    view->view = NULL;
    free(view);
}

void bubble_animation_view_set_interact_callback(
    BubbleAnimationView* view,
    BubbleAnimationInteractCallback callback,
    void* context) {
    furi_assert(view);

    view->interact_callback_context = context;
    view->interact_callback = callback;
}

void bubble_animation_view_set_animation(
    BubbleAnimationView* view,
    const BubbleAnimation* new_animation) {
    furi_assert(view);
    furi_assert(new_animation);

    BubbleAnimationViewModel* model = view_get_model(view->view);
    furi_assert(model);
    model->current = new_animation;

    model->active_ended_at = xTaskGetTickCount() - (model->current->active_cooldown * 1000);
    model->active_bubbles = 0;
    model->passive_bubbles = 0;
    for(int i = 0; i < new_animation->frame_bubbles_count; ++i) {
        if(new_animation->frame_bubbles[i]->starts_at_frame < new_animation->passive_frames) {
            ++model->passive_bubbles;
        } else {
            ++model->active_bubbles;
        }
    }

    /* select bubble sequence */
    model->current_bubble = bubble_animation_pick_bubble(model, false);
    model->current_frame = 0;
    model->active_cycle = 0;
    view_commit_model(view->view, true);

    osTimerStart(view->timer, 1000 / new_animation->frame_rate);
}

void bubble_animation_freeze(BubbleAnimationView* view) {
    furi_assert(view);

    BubbleAnimationViewModel* model = view_get_model(view->view);
    furi_assert(model->current);
    furi_assert(!model->freeze_frame);
    /* always freeze first passive frame, because
     * animation is always activated at unfreezing and played
     * passive frame first, and 2 frames after - active
     */
    uint8_t icon_index = 0;
    model->freeze_frame = bubble_animation_clone_frame(model->current->icons[icon_index]);
    model->current = NULL;
    view_commit_model(view->view, false);
    osTimerStop(view->timer);
}

void bubble_animation_unfreeze(BubbleAnimationView* view) {
    furi_assert(view);
    uint8_t frame_rate;

    BubbleAnimationViewModel* model = view_get_model(view->view);
    furi_assert(model->freeze_frame);
    bubble_animation_release_frame(&model->freeze_frame);
    furi_assert(model->current);
    furi_assert(model->current->icons);
    frame_rate = model->current->frame_rate;
    view_commit_model(view->view, true);

    osTimerStart(view->timer, 1000 / frame_rate);
    bubble_animation_activate(view, false);
}

View* bubble_animation_get_view(BubbleAnimationView* view) {
    furi_assert(view);

    return view->view;
}
