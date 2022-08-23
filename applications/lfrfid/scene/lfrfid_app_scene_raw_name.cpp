
#include "lfrfid_app_scene_raw_name.h"
#include "m-string.h"
#include <lib/toolbox/random_name.h>
#include <lib/toolbox/path.h>

void LfRfidAppSceneRawName::on_enter(LfRfidApp* app, bool /* need_restore */) {
    const char* key_name = string_get_cstr(app->raw_file_name);

    bool key_name_empty = (string_size(app->raw_file_name) == 0);
    if(key_name_empty) {
        app->text_store.set("RfidRecord");
    } else {
        app->text_store.set("%s", key_name);
    }

    auto text_input = app->view_controller.get<TextInputVM>();
    text_input->set_header_text("Name the raw file");

    text_input->set_result_callback(
        save_callback, app, app->text_store.text, LFRFID_KEY_NAME_SIZE, key_name_empty);

    app->view_controller.switch_to<TextInputVM>();
}

bool LfRfidAppSceneRawName::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = false;

    if(event->type == LfRfidApp::EventType::Next) {
        string_set_str(app->raw_file_name, app->text_store.text);
        app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::RawInfo);
    }

    return consumed;
}

void LfRfidAppSceneRawName::on_exit(LfRfidApp* app) {
    app->view_controller.get<TextInputVM>()->clean();
}

void LfRfidAppSceneRawName::save_callback(void* context) {
    LfRfidApp* app = static_cast<LfRfidApp*>(context);
    LfRfidApp::Event event;
    event.type = LfRfidApp::EventType::Next;
    app->view_controller.send_event(&event);
}
