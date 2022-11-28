#pragma once

#include <stdbool.h>
#include "mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * == ValueMutex ==

 * The most simple concept is ValueMutex.
 * It is wrapper around mutex and value pointer.
 * You can take and give mutex to work with value and read and write value.
 */

typedef struct {
    void* value;
    size_t size;
    FuriMutex* mutex;
} ValueMutex;

/**
 * Creates ValueMutex.
 */
bool init_mutex(ValueMutex* valuemutex, void* value, size_t size);

/**
 * Free resources allocated by `init_mutex`.
 * This function doesn't free the memory occupied by `ValueMutex` itself.
 */
bool delete_mutex(ValueMutex* valuemutex);

/**
 * Call for work with data stored in mutex.
 * @return pointer to data if success, NULL otherwise.
 */
void* acquire_mutex(ValueMutex* valuemutex, uint32_t timeout);

/**
 * Helper: infinitely wait for mutex
 */
static inline void* acquire_mutex_block(ValueMutex* valuemutex) {
    return acquire_mutex(valuemutex, FuriWaitForever);
}

/**
 * With statement for value mutex, acts as lambda
 * @param name a resource name, const char*
 * @param function_body a (){} lambda declaration,
 * executed within you parent function context.
 */
#define with_value_mutex(value_mutex, function_body) \
    {                                                \
        void* p = acquire_mutex_block(value_mutex);  \
        furi_check(p);                               \
        ({ void __fn__ function_body __fn__; })(p);  \
        release_mutex(value_mutex, p);               \
    }

/**
 * Release mutex after end of work with data.
 * Call `release_mutex` and pass ValueData instance and pointer to data.
 */
bool release_mutex(ValueMutex* valuemutex, const void* value);

/**
 * Instead of take-access-give sequence you can use `read_mutex` and `write_mutex` functions.
 * Both functions return true in case of success, false otherwise.
 */
bool read_mutex(ValueMutex* valuemutex, void* data, size_t len, uint32_t timeout);

bool write_mutex(ValueMutex* valuemutex, void* data, size_t len, uint32_t timeout);

inline static bool write_mutex_block(ValueMutex* valuemutex, void* data, size_t len) {
    return write_mutex(valuemutex, data, len, FuriWaitForever);
}

inline static bool read_mutex_block(ValueMutex* valuemutex, void* data, size_t len) {
    return read_mutex(valuemutex, data, len, FuriWaitForever);
}

#ifdef __cplusplus
}
#endif

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

    furi_record_create("provider/example", (void*)&example_mutex);

    // we are ready to provide record to other apps
    flapp_ready();

    // get value and increment it
    while(1) {
        uint32_t* value = acquire_mutex(&example_mutex, OsWaitForever);
        if(value != NULL) {
            value++;
        }
        release_mutex(&example_mutex, value);

        furi_delay_ms(100);
    }
}

// MANIFEST
// name="example-consumer-app"
// stack=128
// require="example-provider-app"
void consumer_app(void* _p) {
    // this app run after flapp_ready call in all requirements app

    // open mutex value
    ValueMutex* counter_mutex = furi_record_open("provider/example");
    if(counter_mutex == NULL) {
        printf("critical error\n");
        flapp_exit(NULL);
    }

    // continuously read value every 1s
    uint32_t counter;
    while(1) {
        if(read_mutex(counter_mutex, &counter, sizeof(counter), OsWaitForever)) {
            printf("counter value: %d\n", counter);
        }

        furi_delay_ms(1000);
    }
}
```
*/
