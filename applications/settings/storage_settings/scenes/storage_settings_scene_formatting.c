#include "../storage_settings.h"
#include <notification/notification.h>
#include <notification/notification_messages.h>

static const NotificationMessage message_green_165 = {
    .type = NotificationMessageTypeLedGreen,
    .data.led.value = 165,
};

static const NotificationSequence sequence_set_formatting_leds = {
    &message_red_255,
    &message_green_165,
    &message_blue_0,
    &message_do_not_reset,
    NULL,
};

static const NotificationSequence sequence_reset_formatting_leds = {
    &message_red_0,
    &message_green_0,
    NULL,
};

static void
    storage_settings_scene_formatting_dialog_callback(DialogExResult result, void* context) {
    StorageSettings* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, result);
}

void storage_settings_scene_formatting_on_enter(void* context) {
    StorageSettings* app = context;
    FS_Error error;
    DialogEx* dialog_ex = app->dialog_ex;

    dialog_ex_set_header(dialog_ex, "Formatting...", 70, 32, AlignCenter, AlignCenter);
    dialog_ex_set_icon(dialog_ex, 15, 20, &I_LoadingHourglass_24x24);
    view_dispatcher_switch_to_view(app->view_dispatcher, StorageSettingsViewDialogEx);

    notification_message_block(app->notification, &sequence_set_formatting_leds);
    error = storage_sd_format(app->fs_api);
    notification_message(app->notification, &sequence_reset_formatting_leds);
    notification_message(app->notification, &sequence_blink_green_100);

    dialog_ex_set_context(dialog_ex, app);
    dialog_ex_set_result_callback(dialog_ex, storage_settings_scene_formatting_dialog_callback);

    if(error != FSE_OK) {
        dialog_ex_set_header(dialog_ex, "Cannot Format SD Card", 64, 10, AlignCenter, AlignCenter);
        dialog_ex_set_icon(dialog_ex, 0, 0, NULL);
        dialog_ex_set_text(
            dialog_ex, storage_error_get_desc(error), 64, 32, AlignCenter, AlignCenter);
    } else {
        dialog_ex_set_icon(dialog_ex, 48, 6, &I_DolphinDone_80x58);
        dialog_ex_set_header(dialog_ex, "Formatted", 5, 10, AlignLeft, AlignTop);
        NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
        notification_message(notification, &sequence_single_vibro);
        notification_message(notification, &sequence_set_green_255);
        notification_message(notification, &sequence_success);
        furi_record_close(RECORD_NOTIFICATION);
    }
    dialog_ex_set_left_button_text(dialog_ex, "Finish");
}

bool storage_settings_scene_formatting_on_event(void* context, SceneManagerEvent event) {
    StorageSettings* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DialogExResultLeft:
            consumed = scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, StorageSettingsStart);
            break;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = true;
    }

    return consumed;
}

void storage_settings_scene_formatting_on_exit(void* context) {
    StorageSettings* app = context;
    DialogEx* dialog_ex = app->dialog_ex;

    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
    notification_message(notification, &sequence_reset_green);
    furi_record_close(RECORD_NOTIFICATION);

    dialog_ex_reset(dialog_ex);
}
