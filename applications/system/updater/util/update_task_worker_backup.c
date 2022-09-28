#include "update_task.h"
#include "update_task_i.h"

#include <furi.h>
#include <furi_hal.h>
#include <storage/storage.h>
#include <desktop/helpers/slideshow_filename.h>
#include <toolbox/path.h>
#include <update_util/dfu_file.h>
#include <update_util/lfs_backup.h>
#include <update_util/update_operation.h>
#include <update_util/resources/manifest.h>
#include <toolbox/tar/tar_archive.h>
#include <toolbox/crc32_calc.h>

#define TAG "UpdWorkerBackup"

#define CHECK_RESULT(x) \
    if(!(x)) {          \
        break;          \
    }

static bool update_task_pre_update(UpdateTask* update_task) {
    bool success = false;
    string_t backup_file_path;
    string_init(backup_file_path);
    path_concat(
        string_get_cstr(update_task->update_path), LFS_BACKUP_DEFAULT_FILENAME, backup_file_path);

    update_task_set_progress(update_task, UpdateTaskStageLfsBackup, 0);
    /* to avoid bootloops */
    furi_hal_rtc_set_boot_mode(FuriHalRtcBootModeNormal);
    if((success = lfs_backup_create(update_task->storage, string_get_cstr(backup_file_path)))) {
        furi_hal_rtc_set_boot_mode(FuriHalRtcBootModeUpdate);
    }

    string_clear(backup_file_path);
    return success;
}

typedef struct {
    UpdateTask* update_task;
    int32_t total_files, processed_files;
} TarUnpackProgress;

static bool update_task_resource_unpack_cb(const char* name, bool is_directory, void* context) {
    UNUSED(name);
    UNUSED(is_directory);
    TarUnpackProgress* unpack_progress = context;
    unpack_progress->processed_files++;
    update_task_set_progress(
        unpack_progress->update_task,
        UpdateTaskStageProgress,
        /* For this stage, last 70% of progress = extraction */
        30 + (unpack_progress->processed_files * 70) / (unpack_progress->total_files + 1));
    return true;
}

static void
    update_task_cleanup_resources(UpdateTask* update_task, uint32_t n_approx_file_entries) {
    ResourceManifestReader* manifest_reader = resource_manifest_reader_alloc(update_task->storage);
    do {
        FURI_LOG_I(TAG, "Cleaning up old manifest");
        if(!resource_manifest_reader_open(manifest_reader, EXT_PATH("Manifest"))) {
            FURI_LOG_W(TAG, "No existing manifest");
            break;
        }

        /* We got # of entries in TAR file. Approx 1/4th is dir entries, we skip them */
        n_approx_file_entries = n_approx_file_entries * 3 / 4 + 1;
        uint32_t n_processed_files = 0;

        ResourceManifestEntry* entry_ptr = NULL;
        while((entry_ptr = resource_manifest_reader_next(manifest_reader))) {
            if(entry_ptr->type == ResourceManifestEntryTypeFile) {
                update_task_set_progress(
                    update_task,
                    UpdateTaskStageProgress,
                    /* For this stage, first 30% of progress = cleanup */
                    (n_processed_files++ * 30) / (n_approx_file_entries + 1));

                string_t file_path;
                string_init(file_path);
                path_concat(STORAGE_EXT_PATH_PREFIX, string_get_cstr(entry_ptr->name), file_path);
                FURI_LOG_D(TAG, "Removing %s", string_get_cstr(file_path));
                storage_simply_remove(update_task->storage, string_get_cstr(file_path));
                string_clear(file_path);
            }
        }
    } while(false);
    resource_manifest_reader_free(manifest_reader);
}

static bool update_task_post_update(UpdateTask* update_task) {
    bool success = false;

    string_t file_path;
    string_init(file_path);

    TarArchive* archive = tar_archive_alloc(update_task->storage);
    do {
        path_concat(
            string_get_cstr(update_task->update_path), LFS_BACKUP_DEFAULT_FILENAME, file_path);

        update_task_set_progress(update_task, UpdateTaskStageLfsRestore, 0);

        CHECK_RESULT(lfs_backup_unpack(update_task->storage, string_get_cstr(file_path)));

        if(update_task->state.groups & UpdateTaskStageGroupResources) {
            TarUnpackProgress progress = {
                .update_task = update_task,
                .total_files = 0,
                .processed_files = 0,
            };
            update_task_set_progress(update_task, UpdateTaskStageResourcesUpdate, 0);

            path_concat(
                string_get_cstr(update_task->update_path),
                string_get_cstr(update_task->manifest->resource_bundle),
                file_path);

            tar_archive_set_file_callback(archive, update_task_resource_unpack_cb, &progress);
            CHECK_RESULT(
                tar_archive_open(archive, string_get_cstr(file_path), TAR_OPEN_MODE_READ));

            progress.total_files = tar_archive_get_entries_count(archive);
            if(progress.total_files > 0) {
                update_task_cleanup_resources(update_task, progress.total_files);

                CHECK_RESULT(tar_archive_unpack_to(archive, STORAGE_EXT_PATH_PREFIX, NULL));
            }
        }

        if(update_task->state.groups & UpdateTaskStageGroupSplashscreen) {
            update_task_set_progress(update_task, UpdateTaskStageSplashscreenInstall, 0);
            string_t tmp_path;
            string_init_set(tmp_path, update_task->update_path);
            path_append(tmp_path, string_get_cstr(update_task->manifest->splash_file));
            if(storage_common_copy(
                   update_task->storage,
                   string_get_cstr(tmp_path),
                   INT_PATH(SLIDESHOW_FILE_NAME)) != FSE_OK) {
                // actually, not critical
            }
            string_clear(tmp_path);
            update_task_set_progress(update_task, UpdateTaskStageSplashscreenInstall, 100);
        }
        success = true;
    } while(false);

    tar_archive_free(archive);
    string_clear(file_path);
    return success;
}

int32_t update_task_worker_backup_restore(void* context) {
    furi_assert(context);
    UpdateTask* update_task = context;

    FuriHalRtcBootMode boot_mode = update_task->boot_mode;
    if((boot_mode != FuriHalRtcBootModePreUpdate) && (boot_mode != FuriHalRtcBootModePostUpdate)) {
        /* no idea how we got here. Do nothing */
        return UPDATE_TASK_NOERR;
    }

    bool success = false;
    do {
        if(!update_task_parse_manifest(update_task)) {
            break;
        }

        if(boot_mode == FuriHalRtcBootModePreUpdate) {
            success = update_task_pre_update(update_task);
        } else if(boot_mode == FuriHalRtcBootModePostUpdate) {
            success = update_task_post_update(update_task);
            if(success) {
                update_operation_disarm();
            }
        }
    } while(false);

    if(!success) {
        update_task_set_progress(update_task, UpdateTaskStageError, 0);
        return UPDATE_TASK_FAILED;
    }

    update_task_set_progress(update_task, UpdateTaskStageCompleted, 100);
    return UPDATE_TASK_NOERR;
}
