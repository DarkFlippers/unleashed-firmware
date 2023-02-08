#include "../subghz_i.h"
#include "../helpers/subghz_custom_event.h"

void subghz_scene_receiver_info_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;

    if((result == GuiButtonTypeCenter) && (type == InputTypePress)) {
        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubGhzCustomEventSceneReceiverInfoTxStart);
    } else if((result == GuiButtonTypeCenter) && (type == InputTypeRelease)) {
        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubGhzCustomEventSceneReceiverInfoTxStop);
    } else if((result == GuiButtonTypeRight) && (type == InputTypeShort)) {
        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubGhzCustomEventSceneReceiverInfoSave);
    }
}

static bool subghz_scene_receiver_info_update_parser(void* context) {
    SubGhz* subghz = context;
    subghz->txrx->decoder_result = subghz_receiver_search_decoder_base_by_name(
        subghz->txrx->receiver,
        subghz_history_get_protocol_name(subghz->txrx->history, subghz->txrx->idx_menu_chosen));
    if(subghz->txrx->decoder_result) {
        subghz_protocol_decoder_base_deserialize(
            subghz->txrx->decoder_result,
            subghz_history_get_raw_data(subghz->txrx->history, subghz->txrx->idx_menu_chosen));

        SubGhzRadioPreset* preset =
            subghz_history_get_radio_preset(subghz->txrx->history, subghz->txrx->idx_menu_chosen);
        subghz_preset_init(
            subghz,
            furi_string_get_cstr(preset->name),
            preset->frequency,
            preset->data,
            preset->data_size);

        return true;
    }
    return false;
}

void subghz_scene_receiver_info_on_enter(void* context) {
    SubGhz* subghz = context;

    if(subghz_scene_receiver_info_update_parser(subghz)) {
        FuriString* frequency_str;
        FuriString* modulation_str;
        FuriString* text;

        frequency_str = furi_string_alloc();
        modulation_str = furi_string_alloc();
        text = furi_string_alloc();

        subghz_get_frequency_modulation(subghz, frequency_str, modulation_str);
        widget_add_string_element(
            subghz->widget,
            78,
            0,
            AlignLeft,
            AlignTop,
            FontSecondary,
            furi_string_get_cstr(frequency_str));

        widget_add_string_element(
            subghz->widget,
            113,
            0,
            AlignLeft,
            AlignTop,
            FontSecondary,
            furi_string_get_cstr(modulation_str));
        subghz_protocol_decoder_base_get_string(subghz->txrx->decoder_result, text);
        widget_add_string_multiline_element(
            subghz->widget, 0, 0, AlignLeft, AlignTop, FontSecondary, furi_string_get_cstr(text));

        furi_string_free(frequency_str);
        furi_string_free(modulation_str);
        furi_string_free(text);

        if((subghz->txrx->decoder_result->protocol->flag & SubGhzProtocolFlag_Save) ==
           SubGhzProtocolFlag_Save) {
            widget_add_button_element(
                subghz->widget,
                GuiButtonTypeRight,
                "Save",
                subghz_scene_receiver_info_callback,
                subghz);
        }
        if(((subghz->txrx->decoder_result->protocol->flag & SubGhzProtocolFlag_Send) ==
            SubGhzProtocolFlag_Send) &&
           subghz->txrx->decoder_result->protocol->encoder->deserialize &&
           subghz->txrx->decoder_result->protocol->type == SubGhzProtocolTypeStatic) {
            widget_add_button_element(
                subghz->widget,
                GuiButtonTypeCenter,
                "Send",
                subghz_scene_receiver_info_callback,
                subghz);
        }
    } else {
        widget_add_icon_element(subghz->widget, 37, 15, &I_DolphinCommon_56x48);
        widget_add_string_element(
            subghz->widget, 13, 8, AlignLeft, AlignBottom, FontSecondary, "Error history parse.");
    }

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdWidget);
}

bool subghz_scene_receiver_info_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventSceneReceiverInfoTxStart) {
            //CC1101 Stop RX -> Start TX
            if(subghz->txrx->hopper_state != SubGhzHopperStateOFF) {
                subghz->txrx->hopper_state = SubGhzHopperStatePause;
            }
            if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
                subghz_rx_end(subghz);
            }
            if(!subghz_scene_receiver_info_update_parser(subghz)) {
                return false;
            }
            if(subghz->txrx->txrx_state == SubGhzTxRxStateIDLE ||
               subghz->txrx->txrx_state == SubGhzTxRxStateSleep) {
                if(!subghz_tx_start(
                       subghz,
                       subghz_history_get_raw_data(
                           subghz->txrx->history, subghz->txrx->idx_menu_chosen))) {
                    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowOnlyRx);
                    if(subghz->txrx->txrx_state == SubGhzTxRxStateTx) {
                        subghz_tx_stop(subghz);
                    }
                    if(subghz->txrx->txrx_state == SubGhzTxRxStateIDLE) {
                        subghz_begin(
                            subghz,
                            subghz_setting_get_preset_data_by_name(
                                subghz->setting,
                                furi_string_get_cstr(subghz->txrx->preset->name)));
                        subghz_rx(subghz, subghz->txrx->preset->frequency);
                    }
                    if(subghz->txrx->hopper_state == SubGhzHopperStatePause) {
                        subghz->txrx->hopper_state = SubGhzHopperStateRunnig;
                    }
                    subghz->state_notifications = SubGhzNotificationStateRx;
                } else {
                    subghz->state_notifications = SubGhzNotificationStateTx;
                }
            }
            return true;
        } else if(event.event == SubGhzCustomEventSceneReceiverInfoTxStop) {
            //CC1101 Stop Tx -> Start RX
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            if(subghz->txrx->txrx_state == SubGhzTxRxStateTx) {
                subghz_tx_stop(subghz);
            }
            if(subghz->txrx->txrx_state == SubGhzTxRxStateIDLE) {
                subghz_begin(
                    subghz,
                    subghz_setting_get_preset_data_by_name(
                        subghz->setting, furi_string_get_cstr(subghz->txrx->preset->name)));
                subghz_rx(subghz, subghz->txrx->preset->frequency);
            }
            if(subghz->txrx->hopper_state == SubGhzHopperStatePause) {
                subghz->txrx->hopper_state = SubGhzHopperStateRunnig;
            }
            subghz->state_notifications = SubGhzNotificationStateRx;
            return true;
        } else if(event.event == SubGhzCustomEventSceneReceiverInfoSave) {
            //CC1101 Stop RX -> Save
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            subghz->txrx->hopper_state = SubGhzHopperStateOFF;
            if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
                subghz_rx_end(subghz);
                subghz_sleep(subghz);
            }
            if(!subghz_scene_receiver_info_update_parser(subghz)) {
                return false;
            }

            if((subghz->txrx->decoder_result->protocol->flag & SubGhzProtocolFlag_Save) ==
               SubGhzProtocolFlag_Save) {
                subghz_file_name_clear(subghz);
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveName);
            }
            return true;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        if(subghz->txrx->hopper_state != SubGhzHopperStateOFF) {
            subghz_hopper_update(subghz);
        }
        switch(subghz->state_notifications) {
        case SubGhzNotificationStateTx:
            notification_message(subghz->notifications, &sequence_blink_magenta_10);
            break;
        case SubGhzNotificationStateRx:
            notification_message(subghz->notifications, &sequence_blink_cyan_10);
            break;
        case SubGhzNotificationStateRxDone:
            notification_message(subghz->notifications, &sequence_blink_green_100);
            subghz->state_notifications = SubGhzNotificationStateRx;
            break;
        default:
            break;
        }
    }
    return false;
}

void subghz_scene_receiver_info_on_exit(void* context) {
    SubGhz* subghz = context;
    widget_reset(subghz->widget);
}
