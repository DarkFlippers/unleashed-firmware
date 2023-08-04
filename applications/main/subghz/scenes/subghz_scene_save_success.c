#include "../subghz_i.h"
#include "../helpers/subghz_custom_event.h"

void subghz_scene_save_success_popup_callback(void* context) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventSceneSaveSuccess);
}

void subghz_scene_save_success_on_enter(void* context) {
    SubGhz* subghz = context;

    // Setup view
    Popup* popup = subghz->popup;
    popup_set_icon(popup, 32, 5, &I_DolphinNice_96x59);
    popup_set_header(popup, "Saved!", 13, 22, AlignLeft, AlignBottom);
    popup_set_timeout(popup, 1500);
    popup_set_context(popup, subghz);
    popup_set_callback(popup, subghz_scene_save_success_popup_callback);
    popup_enable_timeout(popup);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdPopup);
}

bool subghz_scene_save_success_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventSceneSaveSuccess) {
            if(!scene_manager_has_previous_scene(subghz->scene_manager, SubGhzSceneDecodeRAW)) {
                if(!scene_manager_search_and_switch_to_previous_scene(
                       subghz->scene_manager, SubGhzSceneReceiver)) {
                    subghz_rx_key_state_set(subghz, SubGhzRxKeyStateRAWSave);
                    if(!scene_manager_search_and_switch_to_previous_scene(
                           subghz->scene_manager, SubGhzSceneReadRAW)) {
                        subghz_rx_key_state_set(subghz, SubGhzRxKeyStateIDLE);
                        if(!scene_manager_search_and_switch_to_previous_scene(
                               subghz->scene_manager, SubGhzSceneSaved)) {
                            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaved);
                        }
                    }
                }
            } else {
                scene_manager_set_scene_state(
                    subghz->scene_manager, SubGhzSceneDecodeRAW, SubGhzDecodeRawStateStart);

                subghz->idx_menu_chosen = 0;
                subghz_txrx_set_rx_callback(subghz->txrx, NULL, subghz);

                if(subghz_file_encoder_worker_is_running(subghz->decode_raw_file_worker_encoder)) {
                    subghz_file_encoder_worker_stop(subghz->decode_raw_file_worker_encoder);
                }
                subghz_file_encoder_worker_free(subghz->decode_raw_file_worker_encoder);

                subghz->state_notifications = SubGhzNotificationStateIDLE;
                scene_manager_set_scene_state(
                    subghz->scene_manager, SubGhzSceneReadRAW, SubGhzCustomEventManagerNoSet);
                if(!scene_manager_search_and_switch_to_previous_scene(
                       subghz->scene_manager, SubGhzSceneSaved)) {
                    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaved);
                }
            }
            return true;
        }
    }
    return false;
}

void subghz_scene_save_success_on_exit(void* context) {
    SubGhz* subghz = context;

    // Clear view
    Popup* popup = subghz->popup;

    popup_reset(popup);
}
