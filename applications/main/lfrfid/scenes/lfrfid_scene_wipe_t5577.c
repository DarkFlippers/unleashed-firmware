#include "../lfrfid_i.h"

#define LFRFID_DATA_NULL 0

static void lfrfid_wipe_t5577_do(LfRfid* app) {
    Popup* popup = app->popup;
    char curr_buf[50] = {};

    uint8_t page = 0;
    bool lock_bit = 0;
    uint32_t data = 0;

    popup_set_header(popup, "Wiping data", 64, 10, AlignCenter, AlignCenter);
    popup_set_text(popup, curr_buf, 66, 33, AlignCenter, AlignCenter);
    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
    furi_delay_ms(1000);
    notification_message(app->notifications, &sequence_blink_start_magenta);
    furi_delay_ms(500);

    for(uint8_t block = 0; block < 8; block++) {
        if(!block) { // page 0 block 0 is configuration data, default 0x00148040
            data =
                (LFRFID_T5577_BITRATE_RF_64 | LFRFID_T5577_MODULATION_MANCHESTER |
                 (2 << LFRFID_T5577_MAXBLOCK_SHIFT));
        } else
            data = LFRFID_DATA_NULL;

        snprintf(
            curr_buf,
            sizeof(curr_buf),
            "Writing: Page %u Block %u\ndata %02lX %02lX %02lX %02lX",
            page,
            block,
            data >> 24,
            (data >> 16) & 0xFF,
            (data >> 8) & 0xFF,
            data & 0xFF);
        view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
        t5577_write_page_block_simple_with_start_and_stop(
            page, block, lock_bit, data);
        furi_delay_ms(150);
        if(!block) furi_delay_ms(2000);
    }
    notification_message(app->notifications, &sequence_blink_stop);
    popup_reset(app->popup);
}

void lfrfid_scene_wipe_t5577_on_enter(void* context) {
    LfRfid* app = context;
    Popup* popup = app->popup;

    lfrfid_wipe_t5577_do(app);

    notification_message(app->notifications, &sequence_success);
    popup_set_header(popup, "Done!", 102, 10, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 7, &I_DolphinSuccess_91x55);
    popup_set_context(popup, app);
    popup_set_callback(popup, lfrfid_popup_timeout_callback);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
    notification_message_block(app->notifications, &sequence_set_green_255);
}

bool lfrfid_scene_wipe_t5577_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        consumed = true; // Ignore Back button presses
    } else if(event.type == SceneManagerEventTypeCustom && event.event == LfRfidEventPopupClosed) {
        scene_manager_search_and_switch_to_previous_scene(
            app->scene_manager, LfRfidSceneExtraActions);
        consumed = true;
    }
    return consumed;
}

void lfrfid_scene_wipe_t5577_on_exit(void* context) {
    LfRfid* app = context;
    popup_reset(app->popup);
    notification_message_block(app->notifications, &sequence_reset_green);
}