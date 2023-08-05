#include "type_code_common.h"
#include <furi_hal_usb_hid.h>
#include <furi/core/kernel.h>
#include "../../services/convert/convert.h"

#define HID_KEYS_MAP_LENGTH (36)

static const uint8_t hid_qwerty_keys_map[HID_KEYS_MAP_LENGTH] = {
    HID_KEYBOARD_0, HID_KEYBOARD_1, HID_KEYBOARD_2, HID_KEYBOARD_3, HID_KEYBOARD_4,
    HID_KEYBOARD_5, HID_KEYBOARD_6, HID_KEYBOARD_7, HID_KEYBOARD_8, HID_KEYBOARD_9,
    HID_KEYBOARD_A, HID_KEYBOARD_B, HID_KEYBOARD_C, HID_KEYBOARD_D, HID_KEYBOARD_E,
    HID_KEYBOARD_F, HID_KEYBOARD_G, HID_KEYBOARD_H, HID_KEYBOARD_I, HID_KEYBOARD_J,
    HID_KEYBOARD_K, HID_KEYBOARD_L, HID_KEYBOARD_M, HID_KEYBOARD_N, HID_KEYBOARD_O,
    HID_KEYBOARD_P, HID_KEYBOARD_Q, HID_KEYBOARD_R, HID_KEYBOARD_S, HID_KEYBOARD_T,
    HID_KEYBOARD_U, HID_KEYBOARD_V, HID_KEYBOARD_W, HID_KEYBOARD_X, HID_KEYBOARD_Y,
    HID_KEYBOARD_Z};

static const uint8_t hid_azerty_keys_map[HID_KEYS_MAP_LENGTH] = {
    HID_KEYBOARD_0, HID_KEYBOARD_1, HID_KEYBOARD_2,         HID_KEYBOARD_3, HID_KEYBOARD_4,
    HID_KEYBOARD_5, HID_KEYBOARD_6, HID_KEYBOARD_7,         HID_KEYBOARD_8, HID_KEYBOARD_9,
    HID_KEYBOARD_Q, HID_KEYBOARD_B, HID_KEYBOARD_C,         HID_KEYBOARD_D, HID_KEYBOARD_E,
    HID_KEYBOARD_F, HID_KEYBOARD_G, HID_KEYBOARD_H,         HID_KEYBOARD_I, HID_KEYBOARD_J,
    HID_KEYBOARD_K, HID_KEYBOARD_L, HID_KEYBOARD_SEMICOLON, HID_KEYBOARD_N, HID_KEYBOARD_O,
    HID_KEYBOARD_P, HID_KEYBOARD_A, HID_KEYBOARD_R,         HID_KEYBOARD_S, HID_KEYBOARD_T,
    HID_KEYBOARD_U, HID_KEYBOARD_V, HID_KEYBOARD_Z,         HID_KEYBOARD_X, HID_KEYBOARD_Y,
    HID_KEYBOARD_W};

static uint32_t get_keystroke_delay(TokenAutomationFeature features) {
    if(features & TokenAutomationFeatureTypeSlower) {
        return 100;
    }

    return 30;
}

static uint32_t get_keypress_delay(TokenAutomationFeature features) {
    if(features & TokenAutomationFeatureTypeSlower) {
        return 60;
    }

    return 30;
}

static void totp_type_code_worker_press_key(
    uint16_t key,
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
    const char* code_buffer,
    uint8_t code_buffer_size,
    TokenAutomationFeature features,
    AutomationKeyboardLayout keyboard_layout) {
    furi_delay_ms(500);
    uint8_t i = 0;
    char cb_char;

    const uint8_t* keyboard_layout_dict;
    switch(keyboard_layout) {
    case AutomationKeyboardLayoutQWERTY:
        keyboard_layout_dict = &hid_qwerty_keys_map[0];
        break;
    case AutomationKeyboardLayoutAZERTY:
        keyboard_layout_dict = &hid_azerty_keys_map[0];
        break;

    default:
        return;
    }

    while(i < code_buffer_size && (cb_char = code_buffer[i]) != 0) {
        uint8_t char_index = CONVERT_CHAR_TO_DIGIT(cb_char);
        if(char_index > 9) {
            char_index = cb_char - 'A' + 10;
        }

        if(char_index >= HID_KEYS_MAP_LENGTH) break;

        uint16_t hid_kb_key = keyboard_layout_dict[char_index];

        // For non-AZERTY press shift for all non-digit chars
        // For AZERTY press shift for all characters
        if(char_index > 9 || keyboard_layout == AutomationKeyboardLayoutAZERTY) {
            hid_kb_key |= KEY_MOD_LEFT_SHIFT;
        }

        totp_type_code_worker_press_key(hid_kb_key, key_press_fn, key_release_fn, features);
        furi_delay_ms(get_keystroke_delay(features));
        i++;
    }

    if(features & TokenAutomationFeatureEnterAtTheEnd) {
        furi_delay_ms(get_keystroke_delay(features));
        totp_type_code_worker_press_key(
            HID_KEYBOARD_RETURN, key_press_fn, key_release_fn, features);
    }

    if(features & TokenAutomationFeatureTabAtTheEnd) {
        furi_delay_ms(get_keystroke_delay(features));
        totp_type_code_worker_press_key(HID_KEYBOARD_TAB, key_press_fn, key_release_fn, features);
    }
}