#include "record.h"
#include "check.h"
#include "memmgr.h"
#include "mutex.h"
#include "event_flag.h"

#include <m-string.h>
#include <m-dict.h>
#include <toolbox/m_cstr_dup.h>

#define FURI_RECORD_FLAG_READY (0x1)

typedef struct {
    FuriEventFlag* flags;
    void* data;
    size_t holders_count;
} FuriRecordData;

DICT_DEF2(FuriRecordDataDict, const char*, M_CSTR_DUP_OPLIST, FuriRecordData, M_POD_OPLIST)

typedef struct {
    FuriMutex* mutex;
    FuriRecordDataDict_t records;
} FuriRecord;

static FuriRecord* furi_record = NULL;

static FuriRecordData* furi_record_get(const char* name) {
    return FuriRecordDataDict_get(furi_record->records, name);
}

static void furi_record_put(const char* name, FuriRecordData* record_data) {
    FuriRecordDataDict_set_at(furi_record->records, name, *record_data);
}

static void furi_record_erase(const char* name, FuriRecordData* record_data) {
    furi_event_flag_free(record_data->flags);
    FuriRecordDataDict_erase(furi_record->records, name);
}

void furi_record_init() {
    furi_record = malloc(sizeof(FuriRecord));
    furi_record->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    furi_check(furi_record->mutex);
    FuriRecordDataDict_init(furi_record->records);
}

static FuriRecordData* furi_record_data_get_or_create(const char* name) {
    furi_assert(furi_record);
    FuriRecordData* record_data = furi_record_get(name);
    if(!record_data) {
        FuriRecordData new_record;
        new_record.flags = furi_event_flag_alloc();
        new_record.data = NULL;
        new_record.holders_count = 0;
        furi_record_put(name, &new_record);
        record_data = furi_record_get(name);
    }
    return record_data;
}

static void furi_record_lock() {
    furi_check(furi_mutex_acquire(furi_record->mutex, FuriWaitForever) == FuriStatusOk);
}

static void furi_record_unlock() {
    furi_check(furi_mutex_release(furi_record->mutex) == FuriStatusOk);
}

bool furi_record_exists(const char* name) {
    furi_assert(furi_record);
    furi_assert(name);

    bool ret = false;

    furi_record_lock();
    ret = (furi_record_get(name) != NULL);
    furi_record_unlock();

    return ret;
}

void furi_record_create(const char* name, void* data) {
    furi_assert(furi_record);

    furi_record_lock();

    // Get record data and fill it
    FuriRecordData* record_data = furi_record_data_get_or_create(name);
    furi_assert(record_data->data == NULL);
    record_data->data = data;
    furi_event_flag_set(record_data->flags, FURI_RECORD_FLAG_READY);

    furi_record_unlock();
}

bool furi_record_destroy(const char* name) {
    furi_assert(furi_record);

    bool ret = false;

    furi_record_lock();

    FuriRecordData* record_data = furi_record_get(name);
    furi_assert(record_data);
    if(record_data->holders_count == 0) {
        furi_record_erase(name, record_data);
        ret = true;
    }

    furi_record_unlock();

    return ret;
}

void* furi_record_open(const char* name) {
    furi_assert(furi_record);

    furi_record_lock();

    FuriRecordData* record_data = furi_record_data_get_or_create(name);
    record_data->holders_count++;

    furi_record_unlock();

    // Wait for record to become ready
    furi_check(
        furi_event_flag_wait(
            record_data->flags,
            FURI_RECORD_FLAG_READY,
            FuriFlagWaitAny | FuriFlagNoClear,
            FuriWaitForever) == FURI_RECORD_FLAG_READY);

    return record_data->data;
}

void furi_record_close(const char* name) {
    furi_assert(furi_record);

    furi_record_lock();

    FuriRecordData* record_data = furi_record_get(name);
    furi_assert(record_data);
    record_data->holders_count--;

    furi_record_unlock();
}
