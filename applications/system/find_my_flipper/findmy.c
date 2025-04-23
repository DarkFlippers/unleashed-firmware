#include "findmy_i.h"

static bool findmy_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    FindMy* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool findmy_back_event_callback(void* context) {
    furi_assert(context);
    FindMy* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static FindMy* findmy_app_alloc() {
    FindMy* app = malloc(sizeof(FindMy));

    app->gui = furi_record_open(RECORD_GUI);
    app->storage = furi_record_open(RECORD_STORAGE);
    app->dialogs = furi_record_open(RECORD_DIALOGS);

    app->view_dispatcher = view_dispatcher_alloc();

    app->scene_manager = scene_manager_alloc(&findmy_scene_handlers, app);

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, findmy_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, findmy_back_event_callback);

    app->findmy_main = findmy_main_alloc(app);
    view_dispatcher_add_view(
        app->view_dispatcher, FindMyViewMain, findmy_main_get_view(app->findmy_main));

    app->byte_input = byte_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, FindMyViewByteInput, byte_input_get_view(app->byte_input));

    app->var_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        FindMyViewVarItemList,
        variable_item_list_get_view(app->var_item_list));

    app->popup = popup_alloc();
    view_dispatcher_add_view(app->view_dispatcher, FindMyViewPopup, popup_get_view(app->popup));

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    findmy_state_load(&app->state);
    findmy_state_apply(&app->state);

    findmy_main_update_active(app->findmy_main, app->state.beacon_active);
    findmy_main_update_interval(app->findmy_main, app->state.broadcast_interval);
    findmy_main_toggle_mac(app->findmy_main, app->state.show_mac);
    findmy_main_update_mac(app->findmy_main, app->state.mac);
    findmy_main_update_type(app->findmy_main, app->state.tag_type);

    return app;
}

static void findmy_app_free(FindMy* app) {
    furi_assert(app);

    view_dispatcher_remove_view(app->view_dispatcher, FindMyViewPopup);
    popup_free(app->popup);

    view_dispatcher_remove_view(app->view_dispatcher, FindMyViewVarItemList);
    variable_item_list_free(app->var_item_list);

    view_dispatcher_remove_view(app->view_dispatcher, FindMyViewByteInput);
    byte_input_free(app->byte_input);

    view_dispatcher_remove_view(app->view_dispatcher, FindMyViewMain);
    findmy_main_free(app->findmy_main);

    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t findmy_main(void* p) {
    UNUSED(p);
    FindMy* app = findmy_app_alloc();

    scene_manager_next_scene(app->scene_manager, FindMySceneMain);

    view_dispatcher_run(app->view_dispatcher);

    findmy_app_free(app);
    return 0;
}

void findmy_change_broadcast_interval(FindMy* app, uint8_t value) {
    if(value > 10 || value < 1) {
        return;
    }
    app->state.broadcast_interval = value;
    findmy_main_update_interval(app->findmy_main, app->state.broadcast_interval);
    findmy_state_save_and_apply(&app->state);
}

void findmy_change_transmit_power(FindMy* app, uint8_t value) {
    if(value > 6) {
        return;
    }
    app->state.transmit_power = value;
    findmy_state_save_and_apply(&app->state);
}

void findmy_toggle_show_mac(FindMy* app, bool show_mac) {
    app->state.show_mac = show_mac;
    findmy_main_toggle_mac(app->findmy_main, app->state.show_mac);
    findmy_state_save_and_apply(&app->state);
}

void findmy_toggle_beacon(FindMy* app) {
    app->state.beacon_active = !app->state.beacon_active;
    findmy_state_save_and_apply(&app->state);
    findmy_main_update_active(app->findmy_main, app->state.beacon_active);
}

void findmy_set_tag_type(FindMy* app, FindMyType type) {
    app->state.tag_type = type;
    findmy_state_save_and_apply(&app->state);
    findmy_main_update_type(app->findmy_main, type);
}

void findmy_reverse_mac_addr(uint8_t mac_addr[GAP_MAC_ADDR_SIZE]) {
    uint8_t tmp;
    for(size_t i = 0; i < GAP_MAC_ADDR_SIZE / 2; i++) {
        tmp = mac_addr[i];
        mac_addr[i] = mac_addr[GAP_MAC_ADDR_SIZE - 1 - i];
        mac_addr[GAP_MAC_ADDR_SIZE - 1 - i] = tmp;
    }
}
