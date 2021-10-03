/**
 * @file icon_animation.h
 * GUI: IconAnimation API
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <assets_icons.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Icon Animation */
typedef struct IconAnimation IconAnimation;

/** Icon Animation Callback. Used for update notification */
typedef void (*IconAnimationCallback)(IconAnimation* instance, void* context);

/** Allocate icon animation instance with const icon data.
 * 
 * always returns Icon or stops system if not enough memory
 *
 * @param[in]  icon  pointer to Icon data
 *
 * @return     IconAnimation instance
 */
IconAnimation* icon_animation_alloc(const Icon* icon);

/** Release icon animation instance
 *
 * @param      instance  IconAnimation instance
 */
void icon_animation_free(IconAnimation* instance);

/** Set IconAnimation update callback
 *
 * Normally you do not need to use this function, use view_tie_icon_animation
 * instead.
 *
 * @param      instance  IconAnimation instance
 * @param[in]  callback  IconAnimationCallback
 * @param      context   callback context
 */
void icon_animation_set_update_callback(
    IconAnimation* instance,
    IconAnimationCallback callback,
    void* context);

/** Get icon animation width
 *
 * @param      instance  IconAnimation instance
 *
 * @return     width in pixels
 */
uint8_t icon_animation_get_width(IconAnimation* instance);

/** Get icon animation height
 *
 * @param      instance  IconAnimation instance
 *
 * @return     height in pixels
 */
uint8_t icon_animation_get_height(IconAnimation* instance);

/** Start icon animation
 *
 * @param      instance  IconAnimation instance
 */
void icon_animation_start(IconAnimation* instance);

/** Stop icon animation
 *
 * @param      instance  IconAnimation instance
 */
void icon_animation_stop(IconAnimation* instance);

/** Returns true if current frame is a last one
 *
 * @param      instance  IconAnimation instance
 *
 * @return     true if last frame
 */
bool icon_animation_is_last_frame(IconAnimation* instance);

#ifdef __cplusplus
}
#endif
