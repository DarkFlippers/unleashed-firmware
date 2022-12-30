#include "infrared_brute_force.h"

#include <stdlib.h>
#include <m-dict.h>
#include <flipper_format/flipper_format.h>

#include "infrared_signal.h"

typedef struct {
    uint32_t index;
    uint32_t count;
} InfraredBruteForceRecord;

DICT_DEF2(
    InfraredBruteForceRecordDict,
    FuriString*,
    FURI_STRING_OPLIST,
    InfraredBruteForceRecord,
    M_POD_OPLIST);

struct InfraredBruteForce {
    FlipperFormat* ff;
    const char* db_filename;
    FuriString* current_record_name;
    InfraredSignal* current_signal;
    InfraredBruteForceRecordDict_t records;
    bool is_started;
};

InfraredBruteForce* infrared_brute_force_alloc() {
    InfraredBruteForce* brute_force = malloc(sizeof(InfraredBruteForce));
    brute_force->ff = NULL;
    brute_force->db_filename = NULL;
    brute_force->current_signal = NULL;
    brute_force->is_started = false;
    brute_force->current_record_name = furi_string_alloc();
    InfraredBruteForceRecordDict_init(brute_force->records);
    return brute_force;
}

void infrared_brute_force_free(InfraredBruteForce* brute_force) {
    furi_assert(!brute_force->is_started);
    InfraredBruteForceRecordDict_clear(brute_force->records);
    furi_string_free(brute_force->current_record_name);
    free(brute_force);
}

void infrared_brute_force_set_db_filename(InfraredBruteForce* brute_force, const char* db_filename) {
    furi_assert(!brute_force->is_started);
    brute_force->db_filename = db_filename;
}

bool infrared_brute_force_calculate_messages(InfraredBruteForce* brute_force) {
    furi_assert(!brute_force->is_started);
    furi_assert(brute_force->db_filename);
    bool success = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_buffered_file_alloc(storage);

    success = flipper_format_buffered_file_open_existing(ff, brute_force->db_filename);
    if(success) {
        FuriString* signal_name;
        signal_name = furi_string_alloc();
        while(flipper_format_read_string(ff, "name", signal_name)) {
            InfraredBruteForceRecord* record =
                InfraredBruteForceRecordDict_get(brute_force->records, signal_name);
            if(record) { //-V547
                ++(record->count);
            }
        }
        furi_string_free(signal_name);
    }

    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);
    return success;
}

bool infrared_brute_force_start(
    InfraredBruteForce* brute_force,
    uint32_t index,
    uint32_t* record_count) {
    furi_assert(!brute_force->is_started);
    bool success = false;
    *record_count = 0;

    InfraredBruteForceRecordDict_it_t it;
    for(InfraredBruteForceRecordDict_it(it, brute_force->records);
        !InfraredBruteForceRecordDict_end_p(it);
        InfraredBruteForceRecordDict_next(it)) {
        const InfraredBruteForceRecordDict_itref_t* record = InfraredBruteForceRecordDict_cref(it);
        if(record->value.index == index) {
            *record_count = record->value.count;
            if(*record_count) {
                furi_string_set(brute_force->current_record_name, record->key);
            }
            break;
        }
    }

    if(*record_count) {
        Storage* storage = furi_record_open(RECORD_STORAGE);
        brute_force->ff = flipper_format_buffered_file_alloc(storage);
        brute_force->current_signal = infrared_signal_alloc();
        brute_force->is_started = true;
        success =
            flipper_format_buffered_file_open_existing(brute_force->ff, brute_force->db_filename);
        if(!success) infrared_brute_force_stop(brute_force);
    }
    return success;
}

bool infrared_brute_force_is_started(InfraredBruteForce* brute_force) {
    return brute_force->is_started;
}

void infrared_brute_force_stop(InfraredBruteForce* brute_force) {
    furi_assert(brute_force->is_started);
    furi_string_reset(brute_force->current_record_name);
    infrared_signal_free(brute_force->current_signal);
    flipper_format_free(brute_force->ff);
    brute_force->current_signal = NULL;
    brute_force->ff = NULL;
    brute_force->is_started = false;
    furi_record_close(RECORD_STORAGE);
}

bool infrared_brute_force_send_next(InfraredBruteForce* brute_force) {
    furi_assert(brute_force->is_started);
    const bool success = infrared_signal_search_and_read(
        brute_force->current_signal, brute_force->ff, brute_force->current_record_name);
    if(success) {
        infrared_signal_transmit(brute_force->current_signal);
    }
    return success;
}

void infrared_brute_force_add_record(
    InfraredBruteForce* brute_force,
    uint32_t index,
    const char* name) {
    InfraredBruteForceRecord value = {.index = index, .count = 0};
    FuriString* key;
    key = furi_string_alloc_set(name);
    InfraredBruteForceRecordDict_set_at(brute_force->records, key, value);
    furi_string_free(key);
}

void infrared_brute_force_reset(InfraredBruteForce* brute_force) {
    furi_assert(!brute_force->is_started);
    InfraredBruteForceRecordDict_reset(brute_force->records);
}
