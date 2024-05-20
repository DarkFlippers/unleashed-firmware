/**
 * @file icon.h
 * GUI: Icon API
 */

#pragma once

#include <stdint.h>
#include <core/common_defines.h>

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
uint16_t icon_get_width(const Icon* instance);

/** Get icon height
 *
 * @param[in]  instance  pointer to Icon data
 *
 * @return     height in pixels
 */
uint16_t icon_get_height(const Icon* instance);

/** Get Icon XBM bitmap data for the first frame
 *
 * @param[in]  instance  pointer to Icon data
 *
 * @return     pointer to compressed XBM bitmap data
 */
FURI_DEPRECATED const uint8_t* icon_get_data(const Icon* instance);

/** Get Icon frame count
 *
 * @param[in]  instance  pointer to Icon data
 *
 * @return     frame count
 */
uint32_t icon_get_frame_count(const Icon* instance);

/** Get Icon XBM bitmap data for a particular frame
 *
 * @param[in]  instance  pointer to Icon data
 * @param[in]  frame     frame index
 *
 * @return     pointer to compressed XBM bitmap data
 */
const uint8_t* icon_get_frame_data(const Icon* instance, uint32_t frame);

#ifdef __cplusplus
}
#endif
