#include "../irda-app.h"
#include "irda/irda-app-event.h"

void IrdaAppSceneRemoteList::on_enter(IrdaApp* app) {
    IrdaAppFileParser file_parser;
    bool success = false;
    auto remote_manager = app->get_remote_manager();
    auto last_selected_remote = remote_manager->get_remote_name();
    auto selected_file = file_parser.file_select(
        last_selected_remote.size() ? last_selected_remote.c_str() : nullptr);
    if(!selected_file.empty()) {
        if(remote_manager->load(selected_file)) {
            app->switch_to_next_scene(IrdaApp::Scene::Remote);
            success = true;
        }
    }

    if(!success) {
        app->switch_to_previous_scene();
    }
}

bool IrdaAppSceneRemoteList::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;

    return consumed;
}

void IrdaAppSceneRemoteList::on_exit(IrdaApp* app) {
}
