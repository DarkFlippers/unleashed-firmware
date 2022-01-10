#include "../irda_app.h"
#include "irda/irda_app_event.h"
#include <text_store.h>
#include <file_worker_cpp.h>

void IrdaAppSceneRemoteList::on_enter(IrdaApp* app) {
    furi_assert(app);

    FileWorkerCpp file_worker;
    bool result = false;
    bool file_select_result;
    auto remote_manager = app->get_remote_manager();
    auto last_selected_remote = remote_manager->get_remote_name();
    const char* last_selected_remote_name =
        last_selected_remote.size() ? last_selected_remote.c_str() : nullptr;
    auto filename_ts = std::make_unique<TextStore>(IrdaAppRemoteManager::max_remote_name_length);

    file_select_result = file_worker.file_select(
        IrdaApp::irda_directory,
        IrdaApp::irda_extension,
        filename_ts->text,
        filename_ts->text_size,
        last_selected_remote_name);

    if(file_select_result) {
        if(remote_manager->load(std::string(filename_ts->text))) {
            app->switch_to_next_scene(IrdaApp::Scene::Remote);
            result = true;
        }
    }

    if(!result) {
        app->switch_to_previous_scene();
    }
}

bool IrdaAppSceneRemoteList::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;

    return consumed;
}

void IrdaAppSceneRemoteList::on_exit(IrdaApp* app) {
}
