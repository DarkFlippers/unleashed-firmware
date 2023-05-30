#include "../mifare_nested_i.h"

enum {
    MifareNestedSceneCheckStateTagSearch,
    MifareNestedSceneCheckStateTagFound,
};

bool mifare_nested_check_worker_callback(MifareNestedWorkerEvent event, void* context) {
    furi_assert(context);

    MifareNested* mifare_nested = context;
    view_dispatcher_send_custom_event(mifare_nested->view_dispatcher, event);

    return true;
}

static void mifare_nested_scene_check_setup_view(MifareNested* mifare_nested) {
    Popup* popup = mifare_nested->popup;
    popup_reset(popup);
    uint32_t state =
        scene_manager_get_scene_state(mifare_nested->scene_manager, MifareNestedSceneCheck);

    if(state == MifareNestedSceneCheckStateTagSearch) {
        popup_set_icon(mifare_nested->popup, 0, 8, &I_ApplyTag);
        popup_set_text(
            mifare_nested->popup, "Apply tag to\nthe back", 128, 32, AlignRight, AlignCenter);
    } else {
        popup_set_icon(popup, 12, 23, &I_Loading);
        popup_set_header(popup, "Checking\nDon't move...", 52, 32, AlignLeft, AlignCenter);
    }

    view_dispatcher_switch_to_view(mifare_nested->view_dispatcher, MifareNestedViewPopup);
}

void mifare_nested_scene_check_on_enter(void* context) {
    MifareNested* mifare_nested = context;

    scene_manager_set_scene_state(
        mifare_nested->scene_manager,
        MifareNestedSceneCheck,
        MifareNestedSceneCheckStateTagSearch);
    mifare_nested_scene_check_setup_view(mifare_nested);

    // Setup and start worker
    mifare_nested_worker_start(
        mifare_nested->worker,
        MifareNestedWorkerStateCheck,
        &mifare_nested->nfc_dev->dev_data,
        mifare_nested_check_worker_callback,
        mifare_nested);
    mifare_nested_blink_start(mifare_nested);
}

bool mifare_nested_scene_check_on_event(void* context, SceneManagerEvent event) {
    MifareNested* mifare_nested = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == MifareNestedWorkerEventCollecting) {
            if(mifare_nested->run == NestedRunAttack) {
                if(mifare_nested->settings->only_hardnested) {
                    FURI_LOG_I("MifareNested", "Using Hard Nested because user settings");
                    mifare_nested->collecting_type = MifareNestedWorkerStateCollectingHard;
                }
                scene_manager_next_scene(
                    mifare_nested->scene_manager, MifareNestedSceneCollecting);
            } else {
                scene_manager_next_scene(mifare_nested->scene_manager, MifareNestedSceneCheckKeys);
            }
            consumed = true;
        } else if(event.event == MifareNestedWorkerEventNoTagDetected) {
            scene_manager_set_scene_state(
                mifare_nested->scene_manager,
                MifareNestedSceneCheck,
                MifareNestedSceneCheckStateTagSearch);
            mifare_nested_scene_check_setup_view(mifare_nested);
            consumed = true;
        }
    }
    return consumed;
}

void mifare_nested_scene_check_on_exit(void* context) {
    MifareNested* mifare_nested = context;

    mifare_nested_worker_stop(mifare_nested->worker);
    scene_manager_set_scene_state(
        mifare_nested->scene_manager,
        MifareNestedSceneCheck,
        MifareNestedSceneCheckStateTagSearch);
    // Clear view
    popup_reset(mifare_nested->popup);

    mifare_nested_blink_stop(mifare_nested);
}
