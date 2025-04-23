#include <furi.h>
#include <furi_hal.h>
#include "notification.h"
#include "notification_messages.h"
#include "notification_settings_filename.h"
#include <SK6805.h>
#include <math.h>

#define NOTIFICATION_LED_COUNT      3
#define NOTIFICATION_EVENT_COMPLETE 0x00000001U

typedef enum {
    NotificationLayerMessage,
    InternalLayerMessage,
    SaveSettingsMessage,
    LoadSettingsMessage,
} NotificationAppMessageType;

typedef struct {
    const NotificationSequence* sequence;
    NotificationAppMessageType type;
    FuriEventFlag* back_event;
} NotificationAppMessage;

typedef enum {
    LayerInternal = 0,
    LayerNotification = 1,
    LayerMAX = 2,
} NotificationLedLayerIndex;

typedef struct {
    uint8_t value_last[LayerMAX];
    uint8_t value[LayerMAX];
    NotificationLedLayerIndex index;
    Light light;
} NotificationLedLayer;

#define NOTIFICATION_SETTINGS_VERSION 0x05
#define NOTIFICATION_SETTINGS_PATH    INT_PATH(NOTIFICATION_SETTINGS_FILE_NAME)

typedef struct {
    //Common settings
    uint8_t rgb_backlight_installed;

    // static gradient mode settings
    uint8_t led_2_color_index;
    uint8_t led_1_color_index;
    uint8_t led_0_color_index;

    // rainbow mode setings
    uint32_t rainbow_mode;
    uint32_t rainbow_speed_ms;
    uint16_t rainbow_step;
    uint8_t rainbow_saturation;
    uint8_t rainbow_wide;
} RGBBacklightSettings;

typedef struct {
    uint8_t version;
    float display_brightness;
    float led_brightness;
    float speaker_volume;
    uint32_t display_off_delay_ms;
    int8_t contrast;
    bool vibro_on;
    float night_shift;
    uint32_t night_shift_start;
    uint32_t night_shift_end;
    bool lcd_inversion;
    RGBBacklightSettings rgb;
} NotificationSettings;

struct NotificationApp {
    FuriMessageQueue* queue;
    FuriPubSub* event_record;
    FuriTimer* display_timer;

    NotificationLedLayer display;
    NotificationLedLayer led[NOTIFICATION_LED_COUNT];
    uint8_t display_led_lock;

    NotificationSettings settings;

    FuriTimer* night_shift_timer;
    float current_night_shift;

    FuriTimer* rainbow_timer;
    uint16_t rainbow_hue;
    uint8_t rainbow_red;
    uint8_t rainbow_green;
    uint8_t rainbow_blue;
};

void notification_message_save_settings(NotificationApp* app);
void night_shift_timer_start(NotificationApp* app);
void night_shift_timer_stop(NotificationApp* app);
void rgb_backlight_update(float brightness);
void rgb_backlight_set_led_static_color(uint8_t led, uint8_t index);
void rainbow_timer_start(NotificationApp* app);
void rainbow_timer_stop(NotificationApp* app);
void rainbow_timer_starter(NotificationApp* app);
const char* rgb_backlight_get_color_text(uint8_t index);
uint8_t rgb_backlight_get_color_count(void);
void set_rgb_backlight_installed_variable(uint8_t var);
