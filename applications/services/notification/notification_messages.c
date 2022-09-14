#include "furi_hal_resources.h"
#include "notification.h"
#include "notification_messages_notes.h"
#include <stddef.h>

/*********************************** Messages **********************************/

/** Display: backlight wakeup */
const NotificationMessage message_display_backlight_on = {
    .type = NotificationMessageTypeLedDisplayBacklight,
    .data.led.value = 0xFF,
};

/** Display: backlight force off */
const NotificationMessage message_display_backlight_off = {
    .type = NotificationMessageTypeLedDisplayBacklight,
    .data.led.value = 0x00,
};

/** Display: backlight always on */
const NotificationMessage message_display_backlight_enforce_on = {
    .type = NotificationMessageTypeLedDisplayBacklightEnforceOn,
    .data.led.value = 0xFF,
};

/** Display: automatic backlight management, with configured timeout */
const NotificationMessage message_display_backlight_enforce_auto = {
    .type = NotificationMessageTypeLedDisplayBacklightEnforceAuto,
    .data.led.value = 0x00,
};

// Led ON
const NotificationMessage message_red_255 = {
    .type = NotificationMessageTypeLedRed,
    .data.led.value = 0xFF,
};

const NotificationMessage message_green_255 = {
    .type = NotificationMessageTypeLedGreen,
    .data.led.value = 0xFF,
};

const NotificationMessage message_blue_255 = {
    .type = NotificationMessageTypeLedBlue,
    .data.led.value = 0xFF,
};

// Led OFF
const NotificationMessage message_red_0 = {
    .type = NotificationMessageTypeLedRed,
    .data.led.value = 0x00,
};

const NotificationMessage message_green_0 = {
    .type = NotificationMessageTypeLedGreen,
    .data.led.value = 0x00,
};

const NotificationMessage message_blue_0 = {
    .type = NotificationMessageTypeLedBlue,
    .data.led.value = 0x00,
};

const NotificationMessage message_blink_start_10 = {
    .type = NotificationMessageTypeLedBlinkStart,
    .data.led_blink.color = 0,
    .data.led_blink.on_time = 10,
    .data.led_blink.period = 100,
};

const NotificationMessage message_blink_start_100 = {
    .type = NotificationMessageTypeLedBlinkStart,
    .data.led_blink.color = 0,
    .data.led_blink.on_time = 100,
    .data.led_blink.period = 1000,
};

const NotificationMessage message_blink_stop = {
    .type = NotificationMessageTypeLedBlinkStop,
};

const NotificationMessage message_blink_set_color_red = {
    .type = NotificationMessageTypeLedBlinkColor,
    .data.led_blink.color = LightRed,
};

const NotificationMessage message_blink_set_color_green = {
    .type = NotificationMessageTypeLedBlinkColor,
    .data.led_blink.color = LightGreen,
};

const NotificationMessage message_blink_set_color_blue = {
    .type = NotificationMessageTypeLedBlinkColor,
    .data.led_blink.color = LightBlue,
};

const NotificationMessage message_blink_set_color_cyan = {
    .type = NotificationMessageTypeLedBlinkColor,
    .data.led_blink.color = LightBlue | LightGreen,
};

const NotificationMessage message_blink_set_color_magenta = {
    .type = NotificationMessageTypeLedBlinkColor,
    .data.led_blink.color = LightBlue | LightRed,
};

const NotificationMessage message_blink_set_color_yellow = {
    .type = NotificationMessageTypeLedBlinkColor,
    .data.led_blink.color = LightGreen | LightRed,
};

const NotificationMessage message_blink_set_color_white = {
    .type = NotificationMessageTypeLedBlinkColor,
    .data.led_blink.color = LightRed | LightGreen | LightBlue,
};

// Delay
const NotificationMessage message_delay_1 = {
    .type = NotificationMessageTypeDelay,
    .data.delay.length = 1,
};

const NotificationMessage message_delay_10 = {
    .type = NotificationMessageTypeDelay,
    .data.delay.length = 10,
};

const NotificationMessage message_delay_25 = {
    .type = NotificationMessageTypeDelay,
    .data.delay.length = 25,
};

const NotificationMessage message_delay_50 = {
    .type = NotificationMessageTypeDelay,
    .data.delay.length = 50,
};

const NotificationMessage message_delay_100 = {
    .type = NotificationMessageTypeDelay,
    .data.delay.length = 100,
};

const NotificationMessage message_delay_250 = {
    .type = NotificationMessageTypeDelay,
    .data.delay.length = 250,
};

const NotificationMessage message_delay_500 = {
    .type = NotificationMessageTypeDelay,
    .data.delay.length = 500,
};

const NotificationMessage message_delay_1000 = {
    .type = NotificationMessageTypeDelay,
    .data.delay.length = 1000,
};

// Sound
const NotificationMessage message_sound_off = {
    .type = NotificationMessageTypeSoundOff,
};

// Vibro
const NotificationMessage message_vibro_on = {
    .type = NotificationMessageTypeVibro,
    .data.vibro.on = true,
};

const NotificationMessage message_vibro_off = {
    .type = NotificationMessageTypeVibro,
    .data.vibro.on = false,
};

// Reset
const NotificationMessage message_do_not_reset = {
    .type = NotificationMessageTypeDoNotReset,
};

// Override user settings
const NotificationMessage message_force_speaker_volume_setting_1f = {
    .type = NotificationMessageTypeForceSpeakerVolumeSetting,
    .data.forced_settings.speaker_volume = 1.0f,
};

const NotificationMessage message_force_vibro_setting_on = {
    .type = NotificationMessageTypeForceVibroSetting,
    .data.forced_settings.vibro = true,
};

const NotificationMessage message_force_vibro_setting_off = {
    .type = NotificationMessageTypeForceVibroSetting,
    .data.forced_settings.vibro = false,
};

const NotificationMessage message_force_display_brightness_setting_1f = {
    .type = NotificationMessageTypeForceDisplayBrightnessSetting,
    .data.forced_settings.display_brightness = 1.0f,
};

/****************************** Message sequences ******************************/

// Reset
const NotificationSequence sequence_reset_red = {
    &message_red_0,
    NULL,
};

const NotificationSequence sequence_reset_green = {
    &message_green_0,
    NULL,
};

const NotificationSequence sequence_reset_blue = {
    &message_blue_0,
    NULL,
};

const NotificationSequence sequence_reset_rgb = {
    &message_red_0,
    &message_blue_0,
    &message_green_0,
    NULL,
};

const NotificationSequence sequence_reset_display = {
    &message_display_backlight_off,
    NULL,
};

const NotificationSequence sequence_reset_sound = {
    &message_sound_off,
    NULL,
};

const NotificationSequence sequence_reset_vibro = {
    &message_vibro_off,
    NULL,
};

// Vibro
const NotificationSequence sequence_set_vibro_on = {
    &message_vibro_on,
    &message_do_not_reset,
    NULL,
};

// Display
const NotificationSequence sequence_display_backlight_on = {
    &message_display_backlight_on,
    NULL,
};

const NotificationSequence sequence_display_backlight_off = {
    &message_display_backlight_off,
    NULL,
};

/** Display: backlight always on lock */
const NotificationSequence sequence_display_backlight_enforce_on = {
    &message_display_backlight_enforce_on,
    NULL,
};

/** Display: backlight always on unlock */
const NotificationSequence sequence_display_backlight_enforce_auto = {
    &message_display_backlight_enforce_auto,
    NULL,
};

const NotificationSequence sequence_display_backlight_off_delay_1000 = {
    &message_delay_1000,
    &message_display_backlight_off,
    NULL,
};

// Charging
const NotificationSequence sequence_charging = {
    &message_red_255,
    &message_green_0,
    NULL,
};

const NotificationSequence sequence_charged = {
    &message_green_255,
    &message_red_0,
    NULL,
};

const NotificationSequence sequence_not_charging = {
    &message_red_0,
    &message_green_0,
    NULL,
};

// Light up
const NotificationSequence sequence_set_only_red_255 = {
    &message_red_255,
    &message_green_0,
    &message_blue_0,
    &message_do_not_reset,
    NULL,
};

const NotificationSequence sequence_set_only_green_255 = {
    &message_red_0,
    &message_green_255,
    &message_blue_0,
    &message_do_not_reset,
    NULL,
};

const NotificationSequence sequence_set_only_blue_255 = {
    &message_red_0,
    &message_green_0,
    &message_blue_255,
    &message_do_not_reset,
    NULL,
};

const NotificationSequence sequence_set_red_255 = {
    &message_red_255,
    &message_do_not_reset,
    NULL,
};

const NotificationSequence sequence_set_green_255 = {
    &message_green_255,
    &message_do_not_reset,
    NULL,
};

const NotificationSequence sequence_set_blue_255 = {
    &message_blue_255,
    &message_do_not_reset,
    NULL,
};

// Solid colors
const NotificationSequence sequence_solid_yellow = {
    &message_red_255,
    &message_green_255,
    &message_blue_0,
    &message_do_not_reset,
    NULL,
};

// Blink
const NotificationSequence sequence_blink_blue_10 = {
    &message_blue_255,
    &message_delay_10,
    NULL,
};

const NotificationSequence sequence_blink_red_10 = {
    &message_red_255,
    &message_delay_10,
    NULL,
};

const NotificationSequence sequence_blink_green_10 = {
    &message_green_255,
    &message_delay_10,
    NULL,
};

const NotificationSequence sequence_blink_yellow_10 = {
    &message_red_255,
    &message_green_255,
    &message_delay_10,
    NULL,
};

const NotificationSequence sequence_blink_cyan_10 = {
    &message_green_255,
    &message_blue_255,
    &message_delay_10,
    NULL,
};

const NotificationSequence sequence_blink_magenta_10 = {
    &message_red_255,
    &message_blue_255,
    &message_delay_10,
    NULL,
};

const NotificationSequence sequence_blink_red_100 = {
    &message_red_255,
    &message_delay_100,
    NULL,
};

const NotificationSequence sequence_blink_green_100 = {
    &message_green_255,
    &message_delay_100,
    NULL,
};

const NotificationSequence sequence_blink_blue_100 = {
    &message_blue_255,
    &message_delay_100,
    NULL,
};

const NotificationSequence sequence_blink_yellow_100 = {
    &message_red_255,
    &message_green_255,
    &message_delay_100,
    NULL,
};

const NotificationSequence sequence_blink_cyan_100 = {
    &message_green_255,
    &message_blue_255,
    &message_delay_100,
    NULL,
};

const NotificationSequence sequence_blink_magenta_100 = {
    &message_red_255,
    &message_blue_255,
    &message_delay_100,
    NULL,
};

const NotificationSequence sequence_blink_white_100 = {
    &message_red_255,
    &message_green_255,
    &message_blue_255,
    &message_delay_100,
    NULL,
};

// Hardware blink
const NotificationSequence sequence_blink_start_blue = {
    &message_blink_start_10,
    &message_blink_set_color_blue,
    &message_do_not_reset,
    NULL,
};

const NotificationSequence sequence_blink_start_red = {
    &message_blink_start_10,
    &message_blink_set_color_red,
    &message_do_not_reset,
    NULL,
};

const NotificationSequence sequence_blink_start_green = {
    &message_blink_start_10,
    &message_blink_set_color_green,
    &message_do_not_reset,
    NULL,
};

const NotificationSequence sequence_blink_start_yellow = {
    &message_blink_start_10,
    &message_blink_set_color_yellow,
    &message_do_not_reset,
    NULL,
};

const NotificationSequence sequence_blink_start_cyan = {
    &message_blink_start_10,
    &message_blink_set_color_cyan,
    &message_do_not_reset,
    NULL,
};

const NotificationSequence sequence_blink_start_magenta = {
    &message_blink_start_10,
    &message_blink_set_color_magenta,
    &message_do_not_reset,
    NULL,
};

const NotificationSequence sequence_blink_stop = {
    &message_blink_stop,
    NULL,
};

//General
const NotificationSequence sequence_single_vibro = {
    &message_vibro_on,
    &message_delay_100,
    &message_vibro_off,
    NULL,
};

const NotificationSequence sequence_double_vibro = {
    &message_vibro_on,
    &message_delay_100,
    &message_vibro_off,
    &message_delay_100,
    &message_vibro_on,
    &message_delay_100,
    &message_vibro_off,
    NULL,
};

const NotificationSequence sequence_success = {
    &message_display_backlight_on,
    &message_green_255,
    &message_vibro_on,
    &message_note_c5,
    &message_delay_50,
    &message_vibro_off,
    &message_note_e5,
    &message_delay_50,
    &message_note_g5,
    &message_delay_50,
    &message_note_c6,
    &message_delay_50,
    &message_sound_off,
    NULL,
};

const NotificationSequence sequence_error = {
    &message_display_backlight_on,
    &message_red_255,
    &message_vibro_on,
    &message_note_c5,
    &message_delay_100,
    &message_vibro_off,
    &message_sound_off,
    &message_delay_100,
    &message_vibro_on,
    &message_note_c5,
    &message_delay_100,
    &message_vibro_off,
    &message_sound_off,
    NULL,
};

const NotificationSequence sequence_audiovisual_alert = {
    &message_force_speaker_volume_setting_1f,
    &message_force_vibro_setting_on,
    &message_force_display_brightness_setting_1f,
    &message_vibro_on,

    &message_display_backlight_on,
    &message_note_c7,
    &message_delay_250,

    &message_display_backlight_off,
    &message_note_c4,
    &message_delay_250,

    &message_display_backlight_on,
    &message_note_c7,
    &message_delay_250,

    &message_display_backlight_off,
    &message_note_c4,
    &message_delay_250,

    &message_display_backlight_on,
    &message_note_c7,
    &message_delay_250,

    &message_display_backlight_off,
    &message_note_c4,
    &message_delay_250,

    &message_sound_off,
    &message_vibro_off,
    NULL,
};
