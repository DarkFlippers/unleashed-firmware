#include "../spi_mem_app_i.h"

static void spi_mem_scene_chip_detected_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    SPIMemApp* app = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(app->view_dispatcher, result);
    }
}

static void spi_mem_scene_chip_detected_print_chip_info(Widget* widget, SPIMemChip* chip_info) {
    FuriString* tmp_string = furi_string_alloc();
    widget_add_string_element(
        widget,
        40,
        12,
        AlignLeft,
        AlignTop,
        FontSecondary,
        spi_mem_chip_get_vendor_name(chip_info));
    widget_add_string_element(
        widget, 40, 20, AlignLeft, AlignTop, FontSecondary, spi_mem_chip_get_model_name(chip_info));
    furi_string_printf(tmp_string, "Size: %zu KB", spi_mem_chip_get_size(chip_info) / 1024);
    widget_add_string_element(
        widget, 40, 28, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(tmp_string));
    furi_string_free(tmp_string);
}

static void spi_mem_scene_chip_detect_draw_next_button(SPIMemApp* app) {
    FuriString* str = furi_string_alloc();
    if(app->mode == SPIMemModeRead) furi_string_printf(str, "%s", "Read");
    if(app->mode == SPIMemModeWrite) furi_string_printf(str, "%s", "Write");
    if(app->mode == SPIMemModeErase) furi_string_printf(str, "%s", "Erase");
    if(app->mode == SPIMemModeCompare) furi_string_printf(str, "%s", "Check");
    widget_add_button_element(
        app->widget,
        GuiButtonTypeRight,
        furi_string_get_cstr(str),
        spi_mem_scene_chip_detected_widget_callback,
        app);
    furi_string_free(str);
}

static void spi_mem_scene_chip_detected_set_previous_scene(SPIMemApp* app) {
    uint32_t scene = SPIMemSceneStart;
    if(app->mode == SPIMemModeCompare || app->mode == SPIMemModeWrite)
        scene = SPIMemSceneSavedFileMenu;
    scene_manager_search_and_switch_to_previous_scene(app->scene_manager, scene);
}

static void spi_mem_scene_chip_detected_set_next_scene(SPIMemApp* app) {
    uint32_t scene = SPIMemSceneStart;
    if(app->mode == SPIMemModeRead) scene = SPIMemSceneReadFilename;
    if(app->mode == SPIMemModeWrite) scene = SPIMemSceneErase;
    if(app->mode == SPIMemModeErase) scene = SPIMemSceneErase;
    if(app->mode == SPIMemModeCompare) scene = SPIMemSceneVerify;
    scene_manager_next_scene(app->scene_manager, scene);
}

void spi_mem_scene_chip_detected_on_enter(void* context) {
    SPIMemApp* app = context;
    widget_add_button_element(
        app->widget, GuiButtonTypeLeft, "Retry", spi_mem_scene_chip_detected_widget_callback, app);
    spi_mem_scene_chip_detect_draw_next_button(app);
    widget_add_icon_element(app->widget, 0, 12, &I_Dip8_32x36);
    widget_add_string_element(
        app->widget, 64, 9, AlignCenter, AlignBottom, FontPrimary, "Detected SPI chip");
    spi_mem_scene_chip_detected_print_chip_info(app->widget, app->chip_info);
    view_dispatcher_switch_to_view(app->view_dispatcher, SPIMemViewWidget);
}

bool spi_mem_scene_chip_detected_on_event(void* context, SceneManagerEvent event) {
    SPIMemApp* app = context;
    bool success = false;
    if(event.type == SceneManagerEventTypeBack) {
        success = true;
        spi_mem_scene_chip_detected_set_previous_scene(app);
    } else if(event.type == SceneManagerEventTypeCustom) {
        success = true;
        if(event.event == GuiButtonTypeLeft) {
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, SPIMemSceneChipDetect);
        } else if(event.event == GuiButtonTypeRight) {
            spi_mem_scene_chip_detected_set_next_scene(app);
        }
    }
    return success;
}
void spi_mem_scene_chip_detected_on_exit(void* context) {
    SPIMemApp* app = context;
    widget_reset(app->widget);
}
