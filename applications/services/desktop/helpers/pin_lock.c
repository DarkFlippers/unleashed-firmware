
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <stddef.h>
#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>

#include "../helpers/pin_lock.h"
#include "../desktop_i.h"
#include <cli/cli.h>
#include <cli/cli_vcp.h>

static const NotificationSequence sequence_pin_fail = {
    &message_display_backlight_on,

    &message_red_255,
    &message_vibro_on,
    &message_delay_100,
    &message_vibro_off,
    &message_red_0,

    &message_delay_250,

    &message_red_255,
    &message_vibro_on,
    &message_delay_100,
    &message_vibro_off,
    &message_red_0,
    NULL,
};

static const uint8_t desktop_helpers_fails_timeout[] = {
    0,
    0,
    0,
    0,
    30,
    60,
    90,
    120,
    150,
    180,
    /* +60 for every next fail */
};

void desktop_pin_lock_error_notify() {
    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
    notification_message(notification, &sequence_pin_fail);
    furi_record_close(RECORD_NOTIFICATION);
}

uint32_t desktop_pin_lock_get_fail_timeout() {
    uint32_t pin_fails = furi_hal_rtc_get_pin_fails();
    uint32_t pin_timeout = 0;
    uint32_t max_index = COUNT_OF(desktop_helpers_fails_timeout) - 1;
    if(pin_fails <= max_index) {
        pin_timeout = desktop_helpers_fails_timeout[pin_fails];
    } else {
        pin_timeout = desktop_helpers_fails_timeout[max_index] + (pin_fails - max_index) * 60;
    }

    return pin_timeout;
}

void desktop_pin_lock(DesktopSettings* settings) {
    furi_assert(settings);

    furi_hal_rtc_set_pin_fails(0);
    furi_hal_rtc_set_flag(FuriHalRtcFlagLock);
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_session_close(cli);
    furi_record_close(RECORD_CLI);
    settings->is_locked = 1;
    DESKTOP_SETTINGS_SAVE(settings);
}

void desktop_pin_unlock(DesktopSettings* settings) {
    furi_assert(settings);

    furi_hal_rtc_reset_flag(FuriHalRtcFlagLock);
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_session_open(cli, &cli_vcp);
    furi_record_close(RECORD_CLI);
    settings->is_locked = 0;
    DESKTOP_SETTINGS_SAVE(settings);
}

void desktop_pin_lock_init(DesktopSettings* settings) {
    furi_assert(settings);

    if(settings->pin_code.length > 0) {
        if(settings->is_locked == 1) {
            furi_hal_rtc_set_flag(FuriHalRtcFlagLock);
        } else {
            if(desktop_pin_lock_is_locked()) {
                settings->is_locked = 1;
                DESKTOP_SETTINGS_SAVE(settings);
            }
        }
    } else {
        furi_hal_rtc_set_pin_fails(0);
        furi_hal_rtc_reset_flag(FuriHalRtcFlagLock);
    }

    if(desktop_pin_lock_is_locked()) {
        Cli* cli = furi_record_open(RECORD_CLI);
        cli_session_close(cli);
        furi_record_close(RECORD_CLI);
    }
}

bool desktop_pin_lock_verify(const PinCode* pin_set, const PinCode* pin_entered) {
    bool result = false;
    if(desktop_pins_are_equal(pin_set, pin_entered)) {
        furi_hal_rtc_set_pin_fails(0);
        result = true;
    } else {
        uint32_t pin_fails = furi_hal_rtc_get_pin_fails();
        furi_hal_rtc_set_pin_fails(pin_fails + 1);
        result = false;
    }
    return result;
}

bool desktop_pin_lock_is_locked() {
    return furi_hal_rtc_is_flag_set(FuriHalRtcFlagLock);
}

bool desktop_pins_are_equal(const PinCode* pin_code1, const PinCode* pin_code2) {
    furi_assert(pin_code1);
    furi_assert(pin_code2);
    bool result = false;

    if(pin_code1->length == pin_code2->length) {
        result = !memcmp(pin_code1->data, pin_code2->data, pin_code1->length);
    }

    return result;
}
