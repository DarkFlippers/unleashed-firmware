#include "../subghz_test_app_i.h"

typedef enum {
    SubmenuIndexSubGhzTestCarrier,
    SubmenuIndexSubGhzTestPacket,
    SubmenuIndexSubGhzTestStatic,
    SubmenuIndexSubGhzTestAbout,
} SubmenuIndex;

void subghz_test_scene_start_submenu_callback(void* context, uint32_t index) {
    SubGhzTestApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void subghz_test_scene_start_on_enter(void* context) {
    SubGhzTestApp* app = context;
    Submenu* submenu = app->submenu;

    submenu_add_item(
        submenu,
        "Carrier",
        SubmenuIndexSubGhzTestCarrier,
        subghz_test_scene_start_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "Packet",
        SubmenuIndexSubGhzTestPacket,
        subghz_test_scene_start_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "Static",
        SubmenuIndexSubGhzTestStatic,
        subghz_test_scene_start_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "About",
        SubmenuIndexSubGhzTestAbout,
        subghz_test_scene_start_submenu_callback,
        app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, SubGhzTestSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, SubGhzTestViewSubmenu);
}

bool subghz_test_scene_start_on_event(void* context, SceneManagerEvent event) {
    SubGhzTestApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexSubGhzTestAbout) {
            scene_manager_next_scene(app->scene_manager, SubGhzTestSceneAbout);
            consumed = true;
        } else if(event.event == SubmenuIndexSubGhzTestCarrier) {
            scene_manager_next_scene(app->scene_manager, SubGhzTestSceneCarrier);
            consumed = true;
        } else if(event.event == SubmenuIndexSubGhzTestPacket) {
            scene_manager_next_scene(app->scene_manager, SubGhzTestScenePacket);
            consumed = true;
        } else if(event.event == SubmenuIndexSubGhzTestStatic) {
            scene_manager_next_scene(app->scene_manager, SubGhzTestSceneStatic);
            consumed = true;
        }
        scene_manager_set_scene_state(app->scene_manager, SubGhzTestSceneStart, event.event);
    }

    return consumed;
}

void subghz_test_scene_start_on_exit(void* context) {
    SubGhzTestApp* app = context;
    submenu_reset(app->submenu);
}
