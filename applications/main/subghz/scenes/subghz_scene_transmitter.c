#include "../subghz_i.h"
#include "../views/transmitter.h"
#include <dolphin/dolphin.h>
#include <lib/subghz/protocols/keeloq.h>
#include <lib/subghz/protocols/star_line.h>

#include <lib/subghz/blocks/custom_btn.h>

void subghz_scene_transmitter_callback(SubGhzCustomEvent event, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, event);
}

bool subghz_scene_transmitter_update_data_show(void* context) {
    SubGhz* subghz = context;
    bool ret = false;
    if(subghz->txrx->decoder_result) {
        FuriString* key_str;
        FuriString* frequency_str;
        FuriString* modulation_str;

        key_str = furi_string_alloc();
        frequency_str = furi_string_alloc();
        modulation_str = furi_string_alloc();
        uint8_t show_button = 0;

        if(subghz_protocol_decoder_base_deserialize(
               subghz->txrx->decoder_result, subghz->txrx->fff_data) == SubGhzProtocolStatusOk) {
            subghz_protocol_decoder_base_get_string(subghz->txrx->decoder_result, key_str);

            if((subghz->txrx->decoder_result->protocol->flag & SubGhzProtocolFlag_Send) ==
               SubGhzProtocolFlag_Send) {
                show_button = 1;
            }

            subghz_get_frequency_modulation(subghz, frequency_str, modulation_str);
            subghz_view_transmitter_add_data_to_show(
                subghz->subghz_transmitter,
                furi_string_get_cstr(key_str),
                furi_string_get_cstr(frequency_str),
                furi_string_get_cstr(modulation_str),
                show_button);
            ret = true;
        }
        furi_string_free(frequency_str);
        furi_string_free(modulation_str);
        furi_string_free(key_str);
    }
    return ret;
}

void subghz_scene_transmitter_on_enter(void* context) {
    SubGhz* subghz = context;
    if(!subghz_scene_transmitter_update_data_show(subghz)) {
        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubGhzCustomEventViewTransmitterError);
    }

    subghz_view_transmitter_set_callback(
        subghz->subghz_transmitter, subghz_scene_transmitter_callback, subghz);

    subghz->state_notifications = SubGhzNotificationStateIDLE;
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdTransmitter);
}

bool subghz_scene_transmitter_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventViewTransmitterSendStart) {
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
                subghz_rx_end(subghz);
            }
            if((subghz->txrx->txrx_state == SubGhzTxRxStateIDLE) ||
               (subghz->txrx->txrx_state == SubGhzTxRxStateSleep)) {
                if(!subghz_tx_start(subghz, subghz->txrx->fff_data)) {
                    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowOnlyRx);
                } else {
                    subghz->state_notifications = SubGhzNotificationStateTx;
                    subghz_scene_transmitter_update_data_show(subghz);
                    DOLPHIN_DEED(DolphinDeedSubGhzSend);
                }
            }
            return true;
        } else if(event.event == SubGhzCustomEventViewTransmitterSendStop) {
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            if(subghz->txrx->txrx_state == SubGhzTxRxStateTx) {
                subghz_tx_stop(subghz);
                subghz_sleep(subghz);
            }
            if(subghz_custom_btn_get() != 0) {
                subghz_custom_btn_set(0);
                uint8_t tmp_counter = furi_hal_subghz_get_rolling_counter_mult();
                furi_hal_subghz_set_rolling_counter_mult(0);
                // Calling restore!
                if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
                    subghz_rx_end(subghz);
                }
                if((subghz->txrx->txrx_state == SubGhzTxRxStateIDLE) ||
                   (subghz->txrx->txrx_state == SubGhzTxRxStateSleep)) {
                    if(!subghz_tx_start(subghz, subghz->txrx->fff_data)) {
                        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowOnlyRx);
                    }
                }
                subghz_tx_stop(subghz);
                subghz_sleep(subghz);
                furi_hal_subghz_set_rolling_counter_mult(tmp_counter);
            }
            return true;
        } else if(event.event == SubGhzCustomEventViewTransmitterBack) {
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            scene_manager_search_and_switch_to_previous_scene(
                subghz->scene_manager, SubGhzSceneStart);
            return true;
        } else if(event.event == SubGhzCustomEventViewTransmitterError) {
            furi_string_set(subghz->error_str, "Protocol not\nfound!");
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowErrorSub);
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        if(subghz->state_notifications == SubGhzNotificationStateTx) {
            notification_message(subghz->notifications, &sequence_blink_magenta_10);
        }
        return true;
    }
    return false;
}

void subghz_scene_transmitter_on_exit(void* context) {
    SubGhz* subghz = context;
    subghz->state_notifications = SubGhzNotificationStateIDLE;
    keeloq_reset_mfname();
    keeloq_reset_kl_type();
    keeloq_reset_original_btn();
    subghz_custom_btns_reset();
    star_line_reset_mfname();
    star_line_reset_kl_type();
}
