#include "../subghz_i.h"
#include "../helpers/subghz_custom_event.h"

void subghz_scene_delete_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    if((result == GuiButtonTypeRight) && (type == InputTypeShort)) {
        view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventSceneDelete);
    }
}

void subghz_scene_delete_on_enter(void* context) {
    SubGhz* subghz = context;
    FuriString* frequency_str;
    FuriString* modulation_str;
    FuriString* text;

    frequency_str = furi_string_alloc();
    modulation_str = furi_string_alloc();
    text = furi_string_alloc();

    subghz_txrx_get_frequency_and_modulation(subghz->txrx, frequency_str, modulation_str);
    widget_add_string_element(
        subghz->widget,
        78,
        0,
        AlignLeft,
        AlignTop,
        FontSecondary,
        furi_string_get_cstr(frequency_str));

    widget_add_string_element(
        subghz->widget,
        113,
        0,
        AlignLeft,
        AlignTop,
        FontSecondary,
        furi_string_get_cstr(modulation_str));
    subghz_protocol_decoder_base_get_string(subghz_txrx_get_decoder(subghz->txrx), text);
    widget_add_string_multiline_element(
        subghz->widget, 0, 0, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(text));

    furi_string_free(frequency_str);
    furi_string_free(modulation_str);
    furi_string_free(text);

    widget_add_button_element(
        subghz->widget, GuiButtonTypeRight, "Delete", subghz_scene_delete_callback, subghz);

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdWidget);
}

bool subghz_scene_delete_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventSceneDelete) {
            furi_string_set(subghz->file_path_tmp, subghz->file_path);
            if(subghz_delete_file(subghz)) {
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneDeleteSuccess);
            } else {
                scene_manager_search_and_switch_to_previous_scene(
                    subghz->scene_manager, SubGhzSceneStart);
            }
            return true;
        }
    }
    return false;
}

void subghz_scene_delete_on_exit(void* context) {
    SubGhz* subghz = context;
    widget_reset(subghz->widget);
}
