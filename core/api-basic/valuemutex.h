#pragma once

#include "flipper.h"

/*
== ValueMutex ==

The most simple concept is ValueMutex.
It is wrapper around mutex and value pointer.
You can take and give mutex to work with value and read and write value.
*/

typedef struct {
    void* value;
    size_t size;
    osMutexId_t mutex;
} ValueMutex;

/*
Creates ValueMutex.
*/
bool init_mutex(ValueMutex* valuemutex, void* value, size_t size);

/*
Call for work with data stored in mutex.
Returns pointer to data if success, NULL otherwise.
*/
void* acquire_mutex(ValueMutex* valuemutex, uint32_t timeout);

/*
Helper: infinitly wait for mutex
*/
static inline void* acquire_mutex_block(ValueMutex* valuemutex) {
    return acquire_mutex(valuemutex, osWaitForever);
}

/*
Release mutex after end of work with data.
Call `release_mutex` and pass ValueData instance and pointer to data.
*/
bool release_mutex(ValueMutex* valuemutex, void* value);

/*
Instead of take-access-give sequence you can use `read_mutex` and `write_mutex` functions.
Both functions return true in case of success, false otherwise.
*/
bool read_mutex(ValueMutex* valuemutex, void* data, size_t len, uint32_t timeout);

bool write_mutex(ValueMutex* valuemutex, void* data, size_t len, uint32_t timeout);

inline static bool write_mutex_block(ValueMutex* valuemutex, void* data, size_t len) {
    return write_mutex(valuemutex, data, len, osWaitForever);
}

inline static bool read_mutex_block(ValueMutex* valuemutex, void* data, size_t len) {
    return read_mutex(valuemutex, data, len, osWaitForever);
}

/*

Usage example

```C
// MANIFEST
// name="example-provider-app"
// stack=128

void provider_app(void* _p) {
    // create record with mutex
    uint32_t example_value = 0;
    ValueMutex example_mutex;
    // call `init_mutex`.
    if(!init_mutex(&example_mutex, (void*)&example_value, sizeof(uint32_t))) {
        printf("critical error\n");
        flapp_exit(NULL);
    }

    if(furi_create("provider/example", (void*)&example_mutex)) {
        printf("critical error\n");
        flapp_exit(NULL);
    }

    // we are ready to provide record to other apps
    flapp_ready();

    // get value and increment it
    while(1) {
        uint32_t* value = acquire_mutex(&example_mutex, OsWaitForever);
        if(value != NULL) {
            value++;
        }
        release_mutex(&example_mutex, value);

        osDelay(100);
    }
}

// MANIFEST
// name="example-consumer-app"
// stack=128
// require="example-provider-app"
void consumer_app(void* _p) {
    // this app run after flapp_ready call in all requirements app

    // open mutex value
    ValueMutex* counter_mutex = furi_open("provider/example");
    if(counter_mutex == NULL) {
        printf("critical error\n");
        flapp_exit(NULL);
    }

    // continously read value every 1s
    uint32_t counter;
    while(1) {
        if(read_mutex(counter_mutex, &counter, sizeof(counter), OsWaitForever)) {
            printf("counter value: %d\n", counter);
        }

        osDelay(1000);
    }
}
```
*/