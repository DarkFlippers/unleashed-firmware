#include "../dap_gui_i.h"
#include "../../dap_link.h"

typedef struct {
    DapState dap_state;
    bool dap_active;
    bool tx_active;
    bool rx_active;
} DapSceneMainState;

static bool process_dap_state(DapGuiApp* app) {
    DapSceneMainState* state =
        (DapSceneMainState*)scene_manager_get_scene_state(app->scene_manager, DapSceneMain);
    if(state == NULL) return true;

    DapState* prev_state = &state->dap_state;
    DapState next_state;
    dap_app_get_state(app->dap_app, &next_state);
    bool need_to_update = false;

    if(prev_state->dap_mode != next_state.dap_mode) {
        switch(next_state.dap_mode) {
        case DapModeDisconnected:
            dap_main_view_set_mode(app->main_view, DapMainViewModeDisconnected);
            notification_message(app->notifications, &sequence_blink_stop);
            break;
        case DapModeSWD:
            dap_main_view_set_mode(app->main_view, DapMainViewModeSWD);
            notification_message(app->notifications, &sequence_blink_start_blue);
            break;
        case DapModeJTAG:
            dap_main_view_set_mode(app->main_view, DapMainViewModeJTAG);
            notification_message(app->notifications, &sequence_blink_start_magenta);
            break;
        }
        need_to_update = true;
    }

    if(prev_state->dap_version != next_state.dap_version) {
        switch(next_state.dap_version) {
        case DapVersionUnknown:
            dap_main_view_set_version(app->main_view, DapMainViewVersionUnknown);
            break;
        case DapVersionV1:
            dap_main_view_set_version(app->main_view, DapMainViewVersionV1);
            break;
        case DapVersionV2:
            dap_main_view_set_version(app->main_view, DapMainViewVersionV2);
            break;
        }
        need_to_update = true;
    }

    if(prev_state->usb_connected != next_state.usb_connected) {
        dap_main_view_set_usb_connected(app->main_view, next_state.usb_connected);
        need_to_update = true;
    }

    if(prev_state->dap_counter != next_state.dap_counter) {
        if(!state->dap_active) {
            state->dap_active = true;
            dap_main_view_set_dap(app->main_view, state->dap_active);
            need_to_update = true;
        }
    } else {
        if(state->dap_active) {
            state->dap_active = false;
            dap_main_view_set_dap(app->main_view, state->dap_active);
            need_to_update = true;
        }
    }

    if(prev_state->cdc_baudrate != next_state.cdc_baudrate) {
        dap_main_view_set_baudrate(app->main_view, next_state.cdc_baudrate);
        need_to_update = true;
    }

    if(prev_state->cdc_tx_counter != next_state.cdc_tx_counter) {
        if(!state->tx_active) {
            state->tx_active = true;
            dap_main_view_set_tx(app->main_view, state->tx_active);
            need_to_update = true;
            notification_message(app->notifications, &sequence_blink_start_red);
        }
    } else {
        if(state->tx_active) {
            state->tx_active = false;
            dap_main_view_set_tx(app->main_view, state->tx_active);
            need_to_update = true;
            notification_message(app->notifications, &sequence_blink_stop);
        }
    }

    if(prev_state->cdc_rx_counter != next_state.cdc_rx_counter) {
        if(!state->rx_active) {
            state->rx_active = true;
            dap_main_view_set_rx(app->main_view, state->rx_active);
            need_to_update = true;
            notification_message(app->notifications, &sequence_blink_start_green);
        }
    } else {
        if(state->rx_active) {
            state->rx_active = false;
            dap_main_view_set_rx(app->main_view, state->rx_active);
            need_to_update = true;
            notification_message(app->notifications, &sequence_blink_stop);
        }
    }

    if(need_to_update) {
        dap_main_view_update(app->main_view);
    }

    *prev_state = next_state;
    return true;
}

static void dap_scene_main_on_left(void* context) {
    DapGuiApp* app = (DapGuiApp*)context;
    view_dispatcher_send_custom_event(app->view_dispatcher, DapAppCustomEventConfig);
}

void dap_scene_main_on_enter(void* context) {
    DapGuiApp* app = context;
    DapSceneMainState* state = malloc(sizeof(DapSceneMainState));
    dap_main_view_set_left_callback(app->main_view, dap_scene_main_on_left, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, DapGuiAppViewMainView);
    scene_manager_set_scene_state(app->scene_manager, DapSceneMain, (uint32_t)state);
}

bool dap_scene_main_on_event(void* context, SceneManagerEvent event) {
    DapGuiApp* app = context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DapAppCustomEventConfig) {
            scene_manager_next_scene(app->scene_manager, DapSceneConfig);
            return true;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        return process_dap_state(app);
    }

    return false;
}

void dap_scene_main_on_exit(void* context) {
    DapGuiApp* app = context;
    DapSceneMainState* state =
        (DapSceneMainState*)scene_manager_get_scene_state(app->scene_manager, DapSceneMain);
    scene_manager_set_scene_state(app->scene_manager, DapSceneMain, (uint32_t)NULL);
    FURI_SW_MEMBARRIER();
    free(state);
    notification_message(app->notifications, &sequence_blink_stop);
}