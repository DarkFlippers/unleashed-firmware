#include "bad_usb_app_i.h"
#include <furi.h>
#include <furi_hal.h>
#include <storage/storage.h>
#include <lib/toolbox/path.h>
#include <flipper_format/flipper_format.h>

#define BAD_USB_SETTINGS_PATH           BAD_USB_APP_BASE_FOLDER "/.badusb.settings"
#define BAD_USB_SETTINGS_FILE_TYPE      "Flipper BadUSB Settings File"
#define BAD_USB_SETTINGS_VERSION        1
#define BAD_USB_SETTINGS_DEFAULT_LAYOUT BAD_USB_APP_PATH_LAYOUT_FOLDER "/en-US.kl"

static bool bad_usb_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    BadUsbApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool bad_usb_app_back_event_callback(void* context) {
    furi_assert(context);
    BadUsbApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void bad_usb_app_tick_event_callback(void* context) {
    furi_assert(context);
    BadUsbApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

static void bad_usb_load_settings(BadUsbApp* app) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff = flipper_format_file_alloc(storage);
    bool loaded = false;

    BadUsbHidConfig* hid_cfg = &app->user_hid_cfg;
    FuriString* temp_str = furi_string_alloc();
    uint32_t temp_uint = 0;

    if(flipper_format_file_open_existing(fff, BAD_USB_SETTINGS_PATH)) {
        do {
            if(!flipper_format_read_header(fff, temp_str, &temp_uint)) break;
            if((strcmp(furi_string_get_cstr(temp_str), BAD_USB_SETTINGS_FILE_TYPE) != 0) ||
               (temp_uint != BAD_USB_SETTINGS_VERSION))
                break;

            if(flipper_format_read_string(fff, "layout", temp_str)) {
                furi_string_set(app->keyboard_layout, temp_str);
                FileInfo layout_file_info;
                FS_Error file_check_err = storage_common_stat(
                    storage, furi_string_get_cstr(app->keyboard_layout), &layout_file_info);
                if((file_check_err != FSE_OK) || (layout_file_info.size != 256)) {
                    furi_string_set(app->keyboard_layout, BAD_USB_SETTINGS_DEFAULT_LAYOUT);
                }
            } else {
                furi_string_set(app->keyboard_layout, BAD_USB_SETTINGS_DEFAULT_LAYOUT);
                flipper_format_rewind(fff);
            }

            if(!flipper_format_read_uint32(fff, "interface", &temp_uint, 1) ||
               temp_uint >= BadUsbHidInterfaceMAX) {
                temp_uint = BadUsbHidInterfaceUsb;
                flipper_format_rewind(fff);
            }
            app->interface = temp_uint;

            if(!flipper_format_read_bool(fff, "ble_bonding", &hid_cfg->ble.bonding, 1)) {
                hid_cfg->ble.bonding = true;
                flipper_format_rewind(fff);
            }

            if(!flipper_format_read_uint32(fff, "ble_pairing", &temp_uint, 1) ||
               temp_uint >= GapPairingCount) {
                temp_uint = GapPairingPinCodeVerifyYesNo;
                flipper_format_rewind(fff);
            }
            hid_cfg->ble.pairing = temp_uint;

            if(flipper_format_read_string(fff, "ble_name", temp_str)) {
                strlcpy(
                    hid_cfg->ble.name, furi_string_get_cstr(temp_str), sizeof(hid_cfg->ble.name));
            } else {
                hid_cfg->ble.name[0] = '\0';
                flipper_format_rewind(fff);
            }

            if(!flipper_format_read_hex(
                   fff, "ble_mac", hid_cfg->ble.mac, sizeof(hid_cfg->ble.mac))) {
                memset(hid_cfg->ble.mac, 0, sizeof(hid_cfg->ble.mac));
                flipper_format_rewind(fff);
            }

            if(flipper_format_read_string(fff, "usb_manuf", temp_str)) {
                strlcpy(
                    hid_cfg->usb.manuf,
                    furi_string_get_cstr(temp_str),
                    sizeof(hid_cfg->usb.manuf));
            } else {
                hid_cfg->usb.manuf[0] = '\0';
                flipper_format_rewind(fff);
            }

            if(flipper_format_read_string(fff, "usb_product", temp_str)) {
                strlcpy(
                    hid_cfg->usb.product,
                    furi_string_get_cstr(temp_str),
                    sizeof(hid_cfg->usb.product));
            } else {
                hid_cfg->usb.product[0] = '\0';
                flipper_format_rewind(fff);
            }

            if(!flipper_format_read_uint32(fff, "usb_vid", &hid_cfg->usb.vid, 1)) {
                hid_cfg->usb.vid = 0;
                flipper_format_rewind(fff);
            }

            if(!flipper_format_read_uint32(fff, "usb_pid", &hid_cfg->usb.pid, 1)) {
                hid_cfg->usb.pid = 0;
                flipper_format_rewind(fff);
            }

            loaded = true;
        } while(0);
    }

    furi_string_free(temp_str);

    flipper_format_free(fff);
    furi_record_close(RECORD_STORAGE);

    if(!loaded) {
        furi_string_set(app->keyboard_layout, BAD_USB_SETTINGS_DEFAULT_LAYOUT);
        app->interface = BadUsbHidInterfaceUsb;
        hid_cfg->ble.name[0] = '\0';
        memset(hid_cfg->ble.mac, 0, sizeof(hid_cfg->ble.mac));
        hid_cfg->ble.bonding = true;
        hid_cfg->ble.pairing = GapPairingPinCodeVerifyYesNo;
        hid_cfg->usb.vid = 0;
        hid_cfg->usb.pid = 0;
        hid_cfg->usb.manuf[0] = '\0';
        hid_cfg->usb.product[0] = '\0';
    }
}

static void bad_usb_save_settings(BadUsbApp* app) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff = flipper_format_file_alloc(storage);
    BadUsbHidConfig* hid_cfg = &app->user_hid_cfg;
    uint32_t temp_uint = 0;

    if(flipper_format_file_open_always(fff, BAD_USB_SETTINGS_PATH)) {
        do {
            if(!flipper_format_write_header_cstr(
                   fff, BAD_USB_SETTINGS_FILE_TYPE, BAD_USB_SETTINGS_VERSION))
                break;
            if(!flipper_format_write_string(fff, "layout", app->keyboard_layout)) break;
            temp_uint = app->interface;
            if(!flipper_format_write_uint32(fff, "interface", &temp_uint, 1)) break;
            if(!flipper_format_write_bool(fff, "ble_bonding", &hid_cfg->ble.bonding, 1)) break;
            temp_uint = hid_cfg->ble.pairing;
            if(!flipper_format_write_uint32(fff, "ble_pairing", &temp_uint, 1)) break;
            if(!flipper_format_write_string_cstr(fff, "ble_name", hid_cfg->ble.name)) break;
            if(!flipper_format_write_hex(
                   fff, "ble_mac", (uint8_t*)&hid_cfg->ble.mac, sizeof(hid_cfg->ble.mac)))
                break;
            if(!flipper_format_write_string_cstr(fff, "usb_manuf", hid_cfg->usb.manuf)) break;
            if(!flipper_format_write_string_cstr(fff, "usb_product", hid_cfg->usb.product)) break;
            if(!flipper_format_write_uint32(fff, "usb_vid", &hid_cfg->usb.vid, 1)) break;
            if(!flipper_format_write_uint32(fff, "usb_pid", &hid_cfg->usb.pid, 1)) break;
        } while(0);
    }

    flipper_format_free(fff);
    furi_record_close(RECORD_STORAGE);
}

void bad_usb_set_interface(BadUsbApp* app, BadUsbHidInterface interface) {
    app->interface = interface;
    bad_usb_view_set_interface(app->bad_usb_view, interface);
}

BadUsbApp* bad_usb_app_alloc(char* arg) {
    BadUsbApp* app = malloc(sizeof(BadUsbApp));

    app->bad_usb_script = NULL;

    app->file_path = furi_string_alloc();
    app->keyboard_layout = furi_string_alloc();
    if(arg && strlen(arg)) {
        furi_string_set(app->file_path, arg);
    }

    bad_usb_load_settings(app);

    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->dialogs = furi_record_open(RECORD_DIALOGS);

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&bad_usb_scene_handlers, app);

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, bad_usb_app_tick_event_callback, 250);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, bad_usb_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, bad_usb_app_back_event_callback);

    // Custom Widget
    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BadUsbAppViewWidget, widget_get_view(app->widget));

    // Popup
    app->popup = popup_alloc();
    view_dispatcher_add_view(app->view_dispatcher, BadUsbAppViewPopup, popup_get_view(app->popup));

    app->var_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        BadUsbAppViewConfig,
        variable_item_list_get_view(app->var_item_list));

    app->bad_usb_view = bad_usb_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BadUsbAppViewWork, bad_usb_view_get_view(app->bad_usb_view));

    app->text_input = text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BadUsbAppViewTextInput, text_input_get_view(app->text_input));

    app->byte_input = byte_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BadUsbAppViewByteInput, byte_input_get_view(app->byte_input));

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    if(furi_hal_usb_is_locked()) {
        app->error = BadUsbAppErrorCloseRpc;
        app->usb_if_prev = NULL;
        scene_manager_next_scene(app->scene_manager, BadUsbSceneError);
    } else {
        app->usb_if_prev = furi_hal_usb_get_config();
        furi_check(furi_hal_usb_set_config(NULL, NULL));

        if(!furi_string_empty(app->file_path)) {
            scene_manager_set_scene_state(app->scene_manager, BadUsbSceneWork, true);
            scene_manager_next_scene(app->scene_manager, BadUsbSceneWork);
        } else {
            furi_string_set(app->file_path, BAD_USB_APP_BASE_FOLDER);
            scene_manager_next_scene(app->scene_manager, BadUsbSceneFileSelect);
        }
    }

    return app;
}

void bad_usb_app_free(BadUsbApp* app) {
    furi_assert(app);

    if(app->bad_usb_script) {
        bad_usb_script_close(app->bad_usb_script);
        app->bad_usb_script = NULL;
    }

    // Views
    view_dispatcher_remove_view(app->view_dispatcher, BadUsbAppViewWork);
    bad_usb_view_free(app->bad_usb_view);

    // Custom Widget
    view_dispatcher_remove_view(app->view_dispatcher, BadUsbAppViewWidget);
    widget_free(app->widget);

    // Popup
    view_dispatcher_remove_view(app->view_dispatcher, BadUsbAppViewPopup);
    popup_free(app->popup);

    // Config menu
    view_dispatcher_remove_view(app->view_dispatcher, BadUsbAppViewConfig);
    variable_item_list_free(app->var_item_list);

    // Text Input
    view_dispatcher_remove_view(app->view_dispatcher, BadUsbAppViewTextInput);
    text_input_free(app->text_input);

    // Byte Input
    view_dispatcher_remove_view(app->view_dispatcher, BadUsbAppViewByteInput);
    byte_input_free(app->byte_input);

    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    // Close records
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_DIALOGS);

    bad_usb_save_settings(app);

    furi_string_free(app->file_path);
    furi_string_free(app->keyboard_layout);

    if(app->usb_if_prev) {
        furi_check(furi_hal_usb_set_config(app->usb_if_prev, NULL));
    }

    free(app);
}

int32_t bad_usb_app(void* p) {
    BadUsbApp* bad_usb_app = bad_usb_app_alloc((char*)p);

    view_dispatcher_run(bad_usb_app->view_dispatcher);

    bad_usb_app_free(bad_usb_app);
    return 0;
}
