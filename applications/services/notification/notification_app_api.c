#include <furi.h>
#include <furi_hal.h>
#include "notification.h"
#include "notification_messages.h"
#include "notification_app.h"

void notification_message(NotificationApp* app, const NotificationSequence* sequence) {
    NotificationAppMessage m = {
        .type = NotificationLayerMessage, .sequence = sequence, .back_event = NULL};
    furi_check(furi_message_queue_put(app->queue, &m, FuriWaitForever) == FuriStatusOk);
};

void notification_internal_message(NotificationApp* app, const NotificationSequence* sequence) {
    NotificationAppMessage m = {
        .type = InternalLayerMessage, .sequence = sequence, .back_event = NULL};
    furi_check(furi_message_queue_put(app->queue, &m, FuriWaitForever) == FuriStatusOk);
};

void notification_message_block(NotificationApp* app, const NotificationSequence* sequence) {
    NotificationAppMessage m = {
        .type = NotificationLayerMessage,
        .sequence = sequence,
        .back_event = furi_event_flag_alloc()};
    furi_check(furi_message_queue_put(app->queue, &m, FuriWaitForever) == FuriStatusOk);
    furi_event_flag_wait(
        m.back_event, NOTIFICATION_EVENT_COMPLETE, FuriFlagWaitAny, FuriWaitForever);
    furi_event_flag_free(m.back_event);
};

void notification_internal_message_block(
    NotificationApp* app,
    const NotificationSequence* sequence) {
    NotificationAppMessage m = {
        .type = InternalLayerMessage, .sequence = sequence, .back_event = furi_event_flag_alloc()};
    furi_check(furi_message_queue_put(app->queue, &m, FuriWaitForever) == FuriStatusOk);
    furi_event_flag_wait(
        m.back_event, NOTIFICATION_EVENT_COMPLETE, FuriFlagWaitAny, FuriWaitForever);
    furi_event_flag_free(m.back_event);
};
