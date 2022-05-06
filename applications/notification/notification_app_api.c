#include <furi.h>
#include <furi_hal.h>
#include "notification.h"
#include "notification_messages.h"
#include "notification_app.h"

void notification_message(NotificationApp* app, const NotificationSequence* sequence) {
    NotificationAppMessage m = {
        .type = NotificationLayerMessage, .sequence = sequence, .back_event = NULL};
    furi_check(osMessageQueuePut(app->queue, &m, 0, osWaitForever) == osOK);
};

void notification_internal_message(NotificationApp* app, const NotificationSequence* sequence) {
    NotificationAppMessage m = {
        .type = InternalLayerMessage, .sequence = sequence, .back_event = NULL};
    furi_check(osMessageQueuePut(app->queue, &m, 0, osWaitForever) == osOK);
};

void notification_message_block(NotificationApp* app, const NotificationSequence* sequence) {
    NotificationAppMessage m = {
        .type = NotificationLayerMessage,
        .sequence = sequence,
        .back_event = osEventFlagsNew(NULL)};
    furi_check(osMessageQueuePut(app->queue, &m, 0, osWaitForever) == osOK);
    osEventFlagsWait(m.back_event, NOTIFICATION_EVENT_COMPLETE, osFlagsWaitAny, osWaitForever);
    osEventFlagsDelete(m.back_event);
};

void notification_internal_message_block(
    NotificationApp* app,
    const NotificationSequence* sequence) {
    NotificationAppMessage m = {
        .type = InternalLayerMessage, .sequence = sequence, .back_event = osEventFlagsNew(NULL)};
    furi_check(osMessageQueuePut(app->queue, &m, 0, osWaitForever) == osOK);
    osEventFlagsWait(m.back_event, NOTIFICATION_EVENT_COMPLETE, osFlagsWaitAny, osWaitForever);
    osEventFlagsDelete(m.back_event);
};
