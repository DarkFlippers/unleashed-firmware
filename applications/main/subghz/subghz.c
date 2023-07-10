/* Abandon hope, all ye who enter here. */

#include <subghz/types.h>
#include <lib/toolbox/path.h>
#include "subghz_i.h"
#include <lib/subghz/protocols/protocol_items.h>

#define TAG "SubGhzApp"

bool subghz_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    SubGhz* subghz = context;
    return scene_manager_handle_custom_event(subghz->scene_manager, event);
}

bool subghz_back_event_callback(void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    return scene_manager_handle_back_event(subghz->scene_manager);
}

void subghz_tick_event_callback(void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    scene_manager_handle_tick_event(subghz->scene_manager);
}

static void subghz_rpc_command_callback(RpcAppSystemEvent event, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;

    furi_assert(subghz->rpc_ctx);

    if(event == RpcAppEventSessionClose) {
        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubGhzCustomEventSceneRpcSessionClose);
        rpc_system_app_set_callback(subghz->rpc_ctx, NULL, NULL);
        subghz->rpc_ctx = NULL;
    } else if(event == RpcAppEventAppExit) {
        view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventSceneExit);
    } else if(event == RpcAppEventLoadFile) {
        view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventSceneRpcLoad);
    } else if(event == RpcAppEventButtonPress) {
        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubGhzCustomEventSceneRpcButtonPress);
    } else if(event == RpcAppEventButtonRelease) {
        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubGhzCustomEventSceneRpcButtonRelease);
    } else {
        rpc_system_app_confirm(subghz->rpc_ctx, event, false);
    }
}

static void subghz_load_custom_presets(SubGhzSetting* setting) {
    furi_assert(setting);

    const char* presets[][2] = {
        {"FM95",
         "02 0D 0B 06 08 32 07 04 14 00 13 02 12 04 11 83 10 67 15 24 18 18 19 16 1D 91 1C 00 1B 07 20 FB 22 10 21 56 00 00 C0 00 00 00 00 00 00 00"},

        // #2-FSK 200khz BW / 135kHz Filter/ 15.86Khz Deviation + Ramping
        {"FM15k",
         "02 0D 03 47 08 32 0B 06 15 32 14 00 13 00 12 00 11 32 10 A7 18 18 19 1D 1D 92 1C 00 1B 04 20 FB 22 17 21 B6 00 00 00 12 0E 34 60 C5 C1 C0"},

        // Pagers
        {"Pagers",
         "02 0D 07 04 08 32 0B 06 10 64 11 93 12 0C 13 02 14 00 15 15 18 18 19 16 1B 07 1C 00 1D 91 20 FB 21 56 22 10 00 00 C0 00 00 00 00 00 00 00"},

        // # HND - FM preset
        {"HND_1",
         "02 0D 0B 06 08 32 07 04 14 00 13 02 12 04 11 36 10 69 15 32 18 18 19 16 1D 91 1C 00 1B 07 20 FB 22 10 21 56 00 00 C0 00 00 00 00 00 00 00"},
    };

    FlipperFormat* fff_temp = flipper_format_string_alloc();

    for(uint8_t i = 0; i < COUNT_OF(presets); i++) {
        flipper_format_insert_or_update_string_cstr(fff_temp, "Custom_preset_data", presets[i][1]);

        flipper_format_rewind(fff_temp);
        subghz_setting_load_custom_preset(setting, presets[i][0], fff_temp);
    }

    flipper_format_free(fff_temp);

#ifdef FURI_DEBUG
    subghz_setting_customs_presets_to_log(setting);
#endif
}

SubGhz* subghz_alloc(bool alloc_for_tx_only) {
    SubGhz* subghz = malloc(sizeof(SubGhz));

    subghz->file_path = furi_string_alloc();
    subghz->file_path_tmp = furi_string_alloc();

    // GUI
    subghz->gui = furi_record_open(RECORD_GUI);

    // View Dispatcher
    subghz->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(subghz->view_dispatcher);

    subghz->scene_manager = scene_manager_alloc(&subghz_scene_handlers, subghz);
    view_dispatcher_set_event_callback_context(subghz->view_dispatcher, subghz);
    view_dispatcher_set_custom_event_callback(
        subghz->view_dispatcher, subghz_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        subghz->view_dispatcher, subghz_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        subghz->view_dispatcher, subghz_tick_event_callback, 100);

    // Open Notification record
    subghz->notifications = furi_record_open(RECORD_NOTIFICATION);

    subghz->txrx = subghz_txrx_alloc();

    if(!alloc_for_tx_only) {
        // SubMenu
        subghz->submenu = submenu_alloc();
        view_dispatcher_add_view(
            subghz->view_dispatcher, SubGhzViewIdMenu, submenu_get_view(subghz->submenu));

        // Receiver
        subghz->subghz_receiver = subghz_view_receiver_alloc();
        view_dispatcher_add_view(
            subghz->view_dispatcher,
            SubGhzViewIdReceiver,
            subghz_view_receiver_get_view(subghz->subghz_receiver));
    }
    // Popup
    subghz->popup = popup_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher, SubGhzViewIdPopup, popup_get_view(subghz->popup));
    if(!alloc_for_tx_only) {
        // Text Input
        subghz->text_input = text_input_alloc();
        view_dispatcher_add_view(
            subghz->view_dispatcher,
            SubGhzViewIdTextInput,
            text_input_get_view(subghz->text_input));

        // Byte Input
        subghz->byte_input = byte_input_alloc();
        view_dispatcher_add_view(
            subghz->view_dispatcher,
            SubGhzViewIdByteInput,
            byte_input_get_view(subghz->byte_input));

        // Custom Widget
        subghz->widget = widget_alloc();
        view_dispatcher_add_view(
            subghz->view_dispatcher, SubGhzViewIdWidget, widget_get_view(subghz->widget));
    }
    //Dialog
    subghz->dialogs = furi_record_open(RECORD_DIALOGS);

    // Transmitter
    subghz->subghz_transmitter = subghz_view_transmitter_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewIdTransmitter,
        subghz_view_transmitter_get_view(subghz->subghz_transmitter));
    if(!alloc_for_tx_only) {
        // Variable Item List
        subghz->variable_item_list = variable_item_list_alloc();
        view_dispatcher_add_view(
            subghz->view_dispatcher,
            SubGhzViewIdVariableItemList,
            variable_item_list_get_view(subghz->variable_item_list));

        // Frequency Analyzer
        // View knows too much
        subghz->subghz_frequency_analyzer = subghz_frequency_analyzer_alloc(subghz->txrx);
        view_dispatcher_add_view(
            subghz->view_dispatcher,
            SubGhzViewIdFrequencyAnalyzer,
            subghz_frequency_analyzer_get_view(subghz->subghz_frequency_analyzer));
    }
    // Read RAW
    subghz->subghz_read_raw = subghz_read_raw_alloc(alloc_for_tx_only);
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewIdReadRAW,
        subghz_read_raw_get_view(subghz->subghz_read_raw));

    //init threshold rssi
    subghz->threshold_rssi = subghz_threshold_rssi_alloc();

    //init TxRx & Protocol & History & KeyBoard
    subghz_unlock(subghz);

    SubGhzSetting* setting = subghz_txrx_get_setting(subghz->txrx);

    subghz_load_custom_presets(setting);

    // Load last used values for Read, Read RAW, etc. or default
    subghz->last_settings = subghz_last_settings_alloc();
    subghz_last_settings_load(subghz->last_settings, 0);
    if(!alloc_for_tx_only) {
#if FURI_DEBUG
        FURI_LOG_D(
            TAG,
            "last frequency: %ld, preset: %ld",
            subghz->last_settings->frequency,
            subghz->last_settings->preset);
#endif
        subghz_setting_set_default_frequency(setting, subghz->last_settings->frequency);
    }

    if(!alloc_for_tx_only) {
        subghz_txrx_set_preset(subghz->txrx, "AM650", subghz->last_settings->frequency, NULL, 0);
    }

    subghz_rx_key_state_set(subghz, SubGhzRxKeyStateIDLE);

    if(!alloc_for_tx_only) {
        subghz->history = subghz_history_alloc();
    }

    subghz->secure_data = malloc(sizeof(SecureData));

    subghz->filter = SubGhzProtocolFlag_Decodable;
    subghz->ignore_filter = 0x0;
    subghz_txrx_receiver_set_filter(subghz->txrx, subghz->filter);
    subghz_txrx_set_need_save_callback(subghz->txrx, subghz_save_to_file, subghz);

    //Init Error_str
    subghz->error_str = furi_string_alloc();

    return subghz;
}

void subghz_free(SubGhz* subghz, bool alloc_for_tx_only) {
    furi_assert(subghz);

    if(subghz->rpc_ctx) {
        rpc_system_app_set_callback(subghz->rpc_ctx, NULL, NULL);
        rpc_system_app_send_exited(subghz->rpc_ctx);
        subghz_blink_stop(subghz);
        subghz->rpc_ctx = NULL;
    }

    subghz_txrx_speaker_off(subghz->txrx);
    subghz_txrx_stop(subghz->txrx);
    subghz_txrx_sleep(subghz->txrx);

    if(!alloc_for_tx_only) {
        // Receiver
        view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdReceiver);
        subghz_view_receiver_free(subghz->subghz_receiver);

        // TextInput
        view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdTextInput);
        text_input_free(subghz->text_input);

        // ByteInput
        view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdByteInput);
        byte_input_free(subghz->byte_input);

        // Custom Widget
        view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdWidget);
        widget_free(subghz->widget);
    }
    //Dialog
    furi_record_close(RECORD_DIALOGS);

    // Transmitter
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdTransmitter);
    subghz_view_transmitter_free(subghz->subghz_transmitter);
    if(!alloc_for_tx_only) {
        // Variable Item List
        view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdVariableItemList);
        variable_item_list_free(subghz->variable_item_list);

        // Frequency Analyzer
        view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdFrequencyAnalyzer);
        subghz_frequency_analyzer_free(subghz->subghz_frequency_analyzer);
    }
    // Read RAW
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdReadRAW);
    subghz_read_raw_free(subghz->subghz_read_raw);
    if(!alloc_for_tx_only) {
        // Submenu
        view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdMenu);
        submenu_free(subghz->submenu);
    }
    // Popup
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdPopup);
    popup_free(subghz->popup);

    // Scene manager
    scene_manager_free(subghz->scene_manager);

    // View Dispatcher
    view_dispatcher_free(subghz->view_dispatcher);

    // GUI
    furi_record_close(RECORD_GUI);
    subghz->gui = NULL;

    subghz_last_settings_free(subghz->last_settings);

    // threshold rssi
    subghz_threshold_rssi_free(subghz->threshold_rssi);

    if(!alloc_for_tx_only) {
        subghz_history_free(subghz->history);
    }

    free(subghz->secure_data);

    //TxRx
    subghz_txrx_free(subghz->txrx);

    //Error string
    furi_string_free(subghz->error_str);

    // Notifications
    furi_record_close(RECORD_NOTIFICATION);
    subghz->notifications = NULL;

    // Path strings
    furi_string_free(subghz->file_path);
    furi_string_free(subghz->file_path_tmp);

    // The rest
    free(subghz);
}

int32_t subghz_app(void* p) {
    bool alloc_for_tx;
    if(p && strlen(p)) {
        alloc_for_tx = true;
    } else {
        alloc_for_tx = false;
    }

    SubGhz* subghz = subghz_alloc(alloc_for_tx);

    if(alloc_for_tx) {
        subghz->raw_send_only = true;
    } else {
        subghz->raw_send_only = false;
    }

    // Check argument and run corresponding scene
    if(p && strlen(p)) {
        uint32_t rpc_ctx = 0;

        if(sscanf(p, "RPC %lX", &rpc_ctx) == 1) {
            subghz->rpc_ctx = (void*)rpc_ctx;
            rpc_system_app_set_callback(subghz->rpc_ctx, subghz_rpc_command_callback, subghz);
            rpc_system_app_send_started(subghz->rpc_ctx);
            view_dispatcher_attach_to_gui(
                subghz->view_dispatcher, subghz->gui, ViewDispatcherTypeDesktop);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneRpc);
        } else {
            view_dispatcher_attach_to_gui(
                subghz->view_dispatcher, subghz->gui, ViewDispatcherTypeFullscreen);
            if(subghz_key_load(subghz, p, true)) {
                furi_string_set(subghz->file_path, (const char*)p);

                if(subghz_get_load_type_file(subghz) == SubGhzLoadTypeFileRaw) {
                    //Load Raw TX
                    subghz_rx_key_state_set(subghz, SubGhzRxKeyStateRAWLoad);
                    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReadRAW);
                } else {
                    //Load transmitter TX
                    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneTransmitter);
                }
            } else {
                //exit app
                scene_manager_stop(subghz->scene_manager);
                view_dispatcher_stop(subghz->view_dispatcher);
            }
        }
    } else {
        view_dispatcher_attach_to_gui(
            subghz->view_dispatcher, subghz->gui, ViewDispatcherTypeFullscreen);
        furi_string_set(subghz->file_path, SUBGHZ_APP_FOLDER);
        if(subghz_txrx_is_database_loaded(subghz->txrx)) {
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneStart);
        } else {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneShowError, SubGhzCustomEventManagerSet);
            furi_string_set(
                subghz->error_str,
                "No SD card or\ndatabase found.\nSome app function\nmay be reduced.");
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
        }
    }

    furi_hal_power_suppress_charge_enter();

    view_dispatcher_run(subghz->view_dispatcher);

    furi_hal_power_suppress_charge_exit();

    subghz_free(subghz, alloc_for_tx);

    return 0;
}
