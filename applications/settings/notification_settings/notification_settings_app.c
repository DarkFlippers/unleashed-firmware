#include <furi.h>
#include <notification/notification_app.h>
#include <gui/modules/variable_item_list.h>
#include <gui/view_dispatcher.h>
#include <lib/toolbox/value_index.h>

#define MAX_NOTIFICATION_SETTINGS 4

typedef struct {
    NotificationApp* notification;
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    VariableItemList* variable_item_list;
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
    view_set_previous_callback(view, notification_app_settings_exit);

    VariableItem* item;
    uint8_t value_index;

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

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_add_view(app->view_dispatcher, 0, view);
    view_dispatcher_switch_to_view(app->view_dispatcher, 0);

    return app;
}

static void free_settings(NotificationAppSettings* app) {
    view_dispatcher_remove_view(app->view_dispatcher, 0);
    variable_item_list_free(app->variable_item_list);
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
    free_settings(app);
    return 0;
}
