#include "../types/common.h"
#include "../config/app/config.h"
#include "scene_director.h"
#include "scenes/authenticate/totp_scene_authenticate.h"
#include "scenes/generate_token/totp_scene_generate_token.h"
#ifdef TOTP_UI_ADD_NEW_TOKEN_ENABLED
#include "scenes/add_new_token/totp_scene_add_new_token.h"
#endif
#include "scenes/token_menu/totp_scene_token_menu.h"
#include "scenes/app_settings/totp_app_settings.h"
#include "scenes/standby/standby.h"

void totp_scene_director_activate_scene(PluginState* const plugin_state, Scene scene) {
    totp_scene_director_deactivate_active_scene(plugin_state);
    switch(scene) {
    case TotpSceneGenerateToken:
        totp_scene_generate_token_activate(plugin_state);
        break;
    case TotpSceneAuthentication:
        totp_scene_authenticate_activate(plugin_state);
        break;
#ifdef TOTP_UI_ADD_NEW_TOKEN_ENABLED
    case TotpSceneAddNewToken:
        totp_scene_add_new_token_activate(plugin_state);
        break;
#endif
    case TotpSceneTokenMenu:
        totp_scene_token_menu_activate(plugin_state);
        break;
    case TotpSceneAppSettings:
        totp_scene_app_settings_activate(plugin_state);
        break;
    case TotpSceneNone:
    case TotpSceneStandby:
        break;
    default:
        break;
    }

    plugin_state->current_scene = scene;
}

void totp_scene_director_deactivate_active_scene(PluginState* const plugin_state) {
    Scene current_scene = plugin_state->current_scene;
    plugin_state->current_scene = TotpSceneNone;
    switch(current_scene) {
    case TotpSceneGenerateToken:
        totp_scene_generate_token_deactivate(plugin_state);
        break;
    case TotpSceneAuthentication:
        totp_scene_authenticate_deactivate(plugin_state);
        break;
#ifdef TOTP_UI_ADD_NEW_TOKEN_ENABLED
    case TotpSceneAddNewToken:
        totp_scene_add_new_token_deactivate(plugin_state);
        break;
#endif
    case TotpSceneTokenMenu:
        totp_scene_token_menu_deactivate(plugin_state);
        break;
    case TotpSceneAppSettings:
        totp_scene_app_settings_deactivate(plugin_state);
        break;
    case TotpSceneNone:
    case TotpSceneStandby:
        break;
    default:
        break;
    }
}

void totp_scene_director_render(Canvas* const canvas, PluginState* const plugin_state) {
    switch(plugin_state->current_scene) {
    case TotpSceneGenerateToken:
        totp_scene_generate_token_render(canvas, plugin_state);
        break;
    case TotpSceneAuthentication:
        totp_scene_authenticate_render(canvas, plugin_state);
        break;
#ifdef TOTP_UI_ADD_NEW_TOKEN_ENABLED
    case TotpSceneAddNewToken:
        totp_scene_add_new_token_render(canvas, plugin_state);
        break;
#endif
    case TotpSceneTokenMenu:
        totp_scene_token_menu_render(canvas, plugin_state);
        break;
    case TotpSceneAppSettings:
        totp_scene_app_settings_render(canvas, plugin_state);
        break;
    case TotpSceneNone:
        break;
    case TotpSceneStandby:
        totp_scene_standby_render(canvas);
        break;
    default:
        break;
    }
}

bool totp_scene_director_handle_event(PluginEvent* const event, PluginState* const plugin_state) {
    bool processing = true;
    switch(plugin_state->current_scene) {
    case TotpSceneGenerateToken:
        processing = totp_scene_generate_token_handle_event(event, plugin_state);
        break;
    case TotpSceneAuthentication:
        processing = totp_scene_authenticate_handle_event(event, plugin_state);
        break;
#ifdef TOTP_UI_ADD_NEW_TOKEN_ENABLED
    case TotpSceneAddNewToken:
        processing = totp_scene_add_new_token_handle_event(event, plugin_state);
        break;
#endif
    case TotpSceneTokenMenu:
        processing = totp_scene_token_menu_handle_event(event, plugin_state);
        break;
    case TotpSceneAppSettings:
        processing = totp_scene_app_settings_handle_event(event, plugin_state);
        break;
    case TotpSceneNone:
    case TotpSceneStandby:
        break;
    default:
        break;
    }

    return processing;
}

void totp_scene_director_force_redraw(PluginState* const plugin_state) {
    PluginEvent event = {.type = EventForceRedraw};
    furi_message_queue_put(plugin_state->event_queue, &event, FuriWaitForever);
}