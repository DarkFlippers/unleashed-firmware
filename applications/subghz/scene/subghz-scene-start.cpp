#include "subghz-scene-start.h"
#include "../subghz-app.h"
#include "../subghz-view-manager.h"
#include "../subghz-event.h"
#include <callback-connector.h>

typedef enum {
    SubmenuIndexSpectrumAnalyzer,
    SubmenuIndexFrequencyScanner,
    SubmenuIndexSignalAnalyzer,
    SubmenuIndexSignalTransmitter,
    SubmenuIndexApplications,
} SubmenuIndex;

void SubghzSceneStart::on_enter(SubghzApp* app) {
    SubghzAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();
    auto callback = cbc::obtain_connector(this, &SubghzSceneStart::submenu_callback);

    submenu_add_item(submenu, "Spectrum Analyzer", SubmenuIndexSpectrumAnalyzer, callback, app);
    submenu_add_item(submenu, "Frequency Scanner", SubmenuIndexFrequencyScanner, callback, app);
    submenu_add_item(submenu, "Signal Analyzer", SubmenuIndexSignalAnalyzer, callback, app);
    submenu_add_item(submenu, "Signal Transmitter", SubmenuIndexSignalTransmitter, callback, app);
    submenu_add_item(submenu, "Applications", SubmenuIndexApplications, callback, app);

    view_manager->switch_to(SubghzAppViewManager::ViewType::Submenu);
}

bool SubghzSceneStart::on_event(SubghzApp* app, SubghzEvent* event) {
    bool consumed = false;

    if(event->type == SubghzEvent::Type::MenuSelected) {
        switch(event->payload.menu_index) {
        case SubmenuIndexSpectrumAnalyzer:
            app->switch_to_next_scene(SubghzApp::Scene::SceneSpectrumSettings);
            break;
        case SubmenuIndexFrequencyScanner:
            break;
        case SubmenuIndexSignalAnalyzer:
            break;
        case SubmenuIndexSignalTransmitter:
            break;
        case SubmenuIndexApplications:
            break;
        }
        consumed = true;
    }

    return consumed;
}

void SubghzSceneStart::on_exit(SubghzApp* app) {
    SubghzAppViewManager* view_manager = app->get_view_manager();
    Submenu* submenu = view_manager->get_submenu();

    submenu_clean(submenu);
}

void SubghzSceneStart::submenu_callback(void* context, uint32_t index) {
    SubghzApp* app = static_cast<SubghzApp*>(context);
    SubghzEvent event;

    event.type = SubghzEvent::Type::MenuSelected;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}