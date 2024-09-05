#include "nfc_app_i.h"
#include "helpers/protocol_support/nfc_protocol_support.h"

#include <dolphin/dolphin.h>

bool nfc_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    NfcApp* nfc = context;
    return scene_manager_handle_custom_event(nfc->scene_manager, event);
}

bool nfc_back_event_callback(void* context) {
    furi_assert(context);
    NfcApp* nfc = context;
    return scene_manager_handle_back_event(nfc->scene_manager);
}

static void nfc_app_rpc_command_callback(const RpcAppSystemEvent* event, void* context) {
    furi_assert(context);
    NfcApp* nfc = (NfcApp*)context;

    furi_assert(nfc->rpc_ctx);

    if(event->type == RpcAppEventTypeSessionClose) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventRpcSessionClose);
        rpc_system_app_set_callback(nfc->rpc_ctx, NULL, NULL);
        nfc->rpc_ctx = NULL;
    } else if(event->type == RpcAppEventTypeAppExit) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventRpcExit);
    } else if(event->type == RpcAppEventTypeLoadFile) {
        furi_assert(event->data.type == RpcAppSystemEventDataTypeString);
        furi_string_set(nfc->file_path, event->data.string);
        view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventRpcLoadFile);
    } else {
        rpc_system_app_confirm(nfc->rpc_ctx, false);
    }
}

NfcApp* nfc_app_alloc(void) {
    NfcApp* instance = malloc(sizeof(NfcApp));

    instance->view_dispatcher = view_dispatcher_alloc();
    instance->scene_manager = scene_manager_alloc(&nfc_scene_handlers, instance);
    view_dispatcher_set_event_callback_context(instance->view_dispatcher, instance);
    view_dispatcher_set_custom_event_callback(
        instance->view_dispatcher, nfc_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        instance->view_dispatcher, nfc_back_event_callback);

    instance->nfc = nfc_alloc();

    instance->detected_protocols = nfc_detected_protocols_alloc();
    instance->felica_auth = felica_auth_alloc();
    instance->mf_ul_auth = mf_ultralight_auth_alloc();
    instance->slix_unlock = slix_unlock_alloc();
    instance->mfc_key_cache = mf_classic_key_cache_alloc();
    instance->nfc_supported_cards = nfc_supported_cards_alloc();

    // Nfc device
    instance->nfc_device = nfc_device_alloc();
    nfc_device_set_loading_callback(instance->nfc_device, nfc_show_loading_popup, instance);

    // Open GUI record
    instance->gui = furi_record_open(RECORD_GUI);

    // Open Notification record
    instance->notifications = furi_record_open(RECORD_NOTIFICATION);

    // Open Storage record
    instance->storage = furi_record_open(RECORD_STORAGE);

    // Open Dialogs record
    instance->dialogs = furi_record_open(RECORD_DIALOGS);

    // Submenu
    instance->submenu = submenu_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher, NfcViewMenu, submenu_get_view(instance->submenu));

    // Dialog
    instance->dialog_ex = dialog_ex_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher, NfcViewDialogEx, dialog_ex_get_view(instance->dialog_ex));

    // Popup
    instance->popup = popup_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher, NfcViewPopup, popup_get_view(instance->popup));

    // Loading
    instance->loading = loading_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher, NfcViewLoading, loading_get_view(instance->loading));

    // Text Input
    instance->text_input = text_input_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher, NfcViewTextInput, text_input_get_view(instance->text_input));

    // Byte Input
    instance->byte_input = byte_input_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher, NfcViewByteInput, byte_input_get_view(instance->byte_input));

    // TextBox
    instance->text_box = text_box_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher, NfcViewTextBox, text_box_get_view(instance->text_box));
    instance->text_box_store = furi_string_alloc();

    // Custom Widget
    instance->widget = widget_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher, NfcViewWidget, widget_get_view(instance->widget));

    // Dict attack
    instance->dict_attack = dict_attack_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher, NfcViewDictAttack, dict_attack_get_view(instance->dict_attack));

    // Detect Reader
    instance->detect_reader = detect_reader_alloc();
    view_dispatcher_add_view(
        instance->view_dispatcher,
        NfcViewDetectReader,
        detect_reader_get_view(instance->detect_reader));

    instance->iso14443_3a_edit_data = iso14443_3a_alloc();
    instance->file_path = furi_string_alloc_set(NFC_APP_FOLDER);
    instance->file_name = furi_string_alloc();

    return instance;
}

void nfc_app_free(NfcApp* instance) {
    furi_assert(instance);

    if(instance->rpc_ctx) {
        rpc_system_app_send_exited(instance->rpc_ctx);
        rpc_system_app_set_callback(instance->rpc_ctx, NULL, NULL);
    }

    nfc_free(instance->nfc);

    nfc_detected_protocols_free(instance->detected_protocols);
    felica_auth_free(instance->felica_auth);
    mf_ultralight_auth_free(instance->mf_ul_auth);
    slix_unlock_free(instance->slix_unlock);
    mf_classic_key_cache_free(instance->mfc_key_cache);
    nfc_supported_cards_free(instance->nfc_supported_cards);

    // Nfc device
    nfc_device_free(instance->nfc_device);

    // Submenu
    view_dispatcher_remove_view(instance->view_dispatcher, NfcViewMenu);
    submenu_free(instance->submenu);

    // DialogEx
    view_dispatcher_remove_view(instance->view_dispatcher, NfcViewDialogEx);
    dialog_ex_free(instance->dialog_ex);

    // Popup
    view_dispatcher_remove_view(instance->view_dispatcher, NfcViewPopup);
    popup_free(instance->popup);

    // Loading
    view_dispatcher_remove_view(instance->view_dispatcher, NfcViewLoading);
    loading_free(instance->loading);

    // TextInput
    view_dispatcher_remove_view(instance->view_dispatcher, NfcViewTextInput);
    text_input_free(instance->text_input);

    // ByteInput
    view_dispatcher_remove_view(instance->view_dispatcher, NfcViewByteInput);
    byte_input_free(instance->byte_input);

    // TextBox
    view_dispatcher_remove_view(instance->view_dispatcher, NfcViewTextBox);
    text_box_free(instance->text_box);
    furi_string_free(instance->text_box_store);

    // Custom Widget
    view_dispatcher_remove_view(instance->view_dispatcher, NfcViewWidget);
    widget_free(instance->widget);

    // Dict attack
    view_dispatcher_remove_view(instance->view_dispatcher, NfcViewDictAttack);
    dict_attack_free(instance->dict_attack);

    // Detect reader
    view_dispatcher_remove_view(instance->view_dispatcher, NfcViewDetectReader);
    detect_reader_free(instance->detect_reader);

    // View Dispatcher
    view_dispatcher_free(instance->view_dispatcher);

    // Scene Manager
    scene_manager_free(instance->scene_manager);

    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_NOTIFICATION);
    // GUI
    furi_record_close(RECORD_GUI);
    instance->gui = NULL;

    instance->notifications = NULL;

    iso14443_3a_free(instance->iso14443_3a_edit_data);
    furi_string_free(instance->file_path);
    furi_string_free(instance->file_name);

    free(instance);
}

void nfc_text_store_set(NfcApp* nfc, const char* text, ...) {
    va_list args;
    va_start(args, text);

    vsnprintf(nfc->text_store, sizeof(nfc->text_store), text, args);

    va_end(args);
}

void nfc_text_store_clear(NfcApp* nfc) {
    memset(nfc->text_store, 0, sizeof(nfc->text_store));
}

void nfc_blink_read_start(NfcApp* nfc) {
    notification_message(nfc->notifications, &sequence_blink_start_yellow);
}

void nfc_blink_emulate_start(NfcApp* nfc) {
    notification_message(nfc->notifications, &sequence_blink_start_magenta);
}

void nfc_blink_detect_start(NfcApp* nfc) {
    notification_message(nfc->notifications, &sequence_blink_start_cyan);
}

void nfc_blink_stop(NfcApp* nfc) {
    notification_message(nfc->notifications, &sequence_blink_stop);
}

void nfc_make_app_folders(NfcApp* instance) {
    furi_assert(instance);

    if(!storage_simply_mkdir(instance->storage, NFC_APP_FOLDER)) {
        dialog_message_show_storage_error(instance->dialogs, "Cannot create\napp folder");
    }
}

bool nfc_save_file(NfcApp* instance, FuriString* path) {
    furi_assert(instance);
    furi_assert(path);

    bool result = nfc_device_save(instance->nfc_device, furi_string_get_cstr(instance->file_path));

    if(!result) {
        dialog_message_show_storage_error(instance->dialogs, "Cannot save\nkey file");
    }

    return result;
}

static bool nfc_set_shadow_file_path(FuriString* file_path, FuriString* shadow_file_path) {
    furi_assert(file_path);
    furi_assert(shadow_file_path);

    bool shadow_file_path_set = false;
    if(furi_string_end_with(file_path, NFC_APP_SHADOW_EXTENSION)) {
        furi_string_set(shadow_file_path, file_path);
        shadow_file_path_set = true;
    } else if(furi_string_end_with(file_path, NFC_APP_EXTENSION)) {
        size_t path_len = furi_string_size(file_path);
        // Cut .nfc
        furi_string_set_n(shadow_file_path, file_path, 0, path_len - 4);
        furi_string_cat_printf(shadow_file_path, "%s", NFC_APP_SHADOW_EXTENSION);
        shadow_file_path_set = true;
    }

    return shadow_file_path_set;
}

static bool nfc_has_shadow_file_internal(NfcApp* instance, FuriString* path) {
    furi_assert(path);

    bool has_shadow_file = false;
    FuriString* shadow_file_path = furi_string_alloc();
    do {
        if(furi_string_empty(path)) break;
        if(!nfc_set_shadow_file_path(path, shadow_file_path)) break;
        has_shadow_file =
            storage_common_exists(instance->storage, furi_string_get_cstr(shadow_file_path));
    } while(false);

    furi_string_free(shadow_file_path);

    return has_shadow_file;
}

bool nfc_has_shadow_file(NfcApp* instance) {
    furi_assert(instance);

    return nfc_has_shadow_file_internal(instance, instance->file_path);
}

static bool nfc_save_internal(NfcApp* instance, const char* extension) {
    furi_assert(instance);
    furi_assert(extension);

    bool result = false;

    nfc_make_app_folders(instance);

    if(furi_string_end_with(instance->file_path, NFC_APP_EXTENSION) ||
       (furi_string_end_with(instance->file_path, NFC_APP_SHADOW_EXTENSION))) {
        size_t filename_start = furi_string_search_rchar(instance->file_path, '/');
        furi_string_left(instance->file_path, filename_start);
    }

    furi_string_cat_printf(
        instance->file_path, "/%s%s", furi_string_get_cstr(instance->file_name), extension);

    result = nfc_save_file(instance, instance->file_path);

    return result;
}

bool nfc_save_shadow_file(NfcApp* instance) {
    furi_assert(instance);

    return nfc_save_internal(instance, NFC_APP_SHADOW_EXTENSION);
}

bool nfc_save(NfcApp* instance) {
    furi_assert(instance);

    return nfc_save_internal(instance, NFC_APP_EXTENSION);
}

bool nfc_load_file(NfcApp* instance, FuriString* path, bool show_dialog) {
    furi_assert(instance);
    furi_assert(path);
    bool result = false;

    nfc_supported_cards_load_cache(instance->nfc_supported_cards);

    FuriString* load_path = furi_string_alloc();
    if(nfc_has_shadow_file_internal(instance, path)) { //-V1051
        nfc_set_shadow_file_path(path, load_path);
    } else if(furi_string_end_with(path, NFC_APP_SHADOW_EXTENSION)) {
        size_t path_len = furi_string_size(path);
        furi_string_set_n(load_path, path, 0, path_len - 4);
        furi_string_cat_printf(load_path, "%s", NFC_APP_EXTENSION);
    } else {
        furi_string_set(load_path, path);
    }

    result = nfc_device_load(instance->nfc_device, furi_string_get_cstr(load_path));

    if(result) {
        path_extract_filename(load_path, instance->file_name, true);
    }

    if((!result) && (show_dialog)) {
        dialog_message_show_storage_error(instance->dialogs, "Cannot load\nkey file");
    }

    furi_string_free(load_path);

    return result;
}

bool nfc_delete(NfcApp* instance) {
    furi_assert(instance);

    if(nfc_has_shadow_file(instance)) {
        nfc_delete_shadow_file(instance);
    }

    if(furi_string_end_with_str(instance->file_path, NFC_APP_SHADOW_EXTENSION)) {
        size_t path_len = furi_string_size(instance->file_path);
        furi_string_replace_at(instance->file_path, path_len - 4, 4, NFC_APP_EXTENSION);
    }

    return storage_simply_remove(instance->storage, furi_string_get_cstr(instance->file_path));
}

bool nfc_delete_shadow_file(NfcApp* instance) {
    furi_assert(instance);

    FuriString* shadow_file_path = furi_string_alloc();

    bool result = nfc_set_shadow_file_path(instance->file_path, shadow_file_path) &&
                  storage_simply_remove(instance->storage, furi_string_get_cstr(shadow_file_path));

    furi_string_free(shadow_file_path);
    return result;
}

bool nfc_load_from_file_select(NfcApp* instance) {
    furi_assert(instance);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, NFC_APP_EXTENSION, &I_Nfc_10px);
    browser_options.base_path = NFC_APP_FOLDER;
    browser_options.hide_dot_files = true;

    bool success = false;
    do {
        // Input events and views are managed by file_browser
        if(!dialog_file_browser_show(
               instance->dialogs, instance->file_path, instance->file_path, &browser_options))
            break;
        success = nfc_load_file(instance, instance->file_path, true);
    } while(!success);

    return success;
}

void nfc_show_loading_popup(void* context, bool show) {
    NfcApp* nfc = context;

    if(show) {
        // Raise timer priority so that animations can play
        furi_timer_set_thread_priority(FuriTimerThreadPriorityElevated);
        view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewLoading);
    } else {
        // Restore default timer priority
        furi_timer_set_thread_priority(FuriTimerThreadPriorityNormal);
    }
}

void nfc_append_filename_string_when_present(NfcApp* instance, FuriString* string) {
    furi_assert(instance);
    furi_assert(string);

    if(!furi_string_empty(instance->file_name)) {
        furi_string_cat_printf(string, "Name: %s\n", furi_string_get_cstr(instance->file_name));
    }
}

static bool nfc_is_hal_ready(void) {
    if(furi_hal_nfc_is_hal_ready() != FuriHalNfcErrorNone) {
        // No connection to the chip, show an error screen
        DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
        DialogMessage* message = dialog_message_alloc();
        dialog_message_set_header(message, "Error: NFC Chip Failed", 64, 0, AlignCenter, AlignTop);
        dialog_message_set_text(
            message, "Send error photo via\nsupport.flipper.net", 0, 63, AlignLeft, AlignBottom);
        dialog_message_set_icon(message, &I_err_09, 128 - 25, 64 - 25);
        dialog_message_show(dialogs, message);
        dialog_message_free(message);
        furi_record_close(RECORD_DIALOGS);
        return false;
    } else {
        return true;
    }
}

static void nfc_show_initial_scene_for_device(NfcApp* nfc) {
    NfcProtocol prot = nfc_device_get_protocol(nfc->nfc_device);
    uint32_t scene = nfc_protocol_support_has_feature(
                         prot, NfcProtocolFeatureEmulateFull | NfcProtocolFeatureEmulateUid) ?
                         NfcSceneEmulate :
                         NfcSceneSavedMenu;
    scene_manager_next_scene(nfc->scene_manager, scene);
}

int32_t nfc_app(void* p) {
    if(!nfc_is_hal_ready()) return 0;

    NfcApp* nfc = nfc_app_alloc();
    const char* args = p;

    if(args && strlen(args)) {
        if(sscanf(args, "RPC %p", &nfc->rpc_ctx) == 1) {
            rpc_system_app_set_callback(nfc->rpc_ctx, nfc_app_rpc_command_callback, nfc);
            rpc_system_app_send_started(nfc->rpc_ctx);
            view_dispatcher_attach_to_gui(
                nfc->view_dispatcher, nfc->gui, ViewDispatcherTypeDesktop);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRpc);
        } else {
            view_dispatcher_attach_to_gui(
                nfc->view_dispatcher, nfc->gui, ViewDispatcherTypeFullscreen);

            furi_string_set(nfc->file_path, args);
            if(nfc_load_file(nfc, nfc->file_path, true)) {
                nfc_show_initial_scene_for_device(nfc);
            } else {
                view_dispatcher_stop(nfc->view_dispatcher);
            }
        }
    } else {
        view_dispatcher_attach_to_gui(
            nfc->view_dispatcher, nfc->gui, ViewDispatcherTypeFullscreen);
        scene_manager_next_scene(nfc->scene_manager, NfcSceneStart);
    }

    view_dispatcher_run(nfc->view_dispatcher);

    nfc_app_free(nfc);

    return 0;
}
