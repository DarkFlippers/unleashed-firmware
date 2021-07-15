#include "nfc_i.h"
#include "api-hal-nfc.h"

bool nfc_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    Nfc* nfc = (Nfc*)context;
    return scene_manager_handle_custom_event(nfc->scene_manager, event);
}

bool nfc_navigation_event_callback(void* context) {
    furi_assert(context);
    Nfc* nfc = (Nfc*)context;
    return scene_manager_handle_navigation_event(nfc->scene_manager);
}

void nfc_tick_event_callback(void* context) {
    furi_assert(context);
    Nfc* nfc = (Nfc*)context;
    scene_manager_handle_tick_event(nfc->scene_manager);
}

Nfc* nfc_alloc() {
    Nfc* nfc = furi_alloc(sizeof(Nfc));

    nfc->nfc_common.worker = nfc_worker_alloc();
    nfc->nfc_common.view_dispatcher = view_dispatcher_alloc();
    nfc->scene_manager = scene_manager_alloc(&nfc_scene_handlers, nfc);
    view_dispatcher_enable_queue(nfc->nfc_common.view_dispatcher);
    view_dispatcher_set_event_callback_context(nfc->nfc_common.view_dispatcher, nfc);
    view_dispatcher_set_custom_event_callback(
        nfc->nfc_common.view_dispatcher, nfc_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        nfc->nfc_common.view_dispatcher, nfc_navigation_event_callback);
    view_dispatcher_set_tick_event_callback(
        nfc->nfc_common.view_dispatcher, nfc_tick_event_callback, 300);

    // Open GUI record
    nfc->gui = furi_record_open("gui");
    view_dispatcher_attach_to_gui(
        nfc->nfc_common.view_dispatcher, nfc->gui, ViewDispatcherTypeFullscreen);

    // Open Notification record
    nfc->notifications = furi_record_open("notification");

    // Submenu
    nfc->submenu = submenu_alloc();
    view_dispatcher_add_view(
        nfc->nfc_common.view_dispatcher, NfcViewMenu, submenu_get_view(nfc->submenu));

    // Dialog
    nfc->dialog_ex = dialog_ex_alloc();
    view_dispatcher_add_view(
        nfc->nfc_common.view_dispatcher, NfcViewDialogEx, dialog_ex_get_view(nfc->dialog_ex));

    // Popup
    nfc->popup = popup_alloc();
    view_dispatcher_add_view(
        nfc->nfc_common.view_dispatcher, NfcViewPopup, popup_get_view(nfc->popup));

    // Text Input
    nfc->text_input = text_input_alloc();
    view_dispatcher_add_view(
        nfc->nfc_common.view_dispatcher, NfcViewTextInput, text_input_get_view(nfc->text_input));

    // Byte Input
    nfc->byte_input = byte_input_alloc();
    view_dispatcher_add_view(
        nfc->nfc_common.view_dispatcher, NfcViewByteInput, byte_input_get_view(nfc->byte_input));

    // TextBox
    nfc->text_box = text_box_alloc();
    view_dispatcher_add_view(
        nfc->nfc_common.view_dispatcher, NfcViewTextBox, text_box_get_view(nfc->text_box));
    string_init(nfc->text_box_store);

    // Detect
    nfc->nfc_detect = nfc_detect_alloc(&nfc->nfc_common);
    view_dispatcher_add_view(
        nfc->nfc_common.view_dispatcher, NfcViewDetect, nfc_detect_get_view(nfc->nfc_detect));

    // Emulate
    nfc->nfc_emulate = nfc_emulate_alloc(&nfc->nfc_common);
    view_dispatcher_add_view(
        nfc->nfc_common.view_dispatcher, NfcViewEmulate, nfc_emulate_get_view(nfc->nfc_emulate));

    // EMV
    nfc->nfc_emv = nfc_emv_alloc(&nfc->nfc_common);
    view_dispatcher_add_view(
        nfc->nfc_common.view_dispatcher, NfcViewEmv, nfc_emv_get_view(nfc->nfc_emv));

    // Mifare Ultralight
    nfc->nfc_mifare_ul = nfc_mifare_ul_alloc(&nfc->nfc_common);
    view_dispatcher_add_view(
        nfc->nfc_common.view_dispatcher,
        NfcViewMifareUl,
        nfc_mifare_ul_get_view(nfc->nfc_mifare_ul));

    return nfc;
}

void nfc_free(Nfc* nfc) {
    furi_assert(nfc);

    // Submenu
    view_dispatcher_remove_view(nfc->nfc_common.view_dispatcher, NfcViewMenu);
    submenu_free(nfc->submenu);

    // DialogEx
    view_dispatcher_remove_view(nfc->nfc_common.view_dispatcher, NfcViewDialogEx);
    dialog_ex_free(nfc->dialog_ex);

    // Popup
    view_dispatcher_remove_view(nfc->nfc_common.view_dispatcher, NfcViewPopup);
    popup_free(nfc->popup);

    // TextInput
    view_dispatcher_remove_view(nfc->nfc_common.view_dispatcher, NfcViewTextInput);
    text_input_free(nfc->text_input);

    // ByteInput
    view_dispatcher_remove_view(nfc->nfc_common.view_dispatcher, NfcViewByteInput);
    byte_input_free(nfc->byte_input);

    // TextBox
    view_dispatcher_remove_view(nfc->nfc_common.view_dispatcher, NfcViewTextBox);
    text_box_free(nfc->text_box);
    string_clear(nfc->text_box_store);

    // Detect
    view_dispatcher_remove_view(nfc->nfc_common.view_dispatcher, NfcViewDetect);
    nfc_detect_free(nfc->nfc_detect);

    // Emulate
    view_dispatcher_remove_view(nfc->nfc_common.view_dispatcher, NfcViewEmulate);
    nfc_emulate_free(nfc->nfc_emulate);

    // EMV
    view_dispatcher_remove_view(nfc->nfc_common.view_dispatcher, NfcViewEmv);
    nfc_emv_free(nfc->nfc_emv);

    // Mifare Ultralight
    view_dispatcher_remove_view(nfc->nfc_common.view_dispatcher, NfcViewMifareUl);
    nfc_mifare_ul_free(nfc->nfc_mifare_ul);

    // Worker
    nfc_worker_stop(nfc->nfc_common.worker);
    nfc_worker_free(nfc->nfc_common.worker);

    // View Dispatcher
    view_dispatcher_free(nfc->nfc_common.view_dispatcher);

    // Scene Manager
    scene_manager_free(nfc->scene_manager);

    // GUI
    furi_record_close("gui");
    nfc->gui = NULL;

    // Notifications
    furi_record_close("notification");
    nfc->notifications = NULL;

    free(nfc);
}

int32_t nfc_task(void* p) {
    Nfc* nfc = nfc_alloc();

    // Check argument and run corresponding scene
    if(p && nfc_device_load(&nfc->device, p)) {
        scene_manager_next_scene(nfc->scene_manager, NfcSceneEmulateUid);
    } else {
        scene_manager_next_scene(nfc->scene_manager, NfcSceneStart);
    }

    view_dispatcher_run(nfc->nfc_common.view_dispatcher);

    nfc_free(nfc);

    return 0;
}

void nfc_text_store_set(Nfc* nfc, const char* text, ...) {
    va_list args;
    va_start(args, text);

    vsnprintf(nfc->text_store, sizeof(nfc->text_store), text, args);

    va_end(args);
}

void nfc_text_store_clear(Nfc* nfc) {
    memset(nfc->text_store, 0, sizeof(nfc->text_store));
}
