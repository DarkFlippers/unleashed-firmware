#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IconData IconData;
typedef struct Icon Icon;

/*
 * Allocate icon instance with const icon data.
 * always returns Icon or stops system if not enough memory
 */
Icon* icon_alloc(const IconData* data);

/*
 * Release icon instance
 */
void icon_free(Icon* icon);

/*
 * Get icon width
 */
uint8_t icon_get_width(Icon* icon);

/*
 * Get icon height
 */
uint8_t icon_get_height(Icon* icon);

/*
 * Check if icon is animated
 */
bool icon_is_animated(Icon* icon);

/*
 * Start icon animation
 */
void icon_start_animation(Icon* icon);

/*
 * Stop icon animation
 */
void icon_stop_animation(Icon* icon);

#ifdef __cplusplus
}
#endif
