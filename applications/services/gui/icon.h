/**
 * @file icon.h
 * GUI: Icon API
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Icon Icon;

/** Get icon width 
 *
 * @param[in]  instance  pointer to Icon data
 *
 * @return     width in pixels
 */
uint8_t icon_get_width(const Icon* instance);

/** Get icon height
 *
 * @param[in]  instance  pointer to Icon data
 *
 * @return     height in pixels
 */
uint8_t icon_get_height(const Icon* instance);

/** Get Icon XBM bitmap data
 *
 * @param[in]  instance  pointer to Icon data
 *
 * @return     pointer to XBM bitmap data
 */
const uint8_t* icon_get_data(const Icon* instance);

#ifdef __cplusplus
}
#endif
