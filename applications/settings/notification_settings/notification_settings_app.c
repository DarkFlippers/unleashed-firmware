#include <furi.h>
#include <furi_hal_rtc.h>
#include <notification/notification_app.h>
#include <gui/modules/variable_item_list.h>
#include <gui/view_dispatcher.h>
#include <lib/toolbox/value_index.h>
#include <gui/gui_i.h>
#include <u8g2_glue.h>

#define MAX_NOTIFICATION_SETTINGS 5

typedef struct {
    NotificationApp* notification;
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    VariableItemList* variable_item_list;
    VariableItemList* variable_item_list_rgb;
} NotificationAppSettings;

static const NotificationSequence sequence_note_c = {
    &message_note_c5,
    &message_delay_100,
    &message_sound_off,
    NULL,
};

#define CONTRAST_COUNT 17
const char* const contrast_text[CONTRAST_COUNT] = {
    "-8",
    "-7",
    "-6",
    "-5",
    "-4",
    "-3",
    "-2",
    "-1",
    "0",
    "+1",
    "+2",
    "+3",
    "+4",
    "+5",
    "+6",
    "+7",
    "+8",
};
const int32_t contrast_value[CONTRAST_COUNT] = {
    -8,
    -7,
    -6,
    -5,
    -4,
    -3,
    -2,
    -1,
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
};

#define BACKLIGHT_COUNT 21
const char* const backlight_text[BACKLIGHT_COUNT] = {
    "0%",  "5%",  "10%", "15%", "20%", "25%", "30%", "35%", "40%", "45%",  "50%",
    "55%", "60%", "65%", "70%", "75%", "80%", "85%", "90%", "95%", "100%",
};
const float backlight_value[BACKLIGHT_COUNT] = {
    0.00f, 0.05f, 0.10f, 0.15f, 0.20f, 0.25f, 0.30f, 0.35f, 0.40f, 0.45f, 0.50f,
    0.55f, 0.60f, 0.65f, 0.70f, 0.75f, 0.80f, 0.85f, 0.90f, 0.95f, 1.00f,
};

#define VOLUME_COUNT 21
const char* const volume_text[VOLUME_COUNT] = {
    "0%",  "5%",  "10%", "15%", "20%", "25%", "30%", "35%", "40%", "45%",  "50%",
    "55%", "60%", "65%", "70%", "75%", "80%", "85%", "90%", "95%", "100%",
};
const float volume_value[VOLUME_COUNT] = {
    0.00f, 0.05f, 0.10f, 0.15f, 0.20f, 0.25f, 0.30f, 0.35f, 0.40f, 0.45f, 0.50f,
    0.55f, 0.60f, 0.65f, 0.70f, 0.75f, 0.80f, 0.85f, 0.90f, 0.95f, 1.00f,
};

#define DELAY_COUNT 11
const char* const delay_text[DELAY_COUNT] = {
    "1s",
    "5s",
    "10s",
    "15s",
    "30s",
    "60s",
    "90s",
    "120s",
    "5min",
    "10min",
    "30min",
};
const uint32_t delay_value[DELAY_COUNT] =
    {1000, 5000, 10000, 15000, 30000, 60000, 90000, 120000, 300000, 600000, 1800000};

#define VIBRO_COUNT 2
const char* const vibro_text[VIBRO_COUNT] = {
    "OFF",
    "ON",
};
const bool vibro_value[VIBRO_COUNT] = {false, true};

// --- RGB BACKLIGHT ---

#define RGB_BACKLIGHT_INSTALLED_COUNT 2
const char* const rgb_backlight_installed_text[RGB_BACKLIGHT_INSTALLED_COUNT] = {
    "OFF",
    "ON",
};
const bool rgb_backlight_installed_value[RGB_BACKLIGHT_INSTALLED_COUNT] = {false, true};

#define RGB_BACKLIGHT_RAINBOW_MODE_COUNT 3
const char* const rgb_backlight_rainbow_mode_text[RGB_BACKLIGHT_RAINBOW_MODE_COUNT] = {
    "OFF",
    "Rainbow",
    "Wave",
};
const uint32_t rgb_backlight_rainbow_mode_value[RGB_BACKLIGHT_RAINBOW_MODE_COUNT] = {0, 1, 2};

#define RGB_BACKLIGHT_RAINBOW_SPEED_COUNT 10
const char* const rgb_backlight_rainbow_speed_text[RGB_BACKLIGHT_RAINBOW_SPEED_COUNT] = {
    "0.1s",
    "0.2s",
    "0.3s",
    "0.4s",
    "0.5s",
    "0.6s",
    "0.7",
    "0.8",
    "0.9",
    "1s",
};

const uint32_t rgb_backlight_rainbow_speed_value[RGB_BACKLIGHT_RAINBOW_SPEED_COUNT] = {
    100,
    200,
    300,
    400,
    500,
    600,
    700,
    800,
    900,
    1000,
};

#define RGB_BACKLIGHT_RAINBOW_STEP_COUNT 3
const char* const rgb_backlight_rainbow_step_text[RGB_BACKLIGHT_RAINBOW_STEP_COUNT] = {
    "1",
    "2",
    "3",
};
const uint32_t rgb_backlight_rainbow_step_value[RGB_BACKLIGHT_RAINBOW_STEP_COUNT] = {
    1,
    2,
    3,
};

#define RGB_BACKLIGHT_RAINBOW_WIDE_COUNT 3
const char* const rgb_backlight_rainbow_wide_text[RGB_BACKLIGHT_RAINBOW_WIDE_COUNT] = {
    "1",
    "2",
    "3",
};
const uint32_t rgb_backlight_rainbow_wide_value[RGB_BACKLIGHT_RAINBOW_WIDE_COUNT] = {
    30,
    40,
    50,
};

typedef enum {
    MainViewId,
    RGBViewId,
} ViewId;

// --- RGB BACKLIGHT END ---

// --- NIGHT SHIFT ---
#define NIGHT_SHIFT_COUNT 7
const char* const night_shift_text[NIGHT_SHIFT_COUNT] =
    {"OFF", "-10%", "-20%", "-30%", "-40%", "-50%", "-60%"

};
const float night_shift_value[NIGHT_SHIFT_COUNT] = {
    1.0f,
    0.9f,
    0.8f,
    0.7f,
    0.6f,
    0.5f,
    0.4f,
};

#define NIGHT_SHIFT_START_COUNT 14
const char* const night_shift_start_text[NIGHT_SHIFT_START_COUNT] = {
    "17:00",
    "17:30",
    "18:00",
    "18:30",
    "19:00",
    "19:30",
    "20:00",
    "20:30",
    "21:00",
    "21:30",
    "22:00",
    "22:30",
    "23:00",
    "23:30",
};
// values in minutes like 23:30 = 23*60+30=1410
const uint32_t night_shift_start_value[NIGHT_SHIFT_START_COUNT] = {
    1020,
    1050,
    1080,
    1110,
    1140,
    1170,
    1200,
    1230,
    1260,
    1290,
    1320,
    1350,
    1380,
    1410,
};

#define NIGHT_SHIFT_END_COUNT 14
const char* const night_shift_end_text[NIGHT_SHIFT_END_COUNT] = {
    "05:00",
    "05:30",
    "06:00",
    "06:30",
    "07:00",
    "07:30",
    "08:00",
    "08:30",
    "09:00",
    "09:30",
    "10:00",
    "10:30",
    "11:00",
    "11:30",
};
// values in minutes like 6:30 = 6*60+30=390
const uint32_t night_shift_end_value[NIGHT_SHIFT_END_COUNT] = {
    300,
    330,
    360,
    390,
    410,
    440,
    470,
    500,
    530,
    560,
    590,
    620,
    650,
    680,
};

// --- NIGHT SHIFT END ---

#define LCD_INVERSION_COUNT 2
const char* const lcd_inversion_text[LCD_INVERSION_COUNT] = {
    "OFF",
    "ON",
};
const bool lcd_inversion_value[LCD_INVERSION_COUNT] = {false, true};

static void contrast_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, contrast_text[index]);
    app->notification->settings.contrast = contrast_value[index];
    notification_message(app->notification, &sequence_lcd_contrast_update);
}

static void backlight_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, backlight_text[index]);
    app->notification->settings.display_brightness = backlight_value[index];

    notification_message(app->notification, &sequence_display_backlight_on);
}

static void screen_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, delay_text[index]);
    app->notification->settings.display_off_delay_ms = delay_value[index];
    notification_message(app->notification, &sequence_display_backlight_on);
}

const NotificationMessage apply_message = {
    .type = NotificationMessageTypeLedBrightnessSettingApply,
};
const NotificationSequence apply_sequence = {
    &apply_message,
    NULL,
};

static void led_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, backlight_text[index]);
    app->notification->settings.led_brightness = backlight_value[index];
    notification_message(app->notification, &apply_sequence);
    notification_internal_message(app->notification, &apply_sequence);
    notification_message(app->notification, &sequence_blink_white_100);
}

static void volume_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, volume_text[index]);
    app->notification->settings.speaker_volume = volume_value[index];
    notification_message(app->notification, &sequence_note_c);
}

static void vibro_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, vibro_text[index]);
    app->notification->settings.vibro_on = vibro_value[index];
    notification_message(app->notification, &sequence_single_vibro);
}

static void lcd_inversion_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, lcd_inversion_text[index]);
    app->notification->settings.lcd_inversion = lcd_inversion_value[index];

    Gui* gui = furi_record_open(RECORD_GUI);
    u8x8_d_st756x_set_inversion(&gui->canvas->fb.u8x8, lcd_inversion_value[index]);
    furi_record_close(RECORD_GUI);

    notification_message_save_settings(app->notification);
}

//--- RGB BACKLIGHT ---

static void rgb_backlight_installed_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, rgb_backlight_installed_text[index]);
    app->notification->settings.rgb.rgb_backlight_installed = rgb_backlight_installed_value[index];
    set_rgb_backlight_installed_variable(rgb_backlight_installed_value[index]);

    // In case of user playing with rgb_backlight_installed swith:
    // if user swith_off rgb_backlight_installed (but may be he have mod installed)
    // then force set default orange color and stop rainbow timer
    if(index == 0) {
        rgb_backlight_set_led_static_color(2, 0);
        rgb_backlight_set_led_static_color(1, 0);
        rgb_backlight_set_led_static_color(0, 0);
        SK6805_update();
        rainbow_timer_stop(app->notification);
        // start rainbow (if its Enabled) or set saved static colors if user swith_on rgb_backlight_installed switch
    } else {
        if(app->notification->settings.rgb.rainbow_mode > 0) {
            rainbow_timer_starter(app->notification);
        } else {
            rgb_backlight_set_led_static_color(
                2, app->notification->settings.rgb.led_2_color_index);
            rgb_backlight_set_led_static_color(
                1, app->notification->settings.rgb.led_1_color_index);
            rgb_backlight_set_led_static_color(
                0, app->notification->settings.rgb.led_0_color_index);
            rgb_backlight_update(
                app->notification->settings.display_brightness *
                app->notification->current_night_shift);
        }
    }

    // Lock/Unlock all rgb settings depent from rgb_backlight_installed switch
    for(int i = 1; i < 9; i++) {
        VariableItem* t_item = variable_item_list_get(app->variable_item_list_rgb, i);
        if(index == 0) {
            variable_item_set_locked(t_item, true, "RGB\nOFF!");
        } else {
            variable_item_set_locked(t_item, false, "RGB\nOFF!");
        }
    }
    notification_message_save_settings(app->notification);
}

static void led_2_color_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, rgb_backlight_get_color_text(index));
    app->notification->settings.rgb.led_2_color_index = index;

    // dont update screen color if rainbow timer working
    if(!furi_timer_is_running(app->notification->rainbow_timer)) {
        rgb_backlight_set_led_static_color(2, index);
        rgb_backlight_update(
            app->notification->settings.display_brightness *
            app->notification->current_night_shift);
    }

    notification_message_save_settings(app->notification);
}

static void led_1_color_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, rgb_backlight_get_color_text(index));
    app->notification->settings.rgb.led_1_color_index = index;

    // dont update screen color if rainbow timer working
    if(!furi_timer_is_running(app->notification->rainbow_timer)) {
        rgb_backlight_set_led_static_color(1, index);
        rgb_backlight_update(
            app->notification->settings.display_brightness *
            app->notification->current_night_shift);
    }

    notification_message_save_settings(app->notification);
}

static void led_0_color_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, rgb_backlight_get_color_text(index));
    app->notification->settings.rgb.led_0_color_index = index;

    // dont update screen color if rainbow timer working
    if(!furi_timer_is_running(app->notification->rainbow_timer)) {
        rgb_backlight_set_led_static_color(0, index);
        rgb_backlight_update(
            app->notification->settings.display_brightness *
            app->notification->current_night_shift);
    }

    notification_message_save_settings(app->notification);
}

static void rgb_backlight_rainbow_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, rgb_backlight_rainbow_mode_text[index]);
    app->notification->settings.rgb.rainbow_mode = rgb_backlight_rainbow_mode_value[index];

    // restore saved rgb backlight settings if we switch_off effects
    if(index == 0) {
        rgb_backlight_set_led_static_color(2, app->notification->settings.rgb.led_2_color_index);
        rgb_backlight_set_led_static_color(1, app->notification->settings.rgb.led_1_color_index);
        rgb_backlight_set_led_static_color(0, app->notification->settings.rgb.led_0_color_index);
        rgb_backlight_update(
            app->notification->settings.display_brightness *
            app->notification->current_night_shift);
        rainbow_timer_stop(app->notification);
    } else {
        rainbow_timer_starter(app->notification);
    }

    notification_message_save_settings(app->notification);
}

static void rgb_backlight_rainbow_speed_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, rgb_backlight_rainbow_speed_text[index]);
    app->notification->settings.rgb.rainbow_speed_ms = rgb_backlight_rainbow_speed_value[index];

    // save settings and restart timer with new speed value
    rainbow_timer_starter(app->notification);
    notification_message_save_settings(app->notification);
}

static void rgb_backlight_rainbow_step_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, rgb_backlight_rainbow_step_text[index]);
    app->notification->settings.rgb.rainbow_step = rgb_backlight_rainbow_step_value[index];

    notification_message_save_settings(app->notification);
}

static void rgb_backlight_rainbow_saturation_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);

    // saturation must be 1..255, so we do (0..254)+1
    uint8_t index = variable_item_get_current_value_index(item) + 1;
    char valtext[4] = {};
    snprintf(valtext, sizeof(valtext), "%d", index);
    variable_item_set_current_value_text(item, valtext);
    app->notification->settings.rgb.rainbow_saturation = index;

    notification_message_save_settings(app->notification);
}

static void rgb_backlight_rainbow_wide_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, rgb_backlight_rainbow_wide_text[index]);
    app->notification->settings.rgb.rainbow_wide = rgb_backlight_rainbow_wide_value[index];

    notification_message_save_settings(app->notification);
}

// open settings.rgb_view if user press OK on last (index=10) menu string
void variable_item_list_enter_callback(void* context, uint32_t index) {
    UNUSED(context);
    NotificationAppSettings* app = context;

    if(index == 10) {
        view_dispatcher_switch_to_view(app->view_dispatcher, RGBViewId);
    }
}

// switch to main view on exit from settings.rgb_view
static uint32_t notification_app_rgb_settings_exit(void* context) {
    UNUSED(context);
    return MainViewId;
}
//--- RGB BACKLIGHT END ---

// --- NIGHT SHIFT ---

static void night_shift_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, night_shift_text[index]);
    app->notification->settings.night_shift = night_shift_value[index];
    app->notification->current_night_shift = night_shift_value[index];
    app->notification->current_night_shift = night_shift_value[index];

    // force demo night_shift brightness to rgb backlight and stock backlight
    notification_message(app->notification, &sequence_display_backlight_on);
    
    for(int i = 4; i < 6; i++) {
        VariableItem* t_item = variable_item_list_get(app->variable_item_list, i);
        if(index == 0) {
            variable_item_set_locked(t_item, true, "Night Shift\nOFF!");
        } else {
            variable_item_set_locked(t_item, false, "Night Shift\nOFF!");
        }
    }

    if(night_shift_value[index] != 1) {
        night_shift_timer_start(app->notification);
    } else {
        night_shift_timer_stop(app->notification);
    }

    notification_message_save_settings(app->notification);
}

static void night_shift_start_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, night_shift_start_text[index]);
    app->notification->settings.night_shift_start = night_shift_start_value[index];

    notification_message_save_settings(app->notification);
}

static void night_shift_end_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, night_shift_end_text[index]);
    app->notification->settings.night_shift_end = night_shift_end_value[index];

    notification_message_save_settings(app->notification);
}

// --- NIGHT SHIFT END ---

static uint32_t notification_app_settings_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

static NotificationAppSettings* alloc_settings(void) {
    NotificationAppSettings* app = malloc(sizeof(NotificationAppSettings));
    app->notification = furi_record_open(RECORD_NOTIFICATION);
    app->gui = furi_record_open(RECORD_GUI);

    app->variable_item_list = variable_item_list_alloc();
    View* view = variable_item_list_get_view(app->variable_item_list);

    VariableItem* item;
    uint8_t value_index;

    //set callback for exit from main view
    view_set_previous_callback(view, notification_app_settings_exit);

    //--- RGB BACKLIGHT ---
    // set callback for OK pressed in notification settings menu
    variable_item_list_set_enter_callback(
        app->variable_item_list, variable_item_list_enter_callback, app);

    item = variable_item_list_add(
        app->variable_item_list, "LCD Contrast", CONTRAST_COUNT, contrast_changed, app);
    value_index =
        value_index_int32(app->notification->settings.contrast, contrast_value, CONTRAST_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, contrast_text[value_index]);

    item = variable_item_list_add(
        app->variable_item_list, "LCD Backlight", BACKLIGHT_COUNT, backlight_changed, app);
    value_index = value_index_float(
        app->notification->settings.display_brightness, backlight_value, BACKLIGHT_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, backlight_text[value_index]);

    item = variable_item_list_add(
        app->variable_item_list, "Backlight Time", DELAY_COUNT, screen_changed, app);
    value_index = value_index_uint32(
        app->notification->settings.display_off_delay_ms, delay_value, DELAY_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, delay_text[value_index]);

    // --- NIGHT SHIFT ---
    item = variable_item_list_add(
        app->variable_item_list, "Night Shift", NIGHT_SHIFT_COUNT, night_shift_changed, app);
    value_index = value_index_float(
        app->notification->settings.night_shift, night_shift_value, NIGHT_SHIFT_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, night_shift_text[value_index]);

    item = variable_item_list_add(
        app->variable_item_list,
        " . Start",
        NIGHT_SHIFT_START_COUNT,
        night_shift_start_changed,
        app);
    value_index = value_index_uint32(
        app->notification->settings.night_shift_start,
        night_shift_start_value,
        NIGHT_SHIFT_START_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, night_shift_start_text[value_index]);
    variable_item_set_locked(
        item, (app->notification->settings.night_shift == 1), "Night Shift \nOFF!");

    item = variable_item_list_add(
        app->variable_item_list, " . End", NIGHT_SHIFT_END_COUNT, night_shift_end_changed, app);
    value_index = value_index_uint32(
        app->notification->settings.night_shift_end, night_shift_end_value, NIGHT_SHIFT_END_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, night_shift_end_text[value_index]);
    variable_item_set_locked(
        item, (app->notification->settings.night_shift == 1), "Night Shift \nOFF!");

    // --- NIGHT SHIFT END---

    item = variable_item_list_add(
        app->variable_item_list, "LED Brightness", BACKLIGHT_COUNT, led_changed, app);
    value_index = value_index_float(
        app->notification->settings.led_brightness, backlight_value, BACKLIGHT_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, backlight_text[value_index]);

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagStealthMode)) {
        item = variable_item_list_add(app->variable_item_list, "Volume", 1, NULL, app);
        value_index = 0;
        variable_item_set_current_value_index(item, value_index);
        variable_item_set_current_value_text(item, "Stealth");
    } else {
        item = variable_item_list_add(
            app->variable_item_list, "Volume", VOLUME_COUNT, volume_changed, app);
        value_index = value_index_float(
            app->notification->settings.speaker_volume, volume_value, VOLUME_COUNT);
        variable_item_set_current_value_index(item, value_index);
        variable_item_set_current_value_text(item, volume_text[value_index]);
    }

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagStealthMode)) {
        item = variable_item_list_add(app->variable_item_list, "Vibro", 1, NULL, app);
        value_index = 0;
        variable_item_set_current_value_index(item, value_index);
        variable_item_set_current_value_text(item, "Stealth");
    } else {
        item = variable_item_list_add(
            app->variable_item_list, "Vibro", VIBRO_COUNT, vibro_changed, app);
        value_index =
            value_index_bool(app->notification->settings.vibro_on, vibro_value, VIBRO_COUNT);
        variable_item_set_current_value_index(item, value_index);
        variable_item_set_current_value_text(item, vibro_text[value_index]);
    }

    item = variable_item_list_add(
        app->variable_item_list, "LCD Inversion", LCD_INVERSION_COUNT, lcd_inversion_changed, app);
    value_index = value_index_bool(
        app->notification->settings.lcd_inversion, lcd_inversion_value, LCD_INVERSION_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, lcd_inversion_text[value_index]);

    //--- RGB BACKLIGHT ---
    item = variable_item_list_add(app->variable_item_list, "RGB Mod Settings", 0, NULL, app);
    //--- RGB BACKLIGHT END ---

    app->variable_item_list_rgb = variable_item_list_alloc();
    View* view_rgb = variable_item_list_get_view(app->variable_item_list_rgb);

    // set callback for exit from rgb settings menu
    view_set_previous_callback(view_rgb, notification_app_rgb_settings_exit);

    item = variable_item_list_add(
        app->variable_item_list_rgb,
        "RGB backlight installed",
        RGB_BACKLIGHT_INSTALLED_COUNT,
        rgb_backlight_installed_changed,
        app);
    value_index = value_index_bool(
        app->notification->settings.rgb.rgb_backlight_installed,
        rgb_backlight_installed_value,
        RGB_BACKLIGHT_INSTALLED_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, rgb_backlight_installed_text[value_index]);

    // We (humans) are numbering LEDs from left to right as 1..3, but hardware have another order from right to left 2..0
    // led_1 color
    item = variable_item_list_add(
        app->variable_item_list_rgb,
        "LED 1 Color",
        rgb_backlight_get_color_count(),
        led_2_color_changed,
        app);
    value_index = app->notification->settings.rgb.led_2_color_index;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, rgb_backlight_get_color_text(value_index));
    variable_item_set_locked(
        item, (app->notification->settings.rgb.rgb_backlight_installed == 0), "RGB MOD \nOFF!");

    // led_2 color
    item = variable_item_list_add(
        app->variable_item_list_rgb,
        "LED 2 Color",
        rgb_backlight_get_color_count(),
        led_1_color_changed,
        app);
    value_index = app->notification->settings.rgb.led_1_color_index;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, rgb_backlight_get_color_text(value_index));
    variable_item_set_locked(
        item, (app->notification->settings.rgb.rgb_backlight_installed == 0), "RGB MOD \nOFF!");

    // led 3 color
    item = variable_item_list_add(
        app->variable_item_list_rgb,
        "LED 3 Color",
        rgb_backlight_get_color_count(),
        led_0_color_changed,
        app);
    value_index = app->notification->settings.rgb.led_0_color_index;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, rgb_backlight_get_color_text(value_index));
    variable_item_set_locked(
        item, (app->notification->settings.rgb.rgb_backlight_installed == 0), "RGB MOD \nOFF!");

    // Efects
    item = variable_item_list_add(
        app->variable_item_list_rgb,
        "Effects",
        RGB_BACKLIGHT_RAINBOW_MODE_COUNT,
        rgb_backlight_rainbow_changed,
        app);
    value_index = value_index_uint32(
        app->notification->settings.rgb.rainbow_mode,
        rgb_backlight_rainbow_mode_value,
        RGB_BACKLIGHT_RAINBOW_MODE_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, rgb_backlight_rainbow_mode_text[value_index]);
    variable_item_set_locked(
        item, (app->notification->settings.rgb.rgb_backlight_installed == 0), "RGB MOD \nOFF!");

    item = variable_item_list_add(
        app->variable_item_list_rgb,
        " . Speed",
        RGB_BACKLIGHT_RAINBOW_SPEED_COUNT,
        rgb_backlight_rainbow_speed_changed,
        app);
    value_index = value_index_uint32(
        app->notification->settings.rgb.rainbow_speed_ms,
        rgb_backlight_rainbow_speed_value,
        RGB_BACKLIGHT_RAINBOW_SPEED_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, rgb_backlight_rainbow_speed_text[value_index]);
    variable_item_set_locked(
        item, (app->notification->settings.rgb.rgb_backlight_installed == 0), "RGB MOD \nOFF!");

    item = variable_item_list_add(
        app->variable_item_list_rgb,
        " . Color step",
        RGB_BACKLIGHT_RAINBOW_STEP_COUNT,
        rgb_backlight_rainbow_step_changed,
        app);
    value_index = value_index_uint32(
        app->notification->settings.rgb.rainbow_step,
        rgb_backlight_rainbow_step_value,
        RGB_BACKLIGHT_RAINBOW_STEP_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, rgb_backlight_rainbow_step_text[value_index]);
    variable_item_set_locked(
        item, (app->notification->settings.rgb.rgb_backlight_installed == 0), "RGB MOD \nOFF!");

    item = variable_item_list_add(
        app->variable_item_list_rgb,
        " . Saturation",
        255,
        rgb_backlight_rainbow_saturation_changed,
        app);
    value_index = app->notification->settings.rgb.rainbow_saturation;
    variable_item_set_current_value_index(item, value_index);
    char valtext[4] = {};
    snprintf(valtext, sizeof(valtext), "%d", value_index);
    variable_item_set_current_value_text(item, valtext);
    variable_item_set_locked(
        item, (app->notification->settings.rgb.rgb_backlight_installed == 0), "RGB MOD \nOFF!");

    item = variable_item_list_add(
        app->variable_item_list_rgb,
        " . Wave wide",
        RGB_BACKLIGHT_RAINBOW_WIDE_COUNT,
        rgb_backlight_rainbow_wide_changed,
        app);
    value_index = value_index_uint32(
        app->notification->settings.rgb.rainbow_wide,
        rgb_backlight_rainbow_wide_value,
        RGB_BACKLIGHT_RAINBOW_WIDE_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, rgb_backlight_rainbow_wide_text[value_index]);
    variable_item_set_locked(
        item, (app->notification->settings.rgb.rgb_backlight_installed == 0), "RGB MOD \nOFF!");

    //--- RGB BACKLIGHT END ---

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_add_view(app->view_dispatcher, MainViewId, view);
    view_dispatcher_add_view(app->view_dispatcher, RGBViewId, view_rgb);
    view_dispatcher_switch_to_view(app->view_dispatcher, MainViewId);
    return app;
}

static void free_settings(NotificationAppSettings* app) {
    view_dispatcher_remove_view(app->view_dispatcher, MainViewId);
    view_dispatcher_remove_view(app->view_dispatcher, RGBViewId);
    variable_item_list_free(app->variable_item_list);
    variable_item_list_free(app->variable_item_list_rgb);
    view_dispatcher_free(app->view_dispatcher);

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    free(app);
}

int32_t notification_settings_app(void* p) {
    UNUSED(p);
    NotificationAppSettings* app = alloc_settings();
    view_dispatcher_run(app->view_dispatcher);
    notification_message_save_settings(app->notification);

    // Automaticaly switch_off debug_mode when user exit from settings with enabled rgb_backlight_installed
    // if(app->notification->settings.rgb_backlight_installed) {
    //     furi_hal_rtc_reset_flag(FuriHalRtcFlagDebug);
    // }

    free_settings(app);
    return 0;
}
