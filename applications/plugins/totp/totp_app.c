#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <dialogs/dialogs.h>
#include <stdlib.h>
#include <flipper_format/flipper_format.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include "services/base32/base32.h"
#include "services/list/list.h"
#include "services/config/config.h"
#include "types/plugin_state.h"
#include "types/token_info.h"
#include "types/plugin_event.h"
#include "types/event_type.h"
#include "types/common.h"
#include "scenes/scene_director.h"

#define IDLE_TIMEOUT 60000

static void render_callback(Canvas* const canvas, void* ctx) {
    PluginState* plugin_state = acquire_mutex((ValueMutex*)ctx, 25);
    if (plugin_state != NULL && !plugin_state->changing_scene) {
        totp_scene_director_render(canvas, plugin_state);
    }

    release_mutex((ValueMutex*)ctx, plugin_state);
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void totp_state_init(PluginState* const plugin_state) {
    plugin_state->gui = furi_record_open(RECORD_GUI);
    plugin_state->notification = furi_record_open(RECORD_NOTIFICATION);
    plugin_state->dialogs = furi_record_open(RECORD_DIALOGS);
    totp_config_file_load_base(plugin_state);

    totp_scene_director_init_scenes(plugin_state);
    totp_scene_director_activate_scene(plugin_state, TotpSceneAuthentication, NULL);
}

static void dispose_plugin_state(PluginState* plugin_state) {
    totp_scene_director_deactivate_active_scene(plugin_state);

    totp_scene_director_dispose(plugin_state);

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_DIALOGS);

    ListNode* node = plugin_state->tokens_list;
    ListNode* tmp;
    while (node != NULL) {
        tmp = node->next;
        TokenInfo* tokenInfo = (TokenInfo*)node->data;
        token_info_free(tokenInfo);
        free(node);
        node = tmp;
    }

    if (plugin_state->crypto_verify_data != NULL) {
        free(plugin_state->crypto_verify_data);
    }
    free(plugin_state);
}

int32_t totp_app() {
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));
    PluginState* plugin_state = malloc(sizeof(PluginState));

    totp_state_init(plugin_state);

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, plugin_state, sizeof(PluginState))) {
        FURI_LOG_E(LOGGING_TAG, "Cannot create mutex\r\n");
        dispose_plugin_state(plugin_state);
        return 255;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Open GUI and register view_port
    gui_add_view_port(plugin_state->gui, view_port, GuiLayerFullscreen);

    PluginEvent event;
    bool processing = true;
    uint32_t last_user_interaction_time = furi_get_tick();
    while(processing) {
        if (plugin_state->changing_scene) continue;
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);

        PluginState* plugin_state = (PluginState*)acquire_mutex_block(&state_mutex);

        if(event_status == FuriStatusOk) {
            if (event.type == EventTypeKey) {
                last_user_interaction_time = furi_get_tick();
            }

            processing = totp_scene_director_handle_event(&event, plugin_state);
        } else if (plugin_state->current_scene != TotpSceneAuthentication && furi_get_tick() - last_user_interaction_time > IDLE_TIMEOUT) {
            totp_scene_director_activate_scene(plugin_state, TotpSceneAuthentication, NULL);
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, plugin_state);
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(plugin_state->gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    delete_mutex(&state_mutex);
    dispose_plugin_state(plugin_state);
    return 0;
}
