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
    [UpdateTaskStageFlashValidate] = "Validating flash",
    [UpdateTaskStageRadioImageValidate] = "Checking radio FW",
    [UpdateTaskStageRadioErase] = "Uninstalling radio FW",
    [UpdateTaskStageRadioWrite] = "Writing radio FW",
    [UpdateTaskStageRadioInstall] = "Installing radio FW",
    [UpdateTaskStageRadioBusy] = "Radio is updating",
    [UpdateTaskStageOBValidation] = "Validating opt. bytes",
    [UpdateTaskStageLfsBackup] = "Backing up LFS",
    [UpdateTaskStageLfsRestore] = "Restoring LFS",
    [UpdateTaskStageResourcesUpdate] = "Updating resources",
    [UpdateTaskStageSplashscreenInstall] = "Installing splashscreen",
    [UpdateTaskStageCompleted] = "Restarting...",
    [UpdateTaskStageError] = "Error",
    [UpdateTaskStageOBError] = "OB, report",
};

typedef struct {
    UpdateTaskStageGroup group;
    uint8_t weight;
} UpdateTaskStageGroupMap;

#define STAGE_DEF(GROUP, WEIGHT) \
    { .group = (GROUP), .weight = (WEIGHT), }

static const UpdateTaskStageGroupMap update_task_stage_progress[] = {
    [UpdateTaskStageProgress] = STAGE_DEF(UpdateTaskStageGroupMisc, 0),

    [UpdateTaskStageReadManifest] = STAGE_DEF(UpdateTaskStageGroupPreUpdate, 5),
    [UpdateTaskStageLfsBackup] = STAGE_DEF(UpdateTaskStageGroupPreUpdate, 15),

    [UpdateTaskStageRadioImageValidate] = STAGE_DEF(UpdateTaskStageGroupRadio, 15),
    [UpdateTaskStageRadioErase] = STAGE_DEF(UpdateTaskStageGroupRadio, 60),
    [UpdateTaskStageRadioWrite] = STAGE_DEF(UpdateTaskStageGroupRadio, 80),
    [UpdateTaskStageRadioInstall] = STAGE_DEF(UpdateTaskStageGroupRadio, 60),
    [UpdateTaskStageRadioBusy] = STAGE_DEF(UpdateTaskStageGroupRadio, 80),

    [UpdateTaskStageOBValidation] = STAGE_DEF(UpdateTaskStageGroupOptionBytes, 10),

    [UpdateTaskStageValidateDFUImage] = STAGE_DEF(UpdateTaskStageGroupFirmware, 50),
    [UpdateTaskStageFlashWrite] = STAGE_DEF(UpdateTaskStageGroupFirmware, 200),
    [UpdateTaskStageFlashValidate] = STAGE_DEF(UpdateTaskStageGroupFirmware, 30),

    [UpdateTaskStageLfsRestore] = STAGE_DEF(UpdateTaskStageGroupPostUpdate, 30),

    [UpdateTaskStageResourcesUpdate] = STAGE_DEF(UpdateTaskStageGroupResources, 255),
    [UpdateTaskStageSplashscreenInstall] = STAGE_DEF(UpdateTaskStageGroupSplashscreen, 5),

    [UpdateTaskStageCompleted] = STAGE_DEF(UpdateTaskStageGroupMisc, 1),
    [UpdateTaskStageError] = STAGE_DEF(UpdateTaskStageGroupMisc, 1),
    [UpdateTaskStageOBError] = STAGE_DEF(UpdateTaskStageGroupMisc, 1),
};

static UpdateTaskStageGroup update_task_get_task_groups(UpdateTask* update_task) {
    UpdateTaskStageGroup ret = UpdateTaskStageGroupPreUpdate | UpdateTaskStageGroupPostUpdate;
    UpdateManifest* manifest = update_task->manifest;
    if(!string_empty_p(manifest->radio_image)) {
        ret |= UpdateTaskStageGroupRadio;
    }
    if(update_manifest_has_obdata(manifest)) {
        ret |= UpdateTaskStageGroupOptionBytes;
    }
    if(!string_empty_p(manifest->firmware_dfu_image)) {
        ret |= UpdateTaskStageGroupFirmware;
    }
    if(!string_empty_p(manifest->resource_bundle)) {
        ret |= UpdateTaskStageGroupResources;
    }
    if(!string_empty_p(manifest->splash_file)) {
        ret |= UpdateTaskStageGroupSplashscreen;
    }
    return ret;
}

static void update_task_calc_completed_stages(UpdateTask* update_task) {
    uint32_t completed_stages_points = 0;
    for(UpdateTaskStage past_stage = UpdateTaskStageProgress;
        past_stage < update_task->state.stage;
        ++past_stage) {
        const UpdateTaskStageGroupMap* grp_descr = &update_task_stage_progress[past_stage];
        if((grp_descr->group & update_task->state.groups) == 0) {
            continue;
        }
        completed_stages_points += grp_descr->weight;
    }
    update_task->state.completed_stages_points = completed_stages_points;
}

void update_task_set_progress(UpdateTask* update_task, UpdateTaskStage stage, uint8_t progress) {
    if(stage != UpdateTaskStageProgress) {
        /* do not override more specific error states */
        if((stage >= UpdateTaskStageError) && (update_task->state.stage >= UpdateTaskStageError)) {
            return;
        }
        /* Build error message with code "[stage_idx-stage_percent]" */
        if(stage >= UpdateTaskStageError) {
            string_printf(
                update_task->state.status,
                "%s #[%d-%d]",
                update_task_stage_descr[stage],
                update_task->state.stage,
                update_task->state.stage_progress);
        } else {
            string_set_str(update_task->state.status, update_task_stage_descr[stage]);
        }
        /* Store stage update */
        update_task->state.stage = stage;
        /* If we are still alive, sum completed stages weights */
        if((stage > UpdateTaskStageProgress) && (stage < UpdateTaskStageCompleted)) {
            update_task_calc_completed_stages(update_task);
        }
    }

    /* Store stage progress for all non-error updates - to provide details on error state */
    if(!update_stage_is_error(stage)) {
        update_task->state.stage_progress = progress;
    }

    /* Calculate "overall" progress, based on stage weights */
    uint32_t adapted_progress = 1;
    if(update_task->state.total_progress_points != 0) {
        if(stage < UpdateTaskStageCompleted) {
            adapted_progress = MIN(
                (update_task->state.completed_stages_points +
                 (update_task_stage_progress[update_task->state.stage].weight * progress / 100)) *
                    100 / (update_task->state.total_progress_points),
                100u);

        } else {
            adapted_progress = update_task->state.overall_progress;
        }
    }
    update_task->state.overall_progress = adapted_progress;

    if(update_task->status_change_cb) {
        (update_task->status_change_cb)(
            string_get_cstr(update_task->state.status),
            adapted_progress,
            update_stage_is_error(update_task->state.stage),
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
    bool exists = storage_file_exists(update_task->storage, string_get_cstr(tmp_path));
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

    if(furi_thread_get_return_code(update_task->thread) == UPDATE_TASK_NOERR) {
        furi_delay_ms(UPDATE_DELAY_OPERATION_OK);
        furi_hal_power_reset();
    }
}

UpdateTask* update_task_alloc() {
    UpdateTask* update_task = malloc(sizeof(UpdateTask));

    update_task->state.stage = UpdateTaskStageProgress;
    update_task->state.stage_progress = 0;
    update_task->state.overall_progress = 0;
    string_init(update_task->state.status);

    update_task->manifest = update_manifest_alloc();
    update_task->storage = furi_record_open(RECORD_STORAGE);
    update_task->file = storage_file_alloc(update_task->storage);
    update_task->status_change_cb = NULL;
    update_task->boot_mode = furi_hal_rtc_get_boot_mode();
    string_init(update_task->update_path);

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

    furi_record_close(RECORD_STORAGE);
    string_clear(update_task->update_path);

    free(update_task);
}

bool update_task_parse_manifest(UpdateTask* update_task) {
    furi_assert(update_task);
    update_task->state.stage_progress = 0;
    update_task->state.overall_progress = 0;
    update_task->state.total_progress_points = 0;
    update_task->state.completed_stages_points = 0;
    update_task->state.groups = 0;

    update_task_set_progress(update_task, UpdateTaskStageReadManifest, 0);
    bool result = false;
    string_t manifest_path;
    string_init(manifest_path);

    do {
        update_task_set_progress(update_task, UpdateTaskStageProgress, 13);
        if(!furi_hal_version_do_i_belong_here()) {
            break;
        }

        update_task_set_progress(update_task, UpdateTaskStageProgress, 20);
        if(!update_operation_get_current_package_manifest_path(
               update_task->storage, manifest_path)) {
            break;
        }

        path_extract_dirname(string_get_cstr(manifest_path), update_task->update_path);
        update_task_set_progress(update_task, UpdateTaskStageProgress, 30);

        UpdateManifest* manifest = update_task->manifest;
        if(!update_manifest_init(manifest, string_get_cstr(manifest_path))) {
            break;
        }

        update_task_set_progress(update_task, UpdateTaskStageProgress, 40);
        if(manifest->manifest_version < UPDATE_OPERATION_MIN_MANIFEST_VERSION) {
            break;
        }

        update_task_set_progress(update_task, UpdateTaskStageProgress, 50);
        if(manifest->target != furi_hal_version_get_hw_target()) {
            break;
        }

        update_task->state.groups = update_task_get_task_groups(update_task);
        for(size_t stage_counter = 0; stage_counter < COUNT_OF(update_task_stage_progress);
            ++stage_counter) {
            const UpdateTaskStageGroupMap* grp_descr = &update_task_stage_progress[stage_counter];
            if((grp_descr->group & update_task->state.groups) != 0) {
                update_task->state.total_progress_points += grp_descr->weight;
            }
        }

        update_task_set_progress(update_task, UpdateTaskStageProgress, 60);
        if((update_task->state.groups & UpdateTaskStageGroupFirmware) &&
           !update_task_check_file_exists(update_task, manifest->firmware_dfu_image)) {
            break;
        }

        update_task_set_progress(update_task, UpdateTaskStageProgress, 80);
        if((update_task->state.groups & UpdateTaskStageGroupRadio) &&
           (!update_task_check_file_exists(update_task, manifest->radio_image) ||
            (manifest->radio_version.version.type == 0))) {
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

void update_task_start(UpdateTask* update_task) {
    furi_assert(update_task);
    furi_thread_start(update_task->thread);
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
