#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <update_util/update_manifest.h>

#include <stdint.h>
#include <stdbool.h>
#include <m-string.h>

#define UPDATE_DELAY_OPERATION_OK 10
#define UPDATE_DELAY_OPERATION_ERROR INT_MAX

typedef enum {
    UpdateTaskStageProgress = 0,

    UpdateTaskStageReadManifest,
    UpdateTaskStageLfsBackup,

    UpdateTaskStageRadioImageValidate,
    UpdateTaskStageRadioErase,
    UpdateTaskStageRadioWrite,
    UpdateTaskStageRadioInstall,
    UpdateTaskStageRadioBusy,

    UpdateTaskStageOBValidation,

    UpdateTaskStageValidateDFUImage,
    UpdateTaskStageFlashWrite,
    UpdateTaskStageFlashValidate,

    UpdateTaskStageLfsRestore,
    UpdateTaskStageResourcesUpdate,
    UpdateTaskStageSplashscreenInstall,

    UpdateTaskStageCompleted,
    UpdateTaskStageError,
    UpdateTaskStageOBError,
    UpdateTaskStageMAX
} UpdateTaskStage;

inline bool update_stage_is_error(const UpdateTaskStage stage) {
    return stage >= UpdateTaskStageError;
}

typedef enum {
    UpdateTaskStageGroupMisc = 0,
    UpdateTaskStageGroupPreUpdate = 1 << 1,
    UpdateTaskStageGroupFirmware = 1 << 2,
    UpdateTaskStageGroupOptionBytes = 1 << 3,
    UpdateTaskStageGroupRadio = 1 << 4,
    UpdateTaskStageGroupPostUpdate = 1 << 5,
    UpdateTaskStageGroupResources = 1 << 6,
    UpdateTaskStageGroupSplashscreen = 1 << 7,
} UpdateTaskStageGroup;

typedef struct {
    UpdateTaskStage stage;
    uint8_t overall_progress, stage_progress;
    string_t status;
    UpdateTaskStageGroup groups;
    uint32_t total_progress_points;
    uint32_t completed_stages_points;
} UpdateTaskState;

typedef struct UpdateTask UpdateTask;

typedef void (
    *updateProgressCb)(const char* status, const uint8_t stage_pct, bool failed, void* state);

UpdateTask* update_task_alloc();

void update_task_free(UpdateTask* update_task);

void update_task_set_progress_cb(UpdateTask* update_task, updateProgressCb cb, void* state);

void update_task_start(UpdateTask* update_task);

bool update_task_is_running(UpdateTask* update_task);

UpdateTaskState const* update_task_get_state(UpdateTask* update_task);

UpdateManifest const* update_task_get_manifest(UpdateTask* update_task);

#ifdef __cplusplus
}
#endif
