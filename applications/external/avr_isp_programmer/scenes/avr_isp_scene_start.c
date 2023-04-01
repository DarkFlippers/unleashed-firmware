#include "../avr_isp_app_i.h"

void avr_isp_scene_start_submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    AvrIspApp* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void avr_isp_scene_start_on_enter(void* context) {
    furi_assert(context);

    AvrIspApp* app = context;
    Submenu* submenu = app->submenu;
    submenu_add_item(
        submenu, "Dump AVR", SubmenuIndexAvrIspReader, avr_isp_scene_start_submenu_callback, app);
    submenu_add_item(
        submenu, "Flash AVR", SubmenuIndexAvrIspWriter, avr_isp_scene_start_submenu_callback, app);
    submenu_add_item(
        submenu,
        "ISP Programmer",
        SubmenuIndexAvrIspProgrammer,
        avr_isp_scene_start_submenu_callback,
        app);
    submenu_add_item(
        submenu, "Wiring", SubmenuIndexAvrIsWiring, avr_isp_scene_start_submenu_callback, app);
    submenu_add_item(
        submenu, "About", SubmenuIndexAvrIspAbout, avr_isp_scene_start_submenu_callback, app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, AvrIspSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, AvrIspViewSubmenu);
}

bool avr_isp_scene_start_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);

    AvrIspApp* app = context;
    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexAvrIspAbout) {
            scene_manager_next_scene(app->scene_manager, AvrIspSceneAbout);
            consumed = true;
        } else if(event.event == SubmenuIndexAvrIspProgrammer) {
            scene_manager_set_scene_state(
                app->scene_manager, AvrIspSceneChipDetect, AvrIspViewProgrammer);
            scene_manager_next_scene(app->scene_manager, AvrIspSceneChipDetect);
            consumed = true;
        } else if(event.event == SubmenuIndexAvrIspReader) {
            scene_manager_set_scene_state(
                app->scene_manager, AvrIspSceneChipDetect, AvrIspViewReader);
            scene_manager_next_scene(app->scene_manager, AvrIspSceneChipDetect);
            consumed = true;
        } else if(event.event == SubmenuIndexAvrIspWriter) {
            scene_manager_set_scene_state(
                app->scene_manager, AvrIspSceneChipDetect, AvrIspViewWriter);
            scene_manager_next_scene(app->scene_manager, AvrIspSceneChipDetect);
            consumed = true;
        } else if(event.event == SubmenuIndexAvrIsWiring) {
            scene_manager_next_scene(app->scene_manager, AvrIspSceneWiring);
            consumed = true;
        }
        scene_manager_set_scene_state(app->scene_manager, AvrIspSceneStart, event.event);
    }

    return consumed;
}

void avr_isp_scene_start_on_exit(void* context) {
    furi_assert(context);

    AvrIspApp* app = context;
    submenu_reset(app->submenu);
}
