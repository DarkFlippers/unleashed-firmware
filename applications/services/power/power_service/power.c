#include "power_i.h"

#include <furi.h>
#include <furi_hal.h>

#define POWER_OFF_TIMEOUT 90

void power_draw_battery_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    Power* power = context;
    canvas_draw_icon(canvas, 0, 0, &I_Battery_26x8);

    if(power->info.gauge_is_ok) {
        canvas_draw_box(canvas, 2, 2, (power->info.charge + 4) / 5, 4);
        if(power->state == PowerStateCharging) {
            canvas_set_bitmap_mode(canvas, 1);
            canvas_set_color(canvas, ColorWhite);
            // TODO: replace -1 magic for uint8_t with re-framing
            canvas_draw_icon(canvas, 8, -1, &I_Charging_lightning_mask_9x10);
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_icon(canvas, 8, -1, &I_Charging_lightning_9x10);
            canvas_set_bitmap_mode(canvas, 0);
        }
    } else {
        canvas_draw_box(canvas, 8, 3, 8, 2);
    }
}

static ViewPort* power_battery_view_port_alloc(Power* power) {
    ViewPort* battery_view_port = view_port_alloc();
    view_port_set_width(battery_view_port, icon_get_width(&I_Battery_26x8));
    view_port_draw_callback_set(battery_view_port, power_draw_battery_callback, power);
    gui_add_view_port(power->gui, battery_view_port, GuiLayerStatusBarRight);
    return battery_view_port;
}

Power* power_alloc() {
    Power* power = malloc(sizeof(Power));

    // Records
    power->notification = furi_record_open(RECORD_NOTIFICATION);
    power->gui = furi_record_open(RECORD_GUI);

    // Pubsub
    power->event_pubsub = furi_pubsub_alloc();

    // State initialization
    power->state = PowerStateNotCharging;
    power->battery_low = false;
    power->power_off_timeout = POWER_OFF_TIMEOUT;
    power->api_mtx = furi_mutex_alloc(FuriMutexTypeNormal);

    // Gui
    power->view_dispatcher = view_dispatcher_alloc();
    power->power_off = power_off_alloc();
    view_dispatcher_add_view(
        power->view_dispatcher, PowerViewOff, power_off_get_view(power->power_off));
    power->power_unplug_usb = power_unplug_usb_alloc();
    view_dispatcher_add_view(
        power->view_dispatcher,
        PowerViewUnplugUsb,
        power_unplug_usb_get_view(power->power_unplug_usb));
    view_dispatcher_attach_to_gui(
        power->view_dispatcher, power->gui, ViewDispatcherTypeFullscreen);

    // Battery view port
    power->battery_view_port = power_battery_view_port_alloc(power);
    power->show_low_bat_level_message = true;

    return power;
}

void power_free(Power* power) {
    furi_assert(power);

    // Gui
    view_dispatcher_remove_view(power->view_dispatcher, PowerViewOff);
    power_off_free(power->power_off);
    view_dispatcher_remove_view(power->view_dispatcher, PowerViewUnplugUsb);
    power_unplug_usb_free(power->power_unplug_usb);

    view_port_free(power->battery_view_port);

    // State
    furi_mutex_free(power->api_mtx);

    // FuriPubSub
    furi_pubsub_free(power->event_pubsub);

    // Records
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);

    free(power);
}

static void power_check_charging_state(Power* power) {
    if(furi_hal_power_is_charging()) {
        if((power->info.charge == 100) || (furi_hal_power_is_charging_done())) {
            if(power->state != PowerStateCharged) {
                notification_internal_message(power->notification, &sequence_charged);
                power->state = PowerStateCharged;
                power->event.type = PowerEventTypeFullyCharged;
                furi_pubsub_publish(power->event_pubsub, &power->event);
            }
        } else {
            if(power->state != PowerStateCharging) {
                notification_internal_message(power->notification, &sequence_charging);
                power->state = PowerStateCharging;
                power->event.type = PowerEventTypeStartCharging;
                furi_pubsub_publish(power->event_pubsub, &power->event);
            }
        }
    } else {
        if(power->state != PowerStateNotCharging) {
            notification_internal_message(power->notification, &sequence_not_charging);
            power->state = PowerStateNotCharging;
            power->event.type = PowerEventTypeStopCharging;
            furi_pubsub_publish(power->event_pubsub, &power->event);
        }
    }
}

static bool power_update_info(Power* power) {
    PowerInfo info;

    info.is_charging = furi_hal_power_is_charging();
    info.gauge_is_ok = furi_hal_power_gauge_is_ok();
    info.charge = furi_hal_power_get_pct();
    info.health = furi_hal_power_get_bat_health_pct();
    info.capacity_remaining = furi_hal_power_get_battery_remaining_capacity();
    info.capacity_full = furi_hal_power_get_battery_full_capacity();
    info.current_charger = furi_hal_power_get_battery_current(FuriHalPowerICCharger);
    info.current_gauge = furi_hal_power_get_battery_current(FuriHalPowerICFuelGauge);
    info.voltage_charger = furi_hal_power_get_battery_voltage(FuriHalPowerICCharger);
    info.voltage_gauge = furi_hal_power_get_battery_voltage(FuriHalPowerICFuelGauge);
    info.voltage_vbus = furi_hal_power_get_usb_voltage();
    info.temperature_charger = furi_hal_power_get_battery_temperature(FuriHalPowerICCharger);
    info.temperature_gauge = furi_hal_power_get_battery_temperature(FuriHalPowerICFuelGauge);

    furi_mutex_acquire(power->api_mtx, FuriWaitForever);
    bool need_refresh = power->info.charge != info.charge;
    need_refresh |= power->info.is_charging != info.is_charging;
    power->info = info;
    furi_mutex_release(power->api_mtx);

    return need_refresh;
}

static void power_check_low_battery(Power* power) {
    if(!power->info.gauge_is_ok) {
        return;
    }

    // Check battery charge and vbus voltage
    if((power->info.charge == 0) && (power->info.voltage_vbus < 4.0f) &&
       power->show_low_bat_level_message) {
        if(!power->battery_low) {
            view_dispatcher_send_to_front(power->view_dispatcher);
            view_dispatcher_switch_to_view(power->view_dispatcher, PowerViewOff);
        }
        power->battery_low = true;
    } else {
        if(power->battery_low) {
            view_dispatcher_switch_to_view(power->view_dispatcher, VIEW_NONE);
            power->power_off_timeout = POWER_OFF_TIMEOUT;
        }
        power->battery_low = false;
    }
    // If battery low, update view and switch off power after timeout
    if(power->battery_low) {
        PowerOffResponse response = power_off_get_response(power->power_off);
        if(response == PowerOffResponseDefault) {
            if(power->power_off_timeout) {
                power_off_set_time_left(power->power_off, power->power_off_timeout--);
            } else {
                power_off(power);
            }
        } else if(response == PowerOffResponseOk) {
            power_off(power);
        } else if(response == PowerOffResponseHide) {
            view_dispatcher_switch_to_view(power->view_dispatcher, VIEW_NONE);
            if(power->power_off_timeout) {
                power_off_set_time_left(power->power_off, power->power_off_timeout--);
            } else {
                power_off(power);
            }
        } else if(response == PowerOffResponseCancel) {
            view_dispatcher_switch_to_view(power->view_dispatcher, VIEW_NONE);
        }
    }
}

static void power_check_battery_level_change(Power* power) {
    if(power->battery_level != power->info.charge) {
        power->battery_level = power->info.charge;
        power->event.type = PowerEventTypeBatteryLevelChanged;
        power->event.data.battery_level = power->battery_level;
        furi_pubsub_publish(power->event_pubsub, &power->event);
    }
}

int32_t power_srv(void* p) {
    UNUSED(p);
    Power* power = power_alloc();
    power_update_info(power);
    furi_record_create(RECORD_POWER, power);

    while(1) {
        // Update data from gauge and charger
        bool need_refresh = power_update_info(power);

        // Check low battery level
        power_check_low_battery(power);

        // Check and notify about charging state
        power_check_charging_state(power);

        // Check and notify about battery level change
        power_check_battery_level_change(power);

        // Update battery view port
        if(need_refresh) view_port_update(power->battery_view_port);

        // Check OTG status and disable it in case of fault
        if(furi_hal_power_is_otg_enabled()) {
            furi_hal_power_check_otg_status();
        }

        furi_delay_ms(1000);
    }

    power_free(power);

    return 0;
}
