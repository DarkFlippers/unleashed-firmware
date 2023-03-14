#include "../spi_mem_app_i.h"

static void spi_mem_scene_select_model_submenu_callback(void* context, uint32_t index) {
    SPIMemApp* app = context;
    spi_mem_chip_copy_chip_info(app->chip_info, *found_chips_get(app->found_chips, index));
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void spi_mem_scene_select_model_on_enter(void* context) {
    SPIMemApp* app = context;
    size_t models_on_vendor = 0;
    for(size_t index = 0; index < found_chips_size(app->found_chips); index++) {
        if(spi_mem_chip_get_vendor_enum(*found_chips_get(app->found_chips, index)) !=
           app->chip_vendor_enum)
            continue;
        submenu_add_item(
            app->submenu,
            spi_mem_chip_get_model_name(*found_chips_get(app->found_chips, index)),
            index,
            spi_mem_scene_select_model_submenu_callback,
            app);
        models_on_vendor++;
    }
    if(models_on_vendor == 1) spi_mem_scene_select_model_submenu_callback(context, 0);
    submenu_set_header(app->submenu, "Choose chip model");
    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, SPIMemSceneSelectVendor));
    view_dispatcher_switch_to_view(app->view_dispatcher, SPIMemViewSubmenu);
}

bool spi_mem_scene_select_model_on_event(void* context, SceneManagerEvent event) {
    SPIMemApp* app = context;
    bool success = false;
    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(app->scene_manager, SPIMemSceneSelectVendor, event.event);
        scene_manager_next_scene(app->scene_manager, SPIMemSceneChipDetected);
        success = true;
    }
    return success;
}

void spi_mem_scene_select_model_on_exit(void* context) {
    SPIMemApp* app = context;
    submenu_reset(app->submenu);
}
