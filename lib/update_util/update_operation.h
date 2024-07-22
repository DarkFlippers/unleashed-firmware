#pragma once

#include <stdbool.h>
#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UPDATE_OPERATION_ROOT_DIR_PACKAGE_MAGIC 0
#define UPDATE_OPERATION_MAX_MANIFEST_PATH_LEN  255u
#define UPDATE_OPERATION_MIN_MANIFEST_VERSION   2

/* 
 * Checks if supplied full manifest path is valid
 * @param full_path Full path to manifest file. Must be named UPDATE_MANIFEST_DEFAULT_NAME
 * @param out_manifest_dir Directory to apply update from, if supplied path is valid. 
 *   May be empty if update is in root update directory
 * @return bool if supplied path is valid and out_manifest_dir contains dir to apply
 */
bool update_operation_get_package_dir_name(const char* full_path, FuriString* out_manifest_dir);

/* When updating this enum, also update assets/protobuf/system.proto */
typedef enum {
    UpdatePrepareResultOK,
    UpdatePrepareResultManifestPathInvalid,
    UpdatePrepareResultManifestFolderNotFound,
    UpdatePrepareResultManifestInvalid,
    UpdatePrepareResultStageMissing,
    UpdatePrepareResultStageIntegrityError,
    UpdatePrepareResultManifestPointerCreateError,
    UpdatePrepareResultManifestPointerCheckError,
    UpdatePrepareResultTargetMismatch,
    UpdatePrepareResultOutdatedManifestVersion,
    UpdatePrepareResultIntFull,
    UpdatePrepareResultUnspecifiedError,
} UpdatePrepareResult;

const char* update_operation_describe_preparation_result(const UpdatePrepareResult value);

/* 
 * Validates next stage and sets up registers to apply update after restart
 * @param manifest_dir_path Full path to manifest for update to apply 
 * @return UpdatePrepareResult validation & arm result
 */
UpdatePrepareResult update_operation_prepare(const char* manifest_file_path);

/* 
 * Gets filesystem path for current update package
 * @param storage Storage API
 * @param out_path Path to manifest. Must be initialized
 * @return true if path was restored successfully
 */
bool update_operation_get_current_package_manifest_path(Storage* storage, FuriString* out_path);

/* 
 * Checks if an update operation step is pending after reset
 */
bool update_operation_is_armed(void);

/* 
 * Cancels pending update operation
 */
void update_operation_disarm(void);

#ifdef __cplusplus
}
#endif
