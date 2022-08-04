#pragma once
#include "notification.h"
#include "notification_messages_notes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************************** Messages **********************************/

// Display
extern const NotificationMessage message_display_backlight_on;
extern const NotificationMessage message_display_backlight_off;
extern const NotificationMessage message_display_backlight_enforce_on;
extern const NotificationMessage message_display_backlight_enforce_auto;

// Led ON
extern const NotificationMessage message_red_255;
extern const NotificationMessage message_green_255;
extern const NotificationMessage message_blue_255;

// Led OFF
extern const NotificationMessage message_red_0;
extern const NotificationMessage message_green_0;
extern const NotificationMessage message_blue_0;

// Led hardware blink control
extern const NotificationMessage message_blink_start_10;
extern const NotificationMessage message_blink_start_100;
extern const NotificationMessage message_blink_stop;

extern const NotificationMessage message_blink_set_color_red;
extern const NotificationMessage message_blink_set_color_green;
extern const NotificationMessage message_blink_set_color_blue;
extern const NotificationMessage message_blink_set_color_cyan;
extern const NotificationMessage message_blink_set_color_magenta;
extern const NotificationMessage message_blink_set_color_yellow;
extern const NotificationMessage message_blink_set_color_white;

// Delay
extern const NotificationMessage message_delay_1;
extern const NotificationMessage message_delay_10;
extern const NotificationMessage message_delay_25;
extern const NotificationMessage message_delay_50;
extern const NotificationMessage message_delay_100;
extern const NotificationMessage message_delay_250;
extern const NotificationMessage message_delay_500;
extern const NotificationMessage message_delay_1000;

// Sound
extern const NotificationMessage message_sound_off;

// Vibro
extern const NotificationMessage message_vibro_on;
extern const NotificationMessage message_vibro_off;

// Reset
extern const NotificationMessage message_do_not_reset;

// Override user settings
extern const NotificationMessage message_force_speaker_volume_setting_1f;
extern const NotificationMessage message_force_vibro_setting_on;
extern const NotificationMessage message_force_vibro_setting_off;
extern const NotificationMessage message_force_display_brightness_setting_1f;

/****************************** Message sequences ******************************/

// Reset
extern const NotificationSequence sequence_reset_red;
extern const NotificationSequence sequence_reset_green;
extern const NotificationSequence sequence_reset_blue;
extern const NotificationSequence sequence_reset_rgb;
extern const NotificationSequence sequence_reset_display;
extern const NotificationSequence sequence_reset_sound;
extern const NotificationSequence sequence_reset_vibro;

// Vibro
extern const NotificationSequence sequence_set_vibro_on;

// Display
/** Display: backlight wakeup */
extern const NotificationSequence sequence_display_backlight_on;
/** Display: backlight force off */
extern const NotificationSequence sequence_display_backlight_off;
/** Display: backlight force off after a delay of 1000ms */
extern const NotificationSequence sequence_display_backlight_off_delay_1000;

/** Display: backlight always on lock */
extern const NotificationSequence sequence_display_backlight_enforce_on;
/** Display: backlight always on unlock */
extern const NotificationSequence sequence_display_backlight_enforce_auto;

// Charging
extern const NotificationSequence sequence_charging;
extern const NotificationSequence sequence_charged;
extern const NotificationSequence sequence_not_charging;

// Light up
extern const NotificationSequence sequence_set_only_red_255;
extern const NotificationSequence sequence_set_only_green_255;
extern const NotificationSequence sequence_set_only_blue_255;
extern const NotificationSequence sequence_set_red_255;
extern const NotificationSequence sequence_set_green_255;
extern const NotificationSequence sequence_set_blue_255;

// Solid colors
extern const NotificationSequence sequence_solid_yellow;

// Blink
extern const NotificationSequence sequence_blink_blue_10;
extern const NotificationSequence sequence_blink_red_10;
extern const NotificationSequence sequence_blink_green_10;
extern const NotificationSequence sequence_blink_yellow_10;
extern const NotificationSequence sequence_blink_cyan_10;
extern const NotificationSequence sequence_blink_magenta_10;

extern const NotificationSequence sequence_blink_red_100;
extern const NotificationSequence sequence_blink_green_100;
extern const NotificationSequence sequence_blink_blue_100;
extern const NotificationSequence sequence_blink_yellow_100;
extern const NotificationSequence sequence_blink_cyan_100;
extern const NotificationSequence sequence_blink_magenta_100;
extern const NotificationSequence sequence_blink_white_100;

// Hardware blink
extern const NotificationSequence sequence_blink_start_blue;
extern const NotificationSequence sequence_blink_start_red;
extern const NotificationSequence sequence_blink_start_green;
extern const NotificationSequence sequence_blink_start_yellow;
extern const NotificationSequence sequence_blink_start_cyan;
extern const NotificationSequence sequence_blink_start_magenta;
extern const NotificationSequence sequence_blink_stop;

// General
extern const NotificationSequence sequence_single_vibro;
extern const NotificationSequence sequence_double_vibro;
extern const NotificationSequence sequence_success;
extern const NotificationSequence sequence_error;
extern const NotificationSequence sequence_audiovisual_alert;

#ifdef __cplusplus
}
#endif
