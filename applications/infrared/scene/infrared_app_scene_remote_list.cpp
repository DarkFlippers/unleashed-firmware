#include "../infrared_app.h"
#include "infrared/infrared_app_event.h"
#include <text_store.h>
#include <file_worker_cpp.h>

void InfraredAppSceneRemoteList::on_enter(InfraredApp* app) {
    furi_assert(app);

    FileWorkerCpp file_worker;
    bool result = false;
    bool file_select_result;
    auto remote_manager = app->get_remote_manager();
    auto last_selected_remote = remote_manager->get_remote_name();
    const char* last_selected_remote_name =
        last_selected_remote.size() ? last_selected_remote.c_str() : nullptr;
    auto filename_ts =
        std::make_unique<TextStore>(InfraredAppRemoteManager::max_remote_name_length);

    InfraredAppViewManager* view_manager = app->get_view_manager();
    ButtonMenu* button_menu = view_manager->get_button_menu();
    button_menu_reset(button_menu);
    view_manager->switch_to(InfraredAppViewManager::ViewId::ButtonMenu);

    file_select_result = file_worker.file_select(
        InfraredApp::infrared_directory,
        InfraredApp::infrared_extension,
        filename_ts->text,
        filename_ts->text_size,
        last_selected_remote_name);

    if(file_select_result) {
        if(remote_manager->load(InfraredApp::infrared_directory, std::string(filename_ts->text))) {
            app->switch_to_next_scene(InfraredApp::Scene::Remote);
            result = true;
        }
    }

    if(!result) {
        app->switch_to_previous_scene();
    }
}

bool InfraredAppSceneRemoteList::on_event(InfraredApp* app, InfraredAppEvent* event) {
    bool consumed = false;

    return consumed;
}

void InfraredAppSceneRemoteList::on_exit(InfraredApp* app) {
}
