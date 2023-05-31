#include "../mifare_nested_i.h"

void mifare_nested_scene_check_keys_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    MifareNested* mifare_nested = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(mifare_nested->view_dispatcher, result);
    }
}

bool mifare_nested_check_keys_worker_callback(MifareNestedWorkerEvent event, void* context) {
    MifareNested* mifare_nested = context;
    CheckKeysState* plugin_state = mifare_nested->keys_state;

    if(event == MifareNestedWorkerEventKeyChecked) {
        mifare_nested_blink_nonce_collection_start(mifare_nested);

        KeyInfo_t* key_info = mifare_nested->keys;

        with_view_model(
            plugin_state->view,
            CheckKeysViewModel * model,
            {
                model->lost_tag = false;
                model->keys_checked = key_info->checked_keys;
                model->keys_found = key_info->found_keys;
                model->keys_total = key_info->sector_keys;
                model->keys_count = key_info->total_keys;
            },
            true);
    } else if(event == MifareNestedWorkerEventNoTagDetected) {
        mifare_nested_blink_start(mifare_nested);

        with_view_model(
            plugin_state->view, CheckKeysViewModel * model, { model->lost_tag = true; }, true);
    } else if(event == MifareNestedWorkerEventProcessingKeys) {
        with_view_model(
            plugin_state->view,
            CheckKeysViewModel * model,
            { model->processing_keys = true; },
            true);
    }

    view_dispatcher_send_custom_event(mifare_nested->view_dispatcher, event);

    return true;
}

void mifare_nested_scene_check_keys_on_enter(void* context) {
    MifareNested* mifare_nested = context;
    CheckKeysState* plugin_state = mifare_nested->keys_state;

    mifare_nested_worker_start(
        mifare_nested->worker,
        MifareNestedWorkerStateValidating,
        &mifare_nested->nfc_dev->dev_data,
        mifare_nested_check_keys_worker_callback,
        mifare_nested);

    mifare_nested_blink_start(mifare_nested);

    with_view_model(
        plugin_state->view,
        CheckKeysViewModel * model,
        {
            model->lost_tag = false;
            model->processing_keys = false;
            model->keys_count = 0;
            model->keys_checked = 0;
            model->keys_found = 0;
        },
        false);

    view_dispatcher_switch_to_view(mifare_nested->view_dispatcher, MifareNestedViewCheckKeys);
}

bool mifare_nested_scene_check_keys_on_event(void* context, SceneManagerEvent event) {
    MifareNested* mifare_nested = context;

    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeCenter) {
            scene_manager_search_and_switch_to_previous_scene(mifare_nested->scene_manager, 0);
            consumed = true;
        } else if(event.event == MifareNestedWorkerEventKeysFound) {
            scene_manager_next_scene(mifare_nested->scene_manager, MifareNestedSceneAddedKeys);
            consumed = true;
        } else if(event.event == MifareNestedWorkerEventNeedKeyRecovery) {
            scene_manager_next_scene(
                mifare_nested->scene_manager, MifareNestedSceneNeedKeyRecovery);
            consumed = true;
        } else if(event.event == MifareNestedWorkerEventNeedCollection) {
            scene_manager_next_scene(
                mifare_nested->scene_manager, MifareNestedSceneNeedCollection);
            consumed = true;
        } else if(
            event.event == MifareNestedWorkerEventKeyChecked ||
            event.event == MifareNestedWorkerEventNoTagDetected ||
            event.event == MifareNestedWorkerEventProcessingKeys) {
            consumed = true;
        }
    }

    return consumed;
}

void mifare_nested_scene_check_keys_on_exit(void* context) {
    MifareNested* mifare_nested = context;
    mifare_nested_worker_stop(mifare_nested->worker);

    // Clear view
    mifare_nested_blink_stop(mifare_nested);
    popup_reset(mifare_nested->popup);
    widget_reset(mifare_nested->widget);
}
