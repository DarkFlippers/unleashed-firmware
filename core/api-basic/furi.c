#include "furi.h"
#include "furi-deprecated.h"

bool furi_create(const char* name, void* ptr) {
    return furi_create_deprecated(name, ptr, sizeof(size_t));
}

void* furi_open(const char* name) {
    FuriRecordSubscriber* record = furi_open_deprecated(name, false, false, NULL, NULL, NULL);
    void* res = furi_take(record);
    furi_give(record);

    return res;
}