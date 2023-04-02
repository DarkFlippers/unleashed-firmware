#include "common.h"
#include <furi/furi.h>
#include <furi_hal.h>
#include "../../services/convert/convert.h"

static const uint8_t hid_number_keys[10] = {
    HID_KEYBOARD_0,
    HID_KEYBOARD_1,
    HID_KEYBOARD_2,
    HID_KEYBOARD_3,
    HID_KEYBOARD_4,
    HID_KEYBOARD_5,
    HID_KEYBOARD_6,
    HID_KEYBOARD_7,
    HID_KEYBOARD_8,
    HID_KEYBOARD_9};

static uint32_t get_keystroke_delay(TokenAutomationFeature features) {
    if(features & TOKEN_AUTOMATION_FEATURE_TYPE_SLOWER) {
        return 100;
    }

    return 30;
}

static uint32_t get_keypress_delay(TokenAutomationFeature features) {
    if(features & TOKEN_AUTOMATION_FEATURE_TYPE_SLOWER) {
        return 60;
    }

    return 30;
}

static void totp_type_code_worker_press_key(
    uint8_t key,
    TOTP_AUTOMATION_KEY_HANDLER key_press_fn,
    TOTP_AUTOMATION_KEY_HANDLER key_release_fn,
    TokenAutomationFeature features) {
    (*key_press_fn)(key);
    furi_delay_ms(get_keypress_delay(features));
    (*key_release_fn)(key);
}

void totp_type_code_worker_execute_automation(
    TOTP_AUTOMATION_KEY_HANDLER key_press_fn,
    TOTP_AUTOMATION_KEY_HANDLER key_release_fn,
    const char* string,
    uint8_t string_length,
    TokenAutomationFeature features) {
    furi_delay_ms(500);
    uint8_t i = 0;
    while(i < string_length && string[i] != 0) {
        uint8_t digit = CONVERT_CHAR_TO_DIGIT(string[i]);
        if(digit > 9) break;
        uint8_t hid_kb_key = hid_number_keys[digit];
        totp_type_code_worker_press_key(hid_kb_key, key_press_fn, key_release_fn, features);
        furi_delay_ms(get_keystroke_delay(features));
        i++;
    }

    if(features & TOKEN_AUTOMATION_FEATURE_ENTER_AT_THE_END) {
        furi_delay_ms(get_keystroke_delay(features));
        totp_type_code_worker_press_key(
            HID_KEYBOARD_RETURN, key_press_fn, key_release_fn, features);
    }

    if(features & TOKEN_AUTOMATION_FEATURE_TAB_AT_THE_END) {
        furi_delay_ms(get_keystroke_delay(features));
        totp_type_code_worker_press_key(HID_KEYBOARD_TAB, key_press_fn, key_release_fn, features);
    }
}