#include "../lfrfid_i.h"

void lfrfid_scene_rpc_on_enter(void* context) {
    LfRfid* app = context;
    app->rpc_state = LfRfidRpcStateIdle;
}

static void lfrfid_rpc_start_emulation(LfRfid* app) {
    Popup* popup = app->popup;

    lfrfid_text_store_set(app, "emulating\n%s", furi_string_get_cstr(app->file_name));

    popup_set_header(popup, "LF RFID", 89, 42, AlignCenter, AlignBottom);
    popup_set_text(popup, app->text_store, 89, 44, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 12, &I_RFIDDolphinSend_97x61);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);

    lfrfid_worker_start_thread(app->lfworker);
    lfrfid_worker_emulate_start(app->lfworker, (LFRFIDProtocol)app->protocol_id);

    notification_message(app->notifications, &sequence_display_backlight_on);
    notification_message(app->notifications, &sequence_blink_start_magenta);
    app->rpc_state = LfRfidRpcStateEmulating;
}

bool lfrfid_scene_rpc_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == LfRfidEventExit) {
            rpc_system_app_confirm(app->rpc_ctx, true);
            scene_manager_stop(app->scene_manager);
            view_dispatcher_stop(app->view_dispatcher);
        } else if(event.event == LfRfidEventRpcSessionClose) {
            scene_manager_stop(app->scene_manager);
            view_dispatcher_stop(app->view_dispatcher);
        } else if(event.event == LfRfidEventRpcLoadFile) {
            bool result = false;
            if(app->rpc_state == LfRfidRpcStateIdle) {
                if(lfrfid_load_key_data(app, app->file_path, false)) {
                    lfrfid_rpc_start_emulation(app);
                    result = true;
                } else {
                    rpc_system_app_set_error_code(app->rpc_ctx, RpcAppSystemErrorCodeParseFile);
                    rpc_system_app_set_error_text(app->rpc_ctx, "Cannot load key file");
                }
            }
            rpc_system_app_confirm(app->rpc_ctx, result);
        }
    }
    return consumed;
}

void lfrfid_scene_rpc_on_exit(void* context) {
    LfRfid* app = context;
    Popup* popup = app->popup;

    if(app->rpc_state == LfRfidRpcStateEmulating) {
        lfrfid_worker_stop(app->lfworker);
        lfrfid_worker_stop_thread(app->lfworker);
        notification_message(app->notifications, &sequence_blink_stop);
    }

    popup_reset(popup);
}
