#include "furi.h"
#include "cmsis_os.h"
#include <string.h>

// TODO: this file contains printf, that not implemented on uC target

#ifdef FURI_DEBUG
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

// TODO: change open-create to only open
bool furi_create(const char* name, void* value, size_t size) {
    #ifdef FURI_DEBUG
        printf("[FURI] creating %s record\n", name);
    #endif

    FuriRecord* record = find_record(name);

    if(record != NULL) {
        #ifdef FURI_DEBUG
            printf("[FURI] record already exist\n");
        #endif

        record->value = value;
        record->size = size;

        return true;
    }

    // record not exist, create new

    if(current_buffer_idx >= MAX_RECORD_COUNT) {
        // max record count exceed
        #ifdef FURI_DEBUG
            printf("[FURI] create: max record count exceed\n");
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
        records[current_buffer_idx].subscribers[i].ctx = NULL;
    }

    current_buffer_idx++;

    return true;
}

FuriRecordSubscriber* furi_open(
    const char* name,
    bool solo,
    bool no_mute,
    FlipperRecordCallback value_callback,
    FlipperRecordStateCallback state_callback,
    void* ctx
) {
    #ifdef FURI_DEBUG
        printf("[FURI] opening %s record\n", name);
    #endif

    // get furi record by name
    FuriRecord* record = find_record(name);

    if(record == NULL) {
        // cannot find record
        #ifdef FURI_DEBUG
            printf("[FURI] cannot find record %s\n", name);
        #endif

        // create record if not exist
        if(!furi_create(name, NULL, 0)) {
            return NULL;
        }

        record = find_record(name);

        if(record == NULL) {
            return NULL;
        }
    }

    // allocate subscriber
    FuriRecordSubscriber* subscriber = NULL;

    for(size_t i = 0; i < MAX_RECORD_SUBSCRIBERS; i++) {
        if(!record->subscribers[i].allocated) {
            subscriber = &record->subscribers[i];
            break;
        }
    }

    if(subscriber == NULL) {
        // cannot add subscriber (full)
        #ifdef FURI_DEBUG
            printf("[FURI] open: cannot add subscriber (full)\n");
        #endif
        
        return NULL;
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
    subscriber->record = record;
    subscriber->ctx = ctx;

    // register record in application
    FuriApp* current_task = find_task(xTaskGetCurrentTaskHandle());

    if(current_task != NULL) {
        current_task->records[current_task->records_count] = record;
        current_task->records_count++;
    } else {
        #ifdef FURI_DEBUG
            printf("[FURI] open: no current task\n");
        #endif
    }

    return subscriber;
}


void furi_close(FuriRecordSubscriber* handler) {
    #ifdef FURI_DEBUG
        printf("[FURI] closing %s record\n", handler->record->name);
    #endif

    // deallocate subscriber
    handler->allocated = false;

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

static void furi_notify(FuriRecordSubscriber* handler, const void* value, size_t size) {
    for(size_t i = 0; i < MAX_RECORD_SUBSCRIBERS; i++) {
        if(handler->record->subscribers[i].allocated) {
            if(handler->record->subscribers[i].cb != NULL) {
                handler->record->subscribers[i].cb(
                    value,
                    size,
                    handler->record->subscribers[i].ctx
                );
            }
        }
    }
}

void* furi_take(FuriRecordSubscriber* handler) {
    if(handler == NULL || handler->record == NULL) return NULL;

    if (xSemaphoreTake(handler->record->mutex, portMAX_DELAY) == pdTRUE) {
        return handler->record->value;
    } else {
        return NULL;
    }
}

void furi_give(FuriRecordSubscriber* handler) {
    if(handler == NULL || handler->record == NULL) return;

    xSemaphoreGive(handler->record->mutex);
}

void furi_commit(FuriRecordSubscriber* handler) {
    if(handler == NULL || handler->record == NULL) return;

    furi_notify(handler, handler->record->value, handler->record->size);
    furi_give(handler);
}

bool furi_read(FuriRecordSubscriber* handler, void* value, size_t size) {
    #ifdef FURI_DEBUG
        printf("[FURI] read from %s\n", handler->record->name);
    #endif

    if(handler == NULL || handler->record == NULL || value == NULL) return false;

    if(size > handler->record->size) return false;

    // return false if read from pipe
    if(handler->record->value == NULL) return false;

    furi_take(handler);
    memcpy(value, handler->record->value, size);
    furi_notify(handler, value, size);
    furi_give(handler);

    return true;
}

bool furi_write(FuriRecordSubscriber* handler, const void* value, size_t size) {
    #ifdef FURI_DEBUG
        printf("[FURI] write to %s\n", handler->record->name);
    #endif

    if(handler == NULL || handler->record == NULL || value == NULL) {
        #ifdef FURI_DEBUG
            printf("[FURI] write: null param %x %x\n", (uint32_t)(size_t)handler, (uint32_t)(size_t)value);
        #endif

        return false;
    }

    // check if closed
    if(!handler->allocated) {
        #ifdef FURI_DEBUG
            printf("[FURI] write: handler closed\n");
        #endif
        return false;
    }

    if(handler->record->value != NULL && size > handler->record->size) {
        #ifdef FURI_DEBUG
            printf("[FURI] write: wrong size %d\n", (uint32_t)size);
        #endif
        return false;
    }

    // check mute
    if(
        handler->record->mute_counter != handler->mute_counter
        && !handler->no_mute
    ) {
        #ifdef FURI_DEBUG
            printf("[FURI] write: muted\n");
        #endif
        return false;
    }

    furi_take(handler);
    if(handler->record->value != NULL) {
        // real write to value
        memcpy(handler->record->value, value, size);

        // notify subscribers
        furi_notify(handler, handler->record->value, handler->record->size);
    } else {
        furi_notify(handler, value, size);
    }
    furi_give(handler);

    return true;
}