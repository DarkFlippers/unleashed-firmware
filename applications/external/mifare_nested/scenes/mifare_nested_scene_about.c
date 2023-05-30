#include "../mifare_nested_i.h"

void mifare_nested_scene_about_widget_callback(GuiButtonType result, InputType type, void* context) {
    MifareNested* mifare_nested = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(mifare_nested->view_dispatcher, result);
    }
}

void mifare_nested_scene_about_on_enter(void* context) {
    MifareNested* mifare_nested = context;

    FuriString* temp_str;
    temp_str = furi_string_alloc();
    furi_string_printf(temp_str, "\e#%s\n", "Information");

    furi_string_cat_printf(temp_str, "Version: %s\n", NESTED_VERSION_APP);
    furi_string_cat_printf(temp_str, "Developed by:\n%s\n\n", NESTED_AUTHOR);
    furi_string_cat_printf(temp_str, "Github: %s\n\n", NESTED_GITHUB_LINK);

    furi_string_cat_printf(temp_str, "\e#%s\n", "Description");
    furi_string_cat_printf(
        temp_str,
        "Ported Nested attacks\nfrom Proxmark3 (Iceman fork)\nCurrently supported attacks:\n - nested attack\n - static nested attack\n - hard nested attack\n\n");
    furi_string_cat_printf(
        temp_str,
        "You will need desktop app to recover keys from collected nonces: %s\n\n",
        NESTED_RECOVER_KEYS_GITHUB_LINK);
    furi_string_cat_printf(temp_str, "\e#%s\n", "Quick guide");
    furi_string_cat_printf(temp_str, "1. Install key recovery script on PC:\n");
    furi_string_cat_printf(temp_str, "pip install FlipperNested\n");
    furi_string_cat_printf(temp_str, "2. Connect Flipper Zero to PC\n");
    furi_string_cat_printf(temp_str, "3. Run key recovery:\n");
    furi_string_cat_printf(temp_str, "FlipperNested");

    widget_add_text_box_element(
        mifare_nested->widget,
        0,
        0,
        128,
        14,
        AlignCenter,
        AlignBottom,
        "\e#\e!                                                      \e!\n",
        false);
    widget_add_text_box_element(
        mifare_nested->widget,
        0,
        2,
        128,
        14,
        AlignCenter,
        AlignBottom,
        "\e#\e! Flipper Nested \e!\n",
        false);
    widget_add_text_scroll_element(
        mifare_nested->widget, 0, 16, 128, 50, furi_string_get_cstr(temp_str));
    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(mifare_nested->view_dispatcher, MifareNestedViewWidget);
}

bool mifare_nested_scene_about_on_event(void* context, SceneManagerEvent event) {
    MifareNested* mifare_nested = context;
    bool consumed = false;
    UNUSED(mifare_nested);
    UNUSED(event);

    return consumed;
}

void mifare_nested_scene_about_on_exit(void* context) {
    MifareNested* mifare_nested = context;

    // Clear views
    widget_reset(mifare_nested->widget);
}
