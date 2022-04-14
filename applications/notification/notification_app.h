#include <furi.h>
#include <furi_hal.h>
#include "notification.h"
#include "notification_messages.h"

#define NOTIFICATION_LED_COUNT 3
#define NOTIFICATION_EVENT_COMPLETE 0x00000001U

typedef enum {
    NotificationLayerMessage,
    InternalLayerMessage,
    SaveSettingsMessage,
} NotificationAppMessageType;

typedef struct {
    const NotificationSequence* sequence;
    NotificationAppMessageType type;
    osEventFlagsId_t back_event;
} NotificationAppMessage;

typedef enum {
    LayerInternal = 0,
    LayerNotification = 1,
    LayerMAX = 2,
} NotificationLedLayerIndex;

typedef struct {
    uint8_t value[LayerMAX];
    NotificationLedLayerIndex index;
    Light light;
} NotificationLedLayer;

#define NOTIFICATION_SETTINGS_VERSION 0x01
#define NOTIFICATION_SETTINGS_PATH "/int/notification.settings"

typedef struct {
    uint8_t version;
    float display_brightness;
    float led_brightness;
    float speaker_volume;
    uint32_t display_off_delay_ms;
    bool vibro_on;
} NotificationSettings;

struct NotificationApp {
    osMessageQueueId_t queue;
    FuriPubSub* event_record;
    osTimerId_t display_timer;

    NotificationLedLayer display;
    NotificationLedLayer led[NOTIFICATION_LED_COUNT];
    uint8_t display_led_lock;

    NotificationSettings settings;
};

void notification_message_save_settings(NotificationApp* app);
