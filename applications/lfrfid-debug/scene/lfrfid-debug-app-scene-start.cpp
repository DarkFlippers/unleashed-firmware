#include "lfrfid-debug-app-scene-start.h"

typedef enum {
    SubmenuTune,
} SubmenuIndex;

void LfRfidDebugAppSceneStart::on_enter(LfRfidDebugApp* app, bool need_restore) {
    auto submenu = app->view_controller.get<SubmenuVM>();
    auto callback = cbc::obtain_connector(this, &LfRfidDebugAppSceneStart::submenu_callback);

    submenu->add_item("Tune", SubmenuTune, callback, app);

    if(need_restore) {
        submenu->set_selected_item(submenu_item_selected);
    }
    app->view_controller.switch_to<SubmenuVM>();
}

bool LfRfidDebugAppSceneStart::on_event(LfRfidDebugApp* app, LfRfidDebugApp::Event* event) {
    bool consumed = false;

    if(event->type == LfRfidDebugApp::EventType::MenuSelected) {
        submenu_item_selected = event->payload.menu_index;
        switch(event->payload.menu_index) {
        case SubmenuTune:
            app->scene_controller.switch_to_next_scene(LfRfidDebugApp::SceneType::TuneScene);
            break;
        }
        consumed = true;
    }

    return consumed;
}

void LfRfidDebugAppSceneStart::on_exit(LfRfidDebugApp* app) {
    app->view_controller.get<SubmenuVM>()->clean();
}

void LfRfidDebugAppSceneStart::submenu_callback(void* context, uint32_t index) {
    LfRfidDebugApp* app = static_cast<LfRfidDebugApp*>(context);
    LfRfidDebugApp::Event event;

    event.type = LfRfidDebugApp::EventType::MenuSelected;
    event.payload.menu_index = index;

    app->view_controller.send_event(&event);
}
