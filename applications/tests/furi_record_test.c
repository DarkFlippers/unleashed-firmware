#include <stdio.h>
#include <string.h>
#include "flipper.h"
#include "log.h"

/*
TEST: pipe record

1. create pipe record
2. Open/subscribe to it 
3. write data
4. check that subscriber get data
5. try to read, get error
6. close record
7. try to write, get error
*/

static uint8_t pipe_record_value = 0;

void pipe_record_cb(const void* value, size_t size) {
    // hold value to static var
    pipe_record_value = *((uint8_t*)value);
}

bool test_furi_pipe_record(FuriRecordSubscriber* log) {
    // 1. create pipe record
    if(!furi_create("test/pipe", NULL, 0)) {
        fuprintf(log, "cannot create record\n");
        return false;
    }

    // 2. Open/subscribe to it 
    FuriRecordSubscriber* pipe_record = furi_open(
        "test/pipe", false, false, pipe_record_cb, NULL
    );
    if(pipe_record == NULL) {
        fuprintf(log, "cannot open record\n");
        return false;
    }

    const uint8_t WRITE_VALUE = 1;
    // 3. write data
    if(!furi_write(pipe_record, &WRITE_VALUE, sizeof(uint8_t))) {
        fuprintf(log, "cannot write to record\n");
        return false;
    }

    // 4. check that subscriber get data
    if(pipe_record_value != WRITE_VALUE) {
        fuprintf(log, "wrong value (get %d, write %d)\n", pipe_record_value, WRITE_VALUE);
        return false;
    }

    // 5. try to read, get error
    uint8_t read_value = 0;
    if(furi_read(pipe_record, &read_value, sizeof(uint8_t))) {
        fuprintf(log, "reading from pipe record not allowed\n");
        return false;
    }

    // 6. close record
    furi_close(pipe_record);

    // 7. try to write, get error
    if(furi_write(pipe_record, &WRITE_VALUE, sizeof(uint8_t))) {
        fuprintf(log, "writing to closed record not allowed\n");
        return false;
    }

    return true;
}

/*
TEST: holding data

1. Create holding record
2. Open/Subscribe on it
3. Write data
4. Check that subscriber get data
5. Read and check data
6. Try to write/read wrong size of data
*/

static uint8_t holding_record_value = 0;

void holding_record_cb(const void* value, size_t size) {
    // hold value to static var
    holding_record_value = *((uint8_t*)value);
}

bool test_furi_holding_data(FuriRecordSubscriber* log) {
    // 1. Create holding record
    uint8_t holder = 0;
    if(!furi_create("test/holding", (void*)&holder, sizeof(holder))) {
        fuprintf(log, "cannot create record\n");
        return false;
    }

    // 2. Open/Subscribe on it
    FuriRecordSubscriber* holding_record = furi_open(
        "test/holding", false, false, holding_record_cb, NULL
    );
    if(holding_record == NULL) {
        fuprintf(log, "cannot open record\n");
        return false;
    }

    const uint8_t WRITE_VALUE = 1;
    // 3. write data
    if(!furi_write(holding_record, &WRITE_VALUE, sizeof(uint8_t))) {
        fuprintf(log, "cannot write to record\n");
        return false;
    }

    // 4. check that subscriber get data
    if(holding_record_value != WRITE_VALUE) {
        fuprintf(log, "wrong sub value (get %d, write %d)\n", holding_record_value, WRITE_VALUE);
        return false;
    }

    // 5. Read and check data
    uint8_t read_value = 0;
    if(!furi_read(holding_record, &read_value, sizeof(uint8_t))) {
        fuprintf(log, "cannot read from record\n");
        return false;
    }

    if(read_value != WRITE_VALUE) {
        fuprintf(log, "wrong read value (get %d, write %d)\n", read_value, WRITE_VALUE);
        return false;
    }

    // 6. Try to write/read wrong size of data
    if(furi_write(holding_record, &WRITE_VALUE, 100)) {
        fuprintf(log, "overflowed write not allowed\n");
        return false;
    }

    if(furi_read(holding_record, &read_value, 100)) {
        fuprintf(log, "overflowed read not allowed\n");
        return false;
    }

    return true;
}

/*
TEST: concurrent access

1. Create holding record
2. Open it twice
3. Change value simultaneously in two app and check integrity
*/

// TODO this test broke because mutex in furi is not implemented

typedef struct {
    // a and b must be equal
    uint8_t a;
    uint8_t b;
} ConcurrentValue;

void furi_concurent_app(void* p) {
    FuriRecordSubscriber* log = (FuriRecordSubscriber*)p;

    FuriRecordSubscriber* holding_record = furi_open(
        "test/concurrent", false, false, NULL, NULL
    );
    if(holding_record == NULL) {
        fuprintf(log, "cannot open record\n");
        furiac_exit(NULL);
    }

    for(size_t i = 0; i < 10; i++) {
        ConcurrentValue* value = (ConcurrentValue*)furi_take(holding_record);

        if(value == NULL) {
            fuprintf(log, "cannot take record\n");
            furi_give(holding_record);
            furiac_exit(NULL);
        }
        // emulate read-modify-write broken by context switching
        uint8_t a = value->a;
        uint8_t b = value->b;
        a++;
        b++;
        delay(2); // this is only for test, do not add delay between take/give in prod!
        value->a = a;
        value->b = b;
        furi_give(holding_record);
    }

    furiac_exit(NULL);
}

bool test_furi_concurrent_access(FuriRecordSubscriber* log) {
    // 1. Create holding record
    ConcurrentValue holder = {.a = 0, .b = 0};
    if(!furi_create("test/concurrent", (void*)&holder, sizeof(ConcurrentValue))) {
        fuprintf(log, "cannot create record\n");
        return false;
    }

    // 2. Open it
    FuriRecordSubscriber* holding_record = furi_open(
        "test/concurrent", false, false, NULL, NULL
    );
    if(holding_record == NULL) {
        fuprintf(log, "cannot open record\n");
        return false;
    }

    // 3. Create second app for interact with it
    FuriApp* second_app = furiac_start(
        furi_concurent_app, "furi concurent app", (void*)log
    );

    // 4. multiply ConcurrentValue::a
    for(size_t i = 0; i < 4; i++) {
        ConcurrentValue* value = (ConcurrentValue*)furi_take(holding_record);

        if(value == NULL) {
            fuprintf(log, "cannot take record\n");
            furi_give(holding_record);
            return false;
        }
        // emulate read-modify-write broken by context switching
        uint8_t a = value->a;
        uint8_t b = value->b;
        a++;
        b++;
        value->a = a;
        delay(10); // this is only for test, do not add delay between take/give in prod!
        value->b = b;
        furi_give(holding_record);
    }

    delay(20);

    if(second_app->handler != NULL) {
        fuprintf(log, "second app still alive\n");
        return false;
    }

    if(holder.a != holder.b) {
        fuprintf(log, "broken integrity: a=%d, b=%d\n", holder.a, holder.b);
        return false;
    }

    return true;
}

/*
TEST: non-existent data
1. Try to open non-existent record
2. Check for NULL handler
3. Try to write/read, get error

TODO: implement this test
*/
bool test_furi_nonexistent_data(FuriRecordSubscriber* log) {

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

void mute_record_cb(const void* value, size_t size) {
    // hold value to static var
    mute_last_value = *((uint8_t*)value);
}

void mute_record_state_cb(FlipperRecordState state) {
    mute_last_state = state;
}

void furi_mute_parent_app(void* p) {
    FuriRecordSubscriber* log = (FuriRecordSubscriber*)p;

    // 1. Create pipe record
    if(!furi_create("test/mute", NULL, 0)) {
        fuprintf(log, "cannot create record\n");
        furiac_exit(NULL);
    }

    // 2. Open watch handler: solo=false, no_mute=false, subscribe to data
    FuriRecordSubscriber* watch_handler = furi_open(
        "test/mute", false, false, mute_record_cb, NULL
    );
    if(watch_handler == NULL) {
        fuprintf(log, "cannot open watch handler\n");
        furiac_exit(NULL);
    }

    while(1) {
        // TODO we don't have thread sleep
        delay(100000);
    }
}

bool test_furi_mute_algorithm(FuriRecordSubscriber* log) {
    // 1. Create "parent" application:
    FuriApp* parent_app = furiac_start(
        furi_mute_parent_app, "parent app", (void*)log
    );

    delay(2); // wait creating record

    // 2. Open handler A: solo=false, no_mute=false, NULL subscriber. Subscribe to state.
    FuriRecordSubscriber* handler_a = furi_open(
        "test/mute", false, false, NULL, mute_record_state_cb
    );
    if(handler_a == NULL) {
        fuprintf(log, "cannot open handler A\n");
        return false;
    }

    uint8_t test_counter = 1;

    // Try to write data to A and check subscriber
    if(!furi_write(handler_a, &test_counter, sizeof(uint8_t))) {
        fuprintf(log, "write to A failed\n");
        return false;
    }

    if(mute_last_value != test_counter) {
        fuprintf(log, "value A mismatch: %d vs %d\n", mute_last_value, test_counter);
        return false;
    }

    // 3. Open handler B: solo=true, no_mute=true, NULL subscriber.
    FuriRecordSubscriber* handler_b = furi_open(
        "test/mute", true, true, NULL, NULL
    );
    if(handler_b == NULL) {
        fuprintf(log, "cannot open handler B\n");
        return false;
    }

    // Check A state cb get FlipperRecordStateMute.
    if(mute_last_state != FlipperRecordStateMute) {
        fuprintf(log, "A state is not FlipperRecordStateMute: %d\n", mute_last_state);
        return false;
    }

    test_counter = 2;

    // Try to write data to A and check that subscriber get no data. (muted)
    if(furi_write(handler_a, &test_counter, sizeof(uint8_t))) {
        fuprintf(log, "A not muted\n");
        return false;
    }

    if(mute_last_value == test_counter) {
        fuprintf(log, "value A must be muted\n");
        return false;
    }

    test_counter = 3;


    // Try to write data to B and check that subscriber get data.
    if(!furi_write(handler_b, &test_counter, sizeof(uint8_t))) {
        fuprintf(log, "write to B failed\n");
        return false;
    }

    if(mute_last_value != test_counter) {
        fuprintf(log, "value B mismatch: %d vs %d\n", mute_last_value, test_counter);
        return false;
    }


    // 4. Open hadler C: solo=true, no_mute=false, NULL subscriber.
    FuriRecordSubscriber* handler_c = furi_open(
        "test/mute", true, false, NULL, NULL
    );
    if(handler_c == NULL) {
        fuprintf(log, "cannot open handler C\n");
        return false;
    }

    // TODO: Try to write data to A and check that subscriber get no data. (muted)
    // TODO: Try to write data to B and check that subscriber get data. (not muted because open with no_mute)
    // TODO: Try to write data to C and check that subscriber get data.

    // 5. Open handler D: solo=false, no_mute=false, NULL subscriber.
    FuriRecordSubscriber* handler_d = furi_open(
        "test/mute", false, false, NULL, NULL
    );
    if(handler_d == NULL) {
        fuprintf(log, "cannot open handler D\n");
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
        fuprintf(log, "kill parent_app fail\n");
        return false;
    }

    // TODO: Check A state cb get FlipperRecordStateDeleted

    return true;
}