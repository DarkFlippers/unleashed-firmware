#include "../picopass_i.h"
#include <dolphin/dolphin.h>

void picopass_write_card_worker_callback(PicopassWorkerEvent event, void* context) {
    UNUSED(event);
    Picopass* picopass = context;
    view_dispatcher_send_custom_event(picopass->view_dispatcher, PicopassCustomEventWorkerExit);
}

void picopass_scene_write_card_on_enter(void* context) {
    Picopass* picopass = context;
    DOLPHIN_DEED(DolphinDeedNfcSave);

    // Setup view
    Popup* popup = picopass->popup;
    popup_set_header(popup, "Writing\npicopass\ncard", 68, 30, AlignLeft, AlignTop);
    popup_set_icon(popup, 0, 3, &I_RFIDDolphinSend_97x61);

    // Start worker
    view_dispatcher_switch_to_view(picopass->view_dispatcher, PicopassViewPopup);
    picopass_worker_start(
        picopass->worker,
        PicopassWorkerStateWrite,
        &picopass->dev->dev_data,
        picopass_write_card_worker_callback,
        picopass);

    picopass_blink_start(picopass);
}

bool picopass_scene_write_card_on_event(void* context, SceneManagerEvent event) {
    Picopass* picopass = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == PicopassCustomEventWorkerExit) {
            scene_manager_next_scene(picopass->scene_manager, PicopassSceneWriteCardSuccess);
            consumed = true;
        }
    }
    return consumed;
}

void picopass_scene_write_card_on_exit(void* context) {
    Picopass* picopass = context;

    // Stop worker
    picopass_worker_stop(picopass->worker);
    // Clear view
    popup_reset(picopass->popup);

    picopass_blink_stop(picopass);
}
