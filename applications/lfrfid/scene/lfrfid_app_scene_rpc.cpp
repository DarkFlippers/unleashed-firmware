#include "lfrfid_app_scene_rpc.h"
#include <core/common_defines.h>
#include <dolphin/dolphin.h>
#include <rpc/rpc_app.h>

void LfRfidAppSceneRpc::on_enter(LfRfidApp* app, bool /* need_restore */) {
    auto popup = app->view_controller.get<PopupVM>();

    popup->set_header("LF RFID", 89, 42, AlignCenter, AlignBottom);
    popup->set_text("RPC mode", 89, 44, AlignCenter, AlignTop);
    popup->set_icon(0, 12, &I_RFIDDolphinSend_97x61);

    app->view_controller.switch_to<PopupVM>();

    notification_message(app->notification, &sequence_display_backlight_on);
}

bool LfRfidAppSceneRpc::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    UNUSED(app);
    UNUSED(event);
    bool consumed = false;

    if(event->type == LfRfidApp::EventType::Exit) {
        consumed = true;
        LfRfidApp::Event view_event;
        view_event.type = LfRfidApp::EventType::Back;
        app->view_controller.send_event(&view_event);
        rpc_system_app_confirm(app->rpc_ctx, RpcAppEventAppExit, true);
    } else if(event->type == LfRfidApp::EventType::RpcSessionClose) {
        consumed = true;
        LfRfidApp::Event view_event;
        view_event.type = LfRfidApp::EventType::Back;
        app->view_controller.send_event(&view_event);
    } else if(event->type == LfRfidApp::EventType::RpcLoadFile) {
        const char* arg = rpc_system_app_get_data(app->rpc_ctx);
        consumed = true;
        bool result = false;
        if(arg && !emulating) {
            string_set_str(app->file_path, arg);
            if(app->load_key_data(app->file_path, false)) {
                lfrfid_worker_start_thread(app->lfworker);
                lfrfid_worker_emulate_start(app->lfworker, (LFRFIDProtocol)app->protocol_id);
                emulating = true;

                auto popup = app->view_controller.get<PopupVM>();
                app->text_store.set("emulating\n%s", string_get_cstr(app->file_name));
                popup->set_text(app->text_store.text, 89, 44, AlignCenter, AlignTop);

                notification_message(app->notification, &sequence_blink_start_magenta);
                result = true;
            }
        }
        rpc_system_app_confirm(app->rpc_ctx, RpcAppEventLoadFile, result);
    }

    return consumed;
}

void LfRfidAppSceneRpc::on_exit(LfRfidApp* app) {
    if(emulating) {
        lfrfid_worker_stop(app->lfworker);
        lfrfid_worker_stop_thread(app->lfworker);
        notification_message(app->notification, &sequence_blink_stop);
    }
    app->view_controller.get<PopupVM>()->clean();
}
