#include "power_i.h"

#include <furi.h>
#include <furi_hal.h>

void power_off(Power* power) {
    furi_hal_power_off();
    // Notify user if USB is plugged
    view_dispatcher_send_to_front(power->view_dispatcher);
    view_dispatcher_switch_to_view(power->view_dispatcher, PowerViewPopup);
    osDelay(10);
    furi_halt("Disconnect USB for safe shutdown");
}

void power_reboot(PowerBootMode mode) {
    if(mode == PowerBootModeNormal) {
        furi_hal_rtc_set_boot_mode(FuriHalRtcBootModeNormal);
    } else if(mode == PowerBootModeDfu) {
        furi_hal_rtc_set_boot_mode(FuriHalRtcBootModeDfu);
    } else if(mode == PowerBootModeUpdateStart) {
        furi_hal_rtc_set_boot_mode(FuriHalRtcBootModePreUpdate);
    }
    furi_hal_power_reset();
}

void power_get_info(Power* power, PowerInfo* info) {
    furi_assert(power);
    furi_assert(info);

    osMutexAcquire(power->api_mtx, osWaitForever);
    memcpy(info, &power->info, sizeof(power->info));
    osMutexRelease(power->api_mtx);
}

FuriPubSub* power_get_pubsub(Power* power) {
    furi_assert(power);
    return power->event_pubsub;
}

bool power_is_battery_healthy(Power* power) {
    furi_assert(power);
    bool is_healthy = false;
    osMutexAcquire(power->api_mtx, osWaitForever);
    is_healthy = power->info.health > POWER_BATTERY_HEALTHY_LEVEL;
    osMutexRelease(power->api_mtx);
    return is_healthy;
}

void power_enable_low_battery_level_notification(Power* power, bool enable) {
    furi_assert(power);
    osMutexAcquire(power->api_mtx, osWaitForever);
    power->show_low_bat_level_message = enable;
    osMutexRelease(power->api_mtx);
}
