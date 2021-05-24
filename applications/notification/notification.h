#pragma once
#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NotificationApp NotificationApp;
typedef struct {
    float frequency;
    float pwm;
} NotificationMessageDataSound;

typedef struct {
    uint8_t value;
} NotificationMessageDataLed;

typedef struct {
    bool on;
} NotificationMessageDataVibro;

typedef struct {
    uint32_t length;
} NotificationMessageDataDelay;

typedef union {
    NotificationMessageDataSound sound;
    NotificationMessageDataLed led;
    NotificationMessageDataVibro vibro;
    NotificationMessageDataDelay delay;
} NotificationMessageData;

typedef enum {
    NotificationMessageTypeVibro,
    NotificationMessageTypeSoundOn,
    NotificationMessageTypeSoundOff,
    NotificationMessageTypeLedRed,
    NotificationMessageTypeLedGreen,
    NotificationMessageTypeLedBlue,
    NotificationMessageTypeDelay,
    NotificationMessageTypeLedDisplay,
    NotificationMessageTypeDoNotReset,
} NotificationMessageType;

typedef struct {
    NotificationMessageType type;
    NotificationMessageData data;
} NotificationMessage;

typedef const NotificationMessage* NotificationSequence[];

void notification_message(NotificationApp* app, const NotificationSequence* sequence);
void notification_message_block(NotificationApp* app, const NotificationSequence* sequence);

/**
 * @brief Send internal (apply to permanent layer) notification message. Think twice before use.
 * 
 * @param app notification record content
 * @param sequence notification sequence
 */
void notification_internal_message(NotificationApp* app, const NotificationSequence* sequence);

/**
 * @brief Send internal (apply to permanent layer) notification message and wait for notification end. Think twice before use.
 * 
 * @param app notification record content
 * @param sequence notification sequence
 */
void notification_internal_message_block(
    NotificationApp* app,
    const NotificationSequence* sequence);

#ifdef __cplusplus
}
#endif