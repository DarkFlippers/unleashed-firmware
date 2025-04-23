#include "desktop/desktop_settings.h"
#include "power_i.h"

#include <furi.h>
#include <furi_hal.h>

#include <update_util/update_operation.h>
#include <notification/notification_messages.h>

#include <loader/loader.h>

#define TAG "Power"

#define POWER_OFF_TIMEOUT_S  (90U)
#define POWER_POLL_PERIOD_MS (1000UL)

#define POWER_VBUS_LOW_THRESHOLD   (4.0f)
#define POWER_HEALTH_LOW_THRESHOLD (70U)

static void power_draw_battery_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    Power* power = context;
    canvas_draw_icon(canvas, 0, 0, &I_Battery_26x8);

    if(power->info.gauge_is_ok) {
        char batteryPercentile[4];
        snprintf(batteryPercentile, sizeof(batteryPercentile), "%d", power->info.charge);
        if((power->displayBatteryPercentage == DISPLAY_BATTERY_PERCENT) &&
           (power->state !=
            PowerStateCharging)) { //if display battery percentage, black background white text
            canvas_set_font(canvas, FontBatteryPercent);
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, 1, 1, 22, 6);
            canvas_set_color(canvas, ColorWhite);
            canvas_draw_str_aligned(canvas, 11, 4, AlignCenter, AlignCenter, batteryPercentile);
        } else if(
            (power->displayBatteryPercentage == DISPLAY_BATTERY_INVERTED_PERCENT) &&
            (power->state !=
             PowerStateCharging)) { //if display inverted percentage, white background black text
            canvas_set_font(canvas, FontBatteryPercent);
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_str_aligned(canvas, 11, 4, AlignCenter, AlignCenter, batteryPercentile);
        } else if(
            (power->displayBatteryPercentage == DISPLAY_BATTERY_RETRO_3) &&
            (power->state != PowerStateCharging)) { //Retro style segmented display, 3 parts
            if(power->info.charge > 25) {
                canvas_draw_box(canvas, 2, 2, 6, 4);
            }
            if(power->info.charge > 50) {
                canvas_draw_box(canvas, 9, 2, 6, 4);
            }
            if(power->info.charge > 75) {
                canvas_draw_box(canvas, 16, 2, 6, 4);
            }
        } else if(
            (power->displayBatteryPercentage == DISPLAY_BATTERY_RETRO_5) &&
            (power->state != PowerStateCharging)) { //Retro style segmented display, 5 parts
            if(power->info.charge > 10) {
                canvas_draw_box(canvas, 2, 2, 3, 4);
            }
            if(power->info.charge > 30) {
                canvas_draw_box(canvas, 6, 2, 3, 4);
            }
            if(power->info.charge > 50) {
                canvas_draw_box(canvas, 10, 2, 3, 4);
            }
            if(power->info.charge > 70) {
                canvas_draw_box(canvas, 14, 2, 3, 4);
            }
            if(power->info.charge > 90) {
                canvas_draw_box(canvas, 18, 2, 3, 4);
            }
        } else if(
            (power->displayBatteryPercentage == DISPLAY_BATTERY_BAR_PERCENT) &&
            (power->state != PowerStateCharging) && // Default bar display with percentage
            (power->info.voltage_battery_charge_limit >=
             4.2)) { // not looking nice with low voltage indicator
            canvas_set_font(canvas, FontBatteryPercent);

            // align charge display value with digits to draw
            uint8_t bar_charge = power->info.charge;
            if(bar_charge > 23 && bar_charge < 38) {
                bar_charge = 23;
            } else if(bar_charge >= 38 && bar_charge < 62) {
                bar_charge = 50;
            } else if(bar_charge >= 62 && bar_charge < 74) {
                bar_charge = 74;
            }

            // drawing digits
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, 1, 1, (bar_charge * 22) / 100, 6);
            if(bar_charge < 38) { // both digits are black
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_str_aligned(
                    canvas, 11, 4, AlignCenter, AlignCenter, batteryPercentile);
            } else if(bar_charge >= 38 && bar_charge < 74) { // first digit is white
                canvas_set_color(canvas, ColorWhite);

                // first
                char batteryPercentileFirstDigit[2];
                snprintf(
                    batteryPercentileFirstDigit,
                    sizeof(batteryPercentileFirstDigit),
                    "%c",
                    batteryPercentile[0]);
                canvas_draw_str_aligned(
                    canvas, 9, 4, AlignCenter, AlignCenter, batteryPercentileFirstDigit);

                // second
                char batteryPercentileSecondDigit[2];
                snprintf(
                    batteryPercentileSecondDigit,
                    sizeof(batteryPercentileSecondDigit),
                    "%c",
                    batteryPercentile[1]);
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_str_aligned(
                    canvas, 15, 4, AlignCenter, AlignCenter, batteryPercentileSecondDigit);
            } else { // charge >= 74, both digits are white
                canvas_set_color(canvas, ColorWhite);
                canvas_draw_str_aligned(
                    canvas, 11, 4, AlignCenter, AlignCenter, batteryPercentile);
            }

        } else { //default bar display, added here to serve as fallback/default behaviour.
            canvas_draw_box(canvas, 2, 2, (power->info.charge + 4) / 5, 4);
        }

        // TODO: Verify if it displays correctly with custom battery skins !!!
        if(power->info.voltage_battery_charge_limit < 4.2) {
            // Battery charging voltage is modified, indicate with cross pattern
            canvas_invert_color(canvas);
            uint8_t battery_bar_width = (power->info.charge + 4) / 5;
            bool cross_odd = false;
            // Start 1 further in from the battery bar's x position
            for(uint8_t x = 3; x <= battery_bar_width; x++) {
                // Cross pattern is from the center of the battery bar
                // y = 2 + 1 (inset) + 1 (for every other)
                canvas_draw_dot(canvas, x, 3 + (uint8_t)cross_odd);
                cross_odd = !cross_odd;
            }
            canvas_invert_color(canvas);
        }

        if(power->state == PowerStateCharging) {
            canvas_set_bitmap_mode(canvas, 1);
            // TODO: replace -1 magic for uint8_t with re-framing
            if(power->displayBatteryPercentage == DISPLAY_BATTERY_PERCENT) {
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_box(canvas, 1, 1, 22, 6);
                canvas_draw_icon(canvas, 2, -1, &I_Charging_lightning_9x10);
                canvas_set_color(canvas, ColorWhite);
                canvas_draw_icon(canvas, 2, -1, &I_Charging_lightning_mask_9x10);
                canvas_set_font(canvas, FontBatteryPercent);
                canvas_draw_str_aligned(
                    canvas, 16, 4, AlignCenter, AlignCenter, batteryPercentile);
            } else if(power->displayBatteryPercentage == DISPLAY_BATTERY_INVERTED_PERCENT) {
                canvas_set_color(canvas, ColorWhite);
                canvas_draw_box(canvas, 1, 1, 22, 6);
                canvas_draw_icon(canvas, 2, -1, &I_Charging_lightning_9x10);
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_icon(canvas, 2, -1, &I_Charging_lightning_mask_9x10);
                canvas_set_font(canvas, FontBatteryPercent);
                canvas_draw_str_aligned(
                    canvas, 16, 4, AlignCenter, AlignCenter, batteryPercentile);
            } else if(power->displayBatteryPercentage == DISPLAY_BATTERY_BAR_PERCENT) {
                // clean-up default charging bar display
                canvas_set_color(canvas, ColorWhite);
                canvas_draw_box(canvas, 1, 1, 22, 6);

                // align charge display value with digits to draw
                uint8_t bar_charge = power->info.charge;

                if(bar_charge > 48 && bar_charge < 63) {
                    bar_charge = 48;
                } else if(bar_charge >= 63 && bar_charge < 84) {
                    bar_charge = 75;
                } else if(bar_charge >= 84 && bar_charge < 96) {
                    bar_charge = 96;
                }
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_box(canvas, 1, 1, (bar_charge * 22) / 100, 6);

                // drawing charge icon
                canvas_draw_icon(canvas, 2, -1, &I_Charging_lightning_9x10);
                canvas_set_color(canvas, ColorWhite);
                canvas_draw_icon(canvas, 2, -1, &I_Charging_lightning_mask_9x10);

                // drawing digits
                canvas_set_font(canvas, FontBatteryPercent);
                if(bar_charge < 64) { // both digits are black
                    canvas_set_color(canvas, ColorBlack);
                    canvas_draw_str_aligned(
                        canvas, 16, 4, AlignCenter, AlignCenter, batteryPercentile);
                } else if(bar_charge >= 64 && bar_charge < 84) { // first digit is white
                    canvas_set_color(canvas, ColorWhite);

                    // first
                    char batteryPercentileFirstDigit[2];
                    snprintf(
                        batteryPercentileFirstDigit,
                        sizeof(batteryPercentileFirstDigit),
                        "%c",
                        batteryPercentile[0]);
                    canvas_draw_str_aligned(
                        canvas, 14, 4, AlignCenter, AlignCenter, batteryPercentileFirstDigit);

                    // second
                    char batteryPercentileSecondDigit[2];
                    snprintf(
                        batteryPercentileSecondDigit,
                        sizeof(batteryPercentileSecondDigit),
                        "%c",
                        batteryPercentile[1]);
                    canvas_set_color(canvas, ColorBlack);
                    canvas_draw_str_aligned(
                        canvas, 20, 4, AlignCenter, AlignCenter, batteryPercentileSecondDigit);
                } else { // charge >= 84, both digits are white
                    canvas_set_color(canvas, ColorWhite);
                    canvas_draw_str_aligned(
                        canvas, 16, 4, AlignCenter, AlignCenter, batteryPercentile);
                }
            } else {
                canvas_set_color(canvas, ColorWhite);
                canvas_draw_icon(canvas, 8, -1, &I_Charging_lightning_mask_9x10);
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_icon(canvas, 8, -1, &I_Charging_lightning_9x10);
            }
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
    return battery_view_port;
}

static bool power_update_info(Power* power) {
    const PowerInfo info = {
        .is_charging = furi_hal_power_is_charging(),
        .gauge_is_ok = furi_hal_power_gauge_is_ok(),
        .is_shutdown_requested = furi_hal_power_is_shutdown_requested(),
        .is_otg_enabled = furi_hal_power_is_otg_enabled(),
        .charge = furi_hal_power_get_pct(),
        .health = furi_hal_power_get_bat_health_pct(),
        .capacity_remaining = furi_hal_power_get_battery_remaining_capacity(),
        .capacity_full = furi_hal_power_get_battery_full_capacity(),
        .current_charger = furi_hal_power_get_battery_current(FuriHalPowerICCharger),
        .current_gauge = furi_hal_power_get_battery_current(FuriHalPowerICFuelGauge),
        .voltage_battery_charge_limit = furi_hal_power_get_battery_charge_voltage_limit(),
        .voltage_charger = furi_hal_power_get_battery_voltage(FuriHalPowerICCharger),
        .voltage_gauge = furi_hal_power_get_battery_voltage(FuriHalPowerICFuelGauge),
        .voltage_vbus = furi_hal_power_get_usb_voltage(),
        .temperature_charger = furi_hal_power_get_battery_temperature(FuriHalPowerICCharger),
        .temperature_gauge = furi_hal_power_get_battery_temperature(FuriHalPowerICFuelGauge),
    };

    const bool need_refresh = (power->info.charge != info.charge) ||
                              (power->info.is_charging != info.is_charging);
    power->info = info;
    return need_refresh;
}

static void power_check_charging_state(Power* power) {
    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);

    if(furi_hal_power_is_charging()) {
        if((power->info.charge == 100) || (furi_hal_power_is_charging_done())) {
            if(power->state != PowerStateCharged) {
                notification_internal_message(notification, &sequence_charged);
                power->state = PowerStateCharged;
                power->event.type = PowerEventTypeFullyCharged;
                furi_pubsub_publish(power->event_pubsub, &power->event);
            }

        } else if(power->state != PowerStateCharging) {
            notification_internal_message(notification, &sequence_charging);
            power->state = PowerStateCharging;
            power->event.type = PowerEventTypeStartCharging;
            furi_pubsub_publish(power->event_pubsub, &power->event);
        }

    } else if(power->state != PowerStateNotCharging) {
        notification_internal_message(notification, &sequence_not_charging);
        power->state = PowerStateNotCharging;
        power->event.type = PowerEventTypeStopCharging;
        furi_pubsub_publish(power->event_pubsub, &power->event);
    }

    furi_record_close(RECORD_NOTIFICATION);
}

static void power_check_low_battery(Power* power) {
    if(!power->info.gauge_is_ok) {
        return;
    }

    // Check battery charge and vbus voltage
    if((power->info.is_shutdown_requested) &&
       (power->info.voltage_vbus < POWER_VBUS_LOW_THRESHOLD) && power->show_battery_low_warning) {
        if(!power->battery_low) {
            view_holder_send_to_front(power->view_holder);
            view_holder_set_view(power->view_holder, power_off_get_view(power->view_power_off));
        }
        power->battery_low = true;
    } else {
        if(power->battery_low) {
            // view_dispatcher_switch_to_view(power->view_dispatcher, VIEW_NONE);
            view_holder_set_view(power->view_holder, NULL);
            power->power_off_timeout = POWER_OFF_TIMEOUT_S;
        }
        power->battery_low = false;
    }
    // If battery low, update view and switch off power after timeout
    if(power->battery_low) {
        PowerOffResponse response = power_off_get_response(power->view_power_off);
        if(response == PowerOffResponseDefault) {
            if(power->power_off_timeout) {
                power_off_set_time_left(power->view_power_off, power->power_off_timeout--);
            } else {
                power_off(power);
            }
        } else if(response == PowerOffResponseOk) {
            power_off(power);
        } else if(response == PowerOffResponseHide) {
            view_holder_set_view(power->view_holder, NULL);
            if(power->power_off_timeout) {
                power_off_set_time_left(power->view_power_off, power->power_off_timeout--);
            } else {
                power_off(power);
            }
        } else if(response == PowerOffResponseCancel) {
            view_holder_set_view(power->view_holder, NULL);
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

void power_trigger_ui_update(Power* power) {
    DesktopSettings* settings = malloc(sizeof(DesktopSettings));
    desktop_settings_load(settings);
    power->displayBatteryPercentage = settings->displayBatteryPercentage;
    free(settings);
    view_port_update(power->battery_view_port);
}

static void power_handle_shutdown(Power* power) {
    furi_hal_power_off();
    // Notify user if USB is plugged
    view_holder_send_to_front(power->view_holder);
    view_holder_set_view(
        power->view_holder, power_unplug_usb_get_view(power->view_power_unplug_usb));
    furi_delay_ms(100);
    furi_halt("Disconnect USB for safe shutdown");
}

static void power_handle_reboot(PowerBootMode mode) {
    if(mode == PowerBootModeNormal) {
        update_operation_disarm();
    } else if(mode == PowerBootModeDfu) {
        furi_hal_rtc_set_boot_mode(FuriHalRtcBootModeDfu);
    } else if(mode == PowerBootModeUpdateStart) {
        furi_hal_rtc_set_boot_mode(FuriHalRtcBootModePreUpdate);
    } else {
        furi_crash();
    }

    furi_hal_power_reset();
}
// get settings from service to settings_app by send message to power queue
void power_api_get_settings(Power* power, PowerSettings* settings) {
    furi_assert(power);
    furi_assert(settings);

    PowerMessage msg = {
        .type = PowerMessageTypeGetSettings,
        .settings = settings,
        .lock = api_lock_alloc_locked(),
    };

    furi_check(
        furi_message_queue_put(power->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(msg.lock);
}

// set settings from settings_app to service by send message to power queue
void power_api_set_settings(Power* power, const PowerSettings* settings) {
    furi_assert(power);
    furi_assert(settings);

    PowerMessage msg = {
        .type = PowerMessageTypeSetSettings,
        .csettings = settings,
        .lock = api_lock_alloc_locked(),
    };

    furi_check(
        furi_message_queue_put(power->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(msg.lock);
}

//start furi timer for autopoweroff
static void power_start_auto_poweroff_timer(Power* power) {
    if(furi_timer_is_running(power->auto_poweroff_timer)) {
        furi_timer_stop(power->auto_poweroff_timer);
    }
    furi_timer_start(
        power->auto_poweroff_timer, furi_ms_to_ticks(power->settings.auto_poweroff_delay_ms));
}

//stop furi timer for autopoweroff
static void power_stop_auto_poweroff_timer(Power* power) {
    if(furi_timer_is_running(power->auto_poweroff_timer)) {
        furi_timer_stop(power->auto_poweroff_timer);
    }
}

static uint32_t power_is_running_auto_poweroff_timer(Power* power) {
    return furi_timer_is_running(power->auto_poweroff_timer);
}

// start|restart poweroff timer
static void power_auto_poweroff_callback(const void* value, void* context) {
    furi_assert(value);
    furi_assert(context);
    Power* power = context;
    power_start_auto_poweroff_timer(power);
}

// callback for poweroff timer (what we do when timer end)
static void power_auto_poweroff_timer_callback(void* context) {
    furi_assert(context);
    Power* power = context;

    //Dont poweroff device if charger connected
    if(furi_hal_power_is_charging()) {
        FURI_LOG_D(TAG, "We dont auto_power_off until battery is charging");
        power_start_auto_poweroff_timer(power);
    } else {
        power_off(power);
    }
}

//start|restart timer and events subscription and callbacks for input events (we restart timer when user press keys)
static void power_auto_poweroff_arm(Power* power) {
    if(power->settings.auto_poweroff_delay_ms) {
        if(power->input_events_subscription == NULL) {
            power->input_events_subscription = furi_pubsub_subscribe(
                power->input_events_pubsub, power_auto_poweroff_callback, power);
        }
        power_start_auto_poweroff_timer(power);
    }
}

// stop timer and event subscription
static void power_auto_poweroff_disarm(Power* power) {
    power_stop_auto_poweroff_timer(power);
    if(power->input_events_subscription) {
        furi_pubsub_unsubscribe(power->input_events_pubsub, power->input_events_subscription);
        power->input_events_subscription = NULL;
    }
}

//check message queue from Loader - is some app started or not (if started we dont do auto poweroff)
static void power_loader_callback(const void* message, void* context) {
    furi_assert(context);
    Power* power = context;
    const LoaderEvent* event = message;

    // disarm timer if some apps started
    if(event->type == LoaderEventTypeApplicationBeforeLoad) {
        power->app_running = true;
        power_auto_poweroff_disarm(power);
        // arm timer if some apps was not loaded or was stoped
    } else if(
        event->type == LoaderEventTypeApplicationLoadFailed ||
        event->type == LoaderEventTypeApplicationStopped) {
        power->app_running = false;
        power_auto_poweroff_arm(power);
    }
}

// apply power settings
static void power_settings_apply(Power* power) {
    //apply auto_poweroff settings
    if(power->settings.auto_poweroff_delay_ms && !power->app_running) {
        power_auto_poweroff_arm(power);
    } else if(power_is_running_auto_poweroff_timer(power)) {
        power_auto_poweroff_disarm(power);
    }
}

// do something depend from power queue message
static void power_message_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    Power* power = context;

    furi_assert(object == power->message_queue);

    PowerMessage msg;
    furi_check(furi_message_queue_get(power->message_queue, &msg, 0) == FuriStatusOk);

    switch(msg.type) {
    case PowerMessageTypeShutdown:
        power_handle_shutdown(power);
        break;
    case PowerMessageTypeReboot:
        power_handle_reboot(msg.boot_mode);
        break;
    case PowerMessageTypeGetInfo:
        *msg.power_info = power->info;
        break;
    case PowerMessageTypeIsBatteryHealthy:
        *msg.bool_param = power->info.health > POWER_HEALTH_LOW_THRESHOLD;
        break;
    case PowerMessageTypeShowBatteryLowWarning:
        power->show_battery_low_warning = *msg.bool_param;
        break;
    case PowerMessageTypeGetSettings:
        furi_assert(msg.lock);
        *msg.settings = power->settings;
        break;
    case PowerMessageTypeSetSettings:
        furi_assert(msg.lock);
        power->settings = *msg.csettings;
        power_settings_apply(power);
        power_settings_save(&power->settings);
        break;
    case PowerMessageTypeReloadSettings:
        power_settings_load(&power->settings);
        power_settings_apply(power);
        break;
    case PowerMessageTypeSwitchOTG:
        power->is_otg_requested = *msg.bool_param;
        if(power->is_otg_requested) {
            // Only try to enable if VBUS voltage is low, otherwise charger will refuse
            if(power->info.voltage_vbus < 4.5f) {
                size_t retries = 5;
                while(retries-- > 0) {
                    if(furi_hal_power_enable_otg()) {
                        break;
                    }
                }
                if(!retries) {
                    FURI_LOG_W(TAG, "Failed to enable OTG, will try later");
                }
            } else {
                FURI_LOG_W(
                    TAG,
                    "Postponing OTG enable: VBUS(%0.1f) >= 4.5v",
                    (double)power->info.voltage_vbus);
            }
        } else {
            furi_hal_power_disable_otg();
        }
        break;
    default:
        furi_crash();
    }

    if(msg.lock) {
        api_lock_unlock(msg.lock);
    }
}

static void power_charge_supress(Power* power) {
    // if charge_supress_percent selected (not OFF) and current charge level equal or higher than selected level
    // then we start supression if we not supress it before.
    if(power->settings.charge_supress_percent &&
       power->info.charge >= power->settings.charge_supress_percent) {
        if(!power->charge_is_supressed) {
            power->charge_is_supressed = true;
            furi_hal_power_suppress_charge_enter();
        }
        // disable supression if charge_supress_percent OFF but charge still supressed
    } else {
        if(power->charge_is_supressed) {
            power->charge_is_supressed = false;
            furi_hal_power_suppress_charge_exit();
        }
    }
}

static void power_tick_callback(void* context) {
    furi_assert(context);
    Power* power = context;

    // Update data from gauge and charger
    const bool need_refresh = power_update_info(power);
    // Check low battery level
    power_check_low_battery(power);
    // Check and notify about charging state
    power_check_charging_state(power);
    // Check and notify about battery level change
    power_check_battery_level_change(power);
    // charge supress arm/disarm
    power_charge_supress(power);
    // Update battery view port
    if(need_refresh) {
        view_port_update(power->battery_view_port);
    }
    // Check OTG status, disable in case of a fault
    if(furi_hal_power_check_otg_fault()) {
        FURI_LOG_E(TAG, "OTG fault detected, disabling OTG");
        furi_hal_power_disable_otg();
        power->is_otg_requested = false;
    }

    // Change OTG state if needed (i.e. after disconnecting USB power)
    if(power->is_otg_requested &&
       (!power->info.is_otg_enabled && power->info.voltage_vbus < 4.5f)) {
        FURI_LOG_D(TAG, "OTG requested but not enabled, enabling OTG");
        furi_hal_power_enable_otg();
    }
}

static void power_storage_callback(const void* message, void* context) {
    furi_assert(context);
    Power* power = context;
    const StorageEvent* event = message;

    if(event->type == StorageEventTypeCardMount) {
        PowerMessage msg = {
            .type = PowerMessageTypeReloadSettings,
        };

        furi_check(
            furi_message_queue_put(power->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
    }
}

// loading and initializing power service settings
static void power_init_settings(Power* power) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    furi_pubsub_subscribe(storage_get_pubsub(storage), power_storage_callback, power);

    if(storage_sd_status(storage) != FSE_OK) {
        FURI_LOG_D(TAG, "SD Card not ready, skipping settings");
        return;
    }

    power_settings_load(&power->settings);
    power_settings_apply(power);
    furi_record_close(RECORD_STORAGE);
    power->charge_is_supressed = false;
}

static Power* power_alloc(void) {
    Power* power = malloc(sizeof(Power));
    // Pubsub
    power->event_pubsub = furi_pubsub_alloc();
    // State initialization
    power->power_off_timeout = POWER_OFF_TIMEOUT_S;
    power->show_battery_low_warning = true;

    // Load UI settings
    DesktopSettings* settings = malloc(sizeof(DesktopSettings));
    desktop_settings_load(settings);
    power->displayBatteryPercentage = settings->displayBatteryPercentage;
    free(settings);

    // auto_poweroff
    //---define subscription to loader events message (info about started apps) and define callback for this
    Loader* loader = furi_record_open(RECORD_LOADER);
    furi_pubsub_subscribe(loader_get_pubsub(loader), power_loader_callback, power);
    power->input_events_pubsub = furi_record_open(RECORD_INPUT_EVENTS);
    //define autopoweroff timer and they callback
    power->auto_poweroff_timer =
        furi_timer_alloc(power_auto_poweroff_timer_callback, FuriTimerTypeOnce, power);

    // Gui
    Gui* gui = furi_record_open(RECORD_GUI);

    power->view_holder = view_holder_alloc();
    power->view_power_off = power_off_alloc();
    power->view_power_unplug_usb = power_unplug_usb_alloc();

    view_holder_attach_to_gui(power->view_holder, gui);
    // Battery view port
    power->battery_view_port = power_battery_view_port_alloc(power);
    gui_add_view_port(gui, power->battery_view_port, GuiLayerStatusBarRight);
    // Event loop
    power->event_loop = furi_event_loop_alloc();
    power->message_queue = furi_message_queue_alloc(4, sizeof(PowerMessage));

    furi_event_loop_subscribe_message_queue(
        power->event_loop,
        power->message_queue,
        FuriEventLoopEventIn,
        power_message_callback,
        power);
    furi_event_loop_tick_set(power->event_loop, 1000, power_tick_callback, power);

    return power;
}

int32_t power_srv(void* p) {
    UNUSED(p);

    if(furi_hal_rtc_get_boot_mode() != FuriHalRtcBootModeNormal) {
        FURI_LOG_W(TAG, "Skipping start in special boot mode");

        furi_thread_suspend(furi_thread_get_current_id());
        return 0;
    }

    Power* power = power_alloc();

    // power service settings initialization
    power_init_settings(power);

    power_update_info(power);

    furi_record_create(RECORD_POWER, power);

    // Can't be done in alloc, other things in startup need power service and it would deadlock by waiting for loader
    Loader* loader = furi_record_open(RECORD_LOADER);
    power->app_running = loader_is_locked(loader);
    furi_record_close(RECORD_LOADER);

    furi_event_loop_run(power->event_loop);

    return 0;
}
