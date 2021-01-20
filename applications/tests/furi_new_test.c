#include <stdio.h>
#include <string.h>
#include <furi.h>
#include "minunit.h"
#include "furi-new.h"

const int int_value_init = 0x1234;
const int int_value_changed = 0x5678;
osMessageQueueId_t test_messages;

typedef struct {
    char text[256];
    bool result;
} test_message;

#define SEND_MESSAGE(value, data)                                            \
    {                                                                        \
        message.result = value;                                              \
        snprintf(message.text, 256, "Error at line %d, %s", __LINE__, data); \
        osMessageQueuePut(test_messages, &message, 0U, 0U);                  \
    }

void _furi_new_wait() {
    osThreadFlagsWait(0x0001U, osFlagsWaitAny, osWaitForever);
}

void _furi_new_continue(FuriAppId thread_id) {
    osThreadFlagsSet(thread_id, 0x0001U);
}

void _furi_new_main_app(void* p) {
    test_message message;

    _furi_new_wait();

    int another_test_value = int_value_init;
    furi_record_create("test/another_app_record", &another_test_value);

    SEND_MESSAGE(false, "dummy text");

    new_flapp_app_exit();
}

void test_furi_new() {
    test_message message;
    test_messages = osMessageQueueNew(1, sizeof(test_message), NULL);

    // init core
    new_furi_init();

    // launch test thread
    FuriAppId main_app = new_flapp_app_start(_furi_new_main_app, "main_app", 512, NULL);
    _furi_new_continue(main_app);

    while(1) {
        if(osMessageQueueGet(test_messages, &message, NULL, osWaitForever) == osOK) {
            if(message.result == true) {
                break;
            } else {
                mu_assert(false, message.text);
            }
        }
    };

    /*
    // test that "create" wont affect pointer value
    furi_record_create("test/record", &test_value);
    mu_assert_int_eq(test_value, int_value_init);

    // test that we get correct pointer
    int* test_value_pointer = furi_record_open("test/record");
    mu_assert_pointers_not_eq(test_value_pointer, NULL);
    mu_assert_pointers_eq(test_value_pointer, &test_value);

    *test_value_pointer = int_value_changed;
    mu_assert_int_eq(test_value, int_value_changed);

    // start another app
    new_record_available = osSemaphoreNew(1, 1, NULL);
    osSemaphoreAcquire(new_record_available, osWaitForever);

    osThreadAttr_t another_app_attr = {.name = "another_app", .stack_size = 512};
    osThreadId_t player = osThreadNew(another_app, NULL, &another_app_attr);

    // wait until app create record
    osSemaphoreAcquire(new_record_available, osWaitForever);

    // open record, test that record pointed to int_value_init
    test_value_pointer = furi_record_open("test/another_app_record");
    mu_assert_pointers_not_eq(test_value_pointer, NULL);
    mu_assert_int_eq(*test_value_pointer, int_value_init);

    // test that we can close, (unsubscribe) from record
    bool close_result = new_furi_close("test/another_app_record");
    mu_assert(close_result, "cannot close record");
    */
}