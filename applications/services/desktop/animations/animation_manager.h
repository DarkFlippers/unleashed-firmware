#pragma once

#include <gui/view.h>
#include <gui/icon_i.h>
#include <stdint.h>
#include <dolphin/dolphin.h>

typedef struct AnimationManager AnimationManager;

typedef struct {
    uint8_t x;
    uint8_t y;
    const char* text;
    Align align_h;
    Align align_v;
} Bubble;

typedef struct FrameBubble {
    Bubble bubble;
    uint8_t start_frame;
    uint8_t end_frame;
    const struct FrameBubble* next_bubble;
} FrameBubble;

typedef struct {
    const FrameBubble* const* frame_bubble_sequences;
    uint8_t frame_bubble_sequences_count;
    const Icon icon_animation;
    const uint8_t* frame_order;
    uint8_t passive_frames;
    uint8_t active_frames;
    uint8_t active_cycles;
    uint16_t duration;
    uint16_t active_cooldown;
} BubbleAnimation;

typedef void (*AnimationManagerSetNewIdleAnimationCallback)(void* context);
typedef void (*AnimationManagerCheckBlockingCallback)(void* context);
typedef void (*AnimationManagerInteractCallback)(void*);

/**
 * Allocate Animation Manager
 *
 * @return animation manager instance
 */
AnimationManager* animation_manager_alloc(void);

/**
 * Free Animation Manager
 *
 * @animation_manager   instance
 */
void animation_manager_free(AnimationManager* animation_manager);

/**
 * Get View of Animation Manager
 *
 * @animation_manager   instance
 * @return      view
 */
View* animation_manager_get_animation_view(AnimationManager* animation_manager);

/**
 * Set context for all callbacks for Animation Manager
 *
 * @animation_manager   instance
 * @context             context
 */
void animation_manager_set_context(AnimationManager* animation_manager, void* context);

/**
 * Set callback for Animation Manager for defered calls
 * for animation_manager_new_idle_process().
 * Animation Manager doesn't have it's own thread, so main thread gives
 * callbacks to A.M. to call when it should perform some inner manipulations.
 * This callback is called from other threads and should notify main thread
 * when to call animation_manager_new_idle_process().
 * So scheme is this:
 * A.M. sets callbacks,
 * callbacks notifies main thread
 * main thread in its own context calls appropriate *_process() function.
 *
 * @animation_manager   instance
 * @callback            callback
 */
void animation_manager_set_new_idle_callback(
    AnimationManager* animation_manager,
    AnimationManagerSetNewIdleAnimationCallback callback);

/**
 * Function to call in main thread as a response to
 * set_new_idle_callback's call.
 *
 * @animation_manager   instance
 */
void animation_manager_new_idle_process(AnimationManager* animation_manager);

/**
 * Set callback for Animation Manager for defered calls
 * for animation_manager_check_blocking_process().
 *
 * @animation_manager   instance
 * @callback            callback
 */
void animation_manager_set_check_callback(
    AnimationManager* animation_manager,
    AnimationManagerCheckBlockingCallback callback);

/**
 * Function to call in main thread as a response to
 * set_new_idle_callback's call.
 *
 * @animation_manager   instance
 */
void animation_manager_check_blocking_process(AnimationManager* animation_manager);

/**
 * Set callback for Animation Manager for defered calls
 * for animation_manager_interact_process().
 *
 * @animation_manager   instance
 * @callback            callback
 */
void animation_manager_set_interact_callback(
    AnimationManager* animation_manager,
    AnimationManagerInteractCallback callback);

/**
 * Function to call in main thread as a response to
 * set_new_idle_callback's call.
 *
 * @animation_manager   instance
 * @return              true if event was consumed
 */
bool animation_manager_interact_process(AnimationManager* animation_manager);

/** Check if animation loaded
 *
 * @animation_manager   instance
 */
bool animation_manager_is_animation_loaded(AnimationManager* animation_manager);

/**
 * Unload and Stall animation actions. Draw callback in view
 * paints first frame of current animation until
 * animation_manager_load_and_continue_animation() is called.
 * Can't be called multiple times. Every Stall has to be finished
 * with Continue.
 *
 * @animation_manager   instance
 */
void animation_manager_unload_and_stall_animation(AnimationManager* animation_manager);

/**
 * Load and Contunue execution of animation manager.
 *
 * @animation_manager   instance
 */
void animation_manager_load_and_continue_animation(AnimationManager* animation_manager);
