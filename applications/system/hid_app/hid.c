#include "hid.h"
#include "views.h"
#include <notification/notification_messages.h>
#include <dolphin/dolphin.h>

#define TAG "HidApp"

enum HidDebugSubmenuIndex {
    HidSubmenuIndexKeynote,
    HidSubmenuIndexKeynoteVertical,
    HidSubmenuIndexKeyboard,
    HidSubmenuIndexNumpad,
    HidSubmenuIndexMedia,
    HidSubmenuIndexMusicMacOs,
    HidSubmenuIndexMovie,
    HidSubmenuIndexTikShorts,
    HidSubmenuIndexMouse,
    HidSubmenuIndexMouseClicker,
    HidSubmenuIndexMouseJiggler,
    HidSubmenuIndexPushToTalk,
};

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
    } else if(index == HidSubmenuIndexTikShorts) {
        app->view_id = BtHidViewTikShorts;
        view_dispatcher_switch_to_view(app->view_dispatcher, BtHidViewTikShorts);
    } else if(index == HidSubmenuIndexMouseClicker) {
        app->view_id = HidViewMouseClicker;
        view_dispatcher_switch_to_view(app->view_dispatcher, HidViewMouseClicker);
    } else if(index == HidSubmenuIndexMouseJiggler) {
        app->view_id = HidViewMouseJiggler;
        view_dispatcher_switch_to_view(app->view_dispatcher, HidViewMouseJiggler);
    } else if(index == HidSubmenuIndexPushToTalk) {
        app->view_id = HidViewPushToTalkMenu;
        view_dispatcher_switch_to_view(app->view_dispatcher, HidViewPushToTalkMenu);
    }
}

static void bt_hid_connection_status_changed_callback(BtStatus status, void* context) {
    furi_assert(context);
    Hid* hid = context;
    bool connected = (status == BtStatusConnected);
    if(hid->transport == HidTransportBle) {
        if(connected) {
            notification_internal_message(hid->notifications, &sequence_set_blue_255);
        } else {
            notification_internal_message(hid->notifications, &sequence_reset_blue);
        }
    }
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
    hid_tikshorts_set_connected_status(hid->hid_tikshorts, connected);
}

static uint32_t hid_menu_view(void* context) {
    UNUSED(context);
    return HidViewSubmenu;
}

static uint32_t hid_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

static uint32_t hid_ptt_menu_view(void* context) {
    UNUSED(context);
    return HidViewPushToTalkMenu;
}

Hid* hid_alloc(HidTransport transport) {
    Hid* app = malloc(sizeof(Hid));
    app->transport = transport;

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
    if(app->transport == HidTransportBle) {
        submenu_add_item(
            app->device_type_submenu,
            "TikTok / YT Shorts",
            HidSubmenuIndexTikShorts,
            hid_submenu_callback,
            app);
    }
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
    view_set_previous_callback(submenu_get_view(app->device_type_submenu), hid_exit);
    view_dispatcher_add_view(
        app->view_dispatcher, HidViewSubmenu, submenu_get_view(app->device_type_submenu));
    app->view_id = HidViewSubmenu;
    view_dispatcher_switch_to_view(app->view_dispatcher, app->view_id);
    return app;
}

Hid* hid_app_alloc_view(void* context) {
    furi_assert(context);
    Hid* app = context;

    // Keynote view
    app->hid_keynote = hid_keynote_alloc(app);
    view_set_previous_callback(hid_keynote_get_view(app->hid_keynote), hid_menu_view);
    view_dispatcher_add_view(
        app->view_dispatcher, HidViewKeynote, hid_keynote_get_view(app->hid_keynote));

    // Keyboard view
    app->hid_keyboard = hid_keyboard_alloc(app);
    view_set_previous_callback(hid_keyboard_get_view(app->hid_keyboard), hid_menu_view);
    view_dispatcher_add_view(
        app->view_dispatcher, HidViewKeyboard, hid_keyboard_get_view(app->hid_keyboard));

    //Numpad keyboard view
    app->hid_numpad = hid_numpad_alloc(app);
    view_set_previous_callback(hid_numpad_get_view(app->hid_numpad), hid_menu_view);
    view_dispatcher_add_view(
        app->view_dispatcher, HidViewNumpad, hid_numpad_get_view(app->hid_numpad));

    // Media view
    app->hid_media = hid_media_alloc(app);
    view_set_previous_callback(hid_media_get_view(app->hid_media), hid_menu_view);
    view_dispatcher_add_view(
        app->view_dispatcher, HidViewMedia, hid_media_get_view(app->hid_media));

    // Music MacOs view
    app->hid_music_macos = hid_music_macos_alloc(app);
    view_set_previous_callback(hid_music_macos_get_view(app->hid_music_macos), hid_menu_view);
    view_dispatcher_add_view(
        app->view_dispatcher, HidViewMusicMacOs, hid_music_macos_get_view(app->hid_music_macos));

    // Movie view
    app->hid_movie = hid_movie_alloc(app);
    view_set_previous_callback(hid_movie_get_view(app->hid_movie), hid_menu_view);
    view_dispatcher_add_view(
        app->view_dispatcher, HidViewMovie, hid_movie_get_view(app->hid_movie));

    // TikTok / YT Shorts view
    app->hid_tikshorts = hid_tikshorts_alloc(app);
    view_set_previous_callback(hid_tikshorts_get_view(app->hid_tikshorts), hid_menu_view);
    view_dispatcher_add_view(
        app->view_dispatcher, BtHidViewTikShorts, hid_tikshorts_get_view(app->hid_tikshorts));

    // Mouse view
    app->hid_mouse = hid_mouse_alloc(app);
    view_set_previous_callback(hid_mouse_get_view(app->hid_mouse), hid_menu_view);
    view_dispatcher_add_view(
        app->view_dispatcher, HidViewMouse, hid_mouse_get_view(app->hid_mouse));

    // Mouse clicker view
    app->hid_mouse_clicker = hid_mouse_clicker_alloc(app);
    view_set_previous_callback(hid_mouse_clicker_get_view(app->hid_mouse_clicker), hid_menu_view);
    view_dispatcher_add_view(
        app->view_dispatcher,
        HidViewMouseClicker,
        hid_mouse_clicker_get_view(app->hid_mouse_clicker));

    // Mouse jiggler view
    app->hid_mouse_jiggler = hid_mouse_jiggler_alloc(app);
    view_set_previous_callback(hid_mouse_jiggler_get_view(app->hid_mouse_jiggler), hid_menu_view);
    view_dispatcher_add_view(
        app->view_dispatcher,
        HidViewMouseJiggler,
        hid_mouse_jiggler_get_view(app->hid_mouse_jiggler));

    // PushToTalk view
    app->hid_ptt_menu = hid_ptt_menu_alloc(app);
    view_set_previous_callback(hid_ptt_menu_get_view(app->hid_ptt_menu), hid_menu_view);
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
    if(app->transport == HidTransportBle) {
        notification_internal_message(app->notifications, &sequence_reset_blue);
    }

    // Free views
    view_dispatcher_remove_view(app->view_dispatcher, HidViewSubmenu);
    submenu_free(app->device_type_submenu);
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
    view_dispatcher_remove_view(app->view_dispatcher, BtHidViewTikShorts);
    hid_tikshorts_free(app->hid_tikshorts);
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

void hid_hal_keyboard_press(Hid* instance, uint16_t event) {
    furi_assert(instance);
    if(instance->transport == HidTransportBle) {
        furi_hal_bt_hid_kb_press(event);
    } else if(instance->transport == HidTransportUsb) {
        furi_hal_hid_kb_press(event);
    } else {
        furi_crash();
    }
}

void hid_hal_keyboard_release(Hid* instance, uint16_t event) {
    furi_assert(instance);
    if(instance->transport == HidTransportBle) {
        furi_hal_bt_hid_kb_release(event);
    } else if(instance->transport == HidTransportUsb) {
        furi_hal_hid_kb_release(event);
    } else {
        furi_crash();
    }
}

void hid_hal_keyboard_release_all(Hid* instance) {
    furi_assert(instance);
    if(instance->transport == HidTransportBle) {
        furi_hal_bt_hid_kb_release_all();
    } else if(instance->transport == HidTransportUsb) {
        furi_hal_hid_kb_release_all();
    } else {
        furi_crash();
    }
}

void hid_hal_consumer_key_press(Hid* instance, uint16_t event) {
    furi_assert(instance);
    if(instance->transport == HidTransportBle) {
        furi_hal_bt_hid_consumer_key_press(event);
    } else if(instance->transport == HidTransportUsb) {
        furi_hal_hid_consumer_key_press(event);
    } else {
        furi_crash();
    }
}

void hid_hal_consumer_key_release(Hid* instance, uint16_t event) {
    furi_assert(instance);
    if(instance->transport == HidTransportBle) {
        furi_hal_bt_hid_consumer_key_release(event);
    } else if(instance->transport == HidTransportUsb) {
        furi_hal_hid_consumer_key_release(event);
    } else {
        furi_crash();
    }
}

void hid_hal_consumer_key_release_all(Hid* instance) {
    furi_assert(instance);
    if(instance->transport == HidTransportBle) {
        furi_hal_bt_hid_consumer_key_release_all();
    } else if(instance->transport == HidTransportUsb) {
        furi_hal_hid_kb_release_all();
    } else {
        furi_crash();
    }
}

void hid_hal_mouse_move(Hid* instance, int8_t dx, int8_t dy) {
    furi_assert(instance);
    if(instance->transport == HidTransportBle) {
        furi_hal_bt_hid_mouse_move(dx, dy);
    } else if(instance->transport == HidTransportUsb) {
        furi_hal_hid_mouse_move(dx, dy);
    } else {
        furi_crash();
    }
}

void hid_hal_mouse_scroll(Hid* instance, int8_t delta) {
    furi_assert(instance);
    if(instance->transport == HidTransportBle) {
        furi_hal_bt_hid_mouse_scroll(delta);
    } else if(instance->transport == HidTransportUsb) {
        furi_hal_hid_mouse_scroll(delta);
    } else {
        furi_crash();
    }
}

void hid_hal_mouse_press(Hid* instance, uint16_t event) {
    furi_assert(instance);
    if(instance->transport == HidTransportBle) {
        furi_hal_bt_hid_mouse_press(event);
    } else if(instance->transport == HidTransportUsb) {
        furi_hal_hid_mouse_press(event);
    } else {
        furi_crash();
    }
}

void hid_hal_mouse_release(Hid* instance, uint16_t event) {
    furi_assert(instance);
    if(instance->transport == HidTransportBle) {
        furi_hal_bt_hid_mouse_release(event);
    } else if(instance->transport == HidTransportUsb) {
        furi_hal_hid_mouse_release(event);
    } else {
        furi_crash();
    }
}

void hid_hal_mouse_release_all(Hid* instance) {
    furi_assert(instance);
    if(instance->transport == HidTransportBle) {
        furi_hal_bt_hid_mouse_release_all();
    } else if(instance->transport == HidTransportUsb) {
        furi_hal_hid_mouse_release(HID_MOUSE_BTN_LEFT);
        furi_hal_hid_mouse_release(HID_MOUSE_BTN_RIGHT);
    } else {
        furi_crash();
    }
}

int32_t hid_usb_app(void* p) {
    UNUSED(p);
    Hid* app = hid_alloc(HidTransportUsb);
    app = hid_app_alloc_view(app);
    FuriHalUsbInterface* usb_mode_prev = furi_hal_usb_get_config();
    furi_hal_usb_unlock();
    furi_check(furi_hal_usb_set_config(&usb_hid, NULL) == true);

    bt_hid_connection_status_changed_callback(BtStatusConnected, app);

    dolphin_deed(DolphinDeedPluginStart);

    view_dispatcher_run(app->view_dispatcher);

    furi_hal_usb_set_config(usb_mode_prev, NULL);

    hid_free(app);

    return 0;
}

int32_t hid_ble_app(void* p) {
    UNUSED(p);
    Hid* app = hid_alloc(HidTransportBle);
    app = hid_app_alloc_view(app);

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

    furi_check(bt_set_profile(app->bt, BtProfileHidKeyboard));

    furi_hal_bt_start_advertising();
    bt_set_status_changed_callback(app->bt, bt_hid_connection_status_changed_callback, app);

    dolphin_deed(DolphinDeedPluginStart);

    view_dispatcher_run(app->view_dispatcher);

    bt_set_status_changed_callback(app->bt, NULL, NULL);

    bt_disconnect(app->bt);

    // Wait 2nd core to update nvm storage
    furi_delay_ms(200);

    bt_keys_storage_set_default_path(app->bt);

    furi_check(bt_set_profile(app->bt, BtProfileSerial));

    hid_free(app);

    return 0;
}
