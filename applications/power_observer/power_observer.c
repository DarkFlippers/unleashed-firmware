#include <furi.h>
#include <furi_hal.h>
#include <notification/notification_messages.h>

typedef struct {
    osThreadId_t thread;

} PowerObserverSrv;

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

typedef enum {
    EventReset = (1 << 0),
    EventRequest = (1 << 1),
} UsbEvent;

static void usb_state_callback(FuriHalUsbStateEvent state, void* context) {
    PowerObserverSrv* srv = (PowerObserverSrv*)(context);
    if(state == FuriHalUsbStateEventReset) {
        osThreadFlagsSet(srv->thread, EventReset);
    } else if(state == FuriHalUsbStateEventDescriptorRequest) {
        osThreadFlagsSet(srv->thread, EventRequest);
    }
}

int32_t power_observer_srv(void* p) {
    NotificationApp* notifications = furi_record_open("notification");
    PowerObserverSrv* srv = furi_alloc(sizeof(PowerObserverSrv));
    srv->thread = osThreadGetId();

    const float overconsumption_limit = 0.03f;
    bool usb_request_pending = false;
    uint8_t usb_wait_time = 0;

    furi_hal_usb_set_state_callback(usb_state_callback, srv);

    while(true) {
        uint32_t flags = osThreadFlagsWait(EventReset | EventRequest, osFlagsWaitAny, 500);
        if((flags & osFlagsError) == 0) {
            if(flags & EventReset) {
                usb_request_pending = true;
                usb_wait_time = 0;
            }
            if(flags & EventRequest) {
                usb_request_pending = false;
            }
        } else if(usb_request_pending) {
            usb_wait_time++;
            if(usb_wait_time > 4) {
                furi_hal_usb_reinit();
                usb_request_pending = false;
            }
        }

        float current = -furi_hal_power_get_battery_current(FuriHalPowerICFuelGauge);
        if(current > overconsumption_limit) {
            notification_message_block(notifications, &sequence_overconsumption);
        }
        if(furi_hal_power_is_otg_enabled()) {
            furi_hal_power_check_otg_status();
        }
    }

    free(srv);
    return 0;
}
