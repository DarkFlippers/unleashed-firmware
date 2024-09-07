#include "pin_code.h"

#include <furi_hal_rtc.h>

#include <furi.h>
#include <notification/notification_messages.h>

#define DESKTOP_PIN_CODE_DIGIT_BIT_WIDTH (2)
#define DESKTOP_PIN_CODE_LENGTH_OFFSET   (28)

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

static uint32_t desktop_pin_code_pack(const DesktopPinCode* pin_code) {
    furi_check(pin_code);
    furi_check(pin_code->length <= sizeof(pin_code->data));

    uint32_t reg_value = 0;

    for(uint8_t i = 0; i < pin_code->length; ++i) {
        furi_check(pin_code->data[i] < (1 << DESKTOP_PIN_CODE_DIGIT_BIT_WIDTH));
        reg_value |= (uint32_t)pin_code->data[i] << (i * DESKTOP_PIN_CODE_DIGIT_BIT_WIDTH);
    }

    reg_value |= (uint32_t)pin_code->length << DESKTOP_PIN_CODE_LENGTH_OFFSET;

    return reg_value;
}

bool desktop_pin_code_is_set(void) {
    uint8_t length = furi_hal_rtc_get_pin_value() >> DESKTOP_PIN_CODE_LENGTH_OFFSET;
    return length >= DESKTOP_PIN_CODE_MIN_LEN && length <= DESKTOP_PIN_CODE_MAX_LEN;
}

void desktop_pin_code_set(const DesktopPinCode* pin_code) {
    furi_hal_rtc_set_pin_value(desktop_pin_code_pack(pin_code));
}

void desktop_pin_code_reset(void) {
    furi_hal_rtc_set_pin_value(0);
}

bool desktop_pin_code_check(const DesktopPinCode* pin_code) {
    return furi_hal_rtc_get_pin_value() == desktop_pin_code_pack(pin_code);
}

bool desktop_pin_code_is_equal(const DesktopPinCode* pin_code1, const DesktopPinCode* pin_code2) {
    furi_check(pin_code1);
    furi_check(pin_code1->length <= sizeof(pin_code1->data));
    furi_check(pin_code2);
    furi_check(pin_code2->length <= sizeof(pin_code2->data));

    return pin_code1->length == pin_code2->length &&
           memcmp(pin_code1->data, pin_code2->data, pin_code1->length) == 0;
}

void desktop_pin_lock_error_notify(void) {
    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
    notification_message(notification, &sequence_pin_fail);
    furi_record_close(RECORD_NOTIFICATION);
}

uint32_t desktop_pin_lock_get_fail_timeout(void) {
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
