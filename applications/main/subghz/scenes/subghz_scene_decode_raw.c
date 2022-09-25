#include "../subghz_i.h"
#include "../views/receiver.h"
#include <lib/subghz/protocols/raw.h>

#include <lib/subghz/subghz_file_encoder_worker.h>

#define TAG "SubGhzDecodeRaw"
#define SAMPLES_TO_READ_PER_TICK 400

// TODO:
// [X] Remember RAW file after decoding
// [X] Decode in tick events instead of on_enter
// [X] Make "Config" label optional in subghz_view_receiver_draw (../views/receiver.c)
// [X] Make "Scanning..." label optional in subghz_view_receiver_draw (../views/receiver.c)
// [X] Add Decoding logo
// [ ] Design nicer Decoding logo
// [X] Check progress in stream_buffer, instead of raw stream
// [X] Blink led while decoding
// [X] Stop rx blink (blue, fast) on history item view
// [X] Don't reparse file on back
// [X] Fix: RX animation+LED returning from decoded detail view
// [X] Find good value for SAMPLES_TO_READ_PER_TICK
// [X] Fix: read errors (slow flash) after aborting decode read

typedef enum {
    SubGhzDecodeRawStateStart,
    SubGhzDecodeRawStateLoading,
    SubGhzDecodeRawStateLoaded,
} SubGhzDecodeRawState;

SubGhzDecodeRawState decode_raw_state = SubGhzDecodeRawStateStart;
SubGhzFileEncoderWorker* file_worker_encoder;

static void subghz_scene_receiver_update_statusbar(void* context) {
    SubGhz* subghz = context;
    string_t history_stat_str;
    string_init(history_stat_str);
    if(!subghz_history_get_text_space_left(subghz->txrx->history, history_stat_str)) {
        string_t frequency_str;
        string_t modulation_str;

        string_init(frequency_str);
        string_init(modulation_str);

        subghz_get_frequency_modulation(subghz, frequency_str, modulation_str);

        subghz_view_receiver_add_data_statusbar(
            subghz->subghz_receiver,
            string_get_cstr(frequency_str),
            string_get_cstr(modulation_str),
            string_get_cstr(history_stat_str));

        string_clear(frequency_str);
        string_clear(modulation_str);
    } else {
        subghz_view_receiver_add_data_statusbar(
            subghz->subghz_receiver, string_get_cstr(history_stat_str), "", "");
    }
    string_clear(history_stat_str);
}

void subghz_scene_decode_raw_callback(SubGhzCustomEvent event, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, event);
}

static void subghz_scene_add_to_history_callback(
    SubGhzReceiver* receiver,
    SubGhzProtocolDecoderBase* decoder_base,
    void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    string_t str_buff;
    string_init(str_buff);

    if(subghz_history_add_to_history(subghz->txrx->history, decoder_base, subghz->txrx->preset)) {
        string_reset(str_buff);

        subghz->state_notifications = SubGhzNotificationStateRxDone;

        subghz_history_get_text_item_menu(
            subghz->txrx->history, str_buff, subghz_history_get_item(subghz->txrx->history) - 1);
        subghz_view_receiver_add_item_to_menu(
            subghz->subghz_receiver,
            string_get_cstr(str_buff),
            subghz_history_get_type_protocol(
                subghz->txrx->history, subghz_history_get_item(subghz->txrx->history) - 1));

        subghz_scene_receiver_update_statusbar(subghz);
    }
    subghz_receiver_reset(receiver);
    string_clear(str_buff);
}

bool subghz_scene_decode_raw_start(SubGhz* subghz) {
    string_t file_name;
    string_init(file_name);
    bool success = false;
    do {
        if(!flipper_format_rewind(subghz->txrx->fff_data)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }

        if(!flipper_format_read_string(subghz->txrx->fff_data, "File_name", file_name)) {
            FURI_LOG_E(TAG, "Missing File_name");
            break;
        }

        success = true;
    } while(false);

    if(success) {
        //FURI_LOG_I(TAG, "Listening at \033[0;33m%s\033[0m.", string_get_cstr(file_name));

        file_worker_encoder = subghz_file_encoder_worker_alloc();
        if(subghz_file_encoder_worker_start(file_worker_encoder, string_get_cstr(file_name))) {
            //the worker needs a file in order to open and read part of the file
            furi_delay_ms(100);
        } else {
            success = false;
        }

        if(!success) {
            subghz_file_encoder_worker_free(file_worker_encoder);
        }
    }

    string_clear(file_name);
    return success;
}

bool subghz_scene_decode_raw_next(SubGhz* subghz) {
    LevelDuration level_duration;

    for(uint32_t read = SAMPLES_TO_READ_PER_TICK; read > 0; --read) {
        level_duration = subghz_file_encoder_worker_get_level_duration(file_worker_encoder);
        if(!level_duration_is_reset(level_duration)) {
            bool level = level_duration_get_level(level_duration);
            uint32_t duration = level_duration_get_duration(level_duration);
            subghz_receiver_decode(subghz->txrx->receiver, level, duration);
        } else {
            decode_raw_state = SubGhzDecodeRawStateLoaded;
            subghz->state_notifications = SubGhzNotificationStateIDLE;

            subghz_view_receiver_add_data_progress(subghz->subghz_receiver, "100%");
            return false; // No more samples available
        }
    }

    // Update progress info
    string_t progress_str;
    string_init(progress_str);
    subghz_file_encoder_worker_get_text_progress(file_worker_encoder, progress_str);

    subghz_view_receiver_add_data_progress(subghz->subghz_receiver, string_get_cstr(progress_str));

    string_clear(progress_str);

    return true; // More samples available
}

void subghz_scene_decode_raw_on_enter(void* context) {
    SubGhz* subghz = context;

    string_t str_buff;
    string_init(str_buff);

    subghz_view_receiver_set_lock(subghz->subghz_receiver, subghz->lock);
    subghz_view_receiver_set_mode(subghz->subghz_receiver, SubGhzViewReceiverModeFile);
    subghz_view_receiver_set_callback(
        subghz->subghz_receiver, subghz_scene_decode_raw_callback, subghz);

    subghz_receiver_set_rx_callback(
        subghz->txrx->receiver, subghz_scene_add_to_history_callback, subghz);

    // make sure we're not in auto-detect mode, which is only meant for the Read app
    subghz_protocol_decoder_raw_set_auto_mode(
        subghz_receiver_search_decoder_base_by_name(
            subghz->txrx->receiver, SUBGHZ_PROTOCOL_RAW_NAME),
        false);
    subghz_receiver_set_filter(subghz->txrx->receiver, SubGhzProtocolFlag_Decodable);

    if(decode_raw_state == SubGhzDecodeRawStateStart) {
        //Decode RAW to history
        subghz_history_reset(subghz->txrx->history);
        if(subghz_scene_decode_raw_start(subghz)) {
            decode_raw_state = SubGhzDecodeRawStateLoading;
            subghz->state_notifications = SubGhzNotificationStateRx;
        }
    } else {
        //Load history to receiver
        subghz_view_receiver_exit(subghz->subghz_receiver);
        for(uint8_t i = 0; i < subghz_history_get_item(subghz->txrx->history); i++) {
            string_reset(str_buff);
            subghz_history_get_text_item_menu(subghz->txrx->history, str_buff, i);
            subghz_view_receiver_add_item_to_menu(
                subghz->subghz_receiver,
                string_get_cstr(str_buff),
                subghz_history_get_type_protocol(subghz->txrx->history, i));
        }
        string_clear(str_buff);
        subghz_view_receiver_set_idx_menu(subghz->subghz_receiver, subghz->txrx->idx_menu_chosen);
    }

    subghz_scene_receiver_update_statusbar(subghz);

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdReceiver);
}

bool subghz_scene_decode_raw_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SubGhzCustomEventViewReceiverBack:
            decode_raw_state = SubGhzDecodeRawStateStart;
            subghz->txrx->idx_menu_chosen = 0;
            subghz_receiver_set_rx_callback(subghz->txrx->receiver, NULL, subghz);

            if(subghz_file_encoder_worker_is_running(file_worker_encoder)) {
                subghz_file_encoder_worker_stop(file_worker_encoder);
            }
            subghz_file_encoder_worker_free(file_worker_encoder);

            subghz->state_notifications = SubGhzNotificationStateIDLE;
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneReadRAW, SubGhzCustomEventManagerNoSet);
            scene_manager_search_and_switch_to_previous_scene(
                subghz->scene_manager, SubGhzSceneMoreRAW);
            consumed = true;
            break;
        case SubGhzCustomEventViewReceiverOK:
            subghz->txrx->idx_menu_chosen =
                subghz_view_receiver_get_idx_menu(subghz->subghz_receiver);
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            subghz->in_decoder_scene = true;
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReceiverInfo);
            consumed = true;
            break;
        case SubGhzCustomEventViewReceiverConfig:
            FURI_LOG_W(TAG, "No config options");
            consumed = true;
            break;
        case SubGhzCustomEventViewReceiverOffDisplay:
            notification_message(subghz->notifications, &sequence_display_backlight_off);
            consumed = true;
            break;
        case SubGhzCustomEventViewReceiverUnlock:
            subghz->lock = SubGhzLockOff;
            consumed = true;
            break;
        default:
            break;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        switch(subghz->state_notifications) {
        case SubGhzNotificationStateRx:
            notification_message(subghz->notifications, &sequence_blink_cyan_10);
            break;
        case SubGhzNotificationStateRxDone:
            notification_message(subghz->notifications, &subghz_sequence_rx);
            subghz->state_notifications = SubGhzNotificationStateRx;
            break;
        default:
            break;
        }

        switch(decode_raw_state) {
        case SubGhzDecodeRawStateLoading:
            subghz_scene_decode_raw_next(subghz);
            break;
        default:
            break;
        }
    }
    return consumed;
}

void subghz_scene_decode_raw_on_exit(void* context) {
    UNUSED(context);
}
