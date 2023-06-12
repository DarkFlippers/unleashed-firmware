#include "../u2f_app_i.h"
#include "../views/u2f_view.h"
#include <dolphin/dolphin.h>
#include <furi_hal.h>
#include "../u2f.h"

#define U2F_REQUEST_TIMEOUT 500
#define U2F_SUCCESS_TIMEOUT 3000

static void u2f_scene_main_ok_callback(InputType type, void* context) {
    UNUSED(type);
    furi_assert(context);
    U2fApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, U2fCustomEventConfirm);
}

static void u2f_scene_main_event_callback(U2fNotifyEvent evt, void* context) {
    furi_assert(context);
    U2fApp* app = context;
    if(evt == U2fNotifyRegister)
        view_dispatcher_send_custom_event(app->view_dispatcher, U2fCustomEventRegister);
    else if(evt == U2fNotifyAuth)
        view_dispatcher_send_custom_event(app->view_dispatcher, U2fCustomEventAuth);
    else if(evt == U2fNotifyAuthSuccess)
        view_dispatcher_send_custom_event(app->view_dispatcher, U2fCustomEventAuthSuccess);
    else if(evt == U2fNotifyWink)
        view_dispatcher_send_custom_event(app->view_dispatcher, U2fCustomEventWink);
    else if(evt == U2fNotifyConnect)
        view_dispatcher_send_custom_event(app->view_dispatcher, U2fCustomEventConnect);
    else if(evt == U2fNotifyDisconnect)
        view_dispatcher_send_custom_event(app->view_dispatcher, U2fCustomEventDisconnect);
    else if(evt == U2fNotifyError)
        view_dispatcher_send_custom_event(app->view_dispatcher, U2fCustomEventDataError);
}

static void u2f_scene_main_timer_callback(void* context) {
    furi_assert(context);
    U2fApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, U2fCustomEventTimeout);
}

bool u2f_scene_main_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    U2fApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == U2fCustomEventConnect) {
            furi_timer_stop(app->timer);
            u2f_view_set_state(app->u2f_view, U2fMsgIdle);
        } else if(event.event == U2fCustomEventDisconnect) {
            furi_timer_stop(app->timer);
            app->event_cur = U2fCustomEventNone;
            u2f_view_set_state(app->u2f_view, U2fMsgNotConnected);
        } else if((event.event == U2fCustomEventRegister) || (event.event == U2fCustomEventAuth)) {
            furi_timer_start(app->timer, U2F_REQUEST_TIMEOUT);
            if(app->event_cur == U2fCustomEventNone) {
                app->event_cur = event.event;
                if(event.event == U2fCustomEventRegister)
                    u2f_view_set_state(app->u2f_view, U2fMsgRegister);
                else if(event.event == U2fCustomEventAuth) //-V547
                    u2f_view_set_state(app->u2f_view, U2fMsgAuth);
                notification_message(app->notifications, &sequence_display_backlight_on);
                notification_message(app->notifications, &sequence_single_vibro);
            }
            notification_message(app->notifications, &sequence_blink_magenta_10);
        } else if(event.event == U2fCustomEventWink) {
            notification_message(app->notifications, &sequence_blink_magenta_10);
        } else if(event.event == U2fCustomEventAuthSuccess) {
            notification_message_block(app->notifications, &sequence_set_green_255);
            dolphin_deed(DolphinDeedU2fAuthorized);
            furi_timer_start(app->timer, U2F_SUCCESS_TIMEOUT);
            app->event_cur = U2fCustomEventNone;
            u2f_view_set_state(app->u2f_view, U2fMsgSuccess);
        } else if(event.event == U2fCustomEventTimeout) {
            notification_message_block(app->notifications, &sequence_reset_rgb);
            app->event_cur = U2fCustomEventNone;
            u2f_view_set_state(app->u2f_view, U2fMsgIdle);
        } else if(event.event == U2fCustomEventConfirm) {
            if(app->event_cur != U2fCustomEventNone) {
                u2f_confirm_user_present(app->u2f_instance);
            }
        } else if(event.event == U2fCustomEventDataError) {
            notification_message(app->notifications, &sequence_set_red_255);
            furi_timer_stop(app->timer);
            u2f_view_set_state(app->u2f_view, U2fMsgError);
        }
        consumed = true;
    }

    return consumed;
}

void u2f_scene_main_on_enter(void* context) {
    U2fApp* app = context;

    app->timer = furi_timer_alloc(u2f_scene_main_timer_callback, FuriTimerTypeOnce, app);

    app->u2f_instance = u2f_alloc();
    app->u2f_ready = u2f_init(app->u2f_instance);
    if(app->u2f_ready == true) {
        u2f_set_event_callback(app->u2f_instance, u2f_scene_main_event_callback, app);
        app->u2f_hid = u2f_hid_start(app->u2f_instance);
        u2f_view_set_ok_callback(app->u2f_view, u2f_scene_main_ok_callback, app);
    } else {
        u2f_free(app->u2f_instance);
        u2f_view_set_state(app->u2f_view, U2fMsgError);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, U2fAppViewMain);
}

void u2f_scene_main_on_exit(void* context) {
    U2fApp* app = context;
    notification_message_block(app->notifications, &sequence_reset_rgb);
    furi_timer_stop(app->timer);
    furi_timer_free(app->timer);
    if(app->u2f_ready == true) {
        u2f_hid_stop(app->u2f_hid);
        u2f_free(app->u2f_instance);
    }
}
