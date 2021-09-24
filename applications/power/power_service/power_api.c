#include "power_i.h"
#include <furi.h>
#include "furi-hal-power.h"
#include "furi-hal-boot.h"

void power_off() {
    furi_hal_power_off();
}

void power_reboot(PowerBootMode mode) {
    if(mode == PowerBootModeNormal) {
        furi_hal_boot_set_mode(FuriHalBootModeNormal);
    } else if(mode == PowerBootModeDfu) {
        furi_hal_boot_set_mode(FuriHalBootModeDFU);
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

PubSub* power_get_pubsub(Power* power) {
    furi_assert(power);
    return &power->event_pubsub;
}
