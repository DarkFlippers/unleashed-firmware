#include "saved_struct.h"
#include <furi.h>
#include <stdint.h>
#include <storage/storage.h>

#define TAG "SavedStruct"

typedef struct {
    uint8_t magic;
    uint8_t version;
    uint8_t checksum;
    uint8_t flags;
    uint32_t timestamp;
} SavedStructHeader;

bool saved_struct_save(
    const char* path,
    const void* data,
    size_t size,
    uint8_t magic,
    uint8_t version) {
    furi_check(path);
    furi_check(data);
    furi_check(size);
    SavedStructHeader header;

    FURI_LOG_I(TAG, "Saving \"%s\"", path);

    // Store
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    bool result = true;
    bool saved = storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS);
    if(!saved) {
        FURI_LOG_E(
            TAG, "Open failed \"%s\". Error: \'%s\'", path, storage_file_get_error_desc(file));
        result = false;
    }

    if(result) {
        // Calculate checksum
        uint8_t checksum = 0;
        const uint8_t* source = data;
        for(size_t i = 0; i < size; i++) {
            checksum += source[i];
        }
        // Set header
        header.magic = magic;
        header.version = version;
        header.checksum = checksum;
        header.flags = 0;
        header.timestamp = 0;

        size_t bytes_count = storage_file_write(file, &header, sizeof(header));
        bytes_count += storage_file_write(file, data, size);

        if(bytes_count != (size + sizeof(header))) {
            FURI_LOG_E(
                TAG, "Write failed \"%s\". Error: \'%s\'", path, storage_file_get_error_desc(file));
            result = false;
        }
    }

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return result;
}

bool saved_struct_load(const char* path, void* data, size_t size, uint8_t magic, uint8_t version) {
    furi_check(path);
    furi_check(data);
    furi_check(size);

    FURI_LOG_I(TAG, "Loading \"%s\"", path);

    SavedStructHeader header;

    uint8_t* data_read = malloc(size);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    bool result = true;
    bool loaded = storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING);
    if(!loaded) {
        FURI_LOG_E(
            TAG, "Failed to read \"%s\". Error: %s", path, storage_file_get_error_desc(file));
        result = false;
    }

    if(result) {
        size_t bytes_count = storage_file_read(file, &header, sizeof(SavedStructHeader));
        bytes_count += storage_file_read(file, data_read, size);

        if(bytes_count != (sizeof(SavedStructHeader) + size)) {
            FURI_LOG_E(TAG, "Size mismatch of file \"%s\"", path);
            result = false;
        }
    }

    if(result && (header.magic != magic || header.version != version)) {
        FURI_LOG_E(
            TAG,
            "Magic(%d != %d) or Version(%d != %d) mismatch of file \"%s\"",
            header.magic,
            magic,
            header.version,
            version,
            path);
        result = false;
    }

    if(result) {
        uint8_t checksum = 0;
        const uint8_t* source = (const uint8_t*)data_read;
        for(size_t i = 0; i < size; i++) {
            checksum += source[i];
        }

        if(header.checksum != checksum) {
            FURI_LOG_E(
                TAG, "Checksum(%d != %d) mismatch of file \"%s\"", header.checksum, checksum, path);
            result = false;
        }
    }

    if(result) {
        memcpy(data, data_read, size);
    }

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    free(data_read);

    return result;
}

bool saved_struct_get_metadata(
    const char* path,
    uint8_t* magic,
    uint8_t* version,
    size_t* payload_size) {
    furi_check(path);

    SavedStructHeader header;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    bool result = false;
    do {
        if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(
                TAG, "Failed to read \"%s\". Error: %s", path, storage_file_get_error_desc(file));
            break;
        }

        if(storage_file_read(file, &header, sizeof(SavedStructHeader)) !=
           sizeof(SavedStructHeader)) {
            FURI_LOG_E(TAG, "Failed to read header");
            break;
        }

        if(magic) {
            *magic = header.magic;
        }
        if(version) {
            *version = header.version;
        }
        if(payload_size) {
            uint64_t file_size = storage_file_size(file);
            *payload_size = file_size - sizeof(SavedStructHeader);
        }

        result = true;
    } while(false);

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return result;
}
