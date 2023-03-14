#include "../pocsag_pager_app_i.h"
#include "../helpers/pocsag_pager_types.h"

void pocsag_pager_scene_about_widget_callback(GuiButtonType result, InputType type, void* context) {
    POCSAGPagerApp* app = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(app->view_dispatcher, result);
    }
}

void pocsag_pager_scene_about_on_enter(void* context) {
    POCSAGPagerApp* app = context;

    FuriString* temp_str;
    temp_str = furi_string_alloc();
    furi_string_printf(temp_str, "\e#%s\n", "Information");

    furi_string_cat_printf(temp_str, "Version: %s\n", PCSG_VERSION_APP);
    furi_string_cat_printf(temp_str, "Developed by:\n%s\n\n", PCSG_DEVELOPED);
    furi_string_cat_printf(temp_str, "Github: %s\n\n", PCSG_GITHUB);

    furi_string_cat_printf(temp_str, "\e#%s\n", "Description");
    furi_string_cat_printf(temp_str, "Receiving POCSAG Pager \nmessages\n\n");

    furi_string_cat_printf(temp_str, "Supported protocols:\n");
    size_t i = 0;
    const char* protocol_name =
        subghz_environment_get_protocol_name_registry(app->txrx->environment, i++);
    do {
        furi_string_cat_printf(temp_str, "%s\n", protocol_name);
        protocol_name = subghz_environment_get_protocol_name_registry(app->txrx->environment, i++);
    } while(protocol_name != NULL);

    widget_add_text_box_element(
        app->widget,
        0,
        0,
        128,
        14,
        AlignCenter,
        AlignBottom,
        "\e#\e!                                                      \e!\n",
        false);
    widget_add_text_box_element(
        app->widget,
        0,
        2,
        128,
        14,
        AlignCenter,
        AlignBottom,
        "\e#\e!        POCSAG Pager       \e!\n",
        false);
    widget_add_text_scroll_element(app->widget, 0, 16, 128, 50, furi_string_get_cstr(temp_str));
    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(app->view_dispatcher, POCSAGPagerViewWidget);
}

bool pocsag_pager_scene_about_on_event(void* context, SceneManagerEvent event) {
    POCSAGPagerApp* app = context;
    bool consumed = false;
    UNUSED(app);
    UNUSED(event);

    return consumed;
}

void pocsag_pager_scene_about_on_exit(void* context) {
    POCSAGPagerApp* app = context;

    // Clear views
    widget_reset(app->widget);
}
