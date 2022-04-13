#include "update_task.h"
#include "update_task_i.h"

#include <furi.h>
#include <furi_hal.h>
#include <storage/storage.h>
#include <toolbox/path.h>
#include <update_util/dfu_file.h>
#include <update_util/lfs_backup.h>
#include <update_util/update_operation.h>

static const char* update_task_stage_descr[] = {
    [UpdateTaskStageProgress] = "...",
    [UpdateTaskStageReadManifest] = "Loading update manifest",
    [UpdateTaskStageValidateDFUImage] = "Checking DFU file",
    [UpdateTaskStageFlashWrite] = "Writing flash",
    [UpdateTaskStageFlashValidate] = "Validating",
    [UpdateTaskStageRadioWrite] = "Writing radio stack",
    [UpdateTaskStageRadioCommit] = "Applying radio stack",
    [UpdateTaskStageLfsBackup] = "Backing up LFS",
    [UpdateTaskStageLfsRestore] = "Restoring LFS",
    [UpdateTaskStageComplete] = "Complete",
    [UpdateTaskStageError] = "Error",
};

static void update_task_set_status(UpdateTask* update_task, const char* status) {
    if(!status) {
        if(update_task->state.stage >= COUNT_OF(update_task_stage_descr)) {
            status = "...";
        } else {
            status = update_task_stage_descr[update_task->state.stage];
        }
    }
    string_set_str(update_task->state.status, status);
}

void update_task_set_progress(UpdateTask* update_task, UpdateTaskStage stage, uint8_t progress) {
    if(stage != UpdateTaskStageProgress) {
        update_task->state.stage = stage;
        update_task->state.current_stage_idx++;
        update_task_set_status(update_task, NULL);
    }

    if(progress > 100) {
        progress = 100;
    }

    update_task->state.progress = progress;
    if(update_task->status_change_cb) {
        (update_task->status_change_cb)(
            string_get_cstr(update_task->state.status),
            progress,
            update_task->state.current_stage_idx,
            update_task->state.total_stages,
            update_task->state.stage == UpdateTaskStageError,
            update_task->status_change_cb_state);
    }
}

static void update_task_close_file(UpdateTask* update_task) {
    furi_assert(update_task);
    if(!storage_file_is_open(update_task->file)) {
        return;
    }

    storage_file_close(update_task->file);
}

static bool update_task_check_file_exists(UpdateTask* update_task, string_t filename) {
    furi_assert(update_task);
    string_t tmp_path;
    string_init_set(tmp_path, update_task->update_path);
    path_append(tmp_path, string_get_cstr(filename));
    bool exists =
        (storage_common_stat(update_task->storage, string_get_cstr(tmp_path), NULL) == FSE_OK);
    string_clear(tmp_path);
    return exists;
}

bool update_task_open_file(UpdateTask* update_task, string_t filename) {
    furi_assert(update_task);
    update_task_close_file(update_task);

    string_t tmp_path;
    string_init_set(tmp_path, update_task->update_path);
    path_append(tmp_path, string_get_cstr(filename));
    bool open_success = storage_file_open(
        update_task->file, string_get_cstr(tmp_path), FSAM_READ, FSOM_OPEN_EXISTING);
    string_clear(tmp_path);
    return open_success;
}

static void update_task_worker_thread_cb(FuriThreadState state, void* context) {
    UpdateTask* update_task = context;

    if(state != FuriThreadStateStopped) {
        return;
    }

    int32_t op_result = furi_thread_get_return_code(update_task->thread);
    if(op_result == UPDATE_TASK_NOERR) {
        osDelay(UPDATE_DELAY_OPERATION_OK);
        furi_hal_power_reset();
    }
}

UpdateTask* update_task_alloc() {
    UpdateTask* update_task = malloc(sizeof(UpdateTask));

    update_task->state.stage = UpdateTaskStageProgress;
    update_task->state.progress = 0;
    string_init(update_task->state.status);

    update_task->manifest = update_manifest_alloc();
    update_task->storage = furi_record_open("storage");
    update_task->file = storage_file_alloc(update_task->storage);
    update_task->status_change_cb = NULL;

    FuriThread* thread = update_task->thread = furi_thread_alloc();

    furi_thread_set_name(thread, "UpdateWorker");
    furi_thread_set_stack_size(thread, 5120);
    furi_thread_set_context(thread, update_task);

    furi_thread_set_state_callback(thread, update_task_worker_thread_cb);
    furi_thread_set_state_context(thread, update_task);
#ifdef FURI_RAM_EXEC
    UNUSED(update_task_worker_backup_restore);
    furi_thread_set_callback(thread, update_task_worker_flash_writer);
#else
    UNUSED(update_task_worker_flash_writer);
    furi_thread_set_callback(thread, update_task_worker_backup_restore);
#endif

    return update_task;
}

void update_task_free(UpdateTask* update_task) {
    furi_assert(update_task);

    furi_thread_join(update_task->thread);

    furi_thread_free(update_task->thread);
    update_task_close_file(update_task);
    storage_file_free(update_task->file);
    update_manifest_free(update_task->manifest);

    furi_record_close("storage");
    string_clear(update_task->update_path);

    free(update_task);
}

bool update_task_init(UpdateTask* update_task) {
    furi_assert(update_task);
    string_init(update_task->update_path);
    return true;
}

bool update_task_parse_manifest(UpdateTask* update_task) {
    furi_assert(update_task);
    update_task_set_progress(update_task, UpdateTaskStageReadManifest, 0);
    bool result = false;
    string_t manifest_path;
    string_init(manifest_path);

    do {
        update_task_set_progress(update_task, UpdateTaskStageProgress, 10);
        if(!update_operation_get_current_package_path(
               update_task->storage, update_task->update_path)) {
            break;
        }

        path_concat(
            string_get_cstr(update_task->update_path),
            UPDATE_MANIFEST_DEFAULT_NAME,
            manifest_path);
        update_task_set_progress(update_task, UpdateTaskStageProgress, 30);
        if(!update_manifest_init(update_task->manifest, string_get_cstr(manifest_path))) {
            break;
        }

        update_task_set_progress(update_task, UpdateTaskStageProgress, 50);
        if(!string_empty_p(update_task->manifest->firmware_dfu_image) &&
           !update_task_check_file_exists(update_task, update_task->manifest->firmware_dfu_image)) {
            break;
        }

        update_task_set_progress(update_task, UpdateTaskStageProgress, 70);
        if(!string_empty_p(update_task->manifest->radio_image) &&
           !update_task_check_file_exists(update_task, update_task->manifest->radio_image)) {
            break;
        }

        update_task_set_progress(update_task, UpdateTaskStageProgress, 100);
        result = true;
    } while(false);

    string_clear(manifest_path);
    return result;
}

void update_task_set_progress_cb(UpdateTask* update_task, updateProgressCb cb, void* state) {
    update_task->status_change_cb = cb;
    update_task->status_change_cb_state = state;
}

bool update_task_start(UpdateTask* update_task) {
    furi_assert(update_task);
    return furi_thread_start(update_task->thread);
}

bool update_task_is_running(UpdateTask* update_task) {
    furi_assert(update_task);
    return furi_thread_get_state(update_task->thread) == FuriThreadStateRunning;
}

UpdateTaskState const* update_task_get_state(UpdateTask* update_task) {
    furi_assert(update_task);
    return &update_task->state;
}

UpdateManifest const* update_task_get_manifest(UpdateTask* update_task) {
    furi_assert(update_task);
    return update_task->manifest;
}
