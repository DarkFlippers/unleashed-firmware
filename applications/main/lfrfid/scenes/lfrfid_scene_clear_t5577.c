#include "../lfrfid_i.h"
#include <bit_lib.h>
#include "tools/t5577.h"
#define TAG "Clear T5577"

static void lfrfid_clear_t5577_password_and_config_to_EM(LfRfid* app) {
    Popup* popup = app->popup;
    char curr_buf[32] = {};

    uint8_t default_passwords_len;
    const uint32_t* default_passwords = lfrfid_get_t5577_default_passwords(&default_passwords_len);

    popup_set_header(popup, "Removing\npassword", 90, 36, AlignCenter, AlignCenter);
    popup_set_icon(popup, 0, 3, &I_RFIDDolphinSend_97x61);
    popup_set_text(popup, curr_buf, 90, 56, AlignCenter, AlignCenter);
    notification_message(app->notifications, &sequence_blink_start_magenta);

    LFRFIDT5577 data = {
        .block[0] = 0b00000000000101001000000001000000,
        .block[7] = 0,
        .mask = 0b10000001,
    };

    // Clear custom password
    uint32_t custom_pass = bit_lib_bytes_to_num_be(app->password, 4);
    snprintf(curr_buf, sizeof(curr_buf), "Custom password");
    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);

    t5577_write_with_mask(&data, 0, true, custom_pass);

    furi_delay_ms(8);

    // Clear default passwords
    for(uint8_t i = 0; i < default_passwords_len; i++) {
        snprintf(curr_buf, sizeof(curr_buf), "Pass %d of %d", i, default_passwords_len);
        view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);

        t5577_write_with_mask(&data, 0, true, default_passwords[i]);
        furi_delay_ms(8);
    }

    notification_message(app->notifications, &sequence_blink_stop);
    popup_reset(app->popup);
}

void lfrfid_scene_clear_t5577_on_enter(void* context) {
    LfRfid* app = context;
    Popup* popup = app->popup;

    lfrfid_clear_t5577_password_and_config_to_EM(app);

    notification_message(app->notifications, &sequence_success);
    popup_set_header(popup, "Success!", 75, 10, AlignLeft, AlignTop);
    popup_set_icon(popup, 0, 9, &I_DolphinSuccess_91x55);
    popup_set_context(popup, app);
    popup_set_callback(popup, lfrfid_popup_timeout_callback);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
    notification_message_block(app->notifications, &sequence_set_green_255);
}

bool lfrfid_scene_clear_t5577_on_event(void* context, SceneManagerEvent event) {
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

void lfrfid_scene_clear_t5577_on_exit(void* context) {
    LfRfid* app = context;
    popup_reset(app->popup);
    notification_message_block(app->notifications, &sequence_reset_green);
}
