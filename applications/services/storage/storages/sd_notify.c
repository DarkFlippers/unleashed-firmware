#include "sd_notify.h"

static const NotificationSequence sd_sequence_success = {
    &message_green_255,
    &message_delay_50,
    &message_green_0,
    &message_delay_50,
    &message_green_255,
    &message_delay_50,
    &message_green_0,
    &message_delay_50,
    &message_green_255,
    &message_delay_50,
    &message_green_0,
    &message_delay_50,
    NULL,
};

static const NotificationSequence sd_sequence_error = {
    &message_red_255,
    &message_delay_50,
    &message_red_0,
    &message_delay_50,
    &message_red_255,
    &message_delay_50,
    &message_red_0,
    &message_delay_50,
    &message_red_255,
    &message_delay_50,
    &message_red_0,
    &message_delay_50,
    NULL,
};

static const NotificationSequence sd_sequence_eject = {
    &message_blue_255,
    &message_delay_50,
    &message_blue_0,
    &message_delay_50,
    &message_blue_255,
    &message_delay_50,
    &message_blue_0,
    &message_delay_50,
    &message_blue_255,
    &message_delay_50,
    &message_blue_0,
    &message_delay_50,
    NULL,
};

static const NotificationSequence sd_sequence_wait = {
    &message_red_255,
    &message_blue_255,
    &message_do_not_reset,
    NULL,
};

static const NotificationSequence sd_sequence_wait_off = {
    &message_red_0,
    &message_blue_0,
    NULL,
};

void sd_notify_wait(NotificationApp* notifications) {
    notification_message(notifications, &sd_sequence_wait);
}

void sd_notify_wait_off(NotificationApp* notifications) {
    notification_message(notifications, &sd_sequence_wait_off);
}

void sd_notify_success(NotificationApp* notifications) {
    notification_message(notifications, &sd_sequence_success);
}

void sd_notify_eject(NotificationApp* notifications) {
    notification_message(notifications, &sd_sequence_eject);
}

void sd_notify_error(NotificationApp* notifications) {
    notification_message(notifications, &sd_sequence_error);
}
