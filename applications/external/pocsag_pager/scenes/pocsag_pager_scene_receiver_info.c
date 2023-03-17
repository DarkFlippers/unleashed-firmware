#include "../pocsag_pager_app_i.h"
#include "../views/pocsag_pager_receiver.h"

void pocsag_pager_scene_receiver_info_callback(PCSGCustomEvent event, void* context) {
    furi_assert(context);
    POCSAGPagerApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

static void pocsag_pager_scene_receiver_info_add_to_history_callback(
    SubGhzReceiver* receiver,
    SubGhzProtocolDecoderBase* decoder_base,
    void* context) {
    furi_assert(context);
    POCSAGPagerApp* app = context;

    if(pcsg_history_add_to_history(app->txrx->history, decoder_base, app->txrx->preset) ==
       PCSGHistoryStateAddKeyUpdateData) {
        pcsg_view_receiver_info_update(
            app->pcsg_receiver_info,
            pcsg_history_get_raw_data(app->txrx->history, app->txrx->idx_menu_chosen));
        subghz_receiver_reset(receiver);

        notification_message(app->notifications, &sequence_blink_green_10);
        app->txrx->rx_key_state = PCSGRxKeyStateAddKey;
    }
}

void pocsag_pager_scene_receiver_info_on_enter(void* context) {
    POCSAGPagerApp* app = context;

    subghz_receiver_set_rx_callback(
        app->txrx->receiver, pocsag_pager_scene_receiver_info_add_to_history_callback, app);
    pcsg_view_receiver_info_update(
        app->pcsg_receiver_info,
        pcsg_history_get_raw_data(app->txrx->history, app->txrx->idx_menu_chosen));
    view_dispatcher_switch_to_view(app->view_dispatcher, POCSAGPagerViewReceiverInfo);
}

bool pocsag_pager_scene_receiver_info_on_event(void* context, SceneManagerEvent event) {
    POCSAGPagerApp* app = context;
    bool consumed = false;
    UNUSED(app);
    UNUSED(event);
    return consumed;
}

void pocsag_pager_scene_receiver_info_on_exit(void* context) {
    UNUSED(context);
}
