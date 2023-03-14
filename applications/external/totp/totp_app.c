#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <dialogs/dialogs.h>
#include <stdlib.h>
#include <flipper_format/flipper_format.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <dolphin/dolphin.h>
#include "services/config/config.h"
#include "types/plugin_state.h"
#include "types/token_info.h"
#include "types/plugin_event.h"
#include "types/event_type.h"
#include "types/common.h"
#include "ui/scene_director.h"
#include "ui/constants.h"
#include "ui/common_dialogs.h"
#include "services/crypto/crypto.h"
#include "cli/cli.h"

#define IDLE_TIMEOUT 60000

static void render_callback(Canvas* const canvas, void* ctx) {
    furi_assert(ctx);
    PluginState* plugin_state = ctx;
    if(furi_mutex_acquire(plugin_state->mutex, 25) == FuriStatusOk) {
        totp_scene_director_render(canvas, plugin_state);
        furi_mutex_release(plugin_state->mutex);
    }
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static bool totp_activate_initial_scene(PluginState* const plugin_state) {
    if(plugin_state->crypto_verify_data == NULL) {
        DialogMessage* message = dialog_message_alloc();
        dialog_message_set_buttons(message, "No", NULL, "Yes");
        dialog_message_set_text(
            message,
            "Would you like to setup PIN?",
            SCREEN_WIDTH_CENTER,
            SCREEN_HEIGHT_CENTER,
            AlignCenter,
            AlignCenter);
        DialogMessageButton dialog_result =
            dialog_message_show(plugin_state->dialogs_app, message);
        dialog_message_free(message);
        if(dialog_result == DialogMessageButtonRight) {
            totp_scene_director_activate_scene(plugin_state, TotpSceneAuthentication, NULL);
        } else {
            if(!totp_crypto_seed_iv(plugin_state, NULL, 0)) {
                totp_dialogs_config_loading_error(plugin_state);
                return false;
            }
            totp_scene_director_activate_scene(plugin_state, TotpSceneGenerateToken, NULL);
        }
    } else if(plugin_state->pin_set) {
        totp_scene_director_activate_scene(plugin_state, TotpSceneAuthentication, NULL);
    } else {
        if(!totp_crypto_seed_iv(plugin_state, NULL, 0)) {
            totp_dialogs_config_loading_error(plugin_state);
            return false;
        }
        if(totp_crypto_verify_key(plugin_state)) {
            totp_scene_director_activate_scene(plugin_state, TotpSceneGenerateToken, NULL);
        } else {
            FURI_LOG_E(
                LOGGING_TAG,
                "Digital signature verification failed. Looks like conf file was created on another flipper and can't be used on any other");
            DialogMessage* message = dialog_message_alloc();
            dialog_message_set_buttons(message, "Exit", NULL, NULL);
            dialog_message_set_text(
                message,
                "Digital signature verification failed",
                SCREEN_WIDTH_CENTER,
                SCREEN_HEIGHT_CENTER,
                AlignCenter,
                AlignCenter);
            dialog_message_show(plugin_state->dialogs_app, message);
            dialog_message_free(message);
            return false;
        }
    }

    return true;
}

static bool totp_plugin_state_init(PluginState* const plugin_state) {
    plugin_state->gui = furi_record_open(RECORD_GUI);
    plugin_state->notification_app = furi_record_open(RECORD_NOTIFICATION);
    plugin_state->dialogs_app = furi_record_open(RECORD_DIALOGS);

    if(totp_config_file_load_base(plugin_state) != TotpConfigFileOpenSuccess) {
        totp_dialogs_config_loading_error(plugin_state);
        return false;
    }

    plugin_state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    if(plugin_state->mutex == NULL) {
        FURI_LOG_E(LOGGING_TAG, "Cannot create mutex\r\n");
        return false;
    }

    return true;
}

static void totp_plugin_state_free(PluginState* plugin_state) {
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_DIALOGS);

    ListNode* node = plugin_state->tokens_list;
    ListNode* tmp;
    while(node != NULL) {
        tmp = node->next;
        TokenInfo* tokenInfo = node->data;
        token_info_free(tokenInfo);
        free(node);
        node = tmp;
    }

    if(plugin_state->crypto_verify_data != NULL) {
        free(plugin_state->crypto_verify_data);
    }

    furi_mutex_free(plugin_state->mutex);
    free(plugin_state);
}

int32_t totp_app() {
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));
    PluginState* plugin_state = malloc(sizeof(PluginState));
    furi_check(plugin_state != NULL);

    if(!totp_plugin_state_init(plugin_state)) {
        FURI_LOG_E(LOGGING_TAG, "App state initialization failed\r\n");
        totp_plugin_state_free(plugin_state);
        return 254;
    }

    TotpCliContext* cli_context = totp_cli_register_command_handler(plugin_state, event_queue);
    totp_scene_director_init_scenes(plugin_state);
    if(!totp_activate_initial_scene(plugin_state)) {
        FURI_LOG_E(LOGGING_TAG, "An error ocurred during activating initial scene\r\n");
        totp_plugin_state_free(plugin_state);
        return 253;
    }

    // Affecting dolphin level
    DOLPHIN_DEED(DolphinDeedPluginStart);

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, plugin_state);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Open GUI and register view_port
    gui_add_view_port(plugin_state->gui, view_port, GuiLayerFullscreen);

    PluginEvent event;
    bool processing = true;
    uint32_t last_user_interaction_time = furi_get_tick();
    while(processing) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);

        if(furi_mutex_acquire(plugin_state->mutex, FuriWaitForever) == FuriStatusOk) {
            if(event_status == FuriStatusOk) {
                if(event.type == EventTypeKey) {
                    last_user_interaction_time = furi_get_tick();
                }

                if(event.type == EventForceCloseApp) {
                    processing = false;
                } else {
                    processing = totp_scene_director_handle_event(&event, plugin_state);
                }
            } else if(
                plugin_state->pin_set && plugin_state->current_scene != TotpSceneAuthentication &&
                furi_get_tick() - last_user_interaction_time > IDLE_TIMEOUT) {
                totp_scene_director_activate_scene(plugin_state, TotpSceneAuthentication, NULL);
            }

            view_port_update(view_port);
            furi_mutex_release(plugin_state->mutex);
        }
    }

    totp_cli_unregister_command_handler(cli_context);
    totp_scene_director_deactivate_active_scene(plugin_state);
    totp_scene_director_dispose(plugin_state);

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(plugin_state->gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    totp_plugin_state_free(plugin_state);
    return 0;
}
