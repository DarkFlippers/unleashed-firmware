#include <stdio.h>
#include <string.h>
#include "flipper.h"
#include "flipper_v2.h"
#include "log.h"
#include "minunit.h"

void test_furi_create_open() {
    // 1. Create record
    uint8_t test_data = 0;
    mu_check(furi_create("test/holding", (void*)&test_data));

    // 2. Open it
    void* record = furi_open("test/holding");
    mu_assert_pointers_eq(record, &test_data);
}

/*
TEST: non-existent data
1. Try to open non-existent record
2. Check for NULL handler
3. Try to write/read, get error

TODO: implement this test
*/
bool test_furi_nonexistent_data() {
    return true;
}

/*
TEST: mute algorithm
1. Create "parent" application:
    1. Create pipe record
    2. Open watch handler: no_mute=false, solo=false, subscribe to data.

2. Open handler A: no_mute=false, solo=false, NULL subscriber. Subscribe to state.
Try to write data to A and check subscriber.

3. Open handler B: no_mute=true, solo=true, NULL subscriber.
Check A state cb get FlipperRecordStateMute.
Try to write data to A and check that subscriber get no data. (muted)
Try to write data to B and check that subscriber get data.

TODO: test 3 not pass beacuse state callback not implemented

4. Open hadler C: no_mute=false, solo=true, NULL subscriber.
Try to write data to A and check that subscriber get no data. (muted)
Try to write data to B and check that subscriber get data. (not muted because open with no_mute)
Try to write data to C and check that subscriber get data.

5. Open handler D: no_mute=false, solo=false, NULL subscriber.
Try to write data to A and check that subscriber get no data. (muted)
Try to write data to B and check that subscriber get data. (not muted because open with no_mute)
Try to write data to C and check that subscriber get data. (not muted because D open without solo)
Try to write data to D and check that subscriber get data.

6. Close C, close B.
Check A state cb get FlipperRecordStateUnmute
Try to write data to A and check that subscriber get data. (unmuted)
Try to write data to D and check that subscriber get data.

TODO: test 6 not pass beacuse cleanup is not implemented
TODO: test 6 not pass because mute algorithm is unfinished.

7. Exit "parent application"
Check A state cb get FlipperRecordStateDeleted

TODO: test 7 not pass beacuse cleanup is not implemented
*/

static uint8_t mute_last_value = 0;
static FlipperRecordState mute_last_state = 255;

void mute_record_cb(const void* value, size_t size, void* ctx) {
    // hold value to static var
    mute_last_value = *((uint8_t*)value);
}

void mute_record_state_cb(FlipperRecordState state, void* ctx) {
    mute_last_state = state;
}

void furi_mute_parent_app(void* p) {
    // 1. Create pipe record
    if(!furi_create_deprecated("test/mute", NULL, 0)) {
        printf("cannot create record\n");
        furiac_exit(NULL);
    }

    // 2. Open watch handler: solo=false, no_mute=false, subscribe to data
    FuriRecordSubscriber* watch_handler =
        furi_open_deprecated("test/mute", false, false, mute_record_cb, NULL, NULL);
    if(watch_handler == NULL) {
        printf("cannot open watch handler\n");
        furiac_exit(NULL);
    }

    while(1) {
        // TODO we don't have thread sleep
        delay(100000);
    }
}

bool test_furi_mute_algorithm() {
    // 1. Create "parent" application:
    FuriApp* parent_app = furiac_start(furi_mute_parent_app, "parent app", NULL);

    delay(2); // wait creating record

    // 2. Open handler A: solo=false, no_mute=false, NULL subscriber. Subscribe to state.
    FuriRecordSubscriber* handler_a =
        furi_open_deprecated("test/mute", false, false, NULL, mute_record_state_cb, NULL);
    if(handler_a == NULL) {
        printf("cannot open handler A\n");
        return false;
    }

    uint8_t test_counter = 1;

    // Try to write data to A and check subscriber
    if(!furi_write(handler_a, &test_counter, sizeof(uint8_t))) {
        printf("write to A failed\n");
        return false;
    }

    if(mute_last_value != test_counter) {
        printf("value A mismatch: %d vs %d\n", mute_last_value, test_counter);
        return false;
    }

    // 3. Open handler B: solo=true, no_mute=true, NULL subscriber.
    FuriRecordSubscriber* handler_b =
        furi_open_deprecated("test/mute", true, true, NULL, NULL, NULL);
    if(handler_b == NULL) {
        printf("cannot open handler B\n");
        return false;
    }

    // Check A state cb get FlipperRecordStateMute.
    if(mute_last_state != FlipperRecordStateMute) {
        printf("A state is not FlipperRecordStateMute: %d\n", mute_last_state);
        return false;
    }

    test_counter = 2;

    // Try to write data to A and check that subscriber get no data. (muted)
    if(furi_write(handler_a, &test_counter, sizeof(uint8_t))) {
        printf("A not muted\n");
        return false;
    }

    if(mute_last_value == test_counter) {
        printf("value A must be muted\n");
        return false;
    }

    test_counter = 3;

    // Try to write data to B and check that subscriber get data.
    if(!furi_write(handler_b, &test_counter, sizeof(uint8_t))) {
        printf("write to B failed\n");
        return false;
    }

    if(mute_last_value != test_counter) {
        printf("value B mismatch: %d vs %d\n", mute_last_value, test_counter);
        return false;
    }

    // 4. Open hadler C: solo=true, no_mute=false, NULL subscriber.
    FuriRecordSubscriber* handler_c =
        furi_open_deprecated("test/mute", true, false, NULL, NULL, NULL);
    if(handler_c == NULL) {
        printf("cannot open handler C\n");
        return false;
    }

    // TODO: Try to write data to A and check that subscriber get no data. (muted)
    // TODO: Try to write data to B and check that subscriber get data. (not muted because open with no_mute)
    // TODO: Try to write data to C and check that subscriber get data.

    // 5. Open handler D: solo=false, no_mute=false, NULL subscriber.
    FuriRecordSubscriber* handler_d =
        furi_open_deprecated("test/mute", false, false, NULL, NULL, NULL);
    if(handler_d == NULL) {
        printf("cannot open handler D\n");
        return false;
    }

    // TODO: Try to write data to A and check that subscriber get no data. (muted)
    // TODO: Try to write data to B and check that subscriber get data. (not muted because open with no_mute)
    // TODO: Try to write data to C and check that subscriber get data. (not muted because D open without solo)
    // TODO: Try to write data to D and check that subscriber get data.

    // 6. Close C, close B.
    // TODO: Check A state cb get FlipperRecordStateUnmute
    // TODO: Try to write data to A and check that subscriber get data. (unmuted)
    // TODO: Try to write data to D and check that subscriber get data.

    // 7. Exit "parent application"
    if(!furiac_kill(parent_app)) {
        printf("kill parent_app fail\n");
        return false;
    }

    // TODO: Check A state cb get FlipperRecordStateDeleted

    return true;
}