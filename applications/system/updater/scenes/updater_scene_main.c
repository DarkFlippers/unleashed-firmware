#include <furi.h>
#include <furi_hal.h>
#include <applications.h>
#include <storage/storage.h>

#include "../updater_i.h"
#include "../views/updater_main.h"
#include "updater_scene.h"

static void sd_mount_callback(const void* message, void* context) {
    Updater* updater = context;
    const StorageEvent* new_event = message;

    switch(new_event->type) {
    case StorageEventTypeCardMount:
        view_dispatcher_send_custom_event(updater->view_dispatcher, UpdaterCustomEventStartUpdate);
        break;
    case StorageEventTypeCardUnmount:
        view_dispatcher_send_custom_event(updater->view_dispatcher, UpdaterCustomEventSdUnmounted);
        break;
    default:
        break;
    }
}

void updater_scene_main_on_enter(void* context) {
    Updater* updater = (Updater*)context;
    notification_message(updater->notification, &sequence_display_backlight_enforce_on);
    UpdaterMainView* main_view = updater->main_view;

    FuriPubSubSubscription* sub =
        furi_pubsub_subscribe(storage_get_pubsub(updater->storage), &sd_mount_callback, updater);
    updater_main_set_storage_pubsub(main_view, sub);

    /* FIXME: there's a misbehavior in storage subsystem. If we produce heavy load on it before it
    * fires an SD card event, it'll never do that until the load is lifted. Meanwhile SD card icon
    * will be missing from UI, however, /ext will be fully operational. So, until it's fixed, this
    * should remain commented out. */
    // If (somehow) we started after SD card is mounted, initiate update immediately
    if(storage_sd_status(updater->storage) == FSE_OK) {
        view_dispatcher_send_custom_event(updater->view_dispatcher, UpdaterCustomEventStartUpdate);
    }

    updater_main_set_view_dispatcher(main_view, updater->view_dispatcher);
    view_dispatcher_switch_to_view(updater->view_dispatcher, UpdaterViewMain);
}

static void updater_scene_cancel_update(void) {
    update_operation_disarm();
    furi_hal_power_reset();
}

bool updater_scene_main_on_event(void* context, SceneManagerEvent event) {
    Updater* updater = (Updater*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeTick) {
        if(!update_task_is_running(updater->update_task)) {
            if(updater->idle_ticks++ >= (UPDATE_DELAY_OPERATION_ERROR / UPDATER_APP_TICK)) {
                updater_scene_cancel_update();
            }
        } else {
            updater->idle_ticks = 0;
        }
    } else if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case UpdaterCustomEventStartUpdate:
        case UpdaterCustomEventRetryUpdate:
            if(!update_task_is_running(updater->update_task) &&
               (update_task_get_state(updater->update_task)->stage != UpdateTaskStageCompleted))
                update_task_start(updater->update_task);
            consumed = true;
            break;

        case UpdaterCustomEventCancelUpdate:
            if(!update_task_is_running(updater->update_task)) {
                updater_scene_cancel_update();
            }
            consumed = true;
            break;

        case UpdaterCustomEventSdUnmounted:
            // TODO FL-3499: error out, stop worker (it's probably dead actually)
            break;
        default:
            break;
        }
    }

    return consumed;
}

void updater_scene_main_on_exit(void* context) {
    Updater* updater = (Updater*)context;

    notification_message(updater->notification, &sequence_display_backlight_enforce_auto);
    furi_pubsub_unsubscribe(
        storage_get_pubsub(updater->storage), updater_main_get_storage_pubsub(updater->main_view));

    scene_manager_set_scene_state(updater->scene_manager, UpdaterSceneMain, 0);
}
