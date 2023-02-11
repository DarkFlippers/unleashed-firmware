#include "../spi_mem_app_i.h"

static void spi_mem_scene_success_popup_callback(void* context) {
    SPIMemApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, SPIMemCustomEventPopupBack);
}

void spi_mem_scene_success_on_enter(void* context) {
    SPIMemApp* app = context;
    popup_set_icon(app->popup, 32, 5, &I_DolphinNice_96x59);
    popup_set_header(app->popup, "Success!", 5, 7, AlignLeft, AlignTop);
    popup_set_callback(app->popup, spi_mem_scene_success_popup_callback);
    popup_set_context(app->popup, app);
    popup_set_timeout(app->popup, 1500);
    popup_enable_timeout(app->popup);
    view_dispatcher_switch_to_view(app->view_dispatcher, SPIMemViewPopup);
}

static void spi_mem_scene_success_set_previous_scene(SPIMemApp* app) {
    uint32_t scene = SPIMemSceneSelectFile;
    if(app->mode == SPIMemModeErase) scene = SPIMemSceneStart;
    scene_manager_search_and_switch_to_another_scene(app->scene_manager, scene);
}

bool spi_mem_scene_success_on_event(void* context, SceneManagerEvent event) {
    SPIMemApp* app = context;
    bool success = false;
    if(event.type == SceneManagerEventTypeCustom) {
        success = true;
        if(event.event == SPIMemCustomEventPopupBack) {
            spi_mem_scene_success_set_previous_scene(app);
        }
    }
    return success;
}

void spi_mem_scene_success_on_exit(void* context) {
    SPIMemApp* app = context;
    popup_reset(app->popup);
}
