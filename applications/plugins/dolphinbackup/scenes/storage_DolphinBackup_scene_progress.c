#include "../storage_DolphinBackup.h"

void storage_DolphinBackup_scene_progress_on_enter(void* context) {
    StorageDolphinBackup* app = context;

    widget_add_string_element(
        app->widget, 64, 10, AlignCenter, AlignCenter, FontPrimary, "Moving...");

    view_dispatcher_switch_to_view(app->view_dispatcher, StorageDolphinBackupViewWidget);

    storage_DolphinBackup_perform();
    view_dispatcher_send_custom_event(app->view_dispatcher, DolphinBackupCustomEventExit);
}

bool storage_DolphinBackup_scene_progress_on_event(void* context, SceneManagerEvent event) {
    StorageDolphinBackup* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        view_dispatcher_stop(app->view_dispatcher);
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = true;
    }

    return consumed;
}

void storage_DolphinBackup_scene_progress_on_exit(void* context) {
    StorageDolphinBackup* app = context;
    widget_reset(app->widget);
}
