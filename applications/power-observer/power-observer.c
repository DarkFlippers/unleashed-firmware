#include <furi.h>
#include <furi-hal.h>
#include <notification/notification-messages.h>

const NotificationMessage message_green_110 = {
    .type = NotificationMessageTypeLedGreen,
    .data.led.value = 110,
};

static const NotificationSequence sequence_overconsumption = {
    &message_green_110,
    &message_red_255,
    &message_delay_100,
    NULL,
};

int32_t power_observer_srv(void* p) {
    NotificationApp* notifications = furi_record_open("notification");

    const float overconsumption_limit = 0.03f;

    while(true) {
        float current = -furi_hal_power_get_battery_current(FuriHalPowerICFuelGauge);

        if(current >= overconsumption_limit) {
            notification_message_block(notifications, &sequence_overconsumption);
        }

        delay(1000);
    }

    return 0;
}