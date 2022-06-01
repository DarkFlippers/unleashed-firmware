#include "../infrared_app.h"
#include "assets_icons.h"
#include "infrared/infrared_app_event.h"
#include <text_store.h>

void InfraredAppSceneRemoteList::on_enter(InfraredApp* app) {
    furi_assert(app);

    bool result = false;
    bool file_select_result;
    auto remote_manager = app->get_remote_manager();
    DialogsApp* dialogs = app->get_dialogs();

    InfraredAppViewManager* view_manager = app->get_view_manager();
    ButtonMenu* button_menu = view_manager->get_button_menu();
    button_menu_reset(button_menu);
    view_manager->switch_to(InfraredAppViewManager::ViewId::ButtonMenu);

    file_select_result = dialog_file_browser_show(
        dialogs,
        app->file_path,
        app->file_path,
        InfraredApp::infrared_extension,
        true,
        &I_ir_10px,
        true);

    if(file_select_result) {
        if(remote_manager->load(app->file_path)) {
            app->switch_to_next_scene(InfraredApp::Scene::Remote);
            result = true;
        }
    }

    if(!result) {
        app->switch_to_previous_scene();
    }
}

bool InfraredAppSceneRemoteList::on_event(InfraredApp*, InfraredAppEvent*) {
    return false;
}

void InfraredAppSceneRemoteList::on_exit(InfraredApp*) {
}
