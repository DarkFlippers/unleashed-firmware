#pragma once

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

/* Initialize timebase
 * Configure and start tick timer
 */
void api_hal_timebase_init();

/* Get current insomnia level
 * @return insomnia level: 0 - no insomnia, >0 - insomnia, bearer count.
 */
uint16_t api_hal_timebase_insomnia_level();

/* Enter insomnia mode
 * Prevents device from going to sleep
 * @warning Internally increases insomnia level
 * Must be paired with api_hal_timebase_insomnia_exit
 */
void api_hal_timebase_insomnia_enter();

/* Exit insomnia mode
 * Allow device to go to sleep
 * @warning Internally decreases insomnia level.
 * Must be paired with api_hal_timebase_insomnia_enter
 */
void api_hal_timebase_insomnia_exit();


#ifdef __cplusplus
}
#endif