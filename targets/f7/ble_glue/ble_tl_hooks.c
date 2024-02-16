#include "ble_glue.h"

#include <interface/patterns/ble_thread/tl/shci_tl.h>
#include <ble/ble.h>
#include <hci_tl.h>
#include <furi.h>

///////////////////////////////////////////////////////////////////////////////

/* 
 * TL hooks to catch hardfaults 
 */

int32_t ble_glue_TL_SYS_SendCmd(uint8_t* buffer, uint16_t size) {
    if(ble_glue_get_hardfault_info()) {
        furi_crash("ST(R) Copro(R) HardFault");
    }

    return TL_SYS_SendCmd(buffer, size);
}

void shci_register_io_bus(tSHciIO* fops) {
    /* Register IO bus services */
    fops->Init = TL_SYS_Init;
    fops->Send = ble_glue_TL_SYS_SendCmd;
}

static int32_t ble_glue_TL_BLE_SendCmd(uint8_t* buffer, uint16_t size) {
    if(ble_glue_get_hardfault_info()) {
        furi_crash("ST(R) Copro(R) HardFault");
    }

    return TL_BLE_SendCmd(buffer, size);
}

void hci_register_io_bus(tHciIO* fops) {
    /* Register IO bus services */
    fops->Init = TL_BLE_Init;
    fops->Send = ble_glue_TL_BLE_SendCmd;
}
