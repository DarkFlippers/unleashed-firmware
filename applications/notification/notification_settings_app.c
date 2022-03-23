#include <furi.h>
#include "notification_app.h"
#include <gui/modules/variable_item_list.h>
#include <gui/view_dispatcher.h>

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

#define BACKLIGHT_COUNT 5
const char* const backlight_text[BACKLIGHT_COUNT] = {
    "0%",
    "25%",
    "50%",
    "75%",
    "100%",
};
const float backlight_value[BACKLIGHT_COUNT] = {
    0.0f,
    0.25f,
    0.5f,
    0.75f,
    1.0f,
};

#define VOLUME_COUNT 5
const char* const volume_text[VOLUME_COUNT] = {
    "0%",
    "25%",
    "50%",
    "75%",
    "100%",
};
const float volume_value[VOLUME_COUNT] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};

#define DELAY_COUNT 6
const char* const delay_text[DELAY_COUNT] = {
    "1s",
    "5s",
    "15s",
    "30s",
    "60s",
    "120s",
};
const uint32_t delay_value[DELAY_COUNT] = {1000, 5000, 15000, 30000, 60000, 120000};

#define VIBRO_COUNT 2
const char* const vibro_text[VIBRO_COUNT] = {
    "OFF",
    "ON",
};
const bool vibro_value[VIBRO_COUNT] = {false, true};

uint8_t float_value_index(const float value, const float values[], uint8_t values_count) {
    const float epsilon = 0.01f;
    float last_value = values[0];
    uint8_t index = 0;
    for(uint8_t i = 0; i < values_count; i++) {
        if((value >= last_value - epsilon) && (value <= values[i] + epsilon)) {
            index = i;
            break;
        }
        last_value = values[i];
    }
    return index;
}

uint8_t uint32_value_index(const uint32_t value, const uint32_t values[], uint8_t values_count) {
    int64_t last_value = INT64_MIN;
    uint8_t index = 0;
    for(uint8_t i = 0; i < values_count; i++) {
        if((value >= last_value) && (value <= values[i])) {
            index = i;
            break;
        }
        last_value = values[i];
    }
    return index;
}

uint8_t bool_value_index(const bool value, const bool values[], uint8_t values_count) {
    uint8_t index = 0;
    for(uint8_t i = 0; i < values_count; i++) {
        if(value == values[i]) {
            index = i;
            break;
        }
    }
    return index;
}

static void backlight_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, backlight_text[index]);
    app->notification->settings.display_brightness = backlight_value[index];
    notification_message(app->notification, &sequence_display_on);
}

static void screen_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, delay_text[index]);
    app->notification->settings.display_off_delay_ms = delay_value[index];
    notification_message(app->notification, &sequence_display_on);
}

static void led_changed(VariableItem* item) {
    NotificationAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, backlight_text[index]);
    app->notification->settings.led_brightness = backlight_value[index];
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
    return VIEW_NONE;
}

static NotificationAppSettings* alloc_settings() {
    NotificationAppSettings* app = malloc(sizeof(NotificationAppSettings));
    app->notification = furi_record_open("notification");
    app->gui = furi_record_open("gui");

    app->variable_item_list = variable_item_list_alloc();
    View* view = variable_item_list_get_view(app->variable_item_list);
    view_set_previous_callback(view, notification_app_settings_exit);

    VariableItem* item;
    uint8_t value_index;

    item = variable_item_list_add(
        app->variable_item_list, "LCD backlight", BACKLIGHT_COUNT, backlight_changed, app);
    value_index = float_value_index(
        app->notification->settings.display_brightness, backlight_value, BACKLIGHT_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, backlight_text[value_index]);

    item = variable_item_list_add(
        app->variable_item_list, "Backlight time", DELAY_COUNT, screen_changed, app);
    value_index = uint32_value_index(
        app->notification->settings.display_off_delay_ms, delay_value, DELAY_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, delay_text[value_index]);

    item = variable_item_list_add(
        app->variable_item_list, "LED brightness", BACKLIGHT_COUNT, led_changed, app);
    value_index = float_value_index(
        app->notification->settings.led_brightness, backlight_value, BACKLIGHT_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, backlight_text[value_index]);

    item = variable_item_list_add(
        app->variable_item_list, "Volume", VOLUME_COUNT, volume_changed, app);
    value_index =
        float_value_index(app->notification->settings.speaker_volume, volume_value, VOLUME_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, volume_text[value_index]);

    item =
        variable_item_list_add(app->variable_item_list, "Vibro", VIBRO_COUNT, vibro_changed, app);
    value_index = bool_value_index(app->notification->settings.vibro_on, vibro_value, VIBRO_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, vibro_text[value_index]);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_add_view(app->view_dispatcher, 0, view);
    view_dispatcher_switch_to_view(app->view_dispatcher, 0);

    return app;
}

static void free_settings(NotificationAppSettings* app) {
    view_dispatcher_remove_view(app->view_dispatcher, 0);
    variable_item_list_free(app->variable_item_list);
    view_dispatcher_free(app->view_dispatcher);

    furi_record_close("gui");
    furi_record_close("notification");
    free(app);
}

int32_t notification_settings_app(void* p) {
    NotificationAppSettings* app = alloc_settings();
    view_dispatcher_run(app->view_dispatcher);
    notification_message_save_settings(app->notification);
    free_settings(app);
    return 0;
}
