#include "../subghz_i.h"

typedef enum {
    SubGhzSceneDeleteInfoCustomEventDelete,
} SubGhzSceneDeleteInfoCustomEvent;

void subghz_scene_delete_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    if((result == GuiButtonTypeRight) && (type == InputTypeShort)) {
        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubGhzSceneDeleteInfoCustomEventDelete);
    }
}

void subghz_scene_delete_on_enter(void* context) {
    SubGhz* subghz = context;

    char buffer_str[16];
    snprintf(
        buffer_str,
        sizeof(buffer_str),
        "%03ld.%02ld",
        subghz->txrx->frequency / 1000000 % 1000,
        subghz->txrx->frequency / 10000 % 100);
    widget_add_string_element(
        subghz->widget, 78, 0, AlignLeft, AlignTop, FontSecondary, buffer_str);
    if(subghz->txrx->preset == FuriHalSubGhzPresetOok650Async ||
       subghz->txrx->preset == FuriHalSubGhzPresetOok270Async) {
        snprintf(buffer_str, sizeof(buffer_str), "AM");
    } else if(
        subghz->txrx->preset == FuriHalSubGhzPreset2FSKDev238Async ||
        subghz->txrx->preset == FuriHalSubGhzPreset2FSKDev476Async) {
        snprintf(buffer_str, sizeof(buffer_str), "FM");
    } else {
        furi_crash(NULL);
    }
    widget_add_string_element(
        subghz->widget, 113, 0, AlignLeft, AlignTop, FontSecondary, buffer_str);
    string_t text;
    string_init(text);
    subghz->txrx->protocol_result->to_string(subghz->txrx->protocol_result, text);
    widget_add_string_multiline_element(
        subghz->widget, 0, 0, AlignLeft, AlignTop, FontSecondary, string_get_cstr(text));
    string_clear(text);

    widget_add_button_element(
        subghz->widget, GuiButtonTypeRight, "Delete", subghz_scene_delete_callback, subghz);

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewWidget);
}

bool subghz_scene_delete_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzSceneDeleteInfoCustomEventDelete) {
            memcpy(subghz->file_name_tmp, subghz->file_name, strlen(subghz->file_name));
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
    widget_clear(subghz->widget);
}
