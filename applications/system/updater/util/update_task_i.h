#pragma once

#include <storage/storage.h>
#include <furi_hal.h>

#define UPDATE_TASK_NOERR 0
#define UPDATE_TASK_FAILED -1

typedef struct UpdateTask {
    UpdateTaskState state;
    string_t update_path;
    UpdateManifest* manifest;
    FuriThread* thread;
    Storage* storage;
    File* file;
    updateProgressCb status_change_cb;
    void* status_change_cb_state;
    FuriHalRtcBootMode boot_mode;
} UpdateTask;

void update_task_set_progress(UpdateTask* update_task, UpdateTaskStage stage, uint8_t progress);
bool update_task_parse_manifest(UpdateTask* update_task);
bool update_task_open_file(UpdateTask* update_task, string_t filename);

int32_t update_task_worker_flash_writer(void* context);
int32_t update_task_worker_backup_restore(void* context);
