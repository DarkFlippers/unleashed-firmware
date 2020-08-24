#include "furi.h"
#include "cmsis_os.h"
#include <string.h>

#define DEBUG

#ifdef DEBUG
#include <stdio.h>
#endif

#define MAX_RECORD_COUNT 32

static FuriRecord records[MAX_RECORD_COUNT];
static size_t current_buffer_idx = 0;

// find record pointer by name
static FuriRecord* find_record(const char* name) {
    if(name == NULL) return NULL;

    FuriRecord* res = NULL;
    for(size_t i = 0; i < MAX_RECORD_COUNT; i++) {
        if(records[i].name != NULL && strcmp(name, records[i].name) == 0) {
            res = &records[i];
        }
    }

    return res;
}

bool furi_create(const char* name, void* value, size_t size) {
    #ifdef DEBUG
        printf("[FURI] creating %s record\n", name);
    #endif

    if(current_buffer_idx >= MAX_RECORD_COUNT) {
        // max record count exceed
        #ifdef DEBUG
            printf("[FURI] max record count exceed\n");
        #endif
        return NULL;
    }

    records[current_buffer_idx].mute_counter = 0;
    records[current_buffer_idx].mutex = xSemaphoreCreateMutexStatic(
        &records[current_buffer_idx].mutex_buffer
    );
    records[current_buffer_idx].value = value;
    records[current_buffer_idx].size = size;
    records[current_buffer_idx].name = name;

    for(size_t i = 0; i < MAX_RECORD_SUBSCRIBERS; i++) {
        records[current_buffer_idx].subscribers[i].allocated = false;
    }

    return true;
}

FuriRecordHandler furi_open(
    const char* name,
    bool solo,
    bool no_mute,
    FlipperRecordCallback value_callback,
    FlipperRecordStateCallback state_callback
) {
    #ifdef DEBUG
        printf("[FURI] opening %s record\n", name);
    #endif

    // get furi record by name
    FuriRecord* record = find_record(name);

    if(record == NULL) {
        // cannot find record
        #ifdef DEBUG
            printf("[FURI] cannot find record %s\n", name);
        #endif

        FuriRecordHandler res = {.record = NULL, .subscriber = NULL};
        return res;
    }

    // allocate subscriber
    FuriRecordSubscriber* subscriber = NULL;

    for(size_t i = 0; i < MAX_RECORD_SUBSCRIBERS; i++) {
        if(!records[current_buffer_idx].subscribers[i].allocated) {
            subscriber = &records[current_buffer_idx].subscribers[i];
            break;
        }
    }

    if(subscriber == NULL) {
        // cannot add subscriber (full)
        #ifdef DEBUG
            printf("[FURI] cannot add subscriber (full)\n");
        #endif

        FuriRecordHandler res = {.record = NULL, .subscriber = NULL};
        return res;
    }

    // increase mute_counter
    if(solo) {
        record->mute_counter++;
    }

    // set all parameters
    subscriber->allocated = true;
    subscriber->mute_counter = record->mute_counter;
    subscriber->no_mute = no_mute;
    subscriber->cb = value_callback;
    subscriber->state_cb = state_callback;

    // register record in application
    FuriApp* current_task = find_task(xTaskGetCurrentTaskHandle());

    current_task->records[current_task->records_count] = record;
    current_task->records_count++;

    FuriRecordHandler res = {.record = record, .subscriber = subscriber};
    return res;
}


void furi_close(FuriRecordHandler* handler) {
    #ifdef DEBUG
        printf("[FURI] closing %s record\n", handler->record->name);
    #endif

    // deallocate subscriber
    handler->subscriber->allocated = false;

    // set mute counter to next max value
    uint8_t max_mute_counter = 0;
    for(size_t i = 0; i < MAX_RECORD_SUBSCRIBERS; i++) {
        if(handler->record->subscribers[i].allocated) {
            if(handler->record->subscribers[i].mute_counter > max_mute_counter) {
                max_mute_counter = handler->record->subscribers[i].mute_counter;
            }
        }
    }
    handler->record->mute_counter = max_mute_counter;
}

static void furi_notify(FuriRecordHandler* handler, const void* value, size_t size) {
    for(size_t i = 0; i < MAX_RECORD_SUBSCRIBERS; i++) {
        if(handler->record->subscribers[i].allocated) {
            if(handler->record->subscribers[i].cb != NULL) {
                handler->record->subscribers[i].cb(value, size);
            }
        }
    }
}

void* furi_take(FuriRecordHandler* handler) {
    // take mutex

    return handler->record->value;
}

void furi_give(FuriRecordHandler* handler) {
    // release mutex
}

bool furi_read(FuriRecordHandler* handler, void* value, size_t size) {
    #ifdef DEBUG
        printf("[FURI] read from %s\n", handler->record->name);
    #endif

    if(handler == NULL || handler->record == NULL || value == NULL) return false;

    if(size > handler->record->size) return false;

    // return false if read from pipe
    if(handler->record->value == NULL) return false;

    furi_take(handler);
    memcpy(value, handler->record->value, size);
    furi_give(handler);
    furi_notify(handler, value, size);

    return true;
}

bool furi_write(FuriRecordHandler* handler, const void* value, size_t size) {
    #ifdef DEBUG
        printf("[FURI] write to %s\n", handler->record->name);
    #endif

    if(handler == NULL || handler->record == NULL || value == NULL) return false;

    // check if closed
    if(!handler->subscriber->allocated) return false;

    if(handler->record->value != NULL && size > handler->record->size) return false;

    // check mute
    if(
        handler->record->mute_counter != handler->subscriber->mute_counter
        && !handler->subscriber->no_mute
    ) return false;

    if(handler->record->value != NULL) {
        // real write to value
        furi_take(handler);
        memcpy(handler->record->value, value, size);
        furi_give(handler);

        // notify subscribers
        furi_notify(handler, handler->record->value, handler->record->size);
    } else {
        furi_notify(handler, value, size);
    }

    return true;
}