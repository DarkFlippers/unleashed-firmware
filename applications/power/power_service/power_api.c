#include "power_i.h"

#include <furi.h>
#include "furi-hal-power.h"
#include "furi-hal-bootloader.h"

void power_off(Power* power) {
    furi_hal_power_off();
    // Notify user if USB is plugged
    view_dispatcher_send_to_front(power->view_dispatcher);
    view_dispatcher_switch_to_view(power->view_dispatcher, PowerViewPopup);
    osDelay(10);
    furi_crash("Disconnect USB for safe shutdown");
}

void power_reboot(PowerBootMode mode) {
    if(mode == PowerBootModeNormal) {
        furi_hal_bootloader_set_mode(FuriHalBootloaderModeNormal);
    } else if(mode == PowerBootModeDfu) {
        furi_hal_bootloader_set_mode(FuriHalBootloaderModeDFU);
    }
    furi_hal_power_reset();
}

void power_get_info(Power* power, PowerInfo* info) {
    furi_assert(power);
    furi_assert(info);

    osMutexAcquire(power->info_mtx, osWaitForever);
    memcpy(info, &power->info, sizeof(power->info));
    osMutexRelease(power->info_mtx);
}

FuriPubSub* power_get_pubsub(Power* power) {
    furi_assert(power);
    return power->event_pubsub;
}
