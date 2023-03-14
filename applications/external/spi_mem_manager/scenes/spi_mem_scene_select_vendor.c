#include "../spi_mem_app_i.h"
#include <m-array.h>
#include <m-algo.h>

ARRAY_DEF(vendors, uint32_t)
ALGO_DEF(vendors, ARRAY_OPLIST(vendors))

static void spi_mem_scene_select_vendor_submenu_callback(void* context, uint32_t index) {
    SPIMemApp* app = context;
    app->chip_vendor_enum = index;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

static void spi_mem_scene_select_vendor_sort_vendors(SPIMemApp* app, vendors_t vendors_arr) {
    for(size_t index = 0; index < found_chips_size(app->found_chips); index++) {
        vendors_push_back(
            vendors_arr, spi_mem_chip_get_vendor_enum(*found_chips_get(app->found_chips, index)));
    }
    vendors_uniq(vendors_arr);
}

void spi_mem_scene_select_vendor_on_enter(void* context) {
    SPIMemApp* app = context;
    vendors_t vendors_arr;
    vendors_init(vendors_arr);
    spi_mem_scene_select_vendor_sort_vendors(app, vendors_arr);
    size_t vendors_arr_size = vendors_size(vendors_arr);
    if(vendors_arr_size == 1)
        spi_mem_scene_select_vendor_submenu_callback(context, *vendors_get(vendors_arr, 0));
    for(size_t index = 0; index < vendors_arr_size; index++) {
        uint32_t vendor_enum = *vendors_get(vendors_arr, index);
        submenu_add_item(
            app->submenu,
            spi_mem_chip_get_vendor_name_by_enum(vendor_enum),
            vendor_enum,
            spi_mem_scene_select_vendor_submenu_callback,
            app);
    }
    vendors_clear(vendors_arr);
    submenu_set_header(app->submenu, "Choose chip vendor");
    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, SPIMemSceneSelectVendor));
    view_dispatcher_switch_to_view(app->view_dispatcher, SPIMemViewSubmenu);
}

static void spi_mem_scene_select_vendor_set_previous_scene(SPIMemApp* app) {
    uint32_t scene = SPIMemSceneStart;
    if(app->mode == SPIMemModeCompare || app->mode == SPIMemModeWrite)
        scene = SPIMemSceneSavedFileMenu;
    scene_manager_search_and_switch_to_previous_scene(app->scene_manager, scene);
}

bool spi_mem_scene_select_vendor_on_event(void* context, SceneManagerEvent event) {
    SPIMemApp* app = context;
    bool success = false;
    if(event.type == SceneManagerEventTypeBack) {
        success = true;
        spi_mem_scene_select_vendor_set_previous_scene(app);
    } else if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(app->scene_manager, SPIMemSceneSelectVendor, event.event);
        scene_manager_next_scene(app->scene_manager, SPIMemSceneSelectModel);
        success = true;
    }
    return success;
}

void spi_mem_scene_select_vendor_on_exit(void* context) {
    SPIMemApp* app = context;
    submenu_reset(app->submenu);
}
