#include <furi.h>
#include <furi-hal.h>
#include <storage/storage.h>
#include "notification.h"
#include "notification-messages.h"
#include "notification-app.h"

#define TAG "NotificationSrv"

static const uint8_t minimal_delay = 100;
static const uint8_t led_off_values[NOTIFICATION_LED_COUNT] = {0x00, 0x00, 0x00};

static const uint8_t reset_red_mask = 1 << 0;
static const uint8_t reset_green_mask = 1 << 1;
static const uint8_t reset_blue_mask = 1 << 2;
static const uint8_t reset_vibro_mask = 1 << 3;
static const uint8_t reset_sound_mask = 1 << 4;
static const uint8_t reset_display_mask = 1 << 5;

void notification_vibro_on();
void notification_vibro_off();
void notification_sound_on(float pwm, float freq);
void notification_sound_off();

uint8_t notification_settings_get_display_brightness(NotificationApp* app, uint8_t value);
uint8_t notification_settings_get_rgb_led_brightness(NotificationApp* app, uint8_t value);
uint32_t notification_settings_display_off_delay_ticks(NotificationApp* app);

void notification_message_save_settings(NotificationApp* app) {
    NotificationAppMessage m = {.type = SaveSettingsMessage, .back_event = osEventFlagsNew(NULL)};
    furi_check(osMessageQueuePut(app->queue, &m, 0, osWaitForever) == osOK);
    osEventFlagsWait(m.back_event, NOTIFICATION_EVENT_COMPLETE, osFlagsWaitAny, osWaitForever);
    osEventFlagsDelete(m.back_event);
};

// internal layer
void notification_apply_internal_led_layer(NotificationLedLayer* layer, uint8_t layer_value) {
    furi_assert(layer);
    furi_assert(layer->index < LayerMAX);

    // set value
    layer->value[LayerInternal] = layer_value;

    // apply if current layer is internal
    if(layer->index == LayerInternal) {
        furi_hal_light_set(layer->light, layer->value[LayerInternal]);
    }
}

bool notification_is_any_led_layer_internal_and_not_empty(NotificationApp* app) {
    bool result = false;
    if((app->led[0].index == LayerInternal) || (app->led[1].index == LayerInternal) ||
       (app->led[2].index == LayerInternal)) {
        if((app->led[0].value[LayerInternal] != 0x00) ||
           (app->led[1].value[LayerInternal] != 0x00) ||
           (app->led[2].value[LayerInternal] != 0x00)) {
            result = true;
        }
    }

    return result;
}

// notification layer
void notification_apply_notification_led_layer(
    NotificationLedLayer* layer,
    const uint8_t layer_value) {
    furi_assert(layer);
    furi_assert(layer->index < LayerMAX);

    // set value
    layer->index = LayerNotification;
    // set layer
    layer->value[LayerNotification] = layer_value;
    // apply
    furi_hal_light_set(layer->light, layer->value[LayerNotification]);
}

void notification_reset_notification_led_layer(NotificationLedLayer* layer) {
    furi_assert(layer);
    furi_assert(layer->index < LayerMAX);

    // set value
    layer->value[LayerNotification] = 0;
    // set layer
    layer->index = LayerInternal;

    // apply
    furi_hal_light_set(layer->light, layer->value[LayerInternal]);
}

void notification_reset_notification_layer(NotificationApp* app, uint8_t reset_mask) {
    if(reset_mask & reset_red_mask) {
        notification_reset_notification_led_layer(&app->led[0]);
    }
    if(reset_mask & reset_green_mask) {
        notification_reset_notification_led_layer(&app->led[1]);
    }
    if(reset_mask & reset_blue_mask) {
        notification_reset_notification_led_layer(&app->led[2]);
    }
    if(reset_mask & reset_vibro_mask) {
        notification_vibro_off();
    }
    if(reset_mask & reset_sound_mask) {
        notification_sound_off();
    }
    if(reset_mask & reset_display_mask) {
        osTimerStart(app->display_timer, notification_settings_display_off_delay_ticks(app));
    }
}

static void notification_apply_notification_leds(NotificationApp* app, const uint8_t* values) {
    for(uint8_t i = 0; i < NOTIFICATION_LED_COUNT; i++) {
        notification_apply_notification_led_layer(
            &app->led[i], notification_settings_get_rgb_led_brightness(app, values[i]));
    }
}

// settings
uint8_t notification_settings_get_display_brightness(NotificationApp* app, uint8_t value) {
    return (value * app->settings.display_brightness);
}

uint8_t notification_settings_get_rgb_led_brightness(NotificationApp* app, uint8_t value) {
    return (value * app->settings.led_brightness);
}

uint32_t notification_settings_display_off_delay_ticks(NotificationApp* app) {
    return ((float)(app->settings.display_off_delay_ms) / (1000.0f / osKernelGetTickFreq()));
}

// generics
void notification_vibro_on() {
    furi_hal_vibro_on(true);
}

void notification_vibro_off() {
    furi_hal_vibro_on(false);
}

void notification_sound_on(float pwm, float freq) {
    hal_pwm_set(pwm, freq, &SPEAKER_TIM, SPEAKER_CH);
}

void notification_sound_off() {
    hal_pwm_stop(&SPEAKER_TIM, SPEAKER_CH);
}

// display timer
static void notification_display_timer(void* ctx) {
    furi_assert(ctx);
    NotificationApp* app = ctx;
    notification_message(app, &sequence_display_off);
}

// message processing
void notification_process_notification_message(
    NotificationApp* app,
    NotificationAppMessage* message) {
    uint32_t notification_message_index = 0;
    const NotificationMessage* notification_message;
    notification_message = (*message->sequence)[notification_message_index];

    bool led_active = false;
    uint8_t led_values[NOTIFICATION_LED_COUNT] = {0x00, 0x00, 0x00};
    bool reset_notifications = true;

    uint8_t reset_mask = 0;

    while(notification_message != NULL) {
        switch(notification_message->type) {
        case NotificationMessageTypeLedDisplay:
            // if on - switch on and start timer
            // if off - switch off and stop timer
            // on timer - switch off
            if(notification_message->data.led.value > 0x00) {
                notification_apply_notification_led_layer(
                    &app->display,
                    notification_settings_get_display_brightness(
                        app, notification_message->data.led.value));
            } else {
                notification_reset_notification_led_layer(&app->display);
                if(osTimerIsRunning(app->display_timer)) {
                    osTimerStop(app->display_timer);
                }
            }
            reset_mask |= reset_display_mask;
            break;
        case NotificationMessageTypeLedRed:
            // store and send on delay or after seq
            led_active = true;
            led_values[0] = notification_message->data.led.value;
            reset_mask |= reset_red_mask;
            break;
        case NotificationMessageTypeLedGreen:
            // store and send on delay or after seq
            led_active = true;
            led_values[1] = notification_message->data.led.value;
            reset_mask |= reset_green_mask;
            break;
        case NotificationMessageTypeLedBlue:
            // store and send on delay or after seq
            led_active = true;
            led_values[2] = notification_message->data.led.value;
            reset_mask |= reset_blue_mask;
            break;
        case NotificationMessageTypeVibro:
            if(notification_message->data.vibro.on) {
                if(app->settings.vibro_on) notification_vibro_on();
            } else {
                notification_vibro_off();
            }
            reset_mask |= reset_vibro_mask;
            break;
        case NotificationMessageTypeSoundOn:
            notification_sound_on(
                notification_message->data.sound.pwm * app->settings.speaker_volume,
                notification_message->data.sound.frequency);
            reset_mask |= reset_sound_mask;
            break;
        case NotificationMessageTypeSoundOff:
            notification_sound_off();
            reset_mask |= reset_sound_mask;
            break;
        case NotificationMessageTypeDelay:
            if(led_active) {
                if(notification_is_any_led_layer_internal_and_not_empty(app)) {
                    notification_apply_notification_leds(app, led_off_values);
                    delay(minimal_delay);
                }

                led_active = false;

                notification_apply_notification_leds(app, led_values);
                reset_mask |= reset_red_mask;
                reset_mask |= reset_green_mask;
                reset_mask |= reset_blue_mask;
            }

            delay(notification_message->data.delay.length);
            break;
        case NotificationMessageTypeDoNotReset:
            reset_notifications = false;
            break;
        }
        notification_message_index++;
        notification_message = (*message->sequence)[notification_message_index];
    };

    // send and do minimal delay
    if(led_active) {
        bool need_minimal_delay = false;
        if(notification_is_any_led_layer_internal_and_not_empty(app)) {
            need_minimal_delay = true;
        }

        led_active = false;
        notification_apply_notification_leds(app, led_values);
        reset_mask |= reset_red_mask;
        reset_mask |= reset_green_mask;
        reset_mask |= reset_blue_mask;

        if(need_minimal_delay) {
            notification_apply_notification_leds(app, led_off_values);
            delay(minimal_delay);
        }
    }

    if(reset_notifications) {
        notification_reset_notification_layer(app, reset_mask);
    }
}

void notification_process_internal_message(NotificationApp* app, NotificationAppMessage* message) {
    uint32_t notification_message_index = 0;
    const NotificationMessage* notification_message;
    notification_message = (*message->sequence)[notification_message_index];

    while(notification_message != NULL) {
        switch(notification_message->type) {
        case NotificationMessageTypeLedDisplay:
            notification_apply_internal_led_layer(
                &app->display,
                notification_settings_get_display_brightness(
                    app, notification_message->data.led.value));
            break;
        case NotificationMessageTypeLedRed:
            notification_apply_internal_led_layer(
                &app->led[0],
                notification_settings_get_rgb_led_brightness(
                    app, notification_message->data.led.value));
            break;
        case NotificationMessageTypeLedGreen:
            notification_apply_internal_led_layer(
                &app->led[1],
                notification_settings_get_rgb_led_brightness(
                    app, notification_message->data.led.value));
            break;
        case NotificationMessageTypeLedBlue:
            notification_apply_internal_led_layer(
                &app->led[2],
                notification_settings_get_rgb_led_brightness(
                    app, notification_message->data.led.value));
            break;
        default:
            break;
        }
        notification_message_index++;
        notification_message = (*message->sequence)[notification_message_index];
    }
}

static bool notification_load_settings(NotificationApp* app) {
    NotificationSettings settings;
    File* file = storage_file_alloc(furi_record_open("storage"));
    const size_t settings_size = sizeof(NotificationSettings);

    FURI_LOG_I(TAG, "loading settings from \"%s\"", NOTIFICATION_SETTINGS_PATH);
    bool fs_result =
        storage_file_open(file, NOTIFICATION_SETTINGS_PATH, FSAM_READ, FSOM_OPEN_EXISTING);

    if(fs_result) {
        uint16_t bytes_count = storage_file_read(file, &settings, settings_size);

        if(bytes_count != settings_size) {
            fs_result = false;
        }
    }

    if(fs_result) {
        FURI_LOG_I(TAG, "load success");

        if(settings.version != NOTIFICATION_SETTINGS_VERSION) {
            FURI_LOG_E(
                TAG, "version(%d != %d) mismatch", settings.version, NOTIFICATION_SETTINGS_VERSION);
        } else {
            osKernelLock();
            memcpy(&app->settings, &settings, settings_size);
            osKernelUnlock();
        }
    } else {
        FURI_LOG_E(TAG, "load failed, %s", storage_file_get_error_desc(file));
    }

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close("storage");

    return fs_result;
};

static bool notification_save_settings(NotificationApp* app) {
    NotificationSettings settings;
    File* file = storage_file_alloc(furi_record_open("storage"));
    const size_t settings_size = sizeof(NotificationSettings);

    FURI_LOG_I(TAG, "saving settings to \"%s\"", NOTIFICATION_SETTINGS_PATH);

    osKernelLock();
    memcpy(&settings, &app->settings, settings_size);
    osKernelUnlock();

    bool fs_result =
        storage_file_open(file, NOTIFICATION_SETTINGS_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS);

    if(fs_result) {
        uint16_t bytes_count = storage_file_write(file, &settings, settings_size);

        if(bytes_count != settings_size) {
            fs_result = false;
        }
    }

    if(fs_result) {
        FURI_LOG_I(TAG, "save success");
    } else {
        FURI_LOG_E(TAG, "save failed, %s", storage_file_get_error_desc(file));
    }

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close("storage");

    return fs_result;
};

static void input_event_callback(const void* value, void* context) {
    NotificationApp* app = context;
    notification_message(app, &sequence_display_on);
}

// App alloc
static NotificationApp* notification_app_alloc() {
    NotificationApp* app = furi_alloc(sizeof(NotificationApp));
    app->queue = osMessageQueueNew(8, sizeof(NotificationAppMessage), NULL);
    app->display_timer = osTimerNew(notification_display_timer, osTimerOnce, app, NULL);

    app->settings.speaker_volume = 1.0f;
    app->settings.display_brightness = 1.0f;
    app->settings.led_brightness = 1.0f;
    app->settings.display_off_delay_ms = 30000;
    app->settings.vibro_on = true;

    app->display.value[LayerInternal] = 0x00;
    app->display.value[LayerNotification] = 0x00;
    app->display.index = LayerInternal;
    app->display.light = LightBacklight;

    app->led[0].value[LayerInternal] = 0x00;
    app->led[0].value[LayerNotification] = 0x00;
    app->led[0].index = LayerInternal;
    app->led[0].light = LightRed;

    app->led[1].value[LayerInternal] = 0x00;
    app->led[1].value[LayerNotification] = 0x00;
    app->led[1].index = LayerInternal;
    app->led[1].light = LightGreen;

    app->led[2].value[LayerInternal] = 0x00;
    app->led[2].value[LayerNotification] = 0x00;
    app->led[2].index = LayerInternal;
    app->led[2].light = LightBlue;

    app->settings.version = NOTIFICATION_SETTINGS_VERSION;

    // display backlight control
    app->event_record = furi_record_open("input_events");
    furi_pubsub_subscribe(app->event_record, input_event_callback, app);
    notification_message(app, &sequence_display_on);

    return app;
};

// App
int32_t notification_srv(void* p) {
    NotificationApp* app = notification_app_alloc();

    if(!notification_load_settings(app)) {
        notification_save_settings(app);
    }

    notification_vibro_off();
    notification_sound_off();
    notification_apply_internal_led_layer(&app->display, 0x00);
    notification_apply_internal_led_layer(&app->led[0], 0x00);
    notification_apply_internal_led_layer(&app->led[1], 0x00);
    notification_apply_internal_led_layer(&app->led[2], 0x00);

    furi_record_create("notification", app);

    NotificationAppMessage message;
    while(1) {
        furi_check(osMessageQueueGet(app->queue, &message, NULL, osWaitForever) == osOK);

        switch(message.type) {
        case NotificationLayerMessage:
            notification_process_notification_message(app, &message);
            break;
        case InternalLayerMessage:
            notification_process_internal_message(app, &message);
            break;
        case SaveSettingsMessage:
            notification_save_settings(app);
            break;
        }

        if(message.back_event != NULL) {
            osEventFlagsSet(message.back_event, NOTIFICATION_EVENT_COMPLETE);
        }
    }

    return 0;
};