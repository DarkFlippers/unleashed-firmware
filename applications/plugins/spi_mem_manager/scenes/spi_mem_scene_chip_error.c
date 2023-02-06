#include "../spi_mem_app_i.h"

static void
    spi_mem_scene_chip_error_widget_callback(GuiButtonType result, InputType type, void* context) {
    SPIMemApp* app = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(app->view_dispatcher, result);
    }
}

void spi_mem_scene_chip_error_on_enter(void* context) {
    SPIMemApp* app = context;
    widget_add_button_element(
        app->widget, GuiButtonTypeLeft, "Back", spi_mem_scene_chip_error_widget_callback, app);
    widget_add_string_element(
        app->widget, 85, 15, AlignCenter, AlignBottom, FontPrimary, "SPI chip error");
    widget_add_string_multiline_element(
        app->widget,
        85,
        52,
        AlignCenter,
        AlignBottom,
        FontSecondary,
        "Error while\ncommunicating\nwith chip");
    widget_add_icon_element(app->widget, 5, 6, &I_Dip8_32x36);
    view_dispatcher_switch_to_view(app->view_dispatcher, SPIMemViewWidget);
}

static void spi_mem_scene_chip_error_set_previous_scene(SPIMemApp* app) {
    uint32_t scene = SPIMemSceneChipDetect;
    if(app->mode == SPIMemModeRead || app->mode == SPIMemModeErase) scene = SPIMemSceneStart;
    scene_manager_search_and_switch_to_previous_scene(app->scene_manager, scene);
}

bool spi_mem_scene_chip_error_on_event(void* context, SceneManagerEvent event) {
    SPIMemApp* app = context;
    bool success = false;
    if(event.type == SceneManagerEventTypeBack) {
        success = true;
        spi_mem_scene_chip_error_set_previous_scene(app);
    } else if(event.type == SceneManagerEventTypeCustom) {
        success = true;
        if(event.event == GuiButtonTypeLeft) {
            spi_mem_scene_chip_error_set_previous_scene(app);
        }
    }
    return success;
}
void spi_mem_scene_chip_error_on_exit(void* context) {
    SPIMemApp* app = context;
    widget_reset(app->widget);
}
