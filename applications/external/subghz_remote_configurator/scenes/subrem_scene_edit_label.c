#include "../subghz_remote_app_i.h"

#include <lib/toolbox/path.h>

typedef enum {
    SubRemSceneEditLabelStateTextInput,
    SubRemSceneEditLabelStateWidget,
} SubRemSceneEditLabelState;

void subrem_scene_edit_label_text_input_callback(void* context) {
    furi_assert(context);
    SubGhzRemoteApp* app = context;
    view_dispatcher_send_custom_event(
        app->view_dispatcher, SubRemCustomEventSceneEditLabelInputDone);
}

void subrem_scene_edit_label_widget_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    SubGhzRemoteApp* app = context;
    if((result == GuiButtonTypeCenter) && (type == InputTypeShort)) {
        view_dispatcher_send_custom_event(
            app->view_dispatcher, SubRemCustomEventSceneEditLabelWidgetAcces);
    } else if((result == GuiButtonTypeLeft) && (type == InputTypeShort)) {
        view_dispatcher_send_custom_event(
            app->view_dispatcher, SubRemCustomEventSceneEditLabelWidgetBack);
    }
}

void subrem_scene_edit_label_on_enter(void* context) {
    SubGhzRemoteApp* app = context;

    SubRemSubFilePreset* sub_preset = app->map_preset->subs_preset[app->chusen_sub];

    FuriString* temp_str = furi_string_alloc();

    if(furi_string_empty(sub_preset->label)) {
        if(furi_string_empty(sub_preset->file_path)) {
            path_extract_filename(sub_preset->file_path, temp_str, true);
            strcpy(app->file_name_tmp, furi_string_get_cstr(temp_str));
        } else {
            strcpy(app->file_name_tmp, "");
        }
    } else {
        strcpy(app->file_name_tmp, furi_string_get_cstr(sub_preset->label));
    }

    TextInput* text_input = app->text_input;
    text_input_set_header_text(text_input, "Label name");
    text_input_set_result_callback(
        text_input,
        subrem_scene_edit_label_text_input_callback,
        app,
        app->file_name_tmp,
        25,
        false);

    text_input_set_minimum_length(app->text_input, 0);

    widget_add_string_element(
        app->widget, 63, 12, AlignCenter, AlignCenter, FontPrimary, "Empty Label Name");
    widget_add_string_element(
        app->widget, 63, 32, AlignCenter, AlignCenter, FontSecondary, "Continue?");

    widget_add_button_element(
        app->widget, GuiButtonTypeCenter, "Ok", subrem_scene_edit_label_widget_callback, app);
    widget_add_button_element(
        app->widget, GuiButtonTypeLeft, "Back", subrem_scene_edit_label_widget_callback, app);

    scene_manager_set_scene_state(
        app->scene_manager, SubRemSceneEditLabel, SubRemSceneEditLabelStateTextInput);
    view_dispatcher_switch_to_view(app->view_dispatcher, SubRemViewIDTextInput);

    furi_string_free(temp_str);
}

bool subrem_scene_edit_label_on_event(void* context, SceneManagerEvent event) {
    SubGhzRemoteApp* app = context;

    FuriString* label = app->map_preset->subs_preset[app->chusen_sub]->label;

    if(event.type == SceneManagerEventTypeBack) {
        if(scene_manager_get_scene_state(app->scene_manager, SubRemSceneEditLabel) ==
           SubRemSceneEditLabelStateWidget) {
            scene_manager_set_scene_state(
                app->scene_manager, SubRemSceneEditLabel, SubRemSceneEditLabelStateTextInput);
            view_dispatcher_switch_to_view(app->view_dispatcher, SubRemViewIDTextInput);
            return true;
        } else if(
            scene_manager_get_scene_state(app->scene_manager, SubRemSceneEditLabel) ==
            SubRemSceneEditLabelStateTextInput) {
            scene_manager_previous_scene(app->scene_manager);
            return true;
        }

        scene_manager_previous_scene(app->scene_manager);
        return true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubRemCustomEventSceneEditLabelInputDone) {
            if(strcmp(app->file_name_tmp, "") == 0) {
                scene_manager_set_scene_state(
                    app->scene_manager, SubRemSceneEditLabel, SubRemSceneEditLabelStateWidget);
                view_dispatcher_switch_to_view(app->view_dispatcher, SubRemViewIDWidget);

            } else {
                furi_string_set(label, app->file_name_tmp);
                app->map_not_saved = true;
                scene_manager_previous_scene(app->scene_manager);
            }
            return true;
        } else if(event.event == SubRemCustomEventSceneEditLabelWidgetAcces) {
            furi_string_set(label, app->file_name_tmp);
            app->map_not_saved = true;
            scene_manager_previous_scene(app->scene_manager);

            return true;
        } else if(event.event == SubRemCustomEventSceneEditLabelWidgetBack) {
            scene_manager_set_scene_state(
                app->scene_manager, SubRemSceneEditLabel, SubRemSceneEditLabelStateTextInput);
            view_dispatcher_switch_to_view(app->view_dispatcher, SubRemViewIDTextInput);

            return true;
        }
    }
    return false;
}

void subrem_scene_edit_label_on_exit(void* context) {
    SubGhzRemoteApp* app = context;

    // Clear view
    text_input_reset(app->text_input);
    widget_reset(app->widget);
}
