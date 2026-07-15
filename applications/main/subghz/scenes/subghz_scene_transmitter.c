#include "../subghz_i.h"
#include "../views/transmitter.h"
#include <dolphin/dolphin.h>

#include <lib/subghz/blocks/custom_btn.h>

#include <lib/subghz/devices/devices.c>
#include "applications/main/subghz/helpers/subghz_txrx_i.h"
#include "lib/subghz/blocks/generic.h"

#define TAG "SubGhzSceneTransmitter"

void subghz_scene_transmitter_callback(SubGhzCustomEvent event, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, event);
}

bool subghz_scene_transmitter_update_data_show(void* context) {
    SubGhz* subghz = context;
    bool ret = false;
    SubGhzProtocolDecoderBase* decoder = subghz_txrx_get_decoder(subghz->txrx);

    if(decoder) {
        FuriString* key_str = furi_string_alloc();
        FuriString* frequency_str = furi_string_alloc();
        FuriString* modulation_str = furi_string_alloc();

        if(subghz_protocol_decoder_base_deserialize(
               decoder, subghz_txrx_get_fff_data(subghz->txrx)) == SubGhzProtocolStatusOk) {
            subghz_protocol_decoder_base_get_string(decoder, key_str);

            subghz_txrx_get_frequency_and_modulation(
                subghz->txrx, frequency_str, modulation_str, false);
            subghz_view_transmitter_add_data_to_show(
                subghz->subghz_transmitter,
                furi_string_get_cstr(key_str),
                furi_string_get_cstr(frequency_str),
                furi_string_get_cstr(modulation_str),
                subghz_txrx_protocol_is_transmittable(subghz->txrx, false));

            ret = true;
        }
        furi_string_free(frequency_str);
        furi_string_free(modulation_str);
        furi_string_free(key_str);
    }
    subghz_view_transmitter_set_radio_device_type(
        subghz->subghz_transmitter, subghz_txrx_radio_device_get(subghz->txrx));
    return ret;
}

void subghz_scene_transmitter_on_enter(void* context) {
    SubGhz* subghz = context;

    subghz_custom_btns_reset();

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
    // key concept: we start endless TX until user release OK button, and after this we send last
    // protocols repeats - this guarantee that one press OK will
    // be guarantee send the required minimum protocol data packets
    // for all of this we use subghz_block_generic_global.endless_tx in protocols _yield function.
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventViewTransmitterSendStart) {
            // user press OK - start endless TX
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            subghz_block_generic_global.endless_tx = true;
            if(subghz_tx_start(subghz, subghz_txrx_get_fff_data(subghz->txrx))) {
                subghz->state_notifications = SubGhzNotificationStateTx;
                subghz_scene_transmitter_update_data_show(subghz);
                dolphin_deed(DolphinDeedSubGhzSend);
            }
            return true;
        } else if(event.event == SubGhzCustomEventViewTransmitterSendStop) {
            // user release OK
            // we switch off endless_tx - that mean protocols yield finish endless transmission,
            // send upload "repeat=xx" times, and after will be stoped by tick event down this code.
            subghz_block_generic_global.endless_tx = false;
            return true;
        } else if(event.event == SubGhzCustomEventViewTransmitterBack) {
            scene_manager_search_and_switch_to_previous_scene(
                subghz->scene_manager, SubGhzSceneStart);
            return true;
        } else if(event.event == SubGhzCustomEventViewTransmitterError) {
            furi_string_set(subghz->error_str, "Protocol not\nfound!");
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowErrorSub);
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        if(subghz->state_notifications == SubGhzNotificationStateTx) {
            // if hardware TX still working at this time so we just blink led and return
            if(!subghz_devices_is_async_complete_tx(subghz->txrx->radio_device)) {
                notification_message(subghz->notifications, &sequence_blink_magenta_10);
                return true;
                // if hardware TX was stoped so we stop TX correctly
            } else {
                subghz->state_notifications = SubGhzNotificationStateIDLE;
                subghz_txrx_stop(subghz->txrx);
                if(subghz_custom_btn_get() != SUBGHZ_CUSTOM_BTN_OK) {
                    subghz_custom_btn_set(SUBGHZ_CUSTOM_BTN_OK);
                    int32_t tmp_counter = furi_hal_subghz_get_rolling_counter_mult();
                    furi_hal_subghz_set_rolling_counter_mult(0);
                    // Calling restore!
                    subghz_tx_start(subghz, subghz_txrx_get_fff_data(subghz->txrx));
                    subghz_txrx_stop(subghz->txrx);
                    // Calling restore 2nd time special for FAAC SLH!
                    // TODO: Find better way to restore after custom button is used!!!
                    subghz_tx_start(subghz, subghz_txrx_get_fff_data(subghz->txrx));
                    subghz_txrx_stop(subghz->txrx);
                    furi_hal_subghz_set_rolling_counter_mult(tmp_counter);
                }
            }
        }
        return true;
    }
    return false;
}

void subghz_scene_transmitter_on_exit(void* context) {
    SubGhz* subghz = context;
    subghz->state_notifications = SubGhzNotificationStateIDLE;

    subghz_txrx_reset_dynamic_and_custom_btns(subghz->txrx);
}
