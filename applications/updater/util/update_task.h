#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <update_util/update_manifest.h>

#include <stdint.h>
#include <stdbool.h>
#include <m-string.h>

#define UPDATE_DELAY_OPERATION_OK 600
#define UPDATE_DELAY_OPERATION_ERROR INT_MAX

typedef enum {
    UpdateTaskStageProgress,
    UpdateTaskStageReadManifest,
    UpdateTaskStageValidateDFUImage,
    UpdateTaskStageFlashWrite,
    UpdateTaskStageFlashValidate,
    UpdateTaskStageRadioWrite,
    UpdateTaskStageRadioCommit,
    UpdateTaskStageLfsBackup,
    UpdateTaskStageLfsRestore,
    UpdateTaskStageComplete,
    UpdateTaskStageError,
} UpdateTaskStage;

typedef struct {
    UpdateTaskStage stage;
    uint8_t progress;
    uint8_t current_stage_idx;
    uint8_t total_stages;
    string_t status;
} UpdateTaskState;

typedef struct UpdateTask UpdateTask;

typedef void (*updateProgressCb)(
    const char* status,
    const uint8_t stage_pct,
    const uint8_t idx_stage,
    const uint8_t total_stages,
    bool failed,
    void* state);

UpdateTask* update_task_alloc();

void update_task_free(UpdateTask* update_task);

bool update_task_init(UpdateTask* update_task);

void update_task_set_progress_cb(UpdateTask* update_task, updateProgressCb cb, void* state);

bool update_task_start(UpdateTask* update_task);

bool update_task_is_running(UpdateTask* update_task);

UpdateTaskState const* update_task_get_state(UpdateTask* update_task);

UpdateManifest const* update_task_get_manifest(UpdateTask* update_task);

#ifdef __cplusplus
}
#endif