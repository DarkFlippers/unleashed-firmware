#include "../spi_mem_app_i.h"
#include "../spi_mem_files.h"
#include "../lib/spi/spi_mem_chip.h"
#include "../lib/spi/spi_mem_tools.h"

void spi_mem_scene_verify_view_result_callback(void* context) {
    SPIMemApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, SPIMemCustomEventViewVerifySkip);
}

static void spi_mem_scene_verify_callback(void* context, SPIMemCustomEventWorker event) {
    SPIMemApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void spi_mem_scene_verify_on_enter(void* context) {
    SPIMemApp* app = context;
    spi_mem_view_progress_set_verify_callback(
        app->view_progress, spi_mem_scene_verify_view_result_callback, app);
    notification_message(app->notifications, &sequence_blink_start_cyan);
    spi_mem_view_progress_set_chip_size(app->view_progress, spi_mem_chip_get_size(app->chip_info));
    spi_mem_view_progress_set_file_size(app->view_progress, spi_mem_file_get_size(app));
    spi_mem_view_progress_set_block_size(
        app->view_progress, spi_mem_tools_get_file_max_block_size(app->chip_info));
    view_dispatcher_switch_to_view(app->view_dispatcher, SPIMemViewProgress);
    spi_mem_worker_start_thread(app->worker);
    spi_mem_worker_verify_start(app->chip_info, app->worker, spi_mem_scene_verify_callback, app);
}

bool spi_mem_scene_verify_on_event(void* context, SceneManagerEvent event) {
    SPIMemApp* app = context;
    UNUSED(app);
    bool success = false;
    if(event.type == SceneManagerEventTypeBack) {
        success = true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        success = true;
        if(event.event == SPIMemCustomEventViewVerifySkip) {
            scene_manager_next_scene(app->scene_manager, SPIMemSceneSuccess);
        } else if(event.event == SPIMemCustomEventWorkerBlockReaded) {
            spi_mem_view_progress_inc_progress(app->view_progress);
        } else if(event.event == SPIMemCustomEventWorkerChipFail) {
            scene_manager_next_scene(app->scene_manager, SPIMemSceneChipError);
        } else if(event.event == SPIMemCustomEventWorkerFileFail) {
            scene_manager_next_scene(app->scene_manager, SPIMemSceneStorageError);
        } else if(event.event == SPIMemCustomEventWorkerDone) {
            scene_manager_next_scene(app->scene_manager, SPIMemSceneSuccess);
        } else if(event.event == SPIMemCustomEventWorkerVerifyFail) {
            scene_manager_next_scene(app->scene_manager, SPIMemSceneVerifyError);
        }
    }
    return success;
}
void spi_mem_scene_verify_on_exit(void* context) {
    SPIMemApp* app = context;
    spi_mem_worker_stop_thread(app->worker);
    spi_mem_view_progress_reset(app->view_progress);
    notification_message(app->notifications, &sequence_blink_stop);
}
