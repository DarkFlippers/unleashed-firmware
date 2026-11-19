#include "sim800_history.h"
#include "sim800_types.h"

#include <storage/storage.h>
#include <furi_hal_rtc.h>

#define SIM800_HISTORY_BODY 160 // one SMS worth of text, longer bodies are cut
#define SIM800_HISTORY_MAX  8192 // where trimming starts
#define SIM800_HISTORY_KEEP 4096 // how much of the tail survives it

static const char* const sim800_history_tags[] = {
    [Sim800HistoryCall] = "CALL",
    [Sim800HistorySms] = "SMS ",
};

/** Drop the oldest half of the file, starting at a line boundary so nothing is left split. */
static void sim800_history_trim(Storage* storage) {
    File* file = storage_file_alloc(storage);
    FuriString* tail = furi_string_alloc();

    do {
        if(!storage_file_open(file, SIM800_HISTORY_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) break;

        uint64_t size = storage_file_size(file);
        if(size <= SIM800_HISTORY_MAX) break;

        if(!storage_file_seek(file, (uint32_t)(size - SIM800_HISTORY_KEEP), true)) break;

        // The log is plain text, so terminating each chunk is enough to append it
        char buffer[129];
        size_t read = 0;
        while((read = storage_file_read(file, buffer, sizeof(buffer) - 1)) > 0) {
            buffer[read] = '\0';
            furi_string_cat_str(tail, buffer);
        }
        storage_file_close(file);

        // Start at the first whole line in what we kept
        size_t cut = furi_string_search_char(tail, '\n');
        if(cut != FURI_STRING_FAILURE) furi_string_right(tail, cut + 1);

        if(storage_file_open(file, SIM800_HISTORY_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            storage_file_write(file, furi_string_get_cstr(tail), furi_string_size(tail));
        }
    } while(false);

    storage_file_close(file);
    storage_file_free(file);
    furi_string_free(tail);
}

void sim800_history_add(Sim800HistoryKind kind, const char* detail) {
    furi_assert(detail);
    if(kind > Sim800HistorySms) return;

    DateTime now;
    furi_hal_rtc_get_datetime(&now);

    FuriString* line = furi_string_alloc_printf(
        "%02u-%02u %02u:%02u  %s  %s\n",
        now.month,
        now.day,
        now.hour,
        now.minute,
        sim800_history_tags[kind],
        detail);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    if(storage_file_open(file, SIM800_HISTORY_PATH, FSAM_WRITE, FSOM_OPEN_APPEND)) {
        storage_file_write(file, furi_string_get_cstr(line), furi_string_size(line));
    }

    storage_file_close(file);
    storage_file_free(file);

    sim800_history_trim(storage);
    furi_record_close(RECORD_STORAGE);
    furi_string_free(line);
}

/** Read the whole log into @p out. One go is fine; the file caps its own size. */
static void sim800_history_slurp(FuriString* out) {
    furi_string_reset(out);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    if(storage_file_open(file, SIM800_HISTORY_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        char buffer[129];
        size_t read = 0;
        while((read = storage_file_read(file, buffer, sizeof(buffer) - 1)) > 0) {
            buffer[read] = '\0';
            furi_string_cat_str(out, buffer);
        }
    }

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

bool sim800_history_contains(const char* needle) {
    furi_assert(needle);
    if(needle[0] == '\0') return false;

    FuriString* whole = furi_string_alloc();
    sim800_history_slurp(whole);
    bool found = (strstr(furi_string_get_cstr(whole), needle) != NULL);
    furi_string_free(whole);
    return found;
}

void sim800_history_add_sms(const char* from, const char* date, const char* body) {
    furi_assert(from);
    furi_assert(date);
    furi_assert(body);

    DateTime now;
    furi_hal_rtc_get_datetime(&now);

    FuriString* line = furi_string_alloc_printf(
        "%02u-%02u %02u:%02u  %s  %s  %s\n",
        now.month,
        now.day,
        now.hour,
        now.minute,
        sim800_history_tags[Sim800HistorySms],
        from,
        date);

    // Two leading spaces mark a continuation, which is what keeps the body with its header
    FuriString* text = furi_string_alloc_set_str(body);
    furi_string_left(text, SIM800_HISTORY_BODY);
    furi_string_replace_str(text, "\r", "");
    furi_string_replace_str(text, "\n", "\n  ");
    furi_string_cat_printf(line, "  %s\n", furi_string_get_cstr(text));
    furi_string_free(text);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    if(storage_file_open(file, SIM800_HISTORY_PATH, FSAM_WRITE, FSOM_OPEN_APPEND)) {
        storage_file_write(file, furi_string_get_cstr(line), furi_string_size(line));
    }

    storage_file_close(file);
    storage_file_free(file);

    sim800_history_trim(storage);
    furi_record_close(RECORD_STORAGE);
    furi_string_free(line);
}

bool sim800_history_read(FuriString* out, int32_t kind_filter) {
    furi_assert(out);
    furi_string_reset(out);

    const char* tag = (kind_filter < 0) ? NULL : sim800_history_tags[kind_filter];

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    FuriString* whole = furi_string_alloc();
    bool found = false;

    if(storage_file_open(file, SIM800_HISTORY_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        char buffer[129];
        size_t read = 0;
        while((read = storage_file_read(file, buffer, sizeof(buffer) - 1)) > 0) {
            buffer[read] = '\0';
            furi_string_cat_str(whole, buffer);
        }
    }

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    // Split on newlines here rather than keeping two copies of the file around
    const char* text = furi_string_get_cstr(whole);
    bool kept_header = false;

    while(*text != '\0') {
        const char* end = strchr(text, '\n');
        size_t len = (end != NULL) ? (size_t)(end - text) : strlen(text);
        bool keep;

        if((len >= 2) && (text[0] == ' ') && (text[1] == ' ')) {
            // A body line belongs to whichever header was kept last
            keep = kept_header;
        } else {
            const char* hit = (tag != NULL) ? strstr(text, tag) : NULL;
            keep = (tag == NULL) || ((hit != NULL) && (hit < text + len));
            kept_header = keep;
        }

        if((len > 0) && keep) {
            furi_string_cat_printf(out, "%.*s\n", (int)len, text);
            found = true;
        }

        if(end == NULL) break;
        text = end + 1;
    }

    furi_string_free(whole);
    return found;
}

void sim800_history_clear(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_simply_remove(storage, SIM800_HISTORY_PATH);
    furi_record_close(RECORD_STORAGE);
}
