/*
Flipper devices inc.

Bootloader API, must be implemented by target
*/

#ifndef __BOOT_H
#define __BOOT_H

/*
 * @brief Request DFU and reboot
*/
void boot_restart_in_dfu();

#endif
