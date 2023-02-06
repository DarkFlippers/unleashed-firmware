#include "../spi_mem_app_i.h"
#include "../lib/spi/spi_mem_chip.h"

static void spi_mem_scene_chip_detect_fail_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    SPIMemApp* app = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(app->view_dispatcher, result);
    }
}

void spi_mem_scene_chip_detect_fail_on_enter(void* context) {
    SPIMemApp* app = context;
    FuriString* str = furi_string_alloc();
    widget_add_button_element(
        app->widget,
        GuiButtonTypeCenter,
        "Retry",
        spi_mem_scene_chip_detect_fail_widget_callback,
        app);
    widget_add_string_element(
        app->widget, 64, 9, AlignCenter, AlignBottom, FontPrimary, "Detected");
    widget_add_string_element(
        app->widget, 64, 20, AlignCenter, AlignBottom, FontPrimary, "unknown SPI chip");
    furi_string_printf(str, "Vendor\nid: 0x%02X", spi_mem_chip_get_vendor_id(app->chip_info));
    widget_add_string_multiline_element(
        app->widget, 16, 44, AlignCenter, AlignBottom, FontSecondary, furi_string_get_cstr(str));
    furi_string_printf(str, "Type\nid: 0x%02X", spi_mem_chip_get_type_id(app->chip_info));
    widget_add_string_multiline_element(
        app->widget, 64, 44, AlignCenter, AlignBottom, FontSecondary, furi_string_get_cstr(str));
    furi_string_printf(str, "Capacity\nid: 0x%02X", spi_mem_chip_get_capacity_id(app->chip_info));
    widget_add_string_multiline_element(
        app->widget, 110, 44, AlignCenter, AlignBottom, FontSecondary, furi_string_get_cstr(str));
    furi_string_free(str);
    view_dispatcher_switch_to_view(app->view_dispatcher, SPIMemViewWidget);
}

bool spi_mem_scene_chip_detect_fail_on_event(void* context, SceneManagerEvent event) {
    SPIMemApp* app = context;
    bool success = false;
    if(event.type == SceneManagerEventTypeBack) {
        success = true;
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, SPIMemSceneStart);
    } else if(event.type == SceneManagerEventTypeCustom) {
        success = true;
        if(event.event == GuiButtonTypeCenter) {
            scene_manager_previous_scene(app->scene_manager);
        }
    }
    return success;
}
void spi_mem_scene_chip_detect_fail_on_exit(void* context) {
    SPIMemApp* app = context;
    widget_reset(app->widget);
}
