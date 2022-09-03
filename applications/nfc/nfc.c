#include "nfc_i.h"
#include "furi_hal_nfc.h"

bool nfc_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    Nfc* nfc = context;
    return scene_manager_handle_custom_event(nfc->scene_manager, event);
}

bool nfc_back_event_callback(void* context) {
    furi_assert(context);
    Nfc* nfc = context;
    return scene_manager_handle_back_event(nfc->scene_manager);
}

static void nfc_rpc_command_callback(RpcAppSystemEvent event, void* context) {
    furi_assert(context);
    Nfc* nfc = context;

    furi_assert(nfc->rpc_ctx);

    if(event == RpcAppEventSessionClose) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventRpcSessionClose);
        rpc_system_app_set_callback(nfc->rpc_ctx, NULL, NULL);
        nfc->rpc_ctx = NULL;
    } else if(event == RpcAppEventAppExit) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventViewExit);
    } else if(event == RpcAppEventLoadFile) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventRpcLoad);
    } else {
        rpc_system_app_confirm(nfc->rpc_ctx, event, false);
    }
}

Nfc* nfc_alloc() {
    Nfc* nfc = malloc(sizeof(Nfc));

    nfc->worker = nfc_worker_alloc();
    nfc->view_dispatcher = view_dispatcher_alloc();
    nfc->scene_manager = scene_manager_alloc(&nfc_scene_handlers, nfc);
    view_dispatcher_enable_queue(nfc->view_dispatcher);
    view_dispatcher_set_event_callback_context(nfc->view_dispatcher, nfc);
    view_dispatcher_set_custom_event_callback(nfc->view_dispatcher, nfc_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(nfc->view_dispatcher, nfc_back_event_callback);

    // Nfc device
    nfc->dev = nfc_device_alloc();

    // Open GUI record
    nfc->gui = furi_record_open(RECORD_GUI);

    // Open Notification record
    nfc->notifications = furi_record_open(RECORD_NOTIFICATION);

    // Submenu
    nfc->submenu = submenu_alloc();
    view_dispatcher_add_view(nfc->view_dispatcher, NfcViewMenu, submenu_get_view(nfc->submenu));

    // Dialog
    nfc->dialog_ex = dialog_ex_alloc();
    view_dispatcher_add_view(
        nfc->view_dispatcher, NfcViewDialogEx, dialog_ex_get_view(nfc->dialog_ex));

    // Popup
    nfc->popup = popup_alloc();
    view_dispatcher_add_view(nfc->view_dispatcher, NfcViewPopup, popup_get_view(nfc->popup));

    // Loading
    nfc->loading = loading_alloc();
    view_dispatcher_add_view(nfc->view_dispatcher, NfcViewLoading, loading_get_view(nfc->loading));

    // Text Input
    nfc->text_input = text_input_alloc();
    view_dispatcher_add_view(
        nfc->view_dispatcher, NfcViewTextInput, text_input_get_view(nfc->text_input));

    // Byte Input
    nfc->byte_input = byte_input_alloc();
    view_dispatcher_add_view(
        nfc->view_dispatcher, NfcViewByteInput, byte_input_get_view(nfc->byte_input));

    // TextBox
    nfc->text_box = text_box_alloc();
    view_dispatcher_add_view(
        nfc->view_dispatcher, NfcViewTextBox, text_box_get_view(nfc->text_box));
    string_init(nfc->text_box_store);

    // Custom Widget
    nfc->widget = widget_alloc();
    view_dispatcher_add_view(nfc->view_dispatcher, NfcViewWidget, widget_get_view(nfc->widget));

    // Mifare Classic Dict Attack
    nfc->dict_attack = dict_attack_alloc();
    view_dispatcher_add_view(
        nfc->view_dispatcher, NfcViewDictAttack, dict_attack_get_view(nfc->dict_attack));

    // Detect Reader
    nfc->detect_reader = detect_reader_alloc();
    view_dispatcher_add_view(
        nfc->view_dispatcher, NfcViewDetectReader, detect_reader_get_view(nfc->detect_reader));

    // Generator
    nfc->generator = NULL;

    return nfc;
}

void nfc_free(Nfc* nfc) {
    furi_assert(nfc);

    if(nfc->rpc_state == NfcRpcStateEmulating) {
        // Stop worker
        nfc_worker_stop(nfc->worker);
    } else if(nfc->rpc_state == NfcRpcStateEmulated) {
        // Stop worker
        nfc_worker_stop(nfc->worker);
        // Save data in shadow file
        nfc_device_save_shadow(nfc->dev, nfc->dev->dev_name);
    }
    if(nfc->rpc_ctx) {
        rpc_system_app_send_exited(nfc->rpc_ctx);
        rpc_system_app_set_callback(nfc->rpc_ctx, NULL, NULL);
        nfc->rpc_ctx = NULL;
    }

    // Nfc device
    nfc_device_free(nfc->dev);

    // Submenu
    view_dispatcher_remove_view(nfc->view_dispatcher, NfcViewMenu);
    submenu_free(nfc->submenu);

    // DialogEx
    view_dispatcher_remove_view(nfc->view_dispatcher, NfcViewDialogEx);
    dialog_ex_free(nfc->dialog_ex);

    // Popup
    view_dispatcher_remove_view(nfc->view_dispatcher, NfcViewPopup);
    popup_free(nfc->popup);

    // Loading
    view_dispatcher_remove_view(nfc->view_dispatcher, NfcViewLoading);
    loading_free(nfc->loading);

    // TextInput
    view_dispatcher_remove_view(nfc->view_dispatcher, NfcViewTextInput);
    text_input_free(nfc->text_input);

    // ByteInput
    view_dispatcher_remove_view(nfc->view_dispatcher, NfcViewByteInput);
    byte_input_free(nfc->byte_input);

    // TextBox
    view_dispatcher_remove_view(nfc->view_dispatcher, NfcViewTextBox);
    text_box_free(nfc->text_box);
    string_clear(nfc->text_box_store);

    // Custom Widget
    view_dispatcher_remove_view(nfc->view_dispatcher, NfcViewWidget);
    widget_free(nfc->widget);

    // Mifare Classic Dict Attack
    view_dispatcher_remove_view(nfc->view_dispatcher, NfcViewDictAttack);
    dict_attack_free(nfc->dict_attack);

    // Detect Reader
    view_dispatcher_remove_view(nfc->view_dispatcher, NfcViewDetectReader);
    detect_reader_free(nfc->detect_reader);

    // Worker
    nfc_worker_stop(nfc->worker);
    nfc_worker_free(nfc->worker);

    // View Dispatcher
    view_dispatcher_free(nfc->view_dispatcher);

    // Scene Manager
    scene_manager_free(nfc->scene_manager);

    // GUI
    furi_record_close(RECORD_GUI);
    nfc->gui = NULL;

    // Notifications
    furi_record_close(RECORD_NOTIFICATION);
    nfc->notifications = NULL;

    free(nfc);
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

void nfc_blink_start(Nfc* nfc) {
    notification_message(nfc->notifications, &sequence_blink_start_blue);
}

void nfc_blink_stop(Nfc* nfc) {
    notification_message(nfc->notifications, &sequence_blink_stop);
}

void nfc_show_loading_popup(void* context, bool show) {
    Nfc* nfc = context;
    TaskHandle_t timer_task = xTaskGetHandle(configTIMER_SERVICE_TASK_NAME);

    if(show) {
        // Raise timer priority so that animations can play
        vTaskPrioritySet(timer_task, configMAX_PRIORITIES - 1);
        view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewLoading);
    } else {
        // Restore default timer priority
        vTaskPrioritySet(timer_task, configTIMER_TASK_PRIORITY);
    }
}

int32_t nfc_app(void* p) {
    Nfc* nfc = nfc_alloc();
    char* args = p;

    // Check argument and run corresponding scene
    if(args && strlen(args)) {
        nfc_device_set_loading_callback(nfc->dev, nfc_show_loading_popup, nfc);
        uint32_t rpc_ctx = 0;
        if(sscanf(p, "RPC %lX", &rpc_ctx) == 1) {
            nfc->rpc_ctx = (void*)rpc_ctx;
            rpc_system_app_set_callback(nfc->rpc_ctx, nfc_rpc_command_callback, nfc);
            rpc_system_app_send_started(nfc->rpc_ctx);
            view_dispatcher_attach_to_gui(
                nfc->view_dispatcher, nfc->gui, ViewDispatcherTypeDesktop);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRpc);
        } else {
            view_dispatcher_attach_to_gui(
                nfc->view_dispatcher, nfc->gui, ViewDispatcherTypeFullscreen);
            if(nfc_device_load(nfc->dev, p, true)) {
                if(nfc->dev->format == NfcDeviceSaveFormatMifareUl) {
                    scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightEmulate);
                } else if(nfc->dev->format == NfcDeviceSaveFormatMifareClassic) {
                    scene_manager_next_scene(nfc->scene_manager, NfcSceneMfClassicEmulate);
                } else {
                    scene_manager_next_scene(nfc->scene_manager, NfcSceneEmulateUid);
                }
            } else {
                // Exit app
                view_dispatcher_stop(nfc->view_dispatcher);
            }
        }
        nfc_device_set_loading_callback(nfc->dev, NULL, nfc);
    } else {
        view_dispatcher_attach_to_gui(
            nfc->view_dispatcher, nfc->gui, ViewDispatcherTypeFullscreen);
        scene_manager_next_scene(nfc->scene_manager, NfcSceneStart);
    }

    view_dispatcher_run(nfc->view_dispatcher);

    nfc_free(nfc);

    return 0;
}
