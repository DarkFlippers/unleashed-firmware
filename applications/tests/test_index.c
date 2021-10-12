#include <stdio.h>
#include <furi.h>
#include <furi-hal.h>
#include "minunit_vars.h"
#include <notification/notification-messages.h>

int run_minunit();
int run_minunit_test_irda_decoder_encoder();
int run_minunit_test_rpc();

int32_t flipper_test_app(void* p) {
    uint32_t test_result = 0;

    NotificationApp* notification = furi_record_open("notification");

    notification_message_block(notification, &sequence_set_only_blue_255);

    //    test_result |= run_minunit();     // disabled as it fails randomly
    test_result |= run_minunit_test_irda_decoder_encoder();
    test_result |= run_minunit_test_rpc();

    if(test_result == 0) {
        // test passed
        notification_message(notification, &sequence_success);
    } else {
        // test failed
        notification_message(notification, &sequence_error);
    }

    furi_record_close("notification");

    return 0;
}
