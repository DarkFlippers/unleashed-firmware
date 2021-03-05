#include "subghz-scene-spectrum-settings.h"
#include "../subghz-app.h"
#include "../subghz-view-manager.h"
#include "../subghz-event.h"
#include <callback-connector.h>

void SubghzSceneSpectrumSettings::on_enter(SubghzApp* app) {
    SubghzAppViewManager* view_manager = app->get_view_manager();
    SubghzViewSpectrumSettings* spectrum_settings = view_manager->get_spectrum_settings();

    auto callback = cbc::obtain_connector(this, &SubghzSceneSpectrumSettings::ok_callback);
    spectrum_settings->set_ok_callback(callback, app);
    spectrum_settings->set_start_freq(433);

    view_manager->switch_to(SubghzAppViewManager::ViewType::SpectrumSettings);
}

bool SubghzSceneSpectrumSettings::on_event(SubghzApp* app, SubghzEvent* event) {
    bool consumed = false;

    if(event->type == SubghzEvent::Type::NextScene) {
        // save data
        // uint32_t start_freq = app->get_view_manager()->get_spectrum_settings()->get_start_freq();
        // app->get_spectrum_analyzer()->set_start_freq(start_freq);

        // switch to next scene
        // app->switch_to_next_scene(SubghzApp::Scene::SceneSpectrumAnalyze);
        consumed = true;
    }

    return consumed;
}

void SubghzSceneSpectrumSettings::on_exit(SubghzApp* app) {
    SubghzAppViewManager* view_manager = app->get_view_manager();
    SubghzViewSpectrumSettings* spectrum_settings = view_manager->get_spectrum_settings();

    spectrum_settings->set_ok_callback(nullptr, nullptr);
    spectrum_settings->set_start_freq(0);
}

void SubghzSceneSpectrumSettings::ok_callback(void* context) {
    SubghzApp* app = static_cast<SubghzApp*>(context);
    SubghzEvent event;

    event.type = SubghzEvent::Type::NextScene;
    app->get_view_manager()->send_event(&event);
}