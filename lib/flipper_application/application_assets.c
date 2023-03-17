#include "application_assets.h"
#include <toolbox/path.h>
#include <storage/storage_i.h>

// #define ELF_ASSETS_DEBUG_LOG 1

#ifndef ELF_ASSETS_DEBUG_LOG
#undef FURI_LOG_D
#define FURI_LOG_D(...)
#undef FURI_LOG_E
#define FURI_LOG_E(...)
#endif

#define FLIPPER_APPLICATION_ASSETS_MAGIC 0x4F4C5A44
#define FLIPPER_APPLICATION_ASSETS_VERSION 1
#define FLIPPER_APPLICATION_ASSETS_SIGNATURE_FILENAME ".assets.signature"

#define BUFFER_SIZE 512

#define TAG "fap_assets"

#pragma pack(push, 1)

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t dirs_count;
    uint32_t files_count;
} FlipperApplicationAssetsHeader;

#pragma pack(pop)

typedef enum {
    AssetsSignatureResultEqual,
    AssetsSignatureResultNotEqual,
    AssetsSignatureResultError,
} AssetsSignatureResult;

static FuriString* flipper_application_assets_alloc_app_full_path(FuriString* app_name) {
    furi_assert(app_name);
    FuriString* full_path = furi_string_alloc_set(APPS_ASSETS_PATH "/");
    furi_string_cat(full_path, app_name);
    return full_path;
}

static FuriString* flipper_application_assets_alloc_signature_file_path(FuriString* app_name) {
    furi_assert(app_name);
    FuriString* signature_file_path = flipper_application_assets_alloc_app_full_path(app_name);
    furi_string_cat(signature_file_path, "/" FLIPPER_APPLICATION_ASSETS_SIGNATURE_FILENAME);

    return signature_file_path;
}

static uint8_t* flipper_application_assets_alloc_and_load_data(File* file, size_t* size) {
    furi_assert(file);

    uint8_t* data = NULL;
    uint32_t length = 0;

    // read data length
    if(storage_file_read(file, &length, sizeof(length)) != sizeof(length)) {
        return NULL;
    }

    data = malloc(length);

    // read data
    if(storage_file_read(file, (void*)data, length) != length) {
        free((void*)data);
        return NULL;
    }

    if(size != NULL) {
        *size = length;
    }

    return data;
}

static bool flipper_application_assets_process_files(
    Storage* storage,
    File* file,
    FuriString* app_name,
    uint32_t files_count) {
    furi_assert(storage);
    furi_assert(file);
    furi_assert(app_name);

    UNUSED(storage);

    bool success = false;
    uint32_t length = 0;
    char* path = NULL;
    FuriString* file_path = furi_string_alloc();
    File* destination = storage_file_alloc(storage);

    FuriString* full_path = flipper_application_assets_alloc_app_full_path(app_name);

    for(uint32_t i = 0; i < files_count; i++) {
        path = (char*)flipper_application_assets_alloc_and_load_data(file, NULL);

        if(path == NULL) {
            break;
        }

        // read file size
        if(storage_file_read(file, &length, sizeof(length)) != sizeof(length)) {
            break;
        }

        furi_string_set(file_path, full_path);
        furi_string_cat(file_path, "/");
        furi_string_cat(file_path, path);

        if(!storage_file_open(
               destination, furi_string_get_cstr(file_path), FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            FURI_LOG_E(TAG, "Can't create file: %s", furi_string_get_cstr(file_path));
            break;
        }

        // copy data to file
        if(!storage_file_copy_to_file(file, destination, length)) {
            FURI_LOG_E(TAG, "Can't copy data to file: %s", furi_string_get_cstr(file_path));
            break;
        }

        storage_file_close(destination);

        free(path);
        path = NULL;

        if(i == files_count - 1) {
            success = true;
        }
    }

    if(path != NULL) {
        free(path);
    }

    storage_file_free(destination);
    furi_string_free(file_path);

    return success;
}

static bool flipper_application_assets_process_dirs(
    Storage* storage,
    File* file,
    FuriString* app_name,
    uint32_t dirs_count) {
    furi_assert(storage);
    furi_assert(file);
    furi_assert(app_name);

    bool success = false;
    FuriString* full_path = flipper_application_assets_alloc_app_full_path(app_name);

    do {
        if(!storage_simply_mkdir(storage, APPS_ASSETS_PATH)) {
            break;
        }

        if(!storage_simply_mkdir(storage, furi_string_get_cstr(full_path))) {
            break;
        }

        FuriString* dir_path = furi_string_alloc();
        char* path = NULL;

        for(uint32_t i = 0; i < dirs_count; i++) {
            path = (char*)flipper_application_assets_alloc_and_load_data(file, NULL);

            if(path == NULL) {
                break;
            }

            furi_string_set(dir_path, full_path);
            furi_string_cat(dir_path, "/");
            furi_string_cat(dir_path, path);

            if(!storage_simply_mkdir(storage, furi_string_get_cstr(dir_path))) {
                FURI_LOG_E(TAG, "Can't create directory: %s", furi_string_get_cstr(dir_path));
                break;
            }

            free(path);
            path = NULL;

            if(i == dirs_count - 1) {
                success = true;
            }
        }

        if(path != NULL) {
            free(path);
        }

        furi_string_free(dir_path);
    } while(false);

    furi_string_free(full_path);

    return success;
}

static AssetsSignatureResult flipper_application_assets_process_signature(
    Storage* storage,
    File* file,
    FuriString* app_name,
    uint8_t** signature_data,
    size_t* signature_data_size) {
    furi_assert(storage);
    furi_assert(file);
    furi_assert(app_name);
    furi_assert(signature_data);
    furi_assert(signature_data_size);

    AssetsSignatureResult result = AssetsSignatureResultError;
    File* signature_file = storage_file_alloc(storage);
    FuriString* signature_file_path =
        flipper_application_assets_alloc_signature_file_path(app_name);

    do {
        // read signature
        *signature_data =
            flipper_application_assets_alloc_and_load_data(file, signature_data_size);

        if(*signature_data == NULL) { //-V547
            FURI_LOG_E(TAG, "Can't read signature");
            break;
        }

        result = AssetsSignatureResultNotEqual;

        if(!storage_file_open(
               signature_file,
               furi_string_get_cstr(signature_file_path),
               FSAM_READ_WRITE,
               FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Can't open signature file");
            break;
        }

        size_t signature_size = storage_file_size(signature_file);
        uint8_t* signature_file_data = malloc(signature_size);
        if(storage_file_read(signature_file, signature_file_data, signature_size) !=
           signature_size) {
            FURI_LOG_E(TAG, "Can't read signature file");
            free(signature_file_data);
            break;
        }

        if(memcmp(*signature_data, signature_file_data, signature_size) == 0) {
            FURI_LOG_D(TAG, "Assets signature is equal");
            result = AssetsSignatureResultEqual;
        }

        free(signature_file_data);
    } while(0);

    storage_file_free(signature_file);
    furi_string_free(signature_file_path);

    return result;
}

bool flipper_application_assets_load(File* file, const char* elf_path, size_t offset, size_t size) {
    UNUSED(size);
    furi_assert(file);
    furi_assert(elf_path);
    FlipperApplicationAssetsHeader header;
    bool result = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    uint8_t* signature_data = NULL;
    size_t signature_data_size = 0;
    FuriString* app_name = furi_string_alloc();
    path_extract_filename_no_ext(elf_path, app_name);

    FURI_LOG_D(TAG, "Loading assets for %s", furi_string_get_cstr(app_name));

    do {
        if(!storage_file_seek(file, offset, true)) {
            break;
        }

        // read header
        if(storage_file_read(file, &header, sizeof(header)) != sizeof(header)) {
            break;
        }

        if(header.magic != FLIPPER_APPLICATION_ASSETS_MAGIC) {
            break;
        }

        if(header.version != FLIPPER_APPLICATION_ASSETS_VERSION) {
            break;
        }

        // process signature
        AssetsSignatureResult signature_result = flipper_application_assets_process_signature(
            storage, file, app_name, &signature_data, &signature_data_size);

        if(signature_result == AssetsSignatureResultError) {
            FURI_LOG_E(TAG, "Assets signature error");
            break;
        } else if(signature_result == AssetsSignatureResultEqual) {
            FURI_LOG_D(TAG, "Assets signature equal, skip loading");
            result = true;
            break;
        } else {
            FURI_LOG_D(TAG, "Assets signature not equal, loading");

            // remove old assets
            FuriString* full_path = flipper_application_assets_alloc_app_full_path(app_name);
            storage_simply_remove_recursive(storage, furi_string_get_cstr(full_path));
            furi_string_free(full_path);

            FURI_LOG_D(TAG, "Assets removed");
        }

        // process directories
        if(!flipper_application_assets_process_dirs(storage, file, app_name, header.dirs_count)) {
            break;
        }

        // process files
        if(!flipper_application_assets_process_files(storage, file, app_name, header.files_count)) {
            break;
        }

        // write signature
        FuriString* signature_file_path =
            flipper_application_assets_alloc_signature_file_path(app_name);
        File* signature_file = storage_file_alloc(storage);

        if(storage_file_open(
               signature_file,
               furi_string_get_cstr(signature_file_path),
               FSAM_WRITE,
               FSOM_CREATE_ALWAYS)) {
            storage_file_write(signature_file, signature_data, signature_data_size);
        }

        storage_file_free(signature_file);
        furi_string_free(signature_file_path);

        result = true;
    } while(false);

    if(signature_data != NULL) {
        free(signature_data);
    }

    furi_record_close(RECORD_STORAGE);
    furi_string_free(app_name);

    FURI_LOG_D(TAG, "Assets loading %s", result ? "success" : "failed");

    return result;
}