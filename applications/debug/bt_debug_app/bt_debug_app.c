#include "bt_debug_app.h"
#include <furi_hal_bt.h>

#define TAG "BtDebugApp"

enum BtDebugSubmenuIndex {
    BtDebugSubmenuIndexCarrierTest,
    BtDebugSubmenuIndexPacketTest,
};

void bt_debug_submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    BtDebugApp* app = context;
    if(index == BtDebugSubmenuIndexCarrierTest) {
        view_dispatcher_switch_to_view(app->view_dispatcher, BtDebugAppViewCarrierTest);
    } else if(index == BtDebugSubmenuIndexPacketTest) {
        view_dispatcher_switch_to_view(app->view_dispatcher, BtDebugAppViewPacketTest);
    }
}

uint32_t bt_debug_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

uint32_t bt_debug_start_view(void* context) {
    UNUSED(context);
    return BtDebugAppViewSubmenu;
}

BtDebugApp* bt_debug_app_alloc() {
    BtDebugApp* app = malloc(sizeof(BtDebugApp));

    // Gui
    app->gui = furi_record_open(RECORD_GUI);

    // View dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Views
    app->submenu = submenu_alloc();
    submenu_add_item(
        app->submenu,
        "Carrier test",
        BtDebugSubmenuIndexCarrierTest,
        bt_debug_submenu_callback,
        app);
    submenu_add_item(
        app->submenu, "Packet test", BtDebugSubmenuIndexPacketTest, bt_debug_submenu_callback, app);
    view_set_previous_callback(submenu_get_view(app->submenu), bt_debug_exit);
    view_dispatcher_add_view(
        app->view_dispatcher, BtDebugAppViewSubmenu, submenu_get_view(app->submenu));
    app->bt_carrier_test = bt_carrier_test_alloc();
    view_set_previous_callback(
        bt_carrier_test_get_view(app->bt_carrier_test), bt_debug_start_view);
    view_dispatcher_add_view(
        app->view_dispatcher,
        BtDebugAppViewCarrierTest,
        bt_carrier_test_get_view(app->bt_carrier_test));
    app->bt_packet_test = bt_packet_test_alloc();
    view_set_previous_callback(bt_packet_test_get_view(app->bt_packet_test), bt_debug_start_view);
    view_dispatcher_add_view(
        app->view_dispatcher,
        BtDebugAppViewPacketTest,
        bt_packet_test_get_view(app->bt_packet_test));

    // Switch to menu
    view_dispatcher_switch_to_view(app->view_dispatcher, BtDebugAppViewSubmenu);

    return app;
}

void bt_debug_app_free(BtDebugApp* app) {
    furi_assert(app);

    // Free views
    view_dispatcher_remove_view(app->view_dispatcher, BtDebugAppViewSubmenu);
    submenu_free(app->submenu);
    view_dispatcher_remove_view(app->view_dispatcher, BtDebugAppViewCarrierTest);
    bt_carrier_test_free(app->bt_carrier_test);
    view_dispatcher_remove_view(app->view_dispatcher, BtDebugAppViewPacketTest);
    bt_packet_test_free(app->bt_packet_test);
    view_dispatcher_free(app->view_dispatcher);

    // Close gui record
    furi_record_close(RECORD_GUI);
    app->gui = NULL;

    // Free rest
    free(app);
}

int32_t bt_debug_app(void* p) {
    UNUSED(p);
    if(!furi_hal_bt_is_testing_supported()) {
        FURI_LOG_E(TAG, "Incorrect radio stack: radio testing features are absent.");
        DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
        dialog_message_show_storage_error(dialogs, "Incorrect\nRadioStack");
        return 255;
    }

    BtDebugApp* app = bt_debug_app_alloc();
    // Was bt active?
    const bool was_active = furi_hal_bt_is_active();
    // Stop advertising
    furi_hal_bt_stop_advertising();

    view_dispatcher_run(app->view_dispatcher);

    // Restart advertising
    if(was_active) {
        furi_hal_bt_start_advertising();
    }
    bt_debug_app_free(app);
    return 0;
}
