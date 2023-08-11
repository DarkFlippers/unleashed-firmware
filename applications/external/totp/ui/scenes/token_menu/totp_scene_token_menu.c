#include "totp_scene_token_menu.h"
#include <gui/gui.h>
#include <dialogs/dialogs.h>
#include "../../ui_controls.h"
#include "../../common_dialogs.h"
#include "../../constants.h"
#include "../../scene_director.h"
#include "../../../services/config/config.h"
#include "../../../types/token_info.h"
#include "../../../config/app/config.h"
#include <roll_value.h>

#define SCREEN_HEIGHT_THIRD (SCREEN_HEIGHT / 3)
#define SCREEN_HEIGHT_THIRD_CENTER (SCREEN_HEIGHT_THIRD >> 1)

typedef enum { AddNewToken, DeleteToken, AppSettings } Control;

typedef struct {
    Control selected_control;
} SceneState;

void totp_scene_token_menu_activate(PluginState* plugin_state) {
    SceneState* scene_state = malloc(sizeof(SceneState));
    furi_check(scene_state != NULL);
    plugin_state->current_scene_state = scene_state;
}

void totp_scene_token_menu_render(Canvas* const canvas, PluginState* plugin_state) {
    const SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;
    const TokenInfoIteratorContext* iterator_context =
        totp_config_get_token_iterator_context(plugin_state);
    if(totp_token_info_iterator_get_total_count(iterator_context) == 0) {
        ui_control_button_render(
            canvas,
            SCREEN_WIDTH_CENTER - 36,
            5,
            72,
            21,
            "Add new token",
            scene_state->selected_control == AddNewToken);
        ui_control_button_render(
            canvas,
            SCREEN_WIDTH_CENTER - 36,
            39,
            72,
            21,
            "Settings",
            scene_state->selected_control == AppSettings);
    } else {
        ui_control_button_render(
            canvas,
            SCREEN_WIDTH_CENTER - 36,
            SCREEN_HEIGHT_THIRD_CENTER - 8,
            72,
            16,
            "Add new token",
            scene_state->selected_control == AddNewToken);
        ui_control_button_render(
            canvas,
            SCREEN_WIDTH_CENTER - 36,
            SCREEN_HEIGHT_THIRD + SCREEN_HEIGHT_THIRD_CENTER - 8,
            72,
            16,
            "Delete token",
            scene_state->selected_control == DeleteToken);
        ui_control_button_render(
            canvas,
            SCREEN_WIDTH_CENTER - 36,
            SCREEN_HEIGHT_THIRD + SCREEN_HEIGHT_THIRD + SCREEN_HEIGHT_THIRD_CENTER - 8,
            72,
            16,
            "Settings",
            scene_state->selected_control == AppSettings);
    }
}

bool totp_scene_token_menu_handle_event(const PluginEvent* const event, PluginState* plugin_state) {
    if(event->type != EventTypeKey) {
        return true;
    }

    SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;
    if(event->input.type == InputTypePress) {
        switch(event->input.key) {
        case InputKeyUp: {
            const TokenInfoIteratorContext* iterator_context =
                totp_config_get_token_iterator_context(plugin_state);
            totp_roll_value_uint8_t(
                &scene_state->selected_control,
                -1,
                AddNewToken,
                AppSettings,
                RollOverflowBehaviorRoll);
            if(scene_state->selected_control == DeleteToken &&
               totp_token_info_iterator_get_total_count(iterator_context) == 0) {
                scene_state->selected_control--;
            }
            break;
        }
        case InputKeyDown: {
            const TokenInfoIteratorContext* iterator_context =
                totp_config_get_token_iterator_context(plugin_state);
            totp_roll_value_uint8_t(
                &scene_state->selected_control,
                1,
                AddNewToken,
                AppSettings,
                RollOverflowBehaviorRoll);
            if(scene_state->selected_control == DeleteToken &&
               totp_token_info_iterator_get_total_count(iterator_context) == 0) {
                scene_state->selected_control++;
            }
            break;
        }
        case InputKeyRight:
            break;
        case InputKeyLeft:
            break;
        case InputKeyOk:
            break;
        case InputKeyBack: {
            totp_scene_director_activate_scene(plugin_state, TotpSceneGenerateToken);
            break;
        }
        default:
            break;
        }
    } else if(event->input.type == InputTypeRelease && event->input.key == InputKeyOk) {
        switch(scene_state->selected_control) {
        case AddNewToken: {
#ifdef TOTP_UI_ADD_NEW_TOKEN_ENABLED
            totp_scene_director_activate_scene(plugin_state, TotpSceneAddNewToken);
#else
            DialogMessage* message = dialog_message_alloc();
            dialog_message_set_buttons(message, "Back", NULL, NULL);
            dialog_message_set_header(message, "Information", 0, 0, AlignLeft, AlignTop);
            dialog_message_set_text(
                message,
                "Read here\nhttps://t.ly/8ZOtj\n how to add new token",
                SCREEN_WIDTH_CENTER,
                SCREEN_HEIGHT_CENTER,
                AlignCenter,
                AlignCenter);
            dialog_message_show(plugin_state->dialogs_app, message);
            dialog_message_free(message);
#endif
            break;
        }
        case DeleteToken: {
            DialogMessage* message = dialog_message_alloc();
            dialog_message_set_buttons(message, "No", NULL, "Yes");
            dialog_message_set_header(message, "Confirmation", 0, 0, AlignLeft, AlignTop);
            dialog_message_set_text(
                message,
                "Are you sure want to delete?",
                SCREEN_WIDTH_CENTER,
                SCREEN_HEIGHT_CENTER,
                AlignCenter,
                AlignCenter);
            DialogMessageButton dialog_result =
                dialog_message_show(plugin_state->dialogs_app, message);
            dialog_message_free(message);
            TokenInfoIteratorContext* iterator_context =
                totp_config_get_token_iterator_context(plugin_state);
            if(dialog_result == DialogMessageButtonRight &&
               totp_token_info_iterator_get_total_count(iterator_context) > 0) {
                if(!totp_token_info_iterator_remove_current_token_info(iterator_context)) {
                    totp_dialogs_config_updating_error(plugin_state);
                    return false;
                }

                totp_scene_director_activate_scene(plugin_state, TotpSceneGenerateToken);
            }
            break;
        }
        case AppSettings: {
            totp_scene_director_activate_scene(plugin_state, TotpSceneAppSettings);
            break;
        }
        default:
            break;
        }
    }

    return true;
}

void totp_scene_token_menu_deactivate(PluginState* plugin_state) {
    if(plugin_state->current_scene_state == NULL) return;

    free(plugin_state->current_scene_state);
    plugin_state->current_scene_state = NULL;
}
