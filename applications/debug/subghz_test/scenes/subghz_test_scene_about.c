#include "../subghz_test_app_i.h"

void subghz_test_scene_about_widget_callback(GuiButtonType result, InputType type, void* context) {
    SubGhzTestApp* app = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(app->view_dispatcher, result);
    }
}

void subghz_test_scene_about_on_enter(void* context) {
    SubGhzTestApp* app = context;

    FuriString* temp_str;
    temp_str = furi_string_alloc();
    furi_string_printf(temp_str, "\e#%s\n", "Information");

    furi_string_cat_printf(temp_str, "Version: %s\n", SUBGHZ_TEST_VERSION_APP);
    furi_string_cat_printf(temp_str, "Developed by: %s\n", SUBGHZ_TEST_DEVELOPED);
    furi_string_cat_printf(temp_str, "Github: %s\n\n", SUBGHZ_TEST_GITHUB);

    furi_string_cat_printf(temp_str, "\e#%s\n", "Description");
    furi_string_cat_printf(
        temp_str,
        "This application is designed\nto test the functionality of the\nbuilt-in CC1101 module.\n\n");

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
        "\e#\e!         Sub-Ghz Test          \e!\n",
        false);
    widget_add_text_scroll_element(app->widget, 0, 16, 128, 50, furi_string_get_cstr(temp_str));
    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubGhzTestViewWidget);
}

bool subghz_test_scene_about_on_event(void* context, SceneManagerEvent event) {
    SubGhzTestApp* app = context;
    bool consumed = false;
    UNUSED(app);
    UNUSED(event);

    return consumed;
}

void subghz_test_scene_about_on_exit(void* context) {
    SubGhzTestApp* app = context;

    // Clear views
    widget_reset(app->widget);
}
