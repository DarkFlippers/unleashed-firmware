#include "sim800_phonebook.h"
#include "sim800_types.h"

#include <storage/storage.h>
#include <flipper_format/flipper_format.h>
#include <toolbox/path.h>

#define TAG "Sim800Phonebook"

#define SIM800_PHONE_FILE_TYPE     "Flipper SIM800 Phonebook Entry"
#define SIM800_PHONE_FILE_VERSION  1
#define SIM800_PRESET_FILE_TYPE    "Flipper SIM800 SMS Preset"
#define SIM800_PRESET_FILE_VERSION 1

#define SIM800_FIELD_NAME   "Name"
#define SIM800_FIELD_NUMBER "Number"
#define SIM800_FIELD_TEXT   "Text"

void sim800_storage_make_folders(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_simply_mkdir(storage, SIM800_PHONEBOOK_FOLDER);
    storage_simply_mkdir(storage, SIM800_PRESET_FOLDER);
    furi_record_close(RECORD_STORAGE);
}

static bool sim800_write_pair(
    const char* path,
    const char* type,
    uint32_t version,
    const char* key_a,
    const char* value_a,
    const char* key_b,
    const char* value_b) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);
    bool saved = false;

    do {
        if(!flipper_format_file_open_always(file, path)) break;
        if(!flipper_format_write_header_cstr(file, type, version)) break;
        if(key_a && !flipper_format_write_string_cstr(file, key_a, value_a)) break;
        if(key_b && !flipper_format_write_string_cstr(file, key_b, value_b)) break;
        saved = true;
    } while(false);

    if(!saved) FURI_LOG_E(TAG, "Failed to write %s", path);

    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);
    return saved;
}

static bool sim800_read_fields(
    const char* path,
    const char* type,
    uint32_t version,
    const char* key_a,
    FuriString* value_a,
    const char* key_b,
    FuriString* value_b) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);
    FuriString* header = furi_string_alloc();
    uint32_t file_version = 0;
    bool loaded = false;

    do {
        if(!flipper_format_file_open_existing(file, path)) break;
        if(!flipper_format_read_header(file, header, &file_version)) break;
        if(strcmp(furi_string_get_cstr(header), type) != 0 || file_version != version) break;
        // Each read picks up where the last stopped, so rewind to keep the fields independent
        if(key_a && value_a) {
            flipper_format_rewind(file);
            if(!flipper_format_read_string(file, key_a, value_a)) break;
        }
        if(key_b && value_b) {
            flipper_format_rewind(file);
            if(!flipper_format_read_string(file, key_b, value_b)) break;
        }
        loaded = true;
    } while(false);

    flipper_format_free(file);
    furi_string_free(header);
    furi_record_close(RECORD_STORAGE);
    return loaded;
}

static bool sim800_remove(const char* path) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool removed = storage_simply_remove(storage, path);
    furi_record_close(RECORD_STORAGE);
    return removed;
}

void sim800_phonebook_path(const char* name, FuriString* path) {
    furi_assert(name);
    furi_assert(path);
    furi_string_printf(path, "%s/%s%s", SIM800_PHONEBOOK_FOLDER, name, SIM800_PHONE_EXTENSION);
}

bool sim800_phonebook_save(const char* name, const char* number) {
    furi_assert(name);
    furi_assert(number);

    FuriString* path = furi_string_alloc();
    sim800_phonebook_path(name, path);
    bool saved = sim800_write_pair(
        furi_string_get_cstr(path),
        SIM800_PHONE_FILE_TYPE,
        SIM800_PHONE_FILE_VERSION,
        SIM800_FIELD_NAME,
        name,
        SIM800_FIELD_NUMBER,
        number);
    furi_string_free(path);
    return saved;
}

bool sim800_phonebook_load(const char* path, FuriString* name, FuriString* number) {
    furi_assert(path);
    furi_assert(number);

    FuriString* local_name = furi_string_alloc();
    bool loaded = sim800_read_fields(
        path,
        SIM800_PHONE_FILE_TYPE,
        SIM800_PHONE_FILE_VERSION,
        SIM800_FIELD_NAME,
        local_name,
        SIM800_FIELD_NUMBER,
        number);

    if(loaded && name) {
        furi_string_set(name, local_name);
    } else if(loaded == false && name) {
        // A hand-written file may still be readable; fall back to its file name
        sim800_path_to_name(path, name);
    }
    furi_string_free(local_name);
    return loaded;
}

bool sim800_phonebook_delete(const char* path) {
    return sim800_remove(path);
}

void sim800_preset_path(const char* name, FuriString* path) {
    furi_assert(name);
    furi_assert(path);
    furi_string_printf(path, "%s/%s%s", SIM800_PRESET_FOLDER, name, SIM800_PRESET_EXTENSION);
}

bool sim800_preset_save(const char* name, const char* text) {
    furi_assert(name);
    furi_assert(text);

    FuriString* path = furi_string_alloc();
    sim800_preset_path(name, path);
    bool saved = sim800_write_pair(
        furi_string_get_cstr(path),
        SIM800_PRESET_FILE_TYPE,
        SIM800_PRESET_FILE_VERSION,
        SIM800_FIELD_TEXT,
        text,
        NULL,
        NULL);
    furi_string_free(path);
    return saved;
}

bool sim800_preset_load(const char* path, FuriString* text) {
    return sim800_read_fields(
        path,
        SIM800_PRESET_FILE_TYPE,
        SIM800_PRESET_FILE_VERSION,
        SIM800_FIELD_TEXT,
        text,
        NULL,
        NULL);
}

bool sim800_preset_delete(const char* path) {
    return sim800_remove(path);
}

size_t sim800_phonebook_count(void) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* dir = storage_file_alloc(storage);
    size_t count = 0;

    if(storage_dir_open(dir, SIM800_PHONEBOOK_FOLDER)) {
        FileInfo info;
        char entry[SIM800_NAME_MAX + sizeof(SIM800_PHONE_EXTENSION) + 1];
        while(storage_dir_read(dir, &info, entry, sizeof(entry))) {
            if(file_info_is_dir(&info)) continue;
            size_t len = strlen(entry);
            size_t ext_len = strlen(SIM800_PHONE_EXTENSION);
            if(len <= ext_len || strcmp(entry + len - ext_len, SIM800_PHONE_EXTENSION) != 0) {
                continue;
            }
            count++;
        }
    }

    storage_dir_close(dir);
    storage_file_free(dir);
    furi_record_close(RECORD_STORAGE);
    return count;
}

size_t sim800_preset_list(FuriString** names, size_t max) {
    furi_assert(names);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* dir = storage_file_alloc(storage);
    size_t count = 0;

    if(storage_dir_open(dir, SIM800_PRESET_FOLDER)) {
        FileInfo info;
        char entry[SIM800_NAME_MAX + sizeof(SIM800_PRESET_EXTENSION) + 1];
        while(count < max && storage_dir_read(dir, &info, entry, sizeof(entry))) {
            if(file_info_is_dir(&info)) continue;
            size_t len = strlen(entry);
            size_t ext_len = strlen(SIM800_PRESET_EXTENSION);
            if(len <= ext_len || strcmp(entry + len - ext_len, SIM800_PRESET_EXTENSION) != 0) {
                continue;
            }
            furi_string_set_strn(names[count], entry, len - ext_len);
            count++;
        }
    }

    storage_dir_close(dir);
    storage_file_free(dir);
    furi_record_close(RECORD_STORAGE);
    return count;
}

void sim800_path_to_name(const char* path, FuriString* name) {
    furi_assert(path);
    furi_assert(name);

    FuriString* full = furi_string_alloc_set_str(path);
    path_extract_filename(full, name, true);
    furi_string_free(full);
}
