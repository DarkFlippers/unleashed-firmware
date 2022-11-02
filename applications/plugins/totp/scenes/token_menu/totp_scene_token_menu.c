#include "totp_scene_token_menu.h"
#include <gui/gui.h>
#include <dialogs/dialogs.h>
#include "../../services/ui/ui_controls.h"
#include "../../services/ui/constants.h"
#include "../scene_director.h"
#include "../../services/config/config.h"
#include "../../services/list/list.h"
#include "../../types/token_info.h"
#include "../generate_token/totp_scene_generate_token.h"
#include "../add_new_token/totp_scene_add_new_token.h"
#include "../app_settings/totp_app_settings.h"

#define SCREEN_HEIGHT_THIRD (SCREEN_HEIGHT / 3)
#define SCREEN_HEIGHT_THIRD_CENTER (SCREEN_HEIGHT_THIRD >> 1)

typedef enum { AddNewToken, DeleteToken, AppSettings } Control;

typedef struct {
    Control selected_control;
    int16_t current_token_index;
} SceneState;

void totp_scene_token_menu_init(PluginState* plugin_state) {
    UNUSED(plugin_state);
}

void totp_scene_token_menu_activate(
    PluginState* plugin_state,
    const TokenMenuSceneContext* context) {
    SceneState* scene_state = malloc(sizeof(SceneState));
    plugin_state->current_scene_state = scene_state;
    if(context != NULL) {
        scene_state->current_token_index = context->current_token_index;
    } else {
        scene_state->current_token_index = -1;
    }
}

void totp_scene_token_menu_render(Canvas* const canvas, PluginState* plugin_state) {
    SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;
    if(scene_state->current_token_index < 0) {
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

bool totp_scene_token_menu_handle_event(PluginEvent* const event, PluginState* plugin_state) {
    if(event->type == EventTypeKey) {
        SceneState* scene_state = (SceneState*)plugin_state->current_scene_state;
        if(event->input.type == InputTypePress) {
            switch(event->input.key) {
            case InputKeyUp:
                if(scene_state->selected_control > AddNewToken) {
                    scene_state->selected_control--;
                    if(scene_state->selected_control == DeleteToken &&
                       scene_state->current_token_index < 0) {
                        scene_state->selected_control--;
                    }
                } else {
                    scene_state->selected_control = AppSettings;
                }
                break;
            case InputKeyDown:
                if(scene_state->selected_control < AppSettings) {
                    scene_state->selected_control++;
                    if(scene_state->selected_control == DeleteToken &&
                       scene_state->current_token_index < 0) {
                        scene_state->selected_control++;
                    }
                } else {
                    scene_state->selected_control = AddNewToken;
                }
                break;
            case InputKeyRight:
                break;
            case InputKeyLeft:
                break;
            case InputKeyOk:
                switch(scene_state->selected_control) {
                case AddNewToken: {
                    TokenAddEditSceneContext add_new_token_scene_context = {
                        .current_token_index = scene_state->current_token_index};
                    totp_scene_director_activate_scene(
                        plugin_state, TotpSceneAddNewToken, &add_new_token_scene_context);
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
                        dialog_message_show(plugin_state->dialogs, message);
                    dialog_message_free(message);
                    if(dialog_result == DialogMessageButtonRight) {
                        ListNode* list_node = list_element_at(
                            plugin_state->tokens_list, scene_state->current_token_index);

                        TokenInfo* tokenInfo = list_node->data;
                        token_info_free(tokenInfo);
                        plugin_state->tokens_list =
                            list_remove(plugin_state->tokens_list, list_node);
                        plugin_state->tokens_count--;

                        totp_full_save_config_file(plugin_state);
                        totp_scene_director_activate_scene(
                            plugin_state, TotpSceneGenerateToken, NULL);
                    }
                    break;
                }
                case AppSettings: {
                    if(scene_state->current_token_index >= 0) {
                        AppSettingsSceneContext app_settings_context = {
                            .current_token_index = scene_state->current_token_index};
                        totp_scene_director_activate_scene(
                            plugin_state, TotpSceneAppSettings, &app_settings_context);
                    } else {
                        totp_scene_director_activate_scene(
                            plugin_state, TotpSceneAppSettings, NULL);
                    }
                    break;
                }
                }
                break;
            case InputKeyBack: {
                if(scene_state->current_token_index >= 0) {
                    GenerateTokenSceneContext generate_scene_context = {
                        .current_token_index = scene_state->current_token_index};
                    totp_scene_director_activate_scene(
                        plugin_state, TotpSceneGenerateToken, &generate_scene_context);
                } else {
                    totp_scene_director_activate_scene(plugin_state, TotpSceneGenerateToken, NULL);
                }
                break;
            }
            default:
                break;
            }
        }
    }
    return true;
}

void totp_scene_token_menu_deactivate(PluginState* plugin_state) {
    if(plugin_state->current_scene_state == NULL) return;

    free(plugin_state->current_scene_state);
    plugin_state->current_scene_state = NULL;
}

void totp_scene_token_menu_free(PluginState* plugin_state) {
    UNUSED(plugin_state);
}
