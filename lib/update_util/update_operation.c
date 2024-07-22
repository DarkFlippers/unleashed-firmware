#include "update_operation.h"

#include "update_manifest.h"

#include <furi.h>
#include <furi_hal.h>
#include <loader/loader.h>
#include <lib/toolbox/path.h>
#include <lib/toolbox/crc32_calc.h>

#define UPDATE_ROOT_DIR EXT_PATH("update")

/* Need at least 4 free LFS pages before update */
#define UPDATE_MIN_INT_FREE_SPACE (2 * 4 * 1024)

static const char* update_prepare_result_descr[] = {
    [UpdatePrepareResultOK] = "OK",
    [UpdatePrepareResultManifestPathInvalid] = "Invalid manifest name or location",
    [UpdatePrepareResultManifestFolderNotFound] = "Update folder not found",
    [UpdatePrepareResultManifestInvalid] = "Invalid manifest data",
    [UpdatePrepareResultStageMissing] = "Missing Stage2 loader",
    [UpdatePrepareResultStageIntegrityError] = "Corrupted Stage2 loader",
    [UpdatePrepareResultManifestPointerCreateError] = "Failed to create update pointer file",
    [UpdatePrepareResultManifestPointerCheckError] = "Update pointer file error (corrupted FS?)",
    [UpdatePrepareResultTargetMismatch] = "Hardware target mismatch",
    [UpdatePrepareResultOutdatedManifestVersion] = "Update package is too old",
    [UpdatePrepareResultIntFull] = "Need more free space in internal storage",
    [UpdatePrepareResultUnspecifiedError] = "Unknown error",
};

const char* update_operation_describe_preparation_result(const UpdatePrepareResult value) {
    if(value >= COUNT_OF(update_prepare_result_descr)) {
        return "...";
    } else {
        return update_prepare_result_descr[value];
    }
}

static bool update_operation_get_current_package_path_rtc(Storage* storage, FuriString* out_path) {
    const uint32_t update_index = furi_hal_rtc_get_register(FuriHalRtcRegisterUpdateFolderFSIndex);
    furi_string_set(out_path, UPDATE_ROOT_DIR);
    if(update_index == UPDATE_OPERATION_ROOT_DIR_PACKAGE_MAGIC) {
        return true;
    }

    bool found = false;
    uint32_t iter_index = 0;
    File* dir = storage_file_alloc(storage);
    FileInfo fi = {0};
    char* name_buffer = malloc(UPDATE_OPERATION_MAX_MANIFEST_PATH_LEN);
    do {
        if(!storage_dir_open(dir, UPDATE_ROOT_DIR)) {
            break;
        }

        while(storage_dir_read(dir, &fi, name_buffer, UPDATE_OPERATION_MAX_MANIFEST_PATH_LEN)) {
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
        furi_string_reset(out_path);
    }

    return found;
}

#define UPDATE_FILE_POINTER_FN       EXT_PATH(UPDATE_MANIFEST_POINTER_FILE_NAME)
#define UPDATE_MANIFEST_MAX_PATH_LEN 256u

bool update_operation_get_current_package_manifest_path(Storage* storage, FuriString* out_path) {
    furi_string_reset(out_path);
    if(storage_common_stat(storage, UPDATE_FILE_POINTER_FN, NULL) == FSE_OK) {
        char* manifest_name_buffer = malloc(UPDATE_MANIFEST_MAX_PATH_LEN);
        File* upd_file = NULL;
        do {
            upd_file = storage_file_alloc(storage);
            if(!storage_file_open(
                   upd_file, UPDATE_FILE_POINTER_FN, FSAM_READ, FSOM_OPEN_EXISTING)) {
                break;
            }
            size_t bytes_read =
                storage_file_read(upd_file, manifest_name_buffer, UPDATE_MANIFEST_MAX_PATH_LEN);
            if((bytes_read == 0) || (bytes_read == UPDATE_MANIFEST_MAX_PATH_LEN)) {
                break;
            }
            if(storage_common_stat(storage, manifest_name_buffer, NULL) != FSE_OK) {
                break;
            }
            furi_string_set(out_path, manifest_name_buffer);
        } while(0);
        free(manifest_name_buffer);
        storage_file_free(upd_file);
    } else {
        /* legacy, will be deprecated */
        FuriString* rtcpath;
        rtcpath = furi_string_alloc();
        do {
            if(!update_operation_get_current_package_path_rtc(storage, rtcpath)) {
                break;
            }
            path_concat(furi_string_get_cstr(rtcpath), UPDATE_MANIFEST_DEFAULT_NAME, out_path);
        } while(0);
        furi_string_free(rtcpath);
    }
    return !furi_string_empty(out_path);
}

static bool update_operation_persist_manifest_path(Storage* storage, const char* manifest_path) {
    const size_t manifest_path_len = strlen(manifest_path);
    furi_check(manifest_path && manifest_path_len);
    bool success = false;
    File* file = storage_file_alloc(storage);
    do {
        if(manifest_path_len >= UPDATE_OPERATION_MAX_MANIFEST_PATH_LEN) {
            break;
        }

        if(!storage_file_open(file, UPDATE_FILE_POINTER_FN, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            break;
        }

        if(storage_file_write(file, manifest_path, manifest_path_len) != manifest_path_len) {
            break;
        }

        success = true;
    } while(0);
    storage_file_free(file);
    return success;
}

UpdatePrepareResult update_operation_prepare(const char* manifest_file_path) {
    UpdatePrepareResult result = UpdatePrepareResultIntFull;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    UpdateManifest* manifest = update_manifest_alloc();
    File* file = storage_file_alloc(storage);

    uint64_t free_int_space;
    FuriString* stage_path = furi_string_alloc();
    FuriString* manifest_path_check = furi_string_alloc();
    do {
        if((storage_common_fs_info(storage, STORAGE_INT_PATH_PREFIX, NULL, &free_int_space) !=
            FSE_OK) ||
           (free_int_space < UPDATE_MIN_INT_FREE_SPACE)) {
            break;
        }

        if(storage_common_stat(storage, manifest_file_path, NULL) != FSE_OK) {
            result = UpdatePrepareResultManifestFolderNotFound;
            break;
        }

        if(!update_manifest_init(manifest, manifest_file_path)) {
            result = UpdatePrepareResultManifestInvalid;
            break;
        }

        if(manifest->manifest_version < UPDATE_OPERATION_MIN_MANIFEST_VERSION) {
            result = UpdatePrepareResultOutdatedManifestVersion;
            break;
        }
        /* Only compare hardware target if it is set - pre-production devices accept any firmware*/
        if(furi_hal_version_get_hw_target() &&
           (furi_hal_version_get_hw_target() != manifest->target)) {
            result = UpdatePrepareResultTargetMismatch;
            break;
        }

        path_extract_dirname(manifest_file_path, stage_path);
        path_append(stage_path, furi_string_get_cstr(manifest->staged_loader_file));

        if(!storage_file_open(
               file, furi_string_get_cstr(stage_path), FSAM_READ, FSOM_OPEN_EXISTING)) {
            result = UpdatePrepareResultStageMissing;
            break;
        }

        uint32_t crc = crc32_calc_file(file, NULL, NULL);
        if(crc != manifest->staged_loader_crc) {
            result = UpdatePrepareResultStageIntegrityError;
            break;
        }

        if(!update_operation_persist_manifest_path(storage, manifest_file_path)) {
            result = UpdatePrepareResultManifestPointerCreateError;
            break;
        }

        if(!update_operation_get_current_package_manifest_path(storage, manifest_path_check) ||
           (furi_string_cmpi_str(manifest_path_check, manifest_file_path) != 0)) {
            FURI_LOG_E(
                "update",
                "Manifest pointer check failed: '%s' != '%s'",
                furi_string_get_cstr(manifest_path_check),
                manifest_file_path);
            result = UpdatePrepareResultManifestPointerCheckError;
            break;
        }

        result = UpdatePrepareResultOK;
        furi_hal_rtc_set_boot_mode(FuriHalRtcBootModePreUpdate);
    } while(false);

    furi_string_free(stage_path);
    furi_string_free(manifest_path_check);
    storage_file_free(file);

    update_manifest_free(manifest);
    furi_record_close(RECORD_STORAGE);

    return result;
}

bool update_operation_is_armed(void) {
    FuriHalRtcBootMode boot_mode = furi_hal_rtc_get_boot_mode();
    const uint32_t rtc_upd_index =
        furi_hal_rtc_get_register(FuriHalRtcRegisterUpdateFolderFSIndex);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    const bool upd_fn_ptr_exists =
        (storage_common_stat(storage, UPDATE_FILE_POINTER_FN, NULL) == FSE_OK);
    furi_record_close(RECORD_STORAGE);
    return (boot_mode >= FuriHalRtcBootModePreUpdate) &&
           (boot_mode <= FuriHalRtcBootModePostUpdate) &&
           ((rtc_upd_index != INT_MAX) || upd_fn_ptr_exists);
}

void update_operation_disarm(void) {
    furi_hal_rtc_set_boot_mode(FuriHalRtcBootModeNormal);
    furi_hal_rtc_set_register(FuriHalRtcRegisterUpdateFolderFSIndex, INT_MAX);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_simply_remove(storage, UPDATE_FILE_POINTER_FN);
    furi_record_close(RECORD_STORAGE);
}
