#include "../lfrfid_i.h"
#include "../helpers/rfid_writer.h"

static void writer_initialize(T55xxTiming* t55xxtiming) {
    t55xxtiming->wait_time = 400;
    t55xxtiming->start_gap = 30;
    t55xxtiming->write_gap = 18;
    t55xxtiming->data_0 = 24;
    t55xxtiming->data_1 = 56;
    t55xxtiming->program = 700;
}

static void lfrfid_clear_t5577_password_and_config_to_EM(LfRfid* app) {
    T55xxTiming* t55xxtiming = malloc(sizeof(T55xxTiming));
    Popup* popup = app->popup;
    char curr_buf[32] = {};
    //TODO: use .txt file in resourses for passwords.
    const uint32_t default_passwords[] = {
        0x51243648, 0x000D8787, 0x19920427, 0x50524F58, 0xF9DCEBA0, 0x65857569, 0x05D73B9F,
        0x89A69E60, 0x314159E0, 0xAA55BBBB, 0xA5B4C3D2, 0x1C0B5848, 0x00434343, 0x444E4752,
        0x4E457854, 0x44B44CAE, 0x88661858, 0xE9920427, 0x575F4F4B, 0x50520901, 0x20206666,
        0x65857569, 0x5469616E, 0x7686962A, 0xC0F5009A, 0x07CEE75D, 0xfeedbeef, 0xdeadc0de,
        0x00000000, 0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666,
        0x77777777, 0x88888888, 0x99999999, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD,
        0xEEEEEEEE, 0xFFFFFFFF, 0xa0a1a2a3, 0xb0b1b2b3, 0x50415353, 0x00000001, 0x00000002,
        0x0000000a, 0x0000000b, 0x01020304, 0x02030405, 0x03040506, 0x04050607, 0x05060708,
        0x06070809, 0x0708090A, 0x08090A0B, 0x090A0B0C, 0x0A0B0C0D, 0x0B0C0D0E, 0x0C0D0E0F,
        0x01234567, 0x12345678, 0x10000000, 0x20000000, 0x30000000, 0x40000000, 0x50000000,
        0x60000000, 0x70000000, 0x80000000, 0x90000000, 0xA0000000, 0xB0000000, 0xC0000000,
        0xD0000000, 0xE0000000, 0xF0000000, 0x10101010, 0x01010101, 0x11223344, 0x22334455,
        0x33445566, 0x44556677, 0x55667788, 0x66778899, 0x778899AA, 0x8899AABB, 0x99AABBCC,
        0xAABBCCDD, 0xBBCCDDEE, 0xCCDDEEFF, 0x0CB7E7FC, 0xFABADA11, 0x87654321, 0x12341234,
        0x69696969, 0x12121212, 0x12344321, 0x1234ABCD, 0x11112222, 0x13131313, 0x10041004,
        0x31415926, 0xabcd1234, 0x20002000, 0x19721972, 0xaa55aa55, 0x55aa55aa, 0x4f271149,
        0x07d7bb0b, 0x9636ef8f, 0xb5f44686, 0x9E3779B9, 0xC6EF3720, 0x7854794A, 0xF1EA5EED,
        0x69314718, 0x57721566, 0x93C467E3, 0x27182818, 0x50415353};
    const uint8_t default_passwords_len = sizeof(default_passwords) / sizeof(uint32_t);
    const uint32_t em_config_block_data =
        0b00000000000101001000000001000000; //no pwd&aor config block

    writer_initialize(t55xxtiming);

    popup_set_header(popup, "Removing\npassword", 90, 36, AlignCenter, AlignCenter);
    popup_set_icon(popup, 0, 3, &I_RFIDDolphinSend_97x61);
    popup_set_text(popup, curr_buf, 90, 56, AlignCenter, AlignCenter);
    notification_message(app->notifications, &sequence_blink_start_magenta);

    for(uint8_t i = 0; i < default_passwords_len; i++) {
        FURI_CRITICAL_ENTER();
        snprintf(curr_buf, sizeof(curr_buf), "Pass %d of %d", i, default_passwords_len);
        view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
        writer_start();
        write_block(t55xxtiming, 0, 0, false, em_config_block_data, true, default_passwords[i]);
        write_reset(t55xxtiming);
        writer_stop();
        FURI_CRITICAL_EXIT();
        furi_delay_ms(8);
    }
    notification_message(app->notifications, &sequence_blink_stop);
    popup_reset(app->popup);
    free(t55xxtiming);
}

void lfrfid_scene_clear_t5577_on_enter(void* context) {
    LfRfid* app = context;
    Popup* popup = app->popup;

    lfrfid_clear_t5577_password_and_config_to_EM(app);

    notification_message(app->notifications, &sequence_success);
    popup_set_header(popup, "Done!", 94, 10, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 7, &I_RFIDDolphinSuccess_108x57);
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

    const uint32_t prev_scene = LfRfidSceneExtraActions;

    if(event.type == SceneManagerEventTypeCustom && event.event == LfRfidEventPopupClosed) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, prev_scene);
        consumed = true;
    }
    return consumed;
}

void lfrfid_scene_clear_t5577_on_exit(void* context) {
    LfRfid* app = context;
    popup_reset(app->popup);
    notification_message_block(app->notifications, &sequence_reset_green);
}
