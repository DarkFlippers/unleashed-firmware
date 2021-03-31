#include "record.h"
#include "check.h"
#include "memmgr.h"

#include <cmsis_os2.h>
#include <m-string.h>
#include <m-dict.h>

#define FURI_RECORD_FLAG_UPDATED 0x00000001U

DICT_SET_DEF(osThreadIdSet, uint32_t)

typedef struct {
    void* data;
    osThreadId_t owner;
    osThreadIdSet_t holders;
} FuriRecord;

DICT_DEF2(FuriRecordDict, string_t, STRING_OPLIST, FuriRecord, M_POD_OPLIST)

typedef struct {
    osMutexId_t records_mutex;
    FuriRecordDict_t records;
} FuriRecordData;

static FuriRecordData* furi_record_data = NULL;

void furi_record_init() {
    furi_record_data = furi_alloc(sizeof(FuriRecordData));
    furi_record_data->records_mutex = osMutexNew(NULL);
    furi_check(furi_record_data->records_mutex);
    FuriRecordDict_init(furi_record_data->records);
}

FuriRecord* furi_record_get_or_create(string_t name_str) {
    furi_assert(furi_record_data);
    FuriRecord* record = FuriRecordDict_get(furi_record_data->records, name_str);
    if(!record) {
        FuriRecord new_record;
        new_record.data = NULL;
        new_record.owner = NULL;
        osThreadIdSet_init(new_record.holders);
        FuriRecordDict_set_at(furi_record_data->records, name_str, new_record);
        record = FuriRecordDict_get(furi_record_data->records, name_str);
    }
    return record;
}

void furi_record_create(const char* name, void* data) {
    furi_assert(furi_record_data);
    osThreadId_t thread_id = osThreadGetId();

    string_t name_str;
    string_init_set_str(name_str, name);

    // Acquire mutex
    furi_check(osMutexAcquire(furi_record_data->records_mutex, osWaitForever) == osOK);
    FuriRecord* record = furi_record_get_or_create(name_str);
    record->data = data;
    record->owner = thread_id;

    // For each holder set event flag
    osThreadIdSet_it_t it;
    for(osThreadIdSet_it(it, record->holders); !osThreadIdSet_end_p(it); osThreadIdSet_next(it)) {
        osThreadFlagsSet((osThreadId_t)*osThreadIdSet_ref(it), FURI_RECORD_FLAG_UPDATED);
    }
    // Release mutex
    furi_check(osMutexRelease(furi_record_data->records_mutex) == osOK);

    string_clear(name_str);
}

bool furi_record_destroy(const char* name) {
    furi_assert(furi_record_data);
    osThreadId_t thread_id = osThreadGetId();

    string_t name_str;
    string_init_set_str(name_str, name);

    bool destroyed = false;
    furi_check(osMutexAcquire(furi_record_data->records_mutex, osWaitForever) == osOK);
    FuriRecord* record = FuriRecordDict_get(furi_record_data->records, name_str);
    if(record && record->owner == thread_id && osThreadIdSet_size(record->holders) == 0) {
        osThreadIdSet_clear(record->holders);
        FuriRecordDict_erase(furi_record_data->records, name_str);
        destroyed = true;
    }
    furi_check(osMutexRelease(furi_record_data->records_mutex) == osOK);

    string_clear(name_str);
    return destroyed;
}

void* furi_record_open(const char* name) {
    furi_assert(furi_record_data);
    osThreadId_t thread_id = osThreadGetId();

    string_t name_str;
    string_init_set_str(name_str, name);

    FuriRecord* record = NULL;
    while(1) {
        furi_check(osMutexAcquire(furi_record_data->records_mutex, osWaitForever) == osOK);
        record = furi_record_get_or_create(name_str);
        osThreadIdSet_push(record->holders, (uint32_t)thread_id);
        furi_check(osMutexRelease(furi_record_data->records_mutex) == osOK);
        // Check if owner is already arrived
        if(record->owner) {
            break;
        }
        // Wait for thread flag to appear
        osThreadFlagsWait(FURI_RECORD_FLAG_UPDATED, osFlagsWaitAny, osWaitForever);
    }

    string_clear(name_str);
    return record->data;
}

void furi_record_close(const char* name) {
    furi_assert(furi_record_data);
    osThreadId_t thread_id = osThreadGetId();

    string_t name_str;
    string_init_set_str(name_str, name);

    furi_check(osMutexAcquire(furi_record_data->records_mutex, osWaitForever) == osOK);
    FuriRecord* record = FuriRecordDict_get(furi_record_data->records, name_str);
    osThreadIdSet_erase(record->holders, (uint32_t)thread_id);
    furi_check(osMutexRelease(furi_record_data->records_mutex) == osOK);

    string_clear(name_str);
}
