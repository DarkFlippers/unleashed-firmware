#include "update_operation.h"

#include "update_manifest.h"

#include <furi.h>
#include <furi_hal.h>
#include <m-string.h>
#include <loader/loader.h>
#include <lib/toolbox/path.h>

static const char* UPDATE_ROOT_DIR = "/ext" UPDATE_DIR_DEFAULT_REL_PATH;
static const char* UPDATE_PREFIX = "/ext" UPDATE_DIR_DEFAULT_REL_PATH "/";
static const char* UPDATE_SUFFIX = "/" UPDATE_MANIFEST_DEFAULT_NAME;
static const uint32_t MAX_DIR_NAME_LEN = 250;

static const char* update_prepare_result_descr[] = {
    [UpdatePrepareResultOK] = "OK",
    [UpdatePrepareResultManifestPathInvalid] = "Invalid manifest name or location",
    [UpdatePrepareResultManifestFolderNotFound] = "Update folder not found",
    [UpdatePrepareResultManifestInvalid] = "Invalid manifest data",
    [UpdatePrepareResultStageMissing] = "Missing Stage2 loader",
    [UpdatePrepareResultStageIntegrityError] = "Corrupted Stage2 loader",
};

const char* update_operation_describe_preparation_result(const UpdatePrepareResult value) {
    if(value >= COUNT_OF(update_prepare_result_descr)) {
        return "...";
    } else {
        return update_prepare_result_descr[value];
    }
}

bool update_operation_get_package_dir_name(const char* full_path, string_t out_manifest_dir) {
    bool path_ok = false;
    string_t full_path_str;
    string_init_set(full_path_str, full_path);
    string_reset(out_manifest_dir);
    bool start_end_ok = string_start_with_str_p(full_path_str, UPDATE_PREFIX) &&
                        string_end_with_str_p(full_path_str, UPDATE_SUFFIX);
    int16_t dir_name_len =
        strlen(full_path) - strlen(UPDATE_PREFIX) - strlen(UPDATE_MANIFEST_DEFAULT_NAME) - 1;
    if(dir_name_len == -1) {
        path_ok = true;
    } else if(start_end_ok && (dir_name_len > 0)) {
        string_set_n(out_manifest_dir, full_path_str, strlen(UPDATE_PREFIX), dir_name_len);
        path_ok = true;
        if(string_search_char(out_manifest_dir, '/') != STRING_FAILURE) {
            string_reset(out_manifest_dir);
            path_ok = false;
        }
    }
    string_clear(full_path_str);
    return path_ok;
}

int32_t update_operation_get_package_index(Storage* storage, const char* update_package_dir) {
    furi_assert(storage);
    furi_assert(update_package_dir);

    if(strlen(update_package_dir) == 0) {
        return 0;
    }

    bool found = false;
    int32_t index = 0;
    File* dir = storage_file_alloc(storage);
    FileInfo fi = {0};
    char* name_buffer = malloc(MAX_DIR_NAME_LEN);
    do {
        if(!storage_dir_open(dir, UPDATE_ROOT_DIR)) {
            break;
        }

        while(storage_dir_read(dir, &fi, name_buffer, MAX_DIR_NAME_LEN)) {
            index++;
            if(strcmp(name_buffer, update_package_dir)) {
                continue;
            } else {
                found = true;
                break;
            }
        }
    } while(false);

    free(name_buffer);
    storage_file_free(dir);

    return found ? index : -1;
}

bool update_operation_get_current_package_path(Storage* storage, string_t out_path) {
    uint32_t update_index = furi_hal_rtc_get_register(FuriHalRtcRegisterUpdateFolderFSIndex);
    string_set_str(out_path, UPDATE_ROOT_DIR);
    if(update_index == 0) {
        return true;
    }

    bool found = false;
    uint32_t iter_index = 0;
    File* dir = storage_file_alloc(storage);
    FileInfo fi = {0};
    char* name_buffer = malloc(MAX_DIR_NAME_LEN);
    do {
        if(!storage_dir_open(dir, UPDATE_ROOT_DIR)) {
            break;
        }

        while(storage_dir_read(dir, &fi, name_buffer, MAX_DIR_NAME_LEN)) {
            if(++iter_index == update_index) {
                found = true;
                path_append(out_path, name_buffer);
                break;
            }
        }
    } while(false);

    free(name_buffer);
    storage_file_free(dir);
    if(!found) {
        string_reset(out_path);
    }

    return found;
}

UpdatePrepareResult update_operation_prepare(const char* manifest_file_path) {
    string_t update_folder;
    string_init(update_folder);
    if(!update_operation_get_package_dir_name(manifest_file_path, update_folder)) {
        string_clear(update_folder);
        return UpdatePrepareResultManifestPathInvalid;
    }

    Storage* storage = furi_record_open("storage");
    int32_t update_index =
        update_operation_get_package_index(storage, string_get_cstr(update_folder));
    string_clear(update_folder);

    if(update_index < 0) {
        furi_record_close("storage");
        return UpdatePrepareResultManifestFolderNotFound;
    }

    string_t update_dir_path;
    string_init(update_dir_path);
    path_extract_dirname(manifest_file_path, update_dir_path);

    UpdatePrepareResult result = UpdatePrepareResultManifestInvalid;
    UpdateManifest* manifest = update_manifest_alloc();
    if(update_manifest_init(manifest, manifest_file_path)) {
        result = UpdatePrepareResultStageMissing;
        File* file = storage_file_alloc(storage);

        string_t stage_path;
        string_init(stage_path);
        path_extract_dirname(manifest_file_path, stage_path);
        path_append(stage_path, string_get_cstr(manifest->staged_loader_file));

        const uint16_t READ_BLOCK = 0x1000;
        uint8_t* read_buffer = malloc(READ_BLOCK);
        uint32_t crc = 0;
        do {
            if(!storage_file_open(
                   file, string_get_cstr(stage_path), FSAM_READ, FSOM_OPEN_EXISTING)) {
                break;
            }

            result = UpdatePrepareResultStageIntegrityError;
            furi_hal_crc_acquire(osWaitForever);

            uint16_t bytes_read = 0;
            do {
                bytes_read = storage_file_read(file, read_buffer, READ_BLOCK);
                crc = furi_hal_crc_feed(read_buffer, bytes_read);
            } while(bytes_read == READ_BLOCK);

            furi_hal_crc_reset();
        } while(false);

        string_clear(stage_path);
        free(read_buffer);
        storage_file_free(file);

        if(crc == manifest->staged_loader_crc) {
            furi_hal_rtc_set_boot_mode(FuriHalRtcBootModePreUpdate);
            update_operation_persist_package_index(update_index);
            result = UpdatePrepareResultOK;
        }
    }
    furi_record_close("storage");
    update_manifest_free(manifest);

    return result;
}

bool update_operation_is_armed() {
    return furi_hal_rtc_get_boot_mode() == FuriHalRtcBootModePreUpdate;
}

void update_operation_disarm() {
    furi_hal_rtc_set_boot_mode(FuriHalRtcBootModeNormal);
    furi_hal_rtc_set_register(FuriHalRtcRegisterUpdateFolderFSIndex, 0);
}

void update_operation_persist_package_index(uint32_t index) {
    furi_hal_rtc_set_register(FuriHalRtcRegisterUpdateFolderFSIndex, index);
}