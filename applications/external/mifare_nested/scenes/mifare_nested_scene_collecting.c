#include "../mifare_nested_i.h"

void mifare_nested_scene_collecting_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    MifareNested* mifare_nested = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(mifare_nested->view_dispatcher, result);
    }
}

bool mifare_nested_collecting_worker_callback(MifareNestedWorkerEvent event, void* context) {
    MifareNested* mifare_nested = context;
    NestedState* plugin_state = mifare_nested->nested_state;

    if(event == MifareNestedWorkerEventNewNonce) {
        mifare_nested_blink_nonce_collection_start(mifare_nested);

        uint8_t collected = 0;
        uint8_t skip = 0;
        NonceList_t* nonces = mifare_nested->nonces;
        for(uint8_t tries = 0; tries < nonces->tries; tries++) {
            for(uint8_t sector = 0; sector < nonces->sector_count; sector++) {
                for(uint8_t keyType = 0; keyType < 2; keyType++) {
                    Nonces* info = nonces->nonces[sector][keyType][tries];
                    if(info->from_start) {
                        skip++;
                    } else if(info->collected) {
                        collected++;
                    }
                }
            }
        }

        with_view_model(
            plugin_state->view,
            NestedAttackViewModel * model,
            {
                model->calibrating = false;
                model->lost_tag = false;
                model->nonces_collected = collected;
                model->keys_count = (nonces->sector_count * nonces->tries * 2) - skip;
            },
            true);
    } else if(event == MifareNestedWorkerEventNoTagDetected) {
        mifare_nested_blink_start(mifare_nested);

        with_view_model(
            plugin_state->view, NestedAttackViewModel * model, { model->lost_tag = true; }, true);
    } else if(event == MifareNestedWorkerEventCalibrating) {
        mifare_nested_blink_calibration_start(mifare_nested);

        with_view_model(
            plugin_state->view,
            NestedAttackViewModel * model,
            {
                model->calibrating = true;
                model->lost_tag = false;
                model->need_prediction = false;
                model->hardnested = false;
            },
            true);
    } else if(event == MifareNestedWorkerEventNeedPrediction) {
        with_view_model(
            plugin_state->view,
            NestedAttackViewModel * model,
            { model->need_prediction = true; },
            true);
    } else if(event == MifareNestedWorkerEventHardnestedStatesFound) {
        NonceList_t* nonces = mifare_nested->nonces;
        with_view_model(
            plugin_state->view,
            NestedAttackViewModel * model,
            {
                model->calibrating = false;
                model->lost_tag = false;
                model->hardnested = true;
                model->hardnested_states = nonces->hardnested_states;
            },
            true);
    }

    view_dispatcher_send_custom_event(mifare_nested->view_dispatcher, event);

    return true;
}

void mifare_nested_scene_collecting_on_enter(void* context) {
    MifareNested* mifare_nested = context;
    NestedState* nested = mifare_nested->nested_state;

    mifare_nested_worker_start(
        mifare_nested->worker,
        mifare_nested->collecting_type,
        &mifare_nested->nfc_dev->dev_data,
        mifare_nested_collecting_worker_callback,
        mifare_nested);

    mifare_nested_blink_start(mifare_nested);

    with_view_model(
        nested->view,
        NestedAttackViewModel * model,
        {
            model->lost_tag = false;
            model->nonces_collected = 0;
        },
        false);

    view_dispatcher_switch_to_view(mifare_nested->view_dispatcher, MifareNestedViewCollecting);
}

bool mifare_nested_scene_collecting_on_event(void* context, SceneManagerEvent event) {
    MifareNested* mifare_nested = context;

    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeCenter) {
            scene_manager_search_and_switch_to_previous_scene(mifare_nested->scene_manager, 0);
            consumed = true;
        } else if(event.event == MifareNestedWorkerEventNoncesCollected) {
            scene_manager_next_scene(
                mifare_nested->scene_manager, MifareNestedSceneNoncesCollected);
            consumed = true;
        } else if(event.event == MifareNestedWorkerEventNoNoncesCollected) {
            scene_manager_next_scene(
                mifare_nested->scene_manager, MifareNestedSceneNoNoncesCollected);
            consumed = true;
        } else if(event.event == MifareNestedWorkerEventAttackFailed) {
            scene_manager_next_scene(mifare_nested->scene_manager, MifareNestedSceneFailed);
            consumed = true;
        } else if(event.event == MifareNestedWorkerEventNeedKey) {
            scene_manager_next_scene(mifare_nested->scene_manager, MifareNestedSceneNoKeys);
            consumed = true;
        } else if(event.event == MifareNestedWorkerEventStaticEncryptedNonce) {
            scene_manager_next_scene(
                mifare_nested->scene_manager, MifareNestedSceneStaticEncryptedNonce);
            consumed = true;
        } else if(
            event.event == MifareNestedWorkerEventNewNonce ||
            event.event == MifareNestedWorkerEventNoTagDetected ||
            event.event == MifareNestedWorkerEventCalibrating ||
            event.event == MifareNestedWorkerEventNeedPrediction ||
            event.event == MifareNestedWorkerEventHardnestedStatesFound) {
            consumed = true;
        }
    }

    return consumed;
}

void mifare_nested_scene_collecting_on_exit(void* context) {
    MifareNested* mifare_nested = context;
    mifare_nested_worker_stop(mifare_nested->worker);

    // Clear view
    mifare_nested_blink_stop(mifare_nested);
    popup_reset(mifare_nested->popup);
    widget_reset(mifare_nested->widget);
}