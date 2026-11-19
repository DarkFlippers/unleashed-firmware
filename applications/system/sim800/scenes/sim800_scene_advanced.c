#include "../sim800_app_i.h"

typedef enum {
    SubmenuIndexDeviceInfo,
    SubmenuIndexNetworkInfo,
    SubmenuIndexInbox,
    SubmenuIndexHistory,
    SubmenuIndexUssd,
    SubmenuIndexDtmf,
    SubmenuIndexAudio,
    SubmenuIndexTerminal,
    SubmenuIndexDetect,
    SubmenuIndexSetBaud,
    SubmenuIndexPower,
} SubmenuIndex;

void sim800_scene_advanced_on_enter(void* context) {
    Sim800App* app = context;

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Advanced");
    submenu_add_item(
        app->submenu, "Device info", SubmenuIndexDeviceInfo, sim800_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Network info", SubmenuIndexNetworkInfo, sim800_submenu_callback, app);
    submenu_add_item(app->submenu, "SMS inbox", SubmenuIndexInbox, sim800_submenu_callback, app);
    submenu_add_item(app->submenu, "History", SubmenuIndexHistory, sim800_submenu_callback, app);
    submenu_add_item(app->submenu, "USSD code", SubmenuIndexUssd, sim800_submenu_callback, app);
    submenu_add_item(app->submenu, "DTMF tones", SubmenuIndexDtmf, sim800_submenu_callback, app);
    submenu_add_item(app->submenu, "Audio", SubmenuIndexAudio, sim800_submenu_callback, app);
    submenu_add_item(
        app->submenu, "AT terminal", SubmenuIndexTerminal, sim800_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Detect baud", SubmenuIndexDetect, sim800_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Set module baud", SubmenuIndexSetBaud, sim800_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Power & reset", SubmenuIndexPower, sim800_submenu_callback, app);

    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, Sim800SceneAdvanced));

    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewSubmenu);
}

bool sim800_scene_advanced_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;
    if(event.event > SubmenuIndexPower) return false;

    static const uint32_t next_scene[] = {
        [SubmenuIndexDeviceInfo] = Sim800SceneDeviceInfo,
        [SubmenuIndexNetworkInfo] = Sim800SceneNetworkInfo,
        [SubmenuIndexInbox] = Sim800SceneInbox,
        [SubmenuIndexHistory] = Sim800SceneHistory,
        [SubmenuIndexUssd] = Sim800SceneUssd,
        [SubmenuIndexDtmf] = Sim800SceneDtmf,
        [SubmenuIndexAudio] = Sim800SceneAudio,
        [SubmenuIndexTerminal] = Sim800SceneTerminal,
        [SubmenuIndexDetect] = Sim800SceneDetect,
        [SubmenuIndexSetBaud] = Sim800SceneSetBaud,
        [SubmenuIndexPower] = Sim800ScenePower,
    };

    scene_manager_set_scene_state(app->scene_manager, Sim800SceneAdvanced, event.event);
    scene_manager_next_scene(app->scene_manager, next_scene[event.event]);
    return true;
}

void sim800_scene_advanced_on_exit(void* context) {
    Sim800App* app = context;
    submenu_reset(app->submenu);
}
