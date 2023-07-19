#include "camera_suite_led.h"
#include "../camera_suite.h"

void camera_suite_led_set_rgb(void* context, int red, int green, int blue) {
    CameraSuite* app = context;
    if(app->led != 1) {
        return;
    }
    NotificationMessage notification_led_message_1;
    notification_led_message_1.type = NotificationMessageTypeLedRed;
    NotificationMessage notification_led_message_2;
    notification_led_message_2.type = NotificationMessageTypeLedGreen;
    NotificationMessage notification_led_message_3;
    notification_led_message_3.type = NotificationMessageTypeLedBlue;

    notification_led_message_1.data.led.value = red;
    notification_led_message_2.data.led.value = green;
    notification_led_message_3.data.led.value = blue;
    const NotificationSequence notification_sequence = {
        &notification_led_message_1,
        &notification_led_message_2,
        &notification_led_message_3,
        &message_do_not_reset,
        NULL,
    };
    notification_message(app->notification, &notification_sequence);
    //Delay, prevent removal from RAM before LED value set.
    furi_thread_flags_wait(0, FuriFlagWaitAny, 10);
}

void camera_suite_led_reset(void* context) {
    CameraSuite* app = context;
    notification_message(app->notification, &sequence_reset_red);
    notification_message(app->notification, &sequence_reset_green);
    notification_message(app->notification, &sequence_reset_blue);
    //Delay, prevent removal from RAM before LED value set.
    furi_thread_flags_wait(0, FuriFlagWaitAny, 300);
}
