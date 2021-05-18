#ifndef TARGET_H
#define TARGET_H

/**
 * Initialize hardware
*/
void target_init();

/**
 * Check if dfu mode requested
 * @return 1 if dfu mode requested, 0 if not
 */
int target_is_dfu_requested();

/**
 * Switch to dfu mode
 */
void target_switch2dfu();

/**
 * Switch to OS
 */
void target_switch2os();

#endif