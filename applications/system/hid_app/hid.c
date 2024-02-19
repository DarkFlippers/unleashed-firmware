#include "hid.h"
#include <extra_profiles/hid_profile.h>
#include <profiles/serial_profile.h>
#include "views.h"
#include <notification/notification_messages.h>
#include <dolphin/dolphin.h>
#include "hid_icons.h"

#define TAG "HidApp"

enum HidDebugSubmenuIndex {
    HidSubmenuIndexKeynote,
    HidSubmenuIndexKeynoteVertical,
    HidSubmenuIndexKeyboard,
    HidSubmenuIndexNumpad,
    HidSubmenuIndexMedia,
    HidSubmenuIndexMusicMacOs,
    HidSubmenuIndexMovie,
    HidSubmenuIndexTikTok,
    HidSubmenuIndexMouse,
    HidSubmenuIndexMouseClicker,
    HidSubmenuIndexMouseJiggler,
    HidSubmenuIndexPushToTalk,
    HidSubmenuIndexRemovePairing,
};

bool hid_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    Hid* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

bool hid_back_event_callback(void* context) {
    furi_assert(context);
    Hid* app = context;
    FURI_LOG_D("HID", "Back event");
    app->view_id = HidViewSubmenu;
    view_dispatcher_switch_to_view(app->view_dispatcher, HidViewSubmenu);
    return true;
}

void bt_hid_remove_pairing(Hid* app) {
    Bt* bt = app->bt;
    bt_disconnect(bt);

    // Wait 2nd core to update nvm storage
    furi_delay_ms(200);

    furi_hal_bt_stop_advertising();

    bt_forget_bonded_devices(bt);

    furi_hal_bt_start_advertising();
}

static void hid_submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    Hid* app = context;
    if(index == HidSubmenuIndexKeynote) {
        app->view_id = HidViewKeynote;
        hid_keynote_set_orientation(app->hid_keynote, false);
        view_dispatcher_switch_to_view(app->view_dispatcher, HidViewKeynote);
    } else if(index == HidSubmenuIndexKeynoteVertical) {
        app->view_id = HidViewKeynote;
        hid_keynote_set_orientation(app->hid_keynote, true);
        view_dispatcher_switch_to_view(app->view_dispatcher, HidViewKeynote);
    } else if(index == HidSubmenuIndexKeyboard) {
        app->view_id = HidViewKeyboard;
        view_dispatcher_switch_to_view(app->view_dispatcher, HidViewKeyboard);
    } else if(index == HidSubmenuIndexNumpad) {
        app->view_id = HidViewNumpad;
        view_dispatcher_switch_to_view(app->view_dispatcher, HidViewNumpad);
    } else if(index == HidSubmenuIndexMedia) {
        app->view_id = HidViewMedia;
        view_dispatcher_switch_to_view(app->view_dispatcher, HidViewMedia);
    } else if(index == HidSubmenuIndexMusicMacOs) {
        app->view_id = HidViewMusicMacOs;
        view_dispatcher_switch_to_view(app->view_dispatcher, HidViewMusicMacOs);
    } else if(index == HidSubmenuIndexMovie) {
        app->view_id = HidViewMovie;
        view_dispatcher_switch_to_view(app->view_dispatcher, HidViewMovie);
    } else if(index == HidSubmenuIndexMouse) {
        app->view_id = HidViewMouse;
        view_dispatcher_switch_to_view(app->view_dispatcher, HidViewMouse);
    } else if(index == HidSubmenuIndexTikTok) {
        app->view_id = BtHidViewTikTok;
        view_dispatcher_switch_to_view(app->view_dispatcher, BtHidViewTikTok);
    } else if(index == HidSubmenuIndexMouseClicker) {
        app->view_id = HidViewMouseClicker;
        view_dispatcher_switch_to_view(app->view_dispatcher, HidViewMouseClicker);
    } else if(index == HidSubmenuIndexMouseJiggler) {
        app->view_id = HidViewMouseJiggler;
        view_dispatcher_switch_to_view(app->view_dispatcher, HidViewMouseJiggler);
    } else if(index == HidSubmenuIndexPushToTalk) {
        app->view_id = HidViewPushToTalkMenu;
        view_dispatcher_switch_to_view(app->view_dispatcher, HidViewPushToTalkMenu);
    } else if(index == HidSubmenuIndexRemovePairing) {
        scene_manager_next_scene(app->scene_manager, HidSceneUnpair);
    }
}

static void bt_hid_connection_status_changed_callback(BtStatus status, void* context) {
    furi_assert(context);
    Hid* hid = context;
    bool connected = (status == BtStatusConnected);
#ifdef HID_TRANSPORT_BLE
    if(connected) {
        notification_internal_message(hid->notifications, &sequence_set_blue_255);
    } else {
        notification_internal_message(hid->notifications, &sequence_reset_blue);
    }
#endif
    hid_keynote_set_connected_status(hid->hid_keynote, connected);
    hid_keyboard_set_connected_status(hid->hid_keyboard, connected);
    hid_numpad_set_connected_status(hid->hid_numpad, connected);
    hid_media_set_connected_status(hid->hid_media, connected);
    hid_music_macos_set_connected_status(hid->hid_music_macos, connected);
    hid_movie_set_connected_status(hid->hid_movie, connected);
    hid_mouse_set_connected_status(hid->hid_mouse, connected);
    hid_mouse_clicker_set_connected_status(hid->hid_mouse_clicker, connected);
    hid_mouse_jiggler_set_connected_status(hid->hid_mouse_jiggler, connected);
    hid_ptt_set_connected_status(hid->hid_ptt, connected);
    hid_tiktok_set_connected_status(hid->hid_tiktok, connected);
}

static uint32_t hid_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

static uint32_t hid_ptt_menu_view(void* context) {
    UNUSED(context);
    return HidViewPushToTalkMenu;
}

Hid* hid_alloc() {
    Hid* app = malloc(sizeof(Hid));

    // Gui
    app->gui = furi_record_open(RECORD_GUI);

    // Bt
    app->bt = furi_record_open(RECORD_BT);

    // Notifications
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    // View dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, hid_back_event_callback);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    // Scene Manager
    app->scene_manager = scene_manager_alloc(&hid_scene_handlers, app);

    // Device Type Submenu view
    app->device_type_submenu = submenu_alloc();
    submenu_add_item(
        app->device_type_submenu, "Keynote", HidSubmenuIndexKeynote, hid_submenu_callback, app);
    submenu_add_item(
        app->device_type_submenu,
        "Keynote Vertical",
        HidSubmenuIndexKeynoteVertical,
        hid_submenu_callback,
        app);
    submenu_add_item(
        app->device_type_submenu, "Keyboard", HidSubmenuIndexKeyboard, hid_submenu_callback, app);
    submenu_add_item(
        app->device_type_submenu, "Numpad", HidSubmenuIndexNumpad, hid_submenu_callback, app);
    submenu_add_item(
        app->device_type_submenu, "Media", HidSubmenuIndexMedia, hid_submenu_callback, app);
    submenu_add_item(
        app->device_type_submenu,
        "Apple Music macOS",
        HidSubmenuIndexMusicMacOs,
        hid_submenu_callback,
        app);
    submenu_add_item(
        app->device_type_submenu, "Movie", HidSubmenuIndexMovie, hid_submenu_callback, app);
    submenu_add_item(
        app->device_type_submenu, "Mouse", HidSubmenuIndexMouse, hid_submenu_callback, app);
    submenu_add_item(
        app->device_type_submenu,
        "TikTok / YT Shorts",
        HidSubmenuIndexTikTok,
        hid_submenu_callback,
        app);
    submenu_add_item(
        app->device_type_submenu,
        "Mouse Clicker",
        HidSubmenuIndexMouseClicker,
        hid_submenu_callback,
        app);
    submenu_add_item(
        app->device_type_submenu,
        "Mouse Jiggler",
        HidSubmenuIndexMouseJiggler,
        hid_submenu_callback,
        app);
    submenu_add_item(
        app->device_type_submenu,
        "PushToTalk",
        HidSubmenuIndexPushToTalk,
        hid_submenu_callback,
        app);
#ifdef HID_TRANSPORT_BLE
    submenu_add_item(
        app->device_type_submenu,
        "Remove Pairing",
        HidSubmenuIndexRemovePairing,
        hid_submenu_callback,
        app);
#endif
    view_set_previous_callback(submenu_get_view(app->device_type_submenu), hid_exit);
    view_dispatcher_add_view(
        app->view_dispatcher, HidViewSubmenu, submenu_get_view(app->device_type_submenu));
    app->view_id = HidViewSubmenu;
    return app;
}

Hid* hid_app_alloc_view(void* context) {
    furi_assert(context);
    Hid* app = context;

    // Dialog view
    app->dialog = dialog_ex_alloc();
    view_dispatcher_add_view(app->view_dispatcher, HidViewDialog, dialog_ex_get_view(app->dialog));

    // Popup view
    app->popup = popup_alloc();
    view_dispatcher_add_view(app->view_dispatcher, HidViewPopup, popup_get_view(app->popup));

    // Keynote view
    app->hid_keynote = hid_keynote_alloc(app);
    view_dispatcher_add_view(
        app->view_dispatcher, HidViewKeynote, hid_keynote_get_view(app->hid_keynote));

    // Keyboard view
    app->hid_keyboard = hid_keyboard_alloc(app);
    view_dispatcher_add_view(
        app->view_dispatcher, HidViewKeyboard, hid_keyboard_get_view(app->hid_keyboard));

    //Numpad keyboard view
    app->hid_numpad = hid_numpad_alloc(app);
    view_dispatcher_add_view(
        app->view_dispatcher, HidViewNumpad, hid_numpad_get_view(app->hid_numpad));

    // Media view
    app->hid_media = hid_media_alloc(app);
    view_dispatcher_add_view(
        app->view_dispatcher, HidViewMedia, hid_media_get_view(app->hid_media));

    // Music MacOs view
    app->hid_music_macos = hid_music_macos_alloc(app);
    view_dispatcher_add_view(
        app->view_dispatcher, HidViewMusicMacOs, hid_music_macos_get_view(app->hid_music_macos));

    // Movie view
    app->hid_movie = hid_movie_alloc(app);
    view_dispatcher_add_view(
        app->view_dispatcher, HidViewMovie, hid_movie_get_view(app->hid_movie));

    // TikTok view
    app->hid_tiktok = hid_tiktok_alloc(app);
    view_dispatcher_add_view(
        app->view_dispatcher, BtHidViewTikTok, hid_tiktok_get_view(app->hid_tiktok));

    // Mouse view
    app->hid_mouse = hid_mouse_alloc(app);
    view_dispatcher_add_view(
        app->view_dispatcher, HidViewMouse, hid_mouse_get_view(app->hid_mouse));

    // Mouse clicker view
    app->hid_mouse_clicker = hid_mouse_clicker_alloc(app);
    view_dispatcher_add_view(
        app->view_dispatcher,
        HidViewMouseClicker,
        hid_mouse_clicker_get_view(app->hid_mouse_clicker));

    // Mouse jiggler view
    app->hid_mouse_jiggler = hid_mouse_jiggler_alloc(app);
    view_dispatcher_add_view(
        app->view_dispatcher,
        HidViewMouseJiggler,
        hid_mouse_jiggler_get_view(app->hid_mouse_jiggler));

    // PushToTalk view
    app->hid_ptt_menu = hid_ptt_menu_alloc(app);
    view_dispatcher_add_view(
        app->view_dispatcher, HidViewPushToTalkMenu, hid_ptt_menu_get_view(app->hid_ptt_menu));
    app->hid_ptt = hid_ptt_alloc(app);
    view_set_previous_callback(hid_ptt_get_view(app->hid_ptt), hid_ptt_menu_view);
    view_dispatcher_add_view(
        app->view_dispatcher, HidViewPushToTalk, hid_ptt_get_view(app->hid_ptt));

    return app;
}

void hid_free(Hid* app) {
    furi_assert(app);

    // Reset notification
#ifdef HID_TRANSPORT_BLE
    notification_internal_message(app->notifications, &sequence_reset_blue);
#endif
    // Free views
    view_dispatcher_remove_view(app->view_dispatcher, HidViewSubmenu);
    submenu_free(app->device_type_submenu);
    view_dispatcher_remove_view(app->view_dispatcher, HidViewDialog);
    dialog_ex_free(app->dialog);
    view_dispatcher_remove_view(app->view_dispatcher, HidViewPopup);
    popup_free(app->popup);
    view_dispatcher_remove_view(app->view_dispatcher, HidViewKeynote);
    hid_keynote_free(app->hid_keynote);
    view_dispatcher_remove_view(app->view_dispatcher, HidViewKeyboard);
    hid_keyboard_free(app->hid_keyboard);
    view_dispatcher_remove_view(app->view_dispatcher, HidViewNumpad);
    hid_numpad_free(app->hid_numpad);
    view_dispatcher_remove_view(app->view_dispatcher, HidViewMedia);
    hid_media_free(app->hid_media);
    view_dispatcher_remove_view(app->view_dispatcher, HidViewMusicMacOs);
    hid_music_macos_free(app->hid_music_macos);
    view_dispatcher_remove_view(app->view_dispatcher, HidViewMovie);
    hid_movie_free(app->hid_movie);
    view_dispatcher_remove_view(app->view_dispatcher, HidViewMouse);
    hid_mouse_free(app->hid_mouse);
    view_dispatcher_remove_view(app->view_dispatcher, HidViewMouseClicker);
    hid_mouse_clicker_free(app->hid_mouse_clicker);
    view_dispatcher_remove_view(app->view_dispatcher, HidViewMouseJiggler);
    hid_mouse_jiggler_free(app->hid_mouse_jiggler);
    view_dispatcher_remove_view(app->view_dispatcher, HidViewPushToTalkMenu);
    hid_ptt_menu_free(app->hid_ptt_menu);
    view_dispatcher_remove_view(app->view_dispatcher, HidViewPushToTalk);
    hid_ptt_free(app->hid_ptt);
    view_dispatcher_remove_view(app->view_dispatcher, BtHidViewTikTok);
    hid_tiktok_free(app->hid_tiktok);
    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);

    // Close records
    furi_record_close(RECORD_GUI);
    app->gui = NULL;
    furi_record_close(RECORD_NOTIFICATION);
    app->notifications = NULL;
    furi_record_close(RECORD_BT);
    app->bt = NULL;

    // Free rest
    free(app);
}

int32_t hid_usb_app(void* p) {
    UNUSED(p);
    Hid* app = hid_alloc();
    app = hid_app_alloc_view(app);
    FURI_LOG_D("HID", "Starting as USB app");

    FuriHalUsbInterface* usb_mode_prev = furi_hal_usb_get_config();
    furi_hal_usb_unlock();
    furi_check(furi_hal_usb_set_config(&usb_hid, NULL) == true);

    bt_hid_connection_status_changed_callback(BtStatusConnected, app);

    dolphin_deed(DolphinDeedPluginStart);

    scene_manager_next_scene(app->scene_manager, HidSceneMain);

    view_dispatcher_run(app->view_dispatcher);

    furi_hal_usb_set_config(usb_mode_prev, NULL);

    hid_free(app);

    return 0;
}

int32_t hid_ble_app(void* p) {
    UNUSED(p);
    Hid* app = hid_alloc();
    app = hid_app_alloc_view(app);

    FURI_LOG_D("HID", "Starting as BLE app");

    bt_disconnect(app->bt);

    // Wait 2nd core to update nvm storage
    furi_delay_ms(200);

    // Migrate data from old sd-card folder
    Storage* storage = furi_record_open(RECORD_STORAGE);

    storage_common_migrate(
        storage,
        EXT_PATH("apps/Tools/" HID_BT_KEYS_STORAGE_NAME),
        APP_DATA_PATH(HID_BT_KEYS_STORAGE_NAME));

    bt_keys_storage_set_storage_path(app->bt, APP_DATA_PATH(HID_BT_KEYS_STORAGE_NAME));

    furi_record_close(RECORD_STORAGE);

    app->ble_hid_profile = bt_profile_start(app->bt, ble_profile_hid, NULL);

    furi_check(app->ble_hid_profile);

    furi_hal_bt_start_advertising();
    bt_set_status_changed_callback(app->bt, bt_hid_connection_status_changed_callback, app);

    dolphin_deed(DolphinDeedPluginStart);

    scene_manager_next_scene(app->scene_manager, HidSceneMain);

    view_dispatcher_run(app->view_dispatcher);

    bt_set_status_changed_callback(app->bt, NULL, NULL);

    bt_disconnect(app->bt);

    // Wait 2nd core to update nvm storage
    furi_delay_ms(200);

    bt_keys_storage_set_default_path(app->bt);

    furi_check(bt_profile_restore_default(app->bt));

    hid_free(app);

    return 0;
}
