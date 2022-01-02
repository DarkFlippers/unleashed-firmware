#pragma once

#include <gui/view.h>
#include "../animation_manager.h"

/** Bubble Animation instance */
typedef struct BubbleAnimationView BubbleAnimationView;

/** Callback type to be called when interact button pressed */
typedef void (*BubbleAnimationInteractCallback)(void*);

/**
 * Allocate bubble animation view.
 * This is animation with bubbles, and 2 phases:
 * active and passive.
 *
 * @return instance of new bubble animation
 */
BubbleAnimationView* bubble_animation_view_alloc(void);

/**
 * Free bubble animation view.
 *
 * @view        bubble animation view instance
 */
void bubble_animation_view_free(BubbleAnimationView* view);

/**
 * Set callback for interact action for animation.
 * Currently this is right button.
 *
 * @view        bubble animation view instance
 * @callback    callback to call when button pressed
 * @context     context
 */
void bubble_animation_view_set_interact_callback(
    BubbleAnimationView* view,
    BubbleAnimationInteractCallback callback,
    void* context);

/**
 * Set new animation.
 * BubbleAnimation doesn't posses Bubble Animation object
 * so it doesn't handle any memory manipulation on Bubble Animations.
 *
 * @view                    bubble animation view instance
 * @new_bubble_animation    new animation to set
 */
void bubble_animation_view_set_animation(
    BubbleAnimationView* view,
    const BubbleAnimation* new_bubble_animation);

/**
 * Get view of bubble animation.
 *
 * @view        bubble animation view instance
 * @return      view
 */
View* bubble_animation_get_view(BubbleAnimationView* view);

/**
 * Freeze current playing animation. Saves a frame to be shown
 * during next unfreeze called.
 * bubble_animation_freeze() stops any reference to 'current' animation
 * so it can be freed. Therefore lock unfreeze should be preceeded with
 * new animation set.
 *
 * Freeze/Unfreeze usage example:
 *
 *  animation_view_alloc()
 *  set_animation()
 *  ...
 *  freeze_animation()
 *   // release animation
 *  ...
 *   // allocate animation
 *  set_animation()
 *  unfreeze()
 *
 * @view        bubble animation view instance
 */
void bubble_animation_freeze(BubbleAnimationView* view);

/**
 * Starts bubble animation after freezing.
 *
 * @view        bubble animation view instance
 */
void bubble_animation_unfreeze(BubbleAnimationView* view);
