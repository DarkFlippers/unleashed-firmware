#include <furi.h>
#include <gui/canvas.h>
#include <gui/gui.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_input.h>
#include <gui/modules/widget.h>
#include <gui/view.h>
#include <input/input.h>
#include <gui/view_dispatcher.h>
#include <storage/storage.h>

#include "fcc_qr_code.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG "FccIdLookup"

#define FCC_DB_PATH              APP_ASSETS_PATH("fcc_freq_v2.bin")
#define FCC_DB_SETUP_HINT        "Reinstall Unleashed to restore the bundled database."
#define FCC_DB_MAGIC             "FCCFRQ2"
#define FCC_DB_VERSION           2U
#define FCC_DB_HEADER_SIZE       168U
#define FCC_DB_CRC_OFFSET        20U
#define FCC_DB_EXPECTED_SIZE     8930222ULL
#define FCC_PREFIX3_COUNT        46656U
#define FCC_PREFIX_INDEX_ENTRIES 46657U
#define FCC_DB_READ_CACHE_SIZE   512U
#define FCC_GRANTEE_TWO_OFFSET   FCC_PREFIX3_COUNT
#define FCC_APPLICANT_ESCAPE     31U

#define FCC_INPUT_LEN        32U
#define FCC_ID_LEN           64U
#define FCC_APPLICANT_LEN    96U
#define FCC_PAGE_SIZE        50U
#define FCC_MAX_INTERVALS    32U
#define FCC_DETAIL_TEXT_LEN  1536U
#define FCC_MESSAGE_TEXT_LEN 512U
#define FCC_LIST_HEADER_LEN  80U

static const uint32_t fcc_interval_scales[] = {1, 10, 100, 1000, 10000, 100000, 1000000};
static const char* const fcc_applicant_tokens[] = {
    " Technology",  " Co., Ltd.",    " Inc.",           " LLC",         " LTD",
    " Electronics", " Limited",      " TECHNOLOGY",     " Ltd.",        " Corporation",
    " Co.,Ltd.",    " LIMITED",      " CO., LTD.",      " Systems",     " International",
    " Equipment",   " Wireless",     " Communications", " Company",     " Digital",
    " Industrial",  " Group",        " Shenzhen ",      " ELECTRONICS", " CORP.",
    " Corp.",       " Incorporated", " Manufacturing",  " Devices",     " China",
};

typedef struct {
    uint8_t magic[8];
    uint32_t version;
    uint32_t header_size;
    uint32_t flags;
    uint32_t crc32;
    uint64_t file_size;
    uint64_t fcc_count;
    uint64_t grantee_count;
    uint64_t applicant_count;
    uint64_t interval_count;
    uint64_t prefix_index_offset;
    uint64_t record_offset;
    uint64_t record_size;
    uint64_t interval_offset;
    uint64_t interval_size;
    uint64_t grantee_index_offset;
    uint64_t grantee_record_offset;
    uint64_t grantee_record_size;
    uint64_t applicant_offset;
    uint64_t applicant_size;
    uint64_t overflow_record_offset;
    uint64_t overflow_record_size;
    uint64_t reserved;
} FccDbHeader;

typedef struct {
    Storage* storage;
    File* file;
    FccDbHeader header;
    bool open;
    uint32_t position; // cached file read position, for seek elision
    bool position_valid;
    uint8_t read_cache[FCC_DB_READ_CACHE_SIZE];
    uint32_t cache_offset;
    size_t cache_size;
    bool cache_valid;
} FccDb;

typedef struct {
    char id[FCC_ID_LEN];
    uint64_t interval_offset;
    uint64_t interval_count;
} FccRecord;

typedef struct {
    uint64_t lower_hz;
    uint64_t upper_hz;
} FccInterval;

typedef struct {
    char fcc_id[FCC_ID_LEN];
    char lookup_key[FCC_ID_LEN];
    char grantee_prefix[8];
    char applicant[FCC_APPLICANT_LEN];
    FccInterval intervals[FCC_MAX_INTERVALS];
    uint32_t interval_count;
    uint64_t total_interval_count;
} FccLookupResult;

typedef struct {
    char id[FCC_ID_LEN];
} FccPrefixItem;

typedef struct {
    FccPrefixItem items[FCC_PAGE_SIZE];
    uint32_t count;
    bool has_more;
} FccPrefixPage;

typedef enum {
    FccViewIntro,
    FccViewInput,
    FccViewList,
    FccViewDetail,
    FccViewMessage,
    FccViewStatus,
} FccView;

typedef enum {
    FccDbPrepIdle,
    FccDbPrepPreparing,
    FccDbPrepReady,
    FccDbPrepError,
} FccDbPrepState;

typedef enum {
    FccCustomEventDbPrep = 1,
} FccCustomEvent;

typedef enum {
    FccListActionPrev = 0xFFFFFFFEU,
    FccListActionNext = 0xFFFFFFFFU,
} FccListAction;

typedef struct {
    FccDb db;
    Gui* gui;
    ViewDispatcher* dispatcher;
    View* intro_view;
    TextInput* text_input;
    Submenu* submenu;
    Widget* detail_widget;
    Widget* message_widget;
    Widget* status_widget;
    FuriThread* prep_thread;
    FuriMutex* prep_mutex;
    FccView view;
    FccView detail_back_view;
    char input[FCC_INPUT_LEN];
    char normalized[FCC_ID_LEN];
    char list_header[FCC_LIST_HEADER_LEN];
    char detail_text[FCC_DETAIL_TEXT_LEN];
    char message_text[FCC_MESSAGE_TEXT_LEN];
    FccPrefixPage* page;
    uint32_t page_offset;
    FccDbPrepState prep_state;
    uint32_t prep_done;
    uint32_t prep_total;
    bool prep_cancel;
    bool pending_search;
} FccApp;

static uint32_t fcc_le32(const uint8_t* data) {
    return ((uint32_t)data[0]) | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16) |
           ((uint32_t)data[3] << 24);
}

static uint64_t fcc_le64(const uint8_t* data) {
    return ((uint64_t)fcc_le32(data)) | ((uint64_t)fcc_le32(data + 4) << 32);
}

static uint32_t fcc_le24(const uint8_t* data) {
    return ((uint32_t)data[0]) | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16);
}

static bool fcc_is_base36(char ch) {
    return ((ch >= '0') && (ch <= '9')) || ((ch >= 'A') && (ch <= 'Z'));
}

static uint32_t fcc_base36_value(char ch) {
    return (ch <= '9') ? (uint32_t)(ch - '0') : (uint32_t)(ch - 'A' + 10);
}

static size_t fcc_normalize(const char* input, char* output, size_t output_size) {
    size_t used = 0;
    if(output_size == 0) return 0;
    for(; *input && used + 1 < output_size; input++) {
        char ch = *input;
        if((ch >= 'a') && (ch <= 'z')) ch = (char)(ch - 'a' + 'A');
        if(fcc_is_base36(ch)) output[used++] = ch;
    }
    output[used] = '\0';
    return used;
}

static void fcc_grantee_prefix(const char* normalized, char* output, size_t output_size) {
    if(output_size == 0) return;
    size_t length = (normalized[0] == '2') ? 5 : 3;
    if(length >= output_size) length = output_size - 1;
    strncpy(output, normalized, length);
    output[length] = '\0';
}

static bool fcc_prefix3_code(const char* normalized, uint32_t* code) {
    if(strlen(normalized) < 3) return false;
    *code = fcc_base36_value(normalized[0]) * 36U * 36U + fcc_base36_value(normalized[1]) * 36U +
            fcc_base36_value(normalized[2]);
    return true;
}

static void fcc_prefix3_from_code(uint32_t code, char* output) {
    uint32_t a = code / (36U * 36U);
    uint32_t rem = code % (36U * 36U);
    uint32_t b = rem / 36U;
    uint32_t c = rem % 36U;
    output[0] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[a];
    output[1] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[b];
    output[2] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[c];
    output[3] = '\0';
}

static bool fcc_grantee_code(const char* prefix, uint32_t* code) {
    if(prefix[0] == '2' && strlen(prefix) >= 5) {
        uint32_t value = 0;
        for(uint8_t i = 1; i < 5; i++) {
            if(!fcc_is_base36(prefix[i])) return false;
            value = value * 36U + fcc_base36_value(prefix[i]);
        }
        *code = FCC_GRANTEE_TWO_OFFSET + value;
        return true;
    }
    return fcc_prefix3_code(prefix, code);
}

static bool fcc_append(char* buffer, size_t buffer_size, const char* format, ...) {
    size_t used = strlen(buffer);
    if(used >= buffer_size) return false;
    va_list args;
    va_start(args, format);
    int written = vsnprintf(buffer + used, buffer_size - used, format, args);
    va_end(args);
    return (written >= 0) && ((size_t)written < buffer_size - used);
}

static void fcc_copy(char* output, const char* input, size_t output_size) {
    if(output_size == 0) return;
    size_t i = 0;
    while(input[i] && i + 1 < output_size) {
        output[i] = input[i];
        i++;
    }
    output[i] = '\0';
}

static bool fcc_db_seek(FccDb* db, uint32_t offset) {
    // The lookup paths read sequentially (ULEB streams, record ids, applicant
    // strings), so the file is almost always already positioned correctly.
    // Skip the storage_file_seek syscall when it would be a no-op.
    if(db->position_valid && db->position == offset) return true;
    if(!storage_file_seek(db->file, offset, true)) {
        db->position_valid = false;
        return false;
    }
    db->position = offset;
    db->position_valid = true;
    return true;
}

static bool fcc_db_read(FccDb* db, void* buffer, size_t size) {
    if(size > UINT32_MAX - db->position) {
        db->position_valid = false;
        return false;
    }

    if(storage_file_read(db->file, buffer, size) != size) {
        db->position_valid = false; // short read leaves the position unknown
        return false;
    }
    db->position += size;
    return true;
}

static bool fcc_db_read_at(FccDb* db, uint32_t offset, void* buffer, size_t size) {
    uint8_t* output = buffer;
    while(size > 0) {
        if(db->cache_valid && db->cache_size > UINT32_MAX - db->cache_offset) {
            db->cache_valid = false;
        }
        if(db->cache_valid) {
            uint32_t cache_end = db->cache_offset + (uint32_t)db->cache_size;
            if(offset >= db->cache_offset && offset < cache_end) {
                size_t available = cache_end - offset;
                if(available > size) available = size;
                memcpy(output, db->read_cache + (offset - db->cache_offset), available);
                output += available;
                offset += available;
                size -= available;
                continue;
            }
        }

        if(size >= sizeof(db->read_cache)) {
            return fcc_db_seek(db, offset) && fcc_db_read(db, output, size);
        }

        if(!fcc_db_seek(db, offset)) return false;
        size_t cached = storage_file_read(db->file, db->read_cache, sizeof(db->read_cache));
        if(cached == 0) {
            db->position_valid = false;
            db->cache_valid = false;
            return false;
        }
        if(cached > UINT32_MAX - db->position) {
            db->position_valid = false;
            db->cache_valid = false;
            return false;
        }

        db->cache_offset = offset;
        db->cache_size = cached;
        db->cache_valid = true;
        db->position += cached;
    }

    return true;
}

static bool fcc_db_read_u24_at(FccDb* db, uint64_t offset, uint32_t* value) {
    uint8_t bytes[3];
    if(offset > UINT32_MAX) return false;
    if(!fcc_db_read_at(db, (uint32_t)offset, bytes, sizeof(bytes))) return false;
    *value = fcc_le24(bytes);
    return true;
}

static bool fcc_db_range_valid(uint64_t offset, uint64_t size, uint64_t file_size) {
    if(offset > file_size) return false;
    if(size > file_size - offset) return false;
    if(offset > UINT32_MAX) return false;
    if(size > UINT32_MAX) return false;
    return true;
}

static bool fcc_db_read_header(FccDb* db) {
    uint8_t bytes[FCC_DB_HEADER_SIZE];
    if(!fcc_db_read_at(db, 0, bytes, sizeof(bytes))) return false;

    memcpy(db->header.magic, bytes, sizeof(db->header.magic));
    db->header.version = fcc_le32(bytes + 8);
    db->header.header_size = fcc_le32(bytes + 12);
    db->header.flags = fcc_le32(bytes + 16);
    db->header.crc32 = fcc_le32(bytes + 20);
    db->header.file_size = fcc_le64(bytes + 24);
    db->header.fcc_count = fcc_le64(bytes + 32);
    db->header.grantee_count = fcc_le64(bytes + 40);
    db->header.applicant_count = fcc_le64(bytes + 48);
    db->header.interval_count = fcc_le64(bytes + 56);
    db->header.prefix_index_offset = fcc_le64(bytes + 64);
    db->header.record_offset = fcc_le64(bytes + 72);
    db->header.record_size = fcc_le64(bytes + 80);
    db->header.interval_offset = fcc_le64(bytes + 88);
    db->header.interval_size = fcc_le64(bytes + 96);
    db->header.grantee_index_offset = fcc_le64(bytes + 104);
    db->header.grantee_record_offset = fcc_le64(bytes + 112);
    db->header.grantee_record_size = fcc_le64(bytes + 120);
    db->header.applicant_offset = fcc_le64(bytes + 128);
    db->header.applicant_size = fcc_le64(bytes + 136);
    db->header.overflow_record_offset = fcc_le64(bytes + 144);
    db->header.overflow_record_size = fcc_le64(bytes + 152);
    db->header.reserved = fcc_le64(bytes + 160);

    if(memcmp(db->header.magic, FCC_DB_MAGIC, 7) != 0) return false;
    if(db->header.magic[7] != '\0') return false;
    if(db->header.version != FCC_DB_VERSION) return false;
    if(db->header.header_size != FCC_DB_HEADER_SIZE) return false;
    if(db->header.file_size != storage_file_size(db->file)) return false;
    if(db->header.file_size > UINT32_MAX) return false;
    if(!fcc_db_range_valid(
           db->header.prefix_index_offset, FCC_PREFIX_INDEX_ENTRIES * 3ULL, db->header.file_size)) {
        return false;
    }
    if(!fcc_db_range_valid(
           db->header.record_offset, db->header.record_size, db->header.file_size)) {
        return false;
    }
    if(!fcc_db_range_valid(
           db->header.overflow_record_offset,
           db->header.overflow_record_size,
           db->header.file_size)) {
        return false;
    }
    if(!fcc_db_range_valid(
           db->header.interval_offset, db->header.interval_size, db->header.file_size)) {
        return false;
    }
    if(!fcc_db_range_valid(
           db->header.grantee_record_offset,
           db->header.grantee_record_size,
           db->header.file_size)) {
        return false;
    }
    if(!fcc_db_range_valid(
           db->header.applicant_offset, db->header.applicant_size, db->header.file_size)) {
        return false;
    }
    if(db->header.grantee_count > UINT32_MAX) return false;
    if(db->header.grantee_count > UINT64_MAX / 6ULL) return false;
    if(db->header.grantee_count * 6ULL > db->header.grantee_record_size) return false;
    return true;
}

static void fcc_db_release_file(FccDb* db) {
    if(db->file) {
        if(storage_file_is_open(db->file)) storage_file_close(db->file);
        storage_file_free(db->file);
    }

    db->file = NULL;
    db->open = false;
    db->position_valid = false;
    db->cache_valid = false;
}

static bool fcc_db_open_file(FccDb* db) {
    db->file = storage_file_alloc(db->storage);
    if(!db->file) return false;

    if(!storage_file_open(db->file, FCC_DB_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        fcc_db_release_file(db);
        return false;
    }
    if(!fcc_db_read_header(db)) {
        fcc_db_release_file(db);
        return false;
    }

    // Header validation probes the file; reset sequential read position before normal lookup.
    db->position_valid = false;
    db->open = true;
    return true;
}

typedef bool (*FccDbPrepareProgressCallback)(void* context, uint32_t done, uint32_t total);

static bool
    fcc_db_open_with_progress(FccDb* db, FccDbPrepareProgressCallback progress, void* context) {
    memset(db, 0, sizeof(*db));
    db->storage = furi_record_open(RECORD_STORAGE);
    if(!db->storage) return false;

    if(progress && !progress(context, 0U, 1U)) goto fail;
    if(fcc_db_open_file(db)) {
        if(progress && !progress(context, 1U, 1U)) {
            fcc_db_release_file(db);
            goto fail;
        }
        return true;
    }

fail:
    furi_record_close(RECORD_STORAGE);
    memset(db, 0, sizeof(*db));
    return false;
}

static bool fcc_db_open(FccDb* db) {
    return fcc_db_open_with_progress(db, NULL, NULL);
}

static void fcc_db_close(FccDb* db) {
    if(db->file) {
        if(db->open) storage_file_close(db->file);
        storage_file_free(db->file);
    }
    if(db->storage) furi_record_close(RECORD_STORAGE);
    memset(db, 0, sizeof(*db));
}

static bool fcc_db_read_uleb(FccDb* db, uint32_t* offset, uint32_t end, uint64_t* value) {
    uint64_t result = 0;
    uint8_t shift = 0;
    while(*offset < end && shift <= 63) {
        uint8_t byte;
        if(!fcc_db_read_at(db, *offset, &byte, 1)) return false;
        (*offset)++;
        result |= ((uint64_t)(byte & 0x7FU)) << shift;
        if((byte & 0x80U) == 0) {
            *value = result;
            return true;
        }
        shift += 7;
    }
    return false;
}

static bool fcc_db_read_scaled_uleb(FccDb* db, uint32_t* offset, uint32_t end, uint64_t* value) {
    uint64_t encoded;
    if(!fcc_db_read_uleb(db, offset, end, &encoded)) return false;
    uint8_t scale_id = encoded & 0x07U;
    if(scale_id >= COUNT_OF(fcc_interval_scales)) return false;
    uint32_t scale = fcc_interval_scales[scale_id];
    uint64_t magnitude = encoded >> 3;
    if(scale != 0 && magnitude > UINT64_MAX / scale) return false;
    *value = magnitude * scale;
    return true;
}

static bool fcc_db_next_record(
    FccDb* db,
    uint32_t* offset,
    uint32_t end,
    const char* bucket_prefix,
    size_t bucket_prefix_length,
    FccRecord* record) {
    uint64_t id_length;
    if(!fcc_db_read_uleb(db, offset, end, &id_length)) return false;
    size_t prefix_length = bucket_prefix ? bucket_prefix_length : 0;
    if(id_length == 0 && prefix_length == 0) return false;
    if(prefix_length + id_length >= FCC_ID_LEN) return false;
    if(((uint64_t)*offset + id_length) > end) return false;
    if(bucket_prefix) memcpy(record->id, bucket_prefix, prefix_length);
    if(!fcc_db_read_at(db, *offset, record->id + prefix_length, (size_t)id_length)) return false;
    record->id[prefix_length + id_length] = '\0';
    *offset += (uint32_t)id_length;
    if(!fcc_db_read_uleb(db, offset, end, &record->interval_offset)) return false;
    if(!fcc_db_read_uleb(db, offset, end, &record->interval_count)) return false;
    return *offset <= end;
}

static bool fcc_db_prefix_directory_value(FccDb* db, uint32_t code, uint32_t* value) {
    if(code >= FCC_PREFIX_INDEX_ENTRIES) return false;
    return fcc_db_read_u24_at(db, db->header.prefix_index_offset + (uint64_t)code * 3U, value);
}

static bool fcc_db_record_range(
    FccDb* db,
    uint32_t relative_start,
    uint32_t relative_end,
    uint32_t* start,
    uint32_t* end) {
    if(relative_start > relative_end) return false;
    if(relative_end > db->header.record_size) return false;
    *start = (uint32_t)db->header.record_offset + relative_start;
    *end = (uint32_t)db->header.record_offset + relative_end;
    return *start <= *end;
}

static bool
    fcc_db_code_range_for_prefix(const char* key, uint32_t* start_code, uint32_t* end_code) {
    size_t length = strlen(key);
    if(length >= 3) {
        if(!fcc_prefix3_code(key, start_code)) return false;
        *end_code = *start_code + 1;
    } else if(length == 2) {
        *start_code = fcc_base36_value(key[0]) * 36U * 36U + fcc_base36_value(key[1]) * 36U;
        *end_code = *start_code + 36U;
    } else if(length == 1) {
        *start_code = fcc_base36_value(key[0]) * 36U * 36U;
        *end_code = *start_code + 36U * 36U;
    } else {
        return false;
    }
    return true;
}

static bool fcc_db_decode_intervals(FccDb* db, FccLookupResult* result, const FccRecord* record) {
    // A corrupt record could point its interval block outside the interval
    // region; keep the truncated 32-bit offset inside the file rather than
    // wrapping and decoding an unrelated part of the database.
    if(record->interval_offset > db->header.interval_size) return false;
    uint64_t interval_start = db->header.interval_offset + record->interval_offset;
    uint64_t interval_end = db->header.interval_offset + db->header.interval_size;
    if(interval_start > UINT32_MAX || interval_end > UINT32_MAX) return false;
    uint32_t offset = (uint32_t)interval_start;
    uint32_t end = (uint32_t)interval_end;
    uint64_t previous_lower = 0;
    result->interval_count = 0;
    result->total_interval_count = record->interval_count;

    // Only the first FCC_MAX_INTERVALS ranges are displayed; the rest are
    // summarised from the header count, so stop reading once the buffer is
    // full instead of decoding the entire (possibly long) interval block.
    for(uint64_t i = 0; i < record->interval_count && result->interval_count < FCC_MAX_INTERVALS;
        i++) {
        uint64_t lower_delta;
        uint64_t span;
        if(!fcc_db_read_scaled_uleb(db, &offset, end, &lower_delta)) return false;
        if(!fcc_db_read_scaled_uleb(db, &offset, end, &span)) return false;
        if(lower_delta > UINT64_MAX - previous_lower) return false;
        uint64_t lower = previous_lower + lower_delta;
        if(span > UINT64_MAX - lower) return false;
        uint64_t upper = lower + span;
        previous_lower = lower;
        result->intervals[result->interval_count].lower_hz = lower;
        result->intervals[result->interval_count].upper_hz = upper;
        result->interval_count++;
    }
    return true;
}

static bool fcc_db_read_grantee_entry(
    FccDb* db,
    uint32_t index,
    uint32_t* code,
    uint32_t* applicant_offset) {
    if(index >= db->header.grantee_count) return false;
    uint64_t entry_offset = db->header.grantee_record_offset + (uint64_t)index * 6ULL;
    if(entry_offset > UINT32_MAX) return false;
    uint32_t offset = (uint32_t)entry_offset;
    if(!fcc_db_read_u24_at(db, offset, code)) return false;
    return fcc_db_read_u24_at(db, offset + 3U, applicant_offset);
}

static bool
    fcc_applicant_append(char* output, size_t output_size, size_t* used, const char* text) {
    while(*text) {
        if(*used + 1 >= output_size) {
            output[*used] = '\0';
            return false;
        }
        output[(*used)++] = *text++;
    }
    output[*used] = '\0';
    return true;
}

static bool
    fcc_db_read_applicant(FccDb* db, uint32_t relative_offset, char* output, size_t output_size) {
    size_t used = 0;
    if(output_size == 0) return false;
    // A corrupt grantee entry could reference an applicant offset past the
    // applicant region; bail rather than read an unrelated string.
    if(relative_offset >= db->header.applicant_size) return false;
    uint64_t applicant_start = db->header.applicant_offset + relative_offset;
    uint64_t applicant_end = db->header.applicant_offset + db->header.applicant_size;
    if(applicant_start > UINT32_MAX || applicant_end > UINT32_MAX) return false;
    uint32_t offset = (uint32_t)applicant_start;
    uint32_t end = (uint32_t)applicant_end;

    while(offset < end) {
        uint8_t byte;
        if(!fcc_db_read_at(db, offset++, &byte, 1)) return false;
        if(byte == 0) {
            output[used] = '\0';
            return true;
        }
        if(byte >= 1 && byte <= COUNT_OF(fcc_applicant_tokens)) {
            if(!fcc_applicant_append(output, output_size, &used, fcc_applicant_tokens[byte - 1])) {
                return true;
            }
        } else if(byte == FCC_APPLICANT_ESCAPE) {
            if(offset >= end) return false;
            if(!fcc_db_read_at(db, offset++, &byte, 1)) return false;
            if(used + 1 < output_size) output[used++] = (char)byte;
        } else {
            if(used + 1 < output_size) output[used++] = (char)byte;
        }
    }
    output[used] = '\0';
    return false;
}

static bool
    fcc_db_applicant_for_prefix(FccDb* db, const char* target, char* output, size_t output_size) {
    uint32_t target_code;
    if(!fcc_grantee_code(target, &target_code)) return false;

    uint32_t low = 0;
    uint32_t high = (uint32_t)db->header.grantee_count;
    while(low < high) {
        uint32_t mid = low + (high - low) / 2U;
        uint32_t found_code;
        uint32_t applicant_offset;
        if(!fcc_db_read_grantee_entry(db, mid, &found_code, &applicant_offset)) return false;
        if(found_code < target_code) {
            low = mid + 1U;
        } else if(found_code > target_code) {
            high = mid;
        } else {
            return fcc_db_read_applicant(db, applicant_offset, output, output_size);
        }
    }
    if(output_size > 0) output[0] = '\0';
    return true;
}

static bool fcc_db_result_from_record(
    FccDb* db,
    const char* lookup_key,
    const FccRecord* record,
    FccLookupResult* result) {
    memset(result, 0, sizeof(*result));
    strncpy(result->fcc_id, record->id, sizeof(result->fcc_id) - 1);
    strncpy(result->lookup_key, lookup_key, sizeof(result->lookup_key) - 1);

    char normalized[FCC_ID_LEN];
    fcc_normalize(record->id, normalized, sizeof(normalized));
    fcc_grantee_prefix(normalized, result->grantee_prefix, sizeof(result->grantee_prefix));
    if(!fcc_db_applicant_for_prefix(
           db, result->grantee_prefix, result->applicant, sizeof(result->applicant))) {
        return false;
    }
    return fcc_db_decode_intervals(db, result, record);
}

static bool fcc_db_scan_for_exact(
    FccDb* db,
    uint32_t start,
    uint32_t end,
    const char* bucket_prefix,
    size_t bucket_prefix_length,
    const char* key,
    FccLookupResult* result) {
    uint32_t offset = start;
    FccRecord record;
    while(offset < end) {
        if(!fcc_db_next_record(db, &offset, end, bucket_prefix, bucket_prefix_length, &record)) {
            return false;
        }
        char normalized[FCC_ID_LEN];
        fcc_normalize(record.id, normalized, sizeof(normalized));
        int compare = strcmp(normalized, key);
        if(compare == 0) {
            return fcc_db_result_from_record(db, key, &record, result);
        }
        if(compare > 0) return false;
    }
    return false;
}

static bool fcc_db_lookup(FccDb* db, const char* input, FccLookupResult* result) {
    char key[FCC_ID_LEN];
    size_t key_length = fcc_normalize(input, key, sizeof(key));
    if(key_length < 3) return false;

    uint32_t code;
    if(fcc_prefix3_code(key, &code)) {
        uint32_t relative_start;
        uint32_t relative_end;
        if(!fcc_db_prefix_directory_value(db, code, &relative_start)) return false;
        if(!fcc_db_prefix_directory_value(db, code + 1, &relative_end)) return false;
        uint32_t start;
        uint32_t end;
        if(!fcc_db_record_range(db, relative_start, relative_end, &start, &end)) return false;
        char bucket_prefix[4];
        fcc_prefix3_from_code(code, bucket_prefix);
        if(fcc_db_scan_for_exact(db, start, end, bucket_prefix, 3, key, result)) return true;
    }

    if(db->header.overflow_record_size > 0) {
        uint32_t start = (uint32_t)db->header.overflow_record_offset;
        uint32_t end = start + (uint32_t)db->header.overflow_record_size;
        if(fcc_db_scan_for_exact(db, start, end, NULL, 0, key, result)) return true;
    }

    return false;
}

static bool fcc_db_prefix_page_scan(
    FccDb* db,
    uint32_t start,
    uint32_t end,
    const char* bucket_prefix,
    size_t bucket_prefix_length,
    const char* key,
    uint32_t* seen,
    uint32_t offset,
    FccPrefixPage* page) {
    uint32_t scan = start;
    size_t key_length = strlen(key);
    FccRecord record;
    while(scan < end) {
        if(!fcc_db_next_record(db, &scan, end, bucket_prefix, bucket_prefix_length, &record)) {
            return false;
        }
        char normalized[FCC_ID_LEN];
        fcc_normalize(record.id, normalized, sizeof(normalized));
        int compare = strncmp(normalized, key, key_length);
        if(compare != 0) {
            if(compare > 0) return true;
            continue;
        }
        if((*seen)++ < offset) continue;
        if(page->count >= FCC_PAGE_SIZE) {
            page->has_more = true;
            return true;
        }
        strncpy(page->items[page->count].id, record.id, FCC_ID_LEN - 1);
        page->items[page->count].id[FCC_ID_LEN - 1] = '\0';
        page->count++;
    }
    return true;
}

static bool fcc_db_prefix_page(
    FccDb* db,
    const char* input,
    uint32_t offset,
    FccPrefixPage* page,
    char* normalized,
    size_t normalized_size) {
    memset(page, 0, sizeof(*page));
    if(fcc_normalize(input, normalized, normalized_size) == 0) return true;

    uint32_t seen = 0;
    uint32_t start_code;
    uint32_t end_code;
    if(!fcc_db_code_range_for_prefix(normalized, &start_code, &end_code)) return false;
    for(uint32_t code = start_code; code < end_code && !page->has_more; code++) {
        uint32_t relative_start;
        uint32_t relative_end;
        if(!fcc_db_prefix_directory_value(db, code, &relative_start)) return false;
        if(!fcc_db_prefix_directory_value(db, code + 1, &relative_end)) return false;
        if(relative_start == relative_end) continue;
        uint32_t start;
        uint32_t end;
        if(!fcc_db_record_range(db, relative_start, relative_end, &start, &end)) return false;
        char bucket_prefix[4];
        fcc_prefix3_from_code(code, bucket_prefix);
        if(!fcc_db_prefix_page_scan(
               db, start, end, bucket_prefix, 3, normalized, &seen, offset, page)) {
            return false;
        }
    }

    if(!page->has_more && db->header.overflow_record_size > 0) {
        uint32_t start = (uint32_t)db->header.overflow_record_offset;
        uint32_t end = start + (uint32_t)db->header.overflow_record_size;
        if(!fcc_db_prefix_page_scan(db, start, end, NULL, 0, normalized, &seen, offset, page)) {
            return false;
        }
    }
    return true;
}

static void fcc_format_frequency(uint64_t hz, char* output, size_t output_size) {
    static const struct {
        uint64_t factor;
        const char* suffix;
    } units[] = {
        {1000000000000ULL, "THz"},
        {1000000000ULL, "GHz"},
        {1000000ULL, "MHz"},
        {1000ULL, "kHz"},
        {1ULL, "Hz"},
    };

    for(size_t i = 0; i < COUNT_OF(units); i++) {
        uint64_t factor = units[i].factor;
        if(hz < factor && factor != 1ULL) continue;
        uint64_t whole = hz / factor;
        uint64_t remainder = hz % factor;
        if(remainder == 0) {
            snprintf(output, output_size, "%llu %s", (unsigned long long)whole, units[i].suffix);
        } else {
            uint64_t frac = (remainder * 1000000ULL + factor / 2ULL) / factor;
            if(frac >= 1000000ULL) {
                whole++;
                frac -= 1000000ULL;
            }
            if(frac == 0) {
                snprintf(
                    output, output_size, "%llu %s", (unsigned long long)whole, units[i].suffix);
                return;
            }
            char frac_text[8];
            uint32_t frac32 = frac;
            for(int8_t digit = 5; digit >= 0; digit--) {
                frac_text[digit] = (char)('0' + (frac32 % 10U));
                frac32 /= 10U;
            }
            frac_text[6] = '\0';
            for(int8_t trim = 5; trim >= 0 && frac_text[trim] == '0'; trim--) {
                frac_text[trim] = '\0';
            }
            snprintf(
                output,
                output_size,
                "%llu.%s %s",
                (unsigned long long)whole,
                frac_text,
                units[i].suffix);
        }
        return;
    }
}

static const char* fcc_frequency_suffix(char* formatted) {
    char* separator = strrchr(formatted, ' ');
    return separator ? separator + 1 : "";
}

static bool fcc_frequency_strip_suffix(char* formatted, const char* suffix) {
    size_t formatted_len = strlen(formatted);
    size_t suffix_len = strlen(suffix);
    if(suffix_len == 0 || formatted_len <= suffix_len) return false;

    size_t suffix_start = formatted_len - suffix_len;
    if(formatted[suffix_start - 1] != ' ') return false;
    if(strcmp(formatted + suffix_start, suffix) != 0) return false;

    formatted[suffix_start - 1] = '\0';
    return true;
}

static void fcc_format_frequency_range(
    uint64_t lower_hz,
    uint64_t upper_hz,
    char* output,
    size_t output_size) {
    char lower[32];
    char upper[32];
    fcc_format_frequency(lower_hz, lower, sizeof(lower));
    fcc_format_frequency(upper_hz, upper, sizeof(upper));

    if(lower_hz == upper_hz) {
        fcc_copy(output, lower, output_size);
        return;
    }

    const char* lower_suffix = fcc_frequency_suffix(lower);
    const char* upper_suffix = fcc_frequency_suffix(upper);
    if(strcmp(lower_suffix, upper_suffix) == 0 &&
       fcc_frequency_strip_suffix(lower, lower_suffix) &&
       fcc_frequency_strip_suffix(upper, upper_suffix)) {
        snprintf(output, output_size, "%s - %s %s", lower, upper, lower_suffix);
    } else {
        snprintf(output, output_size, "%s - %s", lower, upper);
    }
}

static void fcc_switch(FccApp* app, FccView view) {
    app->view = view;
    view_dispatcher_switch_to_view(app->dispatcher, view);
}

static void fcc_intro_draw_callback(Canvas* canvas, void* context) {
    UNUSED(context);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 90, 5, AlignCenter, AlignTop, "FCC ID Search");
    canvas_draw_xbm(canvas, 2, 8, FCC_QR_CODE_WIDTH, FCC_QR_CODE_HEIGHT, fcc_qr_code_bits);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 90, 20, AlignCenter, AlignTop, "wireless");
    canvas_draw_str_aligned(canvas, 90, 30, AlignCenter, AlignTop, "frequency lookup");
    canvas_draw_str_aligned(canvas, 90, 42, AlignCenter, AlignTop, "https://fcc.id");
    canvas_draw_str_aligned(canvas, 64, 62, AlignCenter, AlignBottom, "OK:Search  Back:Exit");
}

static bool fcc_intro_input_callback(InputEvent* event, void* context) {
    FccApp* app = context;
    if(event->type != InputTypeShort && event->type != InputTypePress) return false;
    if(event->key == InputKeyBack) return false;
    text_input_set_header_text(app->text_input, "FCC ID or prefix");
    fcc_switch(app, FccViewInput);
    return true;
}

static void fcc_show_message(FccApp* app, const char* message) {
    widget_reset(app->message_widget);
    fcc_copy(app->message_text, message, sizeof(app->message_text));
    widget_add_text_scroll_element(app->message_widget, 0, 0, 128, 64, app->message_text);
    fcc_switch(app, FccViewMessage);
}

static void fcc_search_callback(void* context);

static bool fcc_prep_lock(FccApp* app) {
    return app->prep_mutex && furi_mutex_acquire(app->prep_mutex, FuriWaitForever) == FuriStatusOk;
}

static void fcc_prep_unlock(FccApp* app) {
    if(app->prep_mutex) furi_mutex_release(app->prep_mutex);
}

static void fcc_prep_snapshot(
    FccApp* app,
    FccDbPrepState* state,
    uint32_t* done,
    uint32_t* total,
    bool* cancel) {
    if(fcc_prep_lock(app)) {
        if(state) *state = app->prep_state;
        if(done) *done = app->prep_done;
        if(total) *total = app->prep_total;
        if(cancel) *cancel = app->prep_cancel;
        fcc_prep_unlock(app);
    } else {
        if(state) *state = app->prep_state;
        if(done) *done = app->prep_done;
        if(total) *total = app->prep_total;
        if(cancel) *cancel = app->prep_cancel;
    }
}

static void fcc_prep_set_state(
    FccApp* app,
    FccDbPrepState state,
    uint32_t done,
    uint32_t total,
    bool notify) {
    if(fcc_prep_lock(app)) {
        app->prep_state = state;
        app->prep_done = done;
        app->prep_total = total;
        fcc_prep_unlock(app);
    } else {
        app->prep_state = state;
        app->prep_done = done;
        app->prep_total = total;
    }

    if(notify && app->dispatcher) {
        view_dispatcher_send_custom_event(app->dispatcher, FccCustomEventDbPrep);
    }
}

static bool fcc_prep_is_ready(FccApp* app) {
    FccDbPrepState state;
    fcc_prep_snapshot(app, &state, NULL, NULL, NULL);
    return state == FccDbPrepReady;
}

static bool fcc_db_prepare_progress_callback(void* context, uint32_t done, uint32_t total) {
    FccApp* app = context;
    bool cancel = false;
    fcc_prep_snapshot(app, NULL, NULL, NULL, &cancel);
    if(cancel) return false;

    bool notify = (done == 0U) || (done == total) || ((done % 8U) == 0U);
    fcc_prep_set_state(app, FccDbPrepPreparing, done, total, notify);
    return true;
}

static void fcc_show_prepare_status(FccApp* app) {
    FccDbPrepState state;
    uint32_t done;
    uint32_t total;
    fcc_prep_snapshot(app, &state, &done, &total, NULL);

    if(state == FccDbPrepReady) {
        snprintf(
            app->message_text, sizeof(app->message_text), "Database ready.\n\nOpening search...");
    } else if(state == FccDbPrepError) {
        snprintf(
            app->message_text,
            sizeof(app->message_text),
            "Database open failed.\n\n"
            "%s\n\n"
            "Ensure SD card space is available.",
            FCC_DB_SETUP_HINT);
    } else if(total > 0U) {
        uint32_t percent = (uint32_t)(((uint64_t)done * 100ULL) / total);
        snprintf(
            app->message_text,
            sizeof(app->message_text),
            "Opening database...\n\n"
            "Search will continue when\n"
            "the database is ready.\n\n"
            "Progress: %lu/%lu steps\n"
            "%lu%% complete",
            (unsigned long)done,
            (unsigned long)total,
            (unsigned long)percent);
    } else {
        snprintf(
            app->message_text,
            sizeof(app->message_text),
            "Opening database...\n\n"
            "Search will continue when\n"
            "the database is ready.\n\n"
            "Progress will appear soon.");
    }

    widget_reset(app->status_widget);
    widget_add_text_scroll_element(app->status_widget, 0, 0, 128, 64, app->message_text);
    fcc_switch(app, FccViewStatus);
}

static int32_t fcc_db_prepare_thread(void* context) {
    FccApp* app = context;
    fcc_prep_set_state(app, FccDbPrepPreparing, 0, 0, true);
    bool ok = fcc_db_open_with_progress(&app->db, fcc_db_prepare_progress_callback, app);

    bool cancel = false;
    fcc_prep_snapshot(app, NULL, NULL, NULL, &cancel);
    if(cancel) return 1;

    uint32_t total = 0;
    fcc_prep_snapshot(app, NULL, NULL, &total, NULL);
    fcc_prep_set_state(app, ok ? FccDbPrepReady : FccDbPrepError, total, total, true);
    return ok ? 0 : 1;
}

static void fcc_start_db_prepare(FccApp* app) {
    fcc_prep_set_state(app, FccDbPrepPreparing, 0, 0, false);
    app->prep_thread = furi_thread_alloc_ex("FccDbPrep", 4096, fcc_db_prepare_thread, app);
    if(!app->prep_thread) {
        fcc_prep_set_state(app, FccDbPrepError, 0, 0, true);
        return;
    }

    furi_thread_start(app->prep_thread);
}

static void fcc_stop_db_prepare(FccApp* app) {
    if(fcc_prep_lock(app)) {
        app->prep_cancel = true;
        fcc_prep_unlock(app);
    } else {
        app->prep_cancel = true;
    }

    if(app->prep_thread) {
        furi_thread_join(app->prep_thread);
        furi_thread_free(app->prep_thread);
        app->prep_thread = NULL;
    }
}

static bool fcc_custom_event_callback(void* context, uint32_t event) {
    FccApp* app = context;
    if(event != FccCustomEventDbPrep) return false;

    if(fcc_prep_is_ready(app) && app->pending_search) {
        app->pending_search = false;
        fcc_search_callback(app);
    } else if(app->view == FccViewStatus) {
        fcc_show_prepare_status(app);
    }

    return true;
}

static bool fcc_ensure_db_open(FccApp* app) {
    if(app->db.open) return true;

    if(fcc_db_open(&app->db)) return true;

    FURI_LOG_E(TAG, "Missing or invalid database: %s", FCC_DB_PATH);
    snprintf(
        app->message_text,
        sizeof(app->message_text),
        "Missing or invalid database.\n\n"
        "%s\n\n"
        "Ensure SD card space is available.\n\n"
        "Data from https://fccid.io",
        FCC_DB_SETUP_HINT);
    fcc_switch(app, FccViewMessage);
    return false;
}

static void fcc_show_detail(FccApp* app, const FccLookupResult* result, FccView back_view) {
    widget_reset(app->detail_widget);
    app->detail_text[0] = '\0';
    fcc_append(app->detail_text, sizeof(app->detail_text), "FCC ID: %s\n", result->fcc_id);
    fcc_append(app->detail_text, sizeof(app->detail_text), "Applicant:\n%s\n", result->applicant);
    fcc_append(app->detail_text, sizeof(app->detail_text), "Frequencies Supported:\n");

    for(uint32_t i = 0; i < result->interval_count; i++) {
        char range[72];
        fcc_format_frequency_range(
            result->intervals[i].lower_hz, result->intervals[i].upper_hz, range, sizeof(range));
        fcc_append(app->detail_text, sizeof(app->detail_text), "%s\n", range);
    }
    if(result->total_interval_count > result->interval_count) {
        fcc_append(
            app->detail_text,
            sizeof(app->detail_text),
            "+%llu more ranges\n",
            (unsigned long long)(result->total_interval_count - result->interval_count));
    }
    fcc_append(
        app->detail_text,
        sizeof(app->detail_text),
        "\nData Source:\nhttps://fcc.id/%s",
        result->fcc_id);

    widget_add_text_scroll_element(app->detail_widget, 0, 0, 128, 64, app->detail_text);
    app->detail_back_view = back_view;
    fcc_switch(app, FccViewDetail);
}

static void fcc_list_callback(void* context, uint32_t index);

static void fcc_show_list(FccApp* app) {
    submenu_reset(app->submenu);
    snprintf(
        app->list_header,
        sizeof(app->list_header),
        "%s %lu+",
        app->normalized,
        (unsigned long)(app->page_offset + 1));
    submenu_set_header(app->submenu, app->list_header);

    if(app->page_offset > 0) {
        submenu_add_item(
            app->submenu, "< Previous page", FccListActionPrev, fcc_list_callback, app);
    }

    for(uint32_t i = 0; i < app->page->count; i++) {
        submenu_add_item(app->submenu, app->page->items[i].id, i, fcc_list_callback, app);
    }

    if(app->page->has_more) {
        submenu_add_item(app->submenu, "Next page >", FccListActionNext, fcc_list_callback, app);
    }

    fcc_switch(app, FccViewList);
}

static bool fcc_load_prefix_page(FccApp* app, uint32_t offset) {
    char normalized[FCC_ID_LEN];
    if(!app->page) {
        app->page = malloc(sizeof(FccPrefixPage));
        if(!app->page) {
            fcc_show_message(app, "Not enough memory.");
            return false;
        }
    }

    if(!fcc_db_prefix_page(
           &app->db, app->input, offset, app->page, normalized, sizeof(normalized))) {
        fcc_show_message(app, "Database read error.");
        return false;
    }
    app->page_offset = offset;
    fcc_copy(app->normalized, normalized, sizeof(app->normalized));
    return true;
}

static void fcc_list_callback(void* context, uint32_t index) {
    FccApp* app = context;
    if(index == FccListActionPrev) {
        uint32_t offset = app->page_offset >= FCC_PAGE_SIZE ? app->page_offset - FCC_PAGE_SIZE : 0;
        if(fcc_load_prefix_page(app, offset)) fcc_show_list(app);
        return;
    }
    if(index == FccListActionNext) {
        if(fcc_load_prefix_page(app, app->page_offset + FCC_PAGE_SIZE)) fcc_show_list(app);
        return;
    }
    if(!app->page || index >= app->page->count) return;

    FccLookupResult result;
    if(fcc_db_lookup(&app->db, app->page->items[index].id, &result)) {
        fcc_show_detail(app, &result, FccViewList);
    } else {
        fcc_show_message(app, "Selected FCC ID was not found.");
    }
}

static void fcc_search_callback(void* context) {
    FccApp* app = context;
    if(fcc_normalize(app->input, app->normalized, sizeof(app->normalized)) == 0) {
        text_input_set_header_text(app->text_input, "Enter FCC ID or prefix");
        fcc_switch(app, FccViewInput);
        return;
    }

    if(!fcc_prep_is_ready(app)) {
        app->pending_search = true;
        fcc_show_prepare_status(app);
        return;
    }

    app->pending_search = false;
    if(!fcc_ensure_db_open(app)) return;

    FccLookupResult result;
    if(fcc_db_lookup(&app->db, app->input, &result)) {
        fcc_show_detail(app, &result, FccViewInput);
        return;
    }

    if(!fcc_load_prefix_page(app, 0)) return;
    if(app->page->count == 0) {
        fcc_show_message(app, "No matching FCC IDs.\n\nData sourced from https://fccid.io");
    } else if(app->page->count == 1 && !app->page->has_more) {
        if(fcc_db_lookup(&app->db, app->page->items[0].id, &result)) {
            fcc_show_detail(app, &result, FccViewInput);
        } else {
            fcc_show_message(app, "Matching FCC ID could not be loaded.");
        }
    } else {
        fcc_show_list(app);
    }
}

static bool fcc_navigation_callback(void* context) {
    FccApp* app = context;
    if(app->view == FccViewIntro) return false;
    if(app->view == FccViewInput) return false;
    if(app->view == FccViewDetail && app->detail_back_view == FccViewList) {
        fcc_switch(app, FccViewList);
        return true;
    }
    if(app->view == FccViewStatus) {
        app->pending_search = false;
        fcc_switch(app, FccViewInput);
        return true;
    }
    if(!app->db.open && app->view == FccViewMessage) return false;
    text_input_set_header_text(app->text_input, "FCC ID or prefix");
    fcc_switch(app, FccViewInput);
    return true;
}

static FccApp* fcc_app_alloc(void) {
    FccApp* app = malloc(sizeof(FccApp));
    if(!app) return NULL;
    memset(app, 0, sizeof(FccApp));

    app->gui = furi_record_open(RECORD_GUI);
    app->dispatcher = view_dispatcher_alloc();
    if(app->dispatcher) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        view_dispatcher_enable_queue(app->dispatcher);
#pragma GCC diagnostic pop
    }
    app->prep_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->intro_view = view_alloc();
    app->text_input = text_input_alloc();
    app->submenu = submenu_alloc();
    app->detail_widget = widget_alloc();
    app->message_widget = widget_alloc();
    app->status_widget = widget_alloc();
    if(!app->gui || !app->dispatcher || !app->intro_view || !app->text_input || !app->submenu ||
       !app->detail_widget || !app->message_widget || !app->status_widget || !app->prep_mutex) {
        if(app->status_widget) widget_free(app->status_widget);
        if(app->message_widget) widget_free(app->message_widget);
        if(app->detail_widget) widget_free(app->detail_widget);
        if(app->submenu) submenu_free(app->submenu);
        if(app->text_input) text_input_free(app->text_input);
        if(app->intro_view) view_free(app->intro_view);
        if(app->dispatcher) view_dispatcher_free(app->dispatcher);
        if(app->prep_mutex) furi_mutex_free(app->prep_mutex);
        if(app->gui) furi_record_close(RECORD_GUI);
        free(app);
        return NULL;
    }

    view_dispatcher_set_event_callback_context(app->dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->dispatcher, fcc_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(app->dispatcher, fcc_navigation_callback);
    view_dispatcher_attach_to_gui(app->dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    view_set_context(app->intro_view, app);
    view_set_draw_callback(app->intro_view, fcc_intro_draw_callback);
    view_set_input_callback(app->intro_view, fcc_intro_input_callback);

    text_input_set_header_text(app->text_input, "FCC ID or prefix");
    text_input_set_result_callback(
        app->text_input, fcc_search_callback, app, app->input, sizeof(app->input), true);

    view_dispatcher_add_view(app->dispatcher, FccViewIntro, app->intro_view);
    view_dispatcher_add_view(app->dispatcher, FccViewInput, text_input_get_view(app->text_input));
    view_dispatcher_add_view(app->dispatcher, FccViewList, submenu_get_view(app->submenu));
    view_dispatcher_add_view(app->dispatcher, FccViewDetail, widget_get_view(app->detail_widget));
    view_dispatcher_add_view(
        app->dispatcher, FccViewMessage, widget_get_view(app->message_widget));
    view_dispatcher_add_view(app->dispatcher, FccViewStatus, widget_get_view(app->status_widget));

    return app;
}

static void fcc_app_free(FccApp* app) {
    fcc_stop_db_prepare(app);

    view_dispatcher_remove_view(app->dispatcher, FccViewIntro);
    view_dispatcher_remove_view(app->dispatcher, FccViewInput);
    view_dispatcher_remove_view(app->dispatcher, FccViewList);
    view_dispatcher_remove_view(app->dispatcher, FccViewDetail);
    view_dispatcher_remove_view(app->dispatcher, FccViewMessage);
    view_dispatcher_remove_view(app->dispatcher, FccViewStatus);

    text_input_free(app->text_input);
    submenu_free(app->submenu);
    widget_free(app->detail_widget);
    widget_free(app->message_widget);
    widget_free(app->status_widget);
    view_free(app->intro_view);
    view_dispatcher_free(app->dispatcher);
    furi_mutex_free(app->prep_mutex);
    furi_record_close(RECORD_GUI);
    fcc_db_close(&app->db);
    free(app->page);
    free(app);
}

int32_t fcc_id_lookup_app(void* p) {
    UNUSED(p);

    FccApp* app = fcc_app_alloc();
    if(!app) return -1;
    fcc_switch(app, FccViewIntro);
    fcc_start_db_prepare(app);

    view_dispatcher_run(app->dispatcher);
    fcc_app_free(app);
    return 0;
}
