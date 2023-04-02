#include "../picopass_i.h"
#include <dolphin/dolphin.h>

#define TAG "IclassEliteDictAttack"

typedef enum {
    DictAttackStateIdle,
    DictAttackStateUserDictInProgress,
    DictAttackStateFlipperDictInProgress,
    DictAttackStateStandardDictInProgress,
} DictAttackState;

void picopass_dict_attack_worker_callback(PicopassWorkerEvent event, void* context) {
    furi_assert(context);
    Picopass* picopass = context;
    view_dispatcher_send_custom_event(picopass->view_dispatcher, event);
}

void picopass_dict_attack_result_callback(void* context) {
    furi_assert(context);
    Picopass* picopass = context;
    view_dispatcher_send_custom_event(
        picopass->view_dispatcher, PicopassCustomEventDictAttackSkip);
}

static void
    picopass_scene_elite_dict_attack_prepare_view(Picopass* picopass, DictAttackState state) {
    IclassEliteDictAttackData* dict_attack_data =
        &picopass->dev->dev_data.iclass_elite_dict_attack_data;
    PicopassWorkerState worker_state = PicopassWorkerStateReady;
    IclassEliteDict* dict = NULL;

    // Identify scene state
    if(state == DictAttackStateIdle) {
        if(iclass_elite_dict_check_presence(IclassEliteDictTypeUser)) {
            FURI_LOG_D(TAG, "Starting with user dictionary");
            state = DictAttackStateUserDictInProgress;
        } else {
            FURI_LOG_D(TAG, "Starting with standard dictionary");
            state = DictAttackStateStandardDictInProgress;
        }
    } else if(state == DictAttackStateUserDictInProgress) {
        FURI_LOG_D(TAG, "Moving from user dictionary to standard dictionary");
        state = DictAttackStateStandardDictInProgress;
    } else if(state == DictAttackStateStandardDictInProgress) {
        FURI_LOG_D(TAG, "Moving from standard dictionary to elite dictionary");
        state = DictAttackStateFlipperDictInProgress;
    }

    // Setup view
    if(state == DictAttackStateUserDictInProgress) {
        worker_state = PicopassWorkerStateEliteDictAttack;
        dict_attack_set_header(picopass->dict_attack, "Elite User Dictionary");
        dict_attack_data->type = IclassEliteDictTypeUser;
        dict = iclass_elite_dict_alloc(IclassEliteDictTypeUser);

        // If failed to load user dictionary - try the system dictionary
        if(!dict) {
            FURI_LOG_E(TAG, "User dictionary not found");
            state = DictAttackStateStandardDictInProgress;
        }
    }
    if(state == DictAttackStateStandardDictInProgress) {
        worker_state = PicopassWorkerStateEliteDictAttack;
        dict_attack_set_header(picopass->dict_attack, "Standard System Dictionary");
        dict_attack_data->type = IclassStandardDictTypeFlipper;
        dict = iclass_elite_dict_alloc(IclassStandardDictTypeFlipper);

        if(!dict) {
            FURI_LOG_E(TAG, "Flipper standard dictionary not found");
            state = DictAttackStateFlipperDictInProgress;
        }
    }
    if(state == DictAttackStateFlipperDictInProgress) {
        worker_state = PicopassWorkerStateEliteDictAttack;
        dict_attack_set_header(picopass->dict_attack, "Elite System Dictionary");
        dict_attack_data->type = IclassEliteDictTypeFlipper;
        dict = iclass_elite_dict_alloc(IclassEliteDictTypeFlipper);
        if(!dict) {
            FURI_LOG_E(TAG, "Flipper Elite dictionary not found");
            // Pass through to let the worker handle the failure
        }
    }
    // Free previous dictionary
    if(dict_attack_data->dict) {
        iclass_elite_dict_free(dict_attack_data->dict);
    }
    dict_attack_data->dict = dict;
    scene_manager_set_scene_state(picopass->scene_manager, PicopassSceneEliteDictAttack, state);
    dict_attack_set_callback(
        picopass->dict_attack, picopass_dict_attack_result_callback, picopass);
    dict_attack_set_current_sector(picopass->dict_attack, 0);
    dict_attack_set_card_detected(picopass->dict_attack);
    dict_attack_set_total_dict_keys(
        picopass->dict_attack, dict ? iclass_elite_dict_get_total_keys(dict) : 0);
    picopass_worker_start(
        picopass->worker,
        worker_state,
        &picopass->dev->dev_data,
        picopass_dict_attack_worker_callback,
        picopass);
}

void picopass_scene_elite_dict_attack_on_enter(void* context) {
    Picopass* picopass = context;
    picopass_scene_elite_dict_attack_prepare_view(picopass, DictAttackStateIdle);
    view_dispatcher_switch_to_view(picopass->view_dispatcher, PicopassViewDictAttack);
    picopass_blink_start(picopass);
    notification_message(picopass->notifications, &sequence_display_backlight_enforce_on);
}

bool picopass_scene_elite_dict_attack_on_event(void* context, SceneManagerEvent event) {
    Picopass* picopass = context;
    bool consumed = false;

    uint32_t state =
        scene_manager_get_scene_state(picopass->scene_manager, PicopassSceneEliteDictAttack);
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == PicopassWorkerEventSuccess ||
           event.event == PicopassWorkerEventAborted) {
            if(state == DictAttackStateUserDictInProgress ||
               state == DictAttackStateStandardDictInProgress) {
                picopass_worker_stop(picopass->worker);
                picopass_scene_elite_dict_attack_prepare_view(picopass, state);
                consumed = true;
            } else {
                scene_manager_next_scene(picopass->scene_manager, PicopassSceneReadCardSuccess);
                consumed = true;
            }
        } else if(event.event == PicopassWorkerEventCardDetected) {
            dict_attack_set_card_detected(picopass->dict_attack);
            consumed = true;
        } else if(event.event == PicopassWorkerEventNoCardDetected) {
            dict_attack_set_card_removed(picopass->dict_attack);
            consumed = true;
        } else if(event.event == PicopassWorkerEventNewDictKeyBatch) {
            dict_attack_inc_current_dict_key(picopass->dict_attack, PICOPASS_DICT_KEY_BATCH_SIZE);
            consumed = true;
        } else if(event.event == PicopassCustomEventDictAttackSkip) {
            if(state == DictAttackStateUserDictInProgress) {
                picopass_worker_stop(picopass->worker);
                consumed = true;
            } else if(state == DictAttackStateFlipperDictInProgress) {
                picopass_worker_stop(picopass->worker);
                consumed = true;
            } else if(state == DictAttackStateStandardDictInProgress) {
                picopass_worker_stop(picopass->worker);
                consumed = true;
            }
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = scene_manager_previous_scene(picopass->scene_manager);
    }
    return consumed;
}

void picopass_scene_elite_dict_attack_on_exit(void* context) {
    Picopass* picopass = context;
    IclassEliteDictAttackData* dict_attack_data =
        &picopass->dev->dev_data.iclass_elite_dict_attack_data;
    // Stop worker
    picopass_worker_stop(picopass->worker);
    if(dict_attack_data->dict) {
        iclass_elite_dict_free(dict_attack_data->dict);
        dict_attack_data->dict = NULL;
    }
    dict_attack_reset(picopass->dict_attack);
    picopass_blink_stop(picopass);
    notification_message(picopass->notifications, &sequence_display_backlight_enforce_auto);
}
