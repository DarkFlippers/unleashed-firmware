#include "lfrfid_app_scene_raw_read.h"
#include <dolphin/dolphin.h>

#define RAW_READ_TIME 5000

static void lfrfid_read_callback(LFRFIDWorkerReadRawResult result, void* ctx) {
    LfRfidApp* app = static_cast<LfRfidApp*>(ctx);
    LfRfidApp::Event event;

    switch(result) {
    case LFRFIDWorkerReadRawFileError:
        event.type = LfRfidApp::EventType::ReadEventError;
        break;
    case LFRFIDWorkerReadRawOverrun:
        event.type = LfRfidApp::EventType::ReadEventOverrun;
        break;
    }

    app->view_controller.send_event(&event);
}

static void timer_callback(void* ctx) {
    LfRfidApp* app = static_cast<LfRfidApp*>(ctx);
    LfRfidApp::Event event;
    event.type = LfRfidApp::EventType::ReadEventDone;
    app->view_controller.send_event(&event);
}

void LfRfidAppSceneRawRead::on_enter(LfRfidApp* app, bool /* need_restore */) {
    string_init(string_file_name);
    auto popup = app->view_controller.get<PopupVM>();
    popup->set_icon(0, 3, &I_RFIDDolphinReceive_97x61);
    app->view_controller.switch_to<PopupVM>();
    lfrfid_worker_start_thread(app->lfworker);
    app->make_app_folder();

    timer = furi_timer_alloc(timer_callback, FuriTimerTypeOnce, app);
    furi_timer_start(timer, RAW_READ_TIME);
    string_printf(
        string_file_name, "%s/%s.ask.raw", app->app_sd_folder, string_get_cstr(app->raw_file_name));
    popup->set_header("Reading\nRAW RFID\nASK", 89, 30, AlignCenter, AlignTop);
    lfrfid_worker_read_raw_start(
        app->lfworker,
        string_get_cstr(string_file_name),
        LFRFIDWorkerReadTypeASKOnly,
        lfrfid_read_callback,
        app);

    notification_message(app->notification, &sequence_blink_start_cyan);

    is_psk = false;
    error = false;
}

bool LfRfidAppSceneRawRead::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    UNUSED(app);
    bool consumed = true;
    auto popup = app->view_controller.get<PopupVM>();

    switch(event->type) {
    case LfRfidApp::EventType::ReadEventError:
        error = true;
        popup->set_header("Reading\nRAW RFID\nFile error", 89, 30, AlignCenter, AlignTop);
        notification_message(app->notification, &sequence_blink_start_red);
        furi_timer_stop(timer);
        break;
    case LfRfidApp::EventType::ReadEventDone:
        if(!error) {
            if(is_psk) {
                notification_message(app->notification, &sequence_success);
                app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::RawSuccess);
            } else {
                popup->set_header("Reading\nRAW RFID\nPSK", 89, 30, AlignCenter, AlignTop);
                notification_message(app->notification, &sequence_blink_start_yellow);
                lfrfid_worker_stop(app->lfworker);
                string_printf(
                    string_file_name,
                    "%s/%s.psk.raw",
                    app->app_sd_folder,
                    string_get_cstr(app->raw_file_name));
                lfrfid_worker_read_raw_start(
                    app->lfworker,
                    string_get_cstr(string_file_name),
                    LFRFIDWorkerReadTypePSKOnly,
                    lfrfid_read_callback,
                    app);
                furi_timer_start(timer, RAW_READ_TIME);
                is_psk = true;
            }
        }
        break;
    default:
        consumed = false;
        break;
    }

    return consumed;
}

void LfRfidAppSceneRawRead::on_exit(LfRfidApp* app) {
    notification_message(app->notification, &sequence_blink_stop);
    app->view_controller.get<PopupVM>()->clean();
    lfrfid_worker_stop(app->lfworker);
    lfrfid_worker_stop_thread(app->lfworker);
    furi_timer_free(timer);
    string_clear(string_file_name);
}
