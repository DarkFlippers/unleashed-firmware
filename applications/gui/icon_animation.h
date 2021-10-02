#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <assets_icons.h>

typedef struct IconAnimation IconAnimation;

typedef void (*IconAnimationCallback)(IconAnimation* instance, void* context);

/** Allocate icon animation instance with const icon data.
 * always returns Icon or stops system if not enough memory
 */
IconAnimation* icon_animation_alloc(const Icon* icon);

/** Release icon animation instance
 */
void icon_animation_free(IconAnimation* instance);

/** Get icon animation width
 */
void icon_animation_set_update_callback(
    IconAnimation* instance,
    IconAnimationCallback callback,
    void* context);

/** Get icon animation width
 */
uint8_t icon_animation_get_width(IconAnimation* instance);

/** Get icon animation height
 */
uint8_t icon_animation_get_height(IconAnimation* instance);

/** Start icon animation
 */
void icon_animation_start(IconAnimation* instance);

/** Stop icon animation
 */
void icon_animation_stop(IconAnimation* instance);

/** Returns true if current frame is a last one
 */
bool icon_animation_is_last_frame(IconAnimation* instance);

#ifdef __cplusplus
}
#endif
