#include "hid_ptt.h"
#include "hid_ptt_menu.h"
#include <gui/elements.h>
#include <notification/notification_messages.h>
#include <gui/modules/widget.h>
#include "../hid.h"
#include "../views.h"

#include "hid_icons.h"

#define TAG "HidPushToTalk"

struct HidPushToTalk {
    View* view;
    Hid* hid;
    Widget* help;
};

typedef void (*PushToTalkActionCallback)(HidPushToTalk* hid_ptt);

typedef struct {
    bool left_pressed;
    bool up_pressed;
    bool right_pressed;
    bool down_pressed;
    bool muted;
    bool ptt_pressed;
    bool mic_pressed;
    bool connected;
    FuriString* os;
    FuriString* app;
    size_t osIndex;
    size_t appIndex;
    size_t window_position;
    PushToTalkActionCallback callback_trigger_mute;
    PushToTalkActionCallback callback_trigger_camera;
    PushToTalkActionCallback callback_trigger_hand;
    PushToTalkActionCallback callback_start_ptt;
    PushToTalkActionCallback callback_stop_ptt;
} HidPushToTalkModel;

enum HidPushToTalkAppIndex {
    HidPushToTalkAppIndexDiscord,
    HidPushToTalkAppIndexFaceTime,
    HidPushToTalkAppIndexGoogleMeet,
    HidPushToTalkAppIndexGoogleHangouts,
    HidPushToTalkAppIndexJamulus,
    HidPushToTalkAppIndexSignal,
    HidPushToTalkAppIndexSkype,
    HidPushToTalkAppIndexSlackCall,
    HidPushToTalkAppIndexSlackHubble,
    HidPushToTalkAppIndexTeams,
    HidPushToTalkAppIndexTeamSpeak,
    HidPushToTalkAppIndexWebex,
    HidPushToTalkAppIndexZoom,
    HidPushToTalkAppIndexSize,
};

// meet, zoom
static void hid_ptt_start_ptt_meet_zoom(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, HID_KEYBOARD_SPACEBAR);
}
static void hid_ptt_stop_ptt_meet_zoom(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_release(hid_ptt->hid, HID_KEYBOARD_SPACEBAR);
}
static void hid_ptt_trigger_mute_macos_meet(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_GUI | HID_KEYBOARD_D);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_GUI | HID_KEYBOARD_D);
}
static void hid_ptt_trigger_mute_linux_meet(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_CTRL | HID_KEYBOARD_D);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_CTRL | HID_KEYBOARD_D);
}
static void hid_ptt_trigger_camera_macos_meet(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_GUI | HID_KEYBOARD_E);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_GUI | HID_KEYBOARD_E);
}
static void hid_ptt_trigger_camera_linux_meet(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_CTRL | HID_KEYBOARD_E);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_CTRL | HID_KEYBOARD_E);
}
static void hid_ptt_trigger_hand_macos_meet(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_GUI | KEY_MOD_LEFT_CTRL | HID_KEYBOARD_H);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_GUI | KEY_MOD_LEFT_CTRL | HID_KEYBOARD_H);
}
static void hid_ptt_trigger_hand_linux_meet(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_CTRL | KEY_MOD_LEFT_ALT | HID_KEYBOARD_H);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_CTRL | KEY_MOD_LEFT_ALT | HID_KEYBOARD_H);
}
static void hid_ptt_trigger_mute_macos_zoom(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_GUI | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_A);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_GUI | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_A);
}
static void hid_ptt_trigger_mute_linux_zoom(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_ALT | HID_KEYBOARD_A);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_ALT | HID_KEYBOARD_A);
}
static void hid_ptt_trigger_camera_macos_zoom(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_GUI | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_V);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_GUI | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_V);
}
static void hid_ptt_trigger_camera_linux_zoom(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_ALT | HID_KEYBOARD_V);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_ALT | HID_KEYBOARD_V);
}
static void hid_ptt_trigger_hand_zoom(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_ALT | HID_KEYBOARD_Y);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_ALT | HID_KEYBOARD_Y);
}

// this one is widely used across different apps
static void hid_ptt_trigger_cmd_shift_m(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_GUI | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_M);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_GUI | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_M);
}

// Hangouts HidPushToTalkAppIndexGoogleHangouts
static void hid_ptt_trigger_mute_macos_hangouts(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_GUI | HID_KEYBOARD_D);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_GUI | HID_KEYBOARD_D);
}
static void hid_ptt_trigger_mute_linux_hangouts(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_CTRL | HID_KEYBOARD_D);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_CTRL | HID_KEYBOARD_D);
}
static void hid_ptt_trigger_camera_macos_hangouts(HidPushToTalk* hid_ptt) { // and hand in teams
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_GUI | HID_KEYBOARD_E);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_GUI | HID_KEYBOARD_E);
}
static void hid_ptt_trigger_camera_linux_hangouts(HidPushToTalk* hid_ptt) { // and hand in teams
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_CTRL | HID_KEYBOARD_E);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_CTRL | HID_KEYBOARD_E);
}

// Signal
static void hid_ptt_trigger_mute_signal(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_M);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_M);
}
static void hid_ptt_trigger_camera_signal(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_V);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_V);
}

// skype
static void hid_ptt_trigger_mute_linux_skype(HidPushToTalk* hid_ptt) { // and webex
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_CTRL | HID_KEYBOARD_M);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_CTRL | HID_KEYBOARD_M);
}
static void hid_ptt_trigger_camera_macos_skype(HidPushToTalk* hid_ptt) { // and hand in teams
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_GUI | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_K);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_GUI | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_K);
}
static void hid_ptt_trigger_camera_linux_skype(HidPushToTalk* hid_ptt) { // and hand in teams
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_CTRL | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_K);
    hid_hal_keyboard_release(
        hid_ptt->hid, KEY_MOD_LEFT_CTRL | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_K);
}

// slack call
static void hid_ptt_trigger_mute_slack_call(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, HID_KEYBOARD_M);
    hid_hal_keyboard_release(hid_ptt->hid, HID_KEYBOARD_M);
}
static void hid_ptt_trigger_camera_slack_call(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, HID_KEYBOARD_V);
    hid_hal_keyboard_release(hid_ptt->hid, HID_KEYBOARD_V);
}

// slack hubble
static void hid_ptt_trigger_mute_macos_slack_hubble(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(
        hid_ptt->hid, KEY_MOD_LEFT_GUI | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_SPACEBAR);
    hid_hal_keyboard_release(
        hid_ptt->hid, KEY_MOD_LEFT_GUI | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_SPACEBAR);
}
static void hid_ptt_trigger_mute_linux_slack_hubble(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(
        hid_ptt->hid, KEY_MOD_LEFT_CTRL | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_SPACEBAR);
    hid_hal_keyboard_release(
        hid_ptt->hid, KEY_MOD_LEFT_CTRL | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_SPACEBAR);
}

// discord
static void hid_ptt_trigger_mute_macos_discord(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(
        hid_ptt->hid,
        KEY_MOD_LEFT_GUI | KEY_MOD_RIGHT_GUI | KEY_MOD_RIGHT_ALT | KEY_MOD_LEFT_SHIFT |
            HID_KEYBOARD_M);
    hid_hal_keyboard_release(
        hid_ptt->hid,
        KEY_MOD_LEFT_GUI | KEY_MOD_RIGHT_GUI | KEY_MOD_RIGHT_ALT | KEY_MOD_LEFT_SHIFT |
            HID_KEYBOARD_M);
}
static void hid_ptt_start_ptt_macos_discord(HidPushToTalk* hid_ptt) { // and TeamSpeak
    hid_hal_keyboard_press(
        hid_ptt->hid,
        KEY_MOD_LEFT_GUI | KEY_MOD_RIGHT_GUI | KEY_MOD_RIGHT_ALT | KEY_MOD_LEFT_SHIFT |
            HID_KEYBOARD_P);
}
static void hid_ptt_stop_ptt_macos_discord(HidPushToTalk* hid_ptt) { // and TeamSpeak
    hid_hal_keyboard_release(
        hid_ptt->hid,
        KEY_MOD_LEFT_GUI | KEY_MOD_RIGHT_GUI | KEY_MOD_RIGHT_ALT | KEY_MOD_LEFT_SHIFT |
            HID_KEYBOARD_P);
}
static void hid_ptt_trigger_mute_linux_discord(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(
        hid_ptt->hid,
        KEY_MOD_LEFT_CTRL | KEY_MOD_RIGHT_CTRL | KEY_MOD_RIGHT_ALT | KEY_MOD_LEFT_SHIFT |
            HID_KEYBOARD_M);
    hid_hal_keyboard_release(
        hid_ptt->hid,
        KEY_MOD_LEFT_CTRL | KEY_MOD_RIGHT_CTRL | KEY_MOD_RIGHT_ALT | KEY_MOD_LEFT_SHIFT |
            HID_KEYBOARD_M);
}
static void hid_ptt_start_ptt_linux_discord(HidPushToTalk* hid_ptt) { // and TeamSpeak
    hid_hal_keyboard_press(
        hid_ptt->hid,
        KEY_MOD_LEFT_CTRL | KEY_MOD_RIGHT_CTRL | KEY_MOD_RIGHT_ALT | KEY_MOD_LEFT_SHIFT |
            HID_KEYBOARD_P);
}
static void hid_ptt_stop_ptt_linux_discord(HidPushToTalk* hid_ptt) { // and TeamSpeak
    hid_hal_keyboard_release(
        hid_ptt->hid,
        KEY_MOD_LEFT_CTRL | KEY_MOD_RIGHT_CTRL | KEY_MOD_RIGHT_ALT | KEY_MOD_LEFT_SHIFT |
            HID_KEYBOARD_P);
}

// teamspeak
static void hid_ptt_trigger_mute_macos_teamspeak(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(
        hid_ptt->hid,
        KEY_MOD_LEFT_GUI | KEY_MOD_RIGHT_GUI | KEY_MOD_RIGHT_ALT | KEY_MOD_RIGHT_SHIFT |
            HID_KEYBOARD_M);
    hid_hal_keyboard_release(
        hid_ptt->hid,
        KEY_MOD_LEFT_GUI | KEY_MOD_RIGHT_GUI | KEY_MOD_RIGHT_ALT | KEY_MOD_RIGHT_SHIFT |
            HID_KEYBOARD_M);
}
static void hid_ptt_start_ptt_macos_teamspeak(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(
        hid_ptt->hid,
        KEY_MOD_LEFT_GUI | KEY_MOD_RIGHT_GUI | KEY_MOD_RIGHT_ALT | KEY_MOD_RIGHT_SHIFT |
            HID_KEYBOARD_P);
}
static void hid_ptt_stop_ptt_macos_teamspeak(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_release(
        hid_ptt->hid,
        KEY_MOD_LEFT_GUI | KEY_MOD_RIGHT_GUI | KEY_MOD_RIGHT_ALT | KEY_MOD_RIGHT_SHIFT |
            HID_KEYBOARD_P);
}
static void hid_ptt_trigger_mute_linux_teamspeak(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(
        hid_ptt->hid,
        KEY_MOD_LEFT_CTRL | KEY_MOD_RIGHT_CTRL | KEY_MOD_RIGHT_ALT | KEY_MOD_RIGHT_SHIFT |
            HID_KEYBOARD_M);
    hid_hal_keyboard_release(
        hid_ptt->hid,
        KEY_MOD_LEFT_CTRL | KEY_MOD_RIGHT_CTRL | KEY_MOD_RIGHT_ALT | KEY_MOD_RIGHT_SHIFT |
            HID_KEYBOARD_M);
}
static void hid_ptt_start_ptt_linux_teamspeak(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(
        hid_ptt->hid,
        KEY_MOD_LEFT_CTRL | KEY_MOD_RIGHT_CTRL | KEY_MOD_RIGHT_ALT | KEY_MOD_RIGHT_SHIFT |
            HID_KEYBOARD_P);
}
static void hid_ptt_stop_ptt_linux_teamspeak(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_release(
        hid_ptt->hid,
        KEY_MOD_LEFT_CTRL | KEY_MOD_RIGHT_CTRL | KEY_MOD_RIGHT_ALT | KEY_MOD_RIGHT_SHIFT |
            HID_KEYBOARD_P);
}

// teams
static void hid_ptt_start_ptt_macos_teams(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_ALT | HID_KEYBOARD_SPACEBAR);
}
static void hid_ptt_start_ptt_linux_teams(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_CTRL | HID_KEYBOARD_SPACEBAR);
}
static void hid_ptt_stop_ptt_macos_teams(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_GUI | HID_KEYBOARD_SPACEBAR);
}
static void hid_ptt_stop_ptt_linux_teams(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_CTRL | HID_KEYBOARD_SPACEBAR);
}
static void hid_ptt_trigger_mute_linux_teams(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_CTRL | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_M);
    hid_hal_keyboard_release(
        hid_ptt->hid, KEY_MOD_LEFT_CTRL | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_M);
}
static void hid_ptt_trigger_camera_macos_teams(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_GUI | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_O);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_GUI | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_O);
}
static void hid_ptt_trigger_camera_linux_teams(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_CTRL | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_O);
    hid_hal_keyboard_release(
        hid_ptt->hid, KEY_MOD_LEFT_CTRL | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_O);
}

// Jamulus
static void hid_ptt_trigger_mute_jamulus(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_ALT | HID_KEYBOARD_M);
    hid_hal_keyboard_release(hid_ptt->hid, KEY_MOD_LEFT_ALT | HID_KEYBOARD_M);
}

// webex

static void hid_ptt_trigger_camera_webex(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_CTRL | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_V);
    hid_hal_keyboard_release(
        hid_ptt->hid, KEY_MOD_LEFT_CTRL | KEY_MOD_LEFT_SHIFT | HID_KEYBOARD_V);
}
static void hid_ptt_trigger_hand_macos_webex(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_GUI | KEY_MOD_RIGHT_SHIFT | HID_KEYBOARD_R);
    hid_hal_keyboard_release(
        hid_ptt->hid, KEY_MOD_LEFT_GUI | KEY_MOD_RIGHT_SHIFT | HID_KEYBOARD_R);
}
static void hid_ptt_trigger_hand_linux_webex(HidPushToTalk* hid_ptt) {
    hid_hal_keyboard_press(hid_ptt->hid, KEY_MOD_LEFT_CTRL | KEY_MOD_RIGHT_SHIFT | HID_KEYBOARD_R);
    hid_hal_keyboard_release(
        hid_ptt->hid, KEY_MOD_LEFT_CTRL | KEY_MOD_RIGHT_SHIFT | HID_KEYBOARD_R);
}

static void hid_ptt_menu_callback(
    void* context,
    uint32_t osIndex,
    FuriString* osLabel,
    uint32_t appIndex,
    FuriString* appLabel) {
    furi_assert(context);
    HidPushToTalk* hid_ptt = context;
    with_view_model(
        hid_ptt->view,
        HidPushToTalkModel * model,
        {
            furi_string_set(model->os, osLabel);
            furi_string_set(model->app, appLabel);
            model->osIndex = osIndex;
            model->appIndex = appIndex;
            model->callback_trigger_mute = NULL;
            model->callback_trigger_camera = NULL;
            model->callback_trigger_hand = NULL;
            model->callback_start_ptt = NULL;
            model->callback_stop_ptt = NULL;
            FURI_LOG_E(TAG, "appIndex: %lu", appIndex);
            if(osIndex == HidPushToTalkMacOS) {
                switch(appIndex) {
                case HidPushToTalkAppIndexDiscord:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_macos_discord;
                    model->callback_start_ptt = hid_ptt_start_ptt_macos_discord;
                    model->callback_stop_ptt = hid_ptt_stop_ptt_macos_discord;
                    break;
                case HidPushToTalkAppIndexFaceTime:
                    model->callback_trigger_mute = hid_ptt_trigger_cmd_shift_m;
                    model->callback_start_ptt = hid_ptt_trigger_cmd_shift_m;
                    model->callback_stop_ptt = hid_ptt_trigger_cmd_shift_m;
                    break;
                case HidPushToTalkAppIndexGoogleHangouts:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_macos_hangouts;
                    model->callback_trigger_camera = hid_ptt_trigger_camera_macos_hangouts;
                    model->callback_start_ptt = hid_ptt_trigger_mute_macos_hangouts;
                    model->callback_stop_ptt = hid_ptt_trigger_mute_macos_hangouts;
                    break;
                case HidPushToTalkAppIndexGoogleMeet:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_macos_meet;
                    model->callback_trigger_camera = hid_ptt_trigger_camera_macos_meet;
                    model->callback_trigger_hand = hid_ptt_trigger_hand_macos_meet;
                    model->callback_start_ptt = hid_ptt_start_ptt_meet_zoom;
                    model->callback_stop_ptt = hid_ptt_stop_ptt_meet_zoom;
                    break;
                case HidPushToTalkAppIndexJamulus:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_jamulus;
                    model->callback_start_ptt = hid_ptt_trigger_mute_jamulus;
                    model->callback_stop_ptt = hid_ptt_trigger_mute_jamulus;
                    break;
                case HidPushToTalkAppIndexTeams:
                    model->callback_trigger_mute = hid_ptt_trigger_cmd_shift_m;
                    model->callback_trigger_camera = hid_ptt_trigger_camera_macos_teams;
                    model->callback_trigger_hand = hid_ptt_trigger_camera_macos_skype;
                    model->callback_start_ptt = hid_ptt_start_ptt_macos_teams;
                    model->callback_stop_ptt = hid_ptt_stop_ptt_macos_teams;
                    break;
                case HidPushToTalkAppIndexTeamSpeak:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_macos_teamspeak;
                    model->callback_start_ptt = hid_ptt_start_ptt_macos_teamspeak;
                    model->callback_stop_ptt = hid_ptt_stop_ptt_macos_teamspeak;
                    break;
                case HidPushToTalkAppIndexSignal:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_signal;
                    model->callback_trigger_camera = hid_ptt_trigger_camera_signal;
                    model->callback_start_ptt = hid_ptt_trigger_mute_signal;
                    model->callback_stop_ptt = hid_ptt_trigger_mute_signal;
                    break;
                case HidPushToTalkAppIndexSkype:
                    model->callback_trigger_mute = hid_ptt_trigger_cmd_shift_m;
                    model->callback_trigger_camera = hid_ptt_trigger_camera_macos_skype;
                    model->callback_start_ptt = hid_ptt_trigger_cmd_shift_m;
                    model->callback_stop_ptt = hid_ptt_trigger_cmd_shift_m;
                    break;
                case HidPushToTalkAppIndexSlackCall:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_slack_call;
                    model->callback_trigger_camera = hid_ptt_trigger_camera_slack_call;
                    model->callback_start_ptt = hid_ptt_trigger_mute_slack_call;
                    model->callback_stop_ptt = hid_ptt_trigger_mute_slack_call;
                    break;
                case HidPushToTalkAppIndexSlackHubble:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_macos_slack_hubble;
                    model->callback_start_ptt = hid_ptt_trigger_mute_macos_slack_hubble;
                    model->callback_stop_ptt = hid_ptt_trigger_mute_macos_slack_hubble;
                    break;
                case HidPushToTalkAppIndexWebex:
                    model->callback_trigger_mute = hid_ptt_trigger_cmd_shift_m;
                    model->callback_trigger_camera = hid_ptt_trigger_camera_webex;
                    model->callback_trigger_hand = hid_ptt_trigger_hand_macos_webex;
                    model->callback_start_ptt = hid_ptt_trigger_cmd_shift_m;
                    model->callback_stop_ptt = hid_ptt_trigger_cmd_shift_m;
                    break;
                case HidPushToTalkAppIndexZoom:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_macos_zoom;
                    model->callback_trigger_camera = hid_ptt_trigger_camera_macos_zoom;
                    model->callback_trigger_hand = hid_ptt_trigger_hand_zoom;
                    model->callback_start_ptt = hid_ptt_start_ptt_meet_zoom;
                    model->callback_stop_ptt = hid_ptt_stop_ptt_meet_zoom;
                    break;
                }
            } else if(osIndex == HidPushToTalkLinux) {
                switch(appIndex) {
                case HidPushToTalkAppIndexDiscord:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_linux_discord;
                    model->callback_start_ptt = hid_ptt_start_ptt_linux_discord;
                    model->callback_stop_ptt = hid_ptt_stop_ptt_linux_discord;
                    break;
                case HidPushToTalkAppIndexGoogleHangouts:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_linux_hangouts;
                    model->callback_trigger_camera = hid_ptt_trigger_camera_linux_hangouts;
                    model->callback_start_ptt = hid_ptt_trigger_mute_linux_hangouts;
                    model->callback_stop_ptt = hid_ptt_trigger_mute_linux_hangouts;
                    break;
                case HidPushToTalkAppIndexGoogleMeet:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_linux_meet;
                    model->callback_trigger_camera = hid_ptt_trigger_camera_linux_meet;
                    model->callback_trigger_hand = hid_ptt_trigger_hand_linux_meet;
                    model->callback_start_ptt = hid_ptt_start_ptt_meet_zoom;
                    model->callback_stop_ptt = hid_ptt_stop_ptt_meet_zoom;
                    break;
                case HidPushToTalkAppIndexJamulus:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_jamulus;
                    model->callback_start_ptt = hid_ptt_trigger_mute_jamulus;
                    model->callback_stop_ptt = hid_ptt_trigger_mute_jamulus;
                    break;
                case HidPushToTalkAppIndexTeams:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_linux_teams;
                    model->callback_trigger_camera = hid_ptt_trigger_camera_linux_teams;
                    model->callback_trigger_hand = hid_ptt_trigger_camera_linux_skype;
                    model->callback_start_ptt = hid_ptt_start_ptt_linux_teams;
                    model->callback_stop_ptt = hid_ptt_stop_ptt_linux_teams;
                    break;
                case HidPushToTalkAppIndexTeamSpeak:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_linux_teamspeak;
                    model->callback_start_ptt = hid_ptt_start_ptt_linux_teamspeak;
                    model->callback_stop_ptt = hid_ptt_stop_ptt_linux_teamspeak;
                    break;
                case HidPushToTalkAppIndexSignal:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_signal;
                    model->callback_trigger_camera = hid_ptt_trigger_camera_signal;
                    model->callback_start_ptt = hid_ptt_trigger_mute_signal;
                    model->callback_stop_ptt = hid_ptt_trigger_mute_signal;
                    break;
                case HidPushToTalkAppIndexSkype:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_linux_skype;
                    model->callback_trigger_camera = hid_ptt_trigger_camera_linux_skype;
                    model->callback_start_ptt = hid_ptt_trigger_mute_linux_skype;
                    model->callback_stop_ptt = hid_ptt_trigger_mute_linux_skype;
                    break;
                case HidPushToTalkAppIndexSlackCall:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_slack_call;
                    model->callback_trigger_camera = hid_ptt_trigger_camera_slack_call;
                    model->callback_start_ptt = hid_ptt_trigger_mute_slack_call;
                    model->callback_stop_ptt = hid_ptt_trigger_mute_slack_call;
                    break;
                case HidPushToTalkAppIndexSlackHubble:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_linux_slack_hubble;
                    model->callback_start_ptt = hid_ptt_trigger_mute_linux_slack_hubble;
                    model->callback_stop_ptt = hid_ptt_trigger_mute_linux_slack_hubble;
                    break;
                case HidPushToTalkAppIndexZoom:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_linux_zoom;
                    model->callback_trigger_camera = hid_ptt_trigger_camera_linux_zoom;
                    model->callback_trigger_hand = hid_ptt_trigger_hand_zoom;
                    model->callback_start_ptt = hid_ptt_start_ptt_meet_zoom;
                    model->callback_stop_ptt = hid_ptt_stop_ptt_meet_zoom;
                    break;
                case HidPushToTalkAppIndexWebex:
                    model->callback_trigger_mute = hid_ptt_trigger_mute_linux_skype;
                    model->callback_trigger_camera = hid_ptt_trigger_camera_webex;
                    model->callback_trigger_hand = hid_ptt_trigger_hand_linux_webex;
                    model->callback_start_ptt = hid_ptt_trigger_mute_linux_skype;
                    model->callback_stop_ptt = hid_ptt_trigger_mute_linux_skype;
                    break;
                }
            }

            char* app_specific_help = "";
            switch(appIndex) {
            case HidPushToTalkAppIndexGoogleMeet:
                app_specific_help =
                    "Google Meet:\n"
                    "This feature is off by default in your audio settings "
                    "and may not work for Windows users who use their screen "
                    "reader. In this situation, the spacebar performs a different action.\n\n";
                break;
            case HidPushToTalkAppIndexDiscord:
                app_specific_help =
                    "Discord:\n"
                    "1. Under App Settings, click Voice & Video. Under Input Mode, "
                    "check the box next to Push to Talk.\n"
                    "2. Scroll down to SHORTCUT, click Record Keybinder.\n"
                    "3. Press PTT in the app to bind it."
                    "4. Go to Keybinds and assign mute button.\n\n";
                break;
            case HidPushToTalkAppIndexTeamSpeak:
                app_specific_help = "TeamSpeak:\n"
                                    "To make keys working bind them in TeamSpeak settings.\n\n";
                break;
            case HidPushToTalkAppIndexTeams:
                app_specific_help =
                    "Teams:\n"
                    "Go to Settings > Privacy. Make sure Keyboard shortcut to unmute is toggled on.\n\n";
                break;
            }

            FuriString* msg = furi_string_alloc();
            furi_string_cat_printf(
                msg,
                "%sGeneral:\n"
                "To operate properly flipper microphone "
                "status must be in sync with your computer.\n"
                "Hold > to change mic status.\n"
                "Hold < to open this help.\n"
                "Press BACK to switch mic on/off.\n"
                "Hold 'o' for PTT mode (mic will be off once you release 'o')\n"
                "Hold BACK to exit.",
                app_specific_help);
            widget_add_text_scroll_element(
                hid_ptt->help, 0, 0, 128, 64, furi_string_get_cstr(msg));
            furi_string_free(msg);
        },
        true);
    view_dispatcher_switch_to_view(hid_ptt->hid->view_dispatcher, HidViewPushToTalk);
}

static void hid_ptt_draw_camera(Canvas* canvas, uint8_t x, uint8_t y) {
    canvas_draw_icon(canvas, x + 7, y, &I_ButtonLeft_4x7);
    canvas_draw_box(canvas, x, y, 7, 7);
}

static void hid_ptt_draw_text_centered(Canvas* canvas, uint8_t y, FuriString* str) {
    FuriString* disp_str;
    disp_str = furi_string_alloc_set(str);
    elements_string_fit_width(canvas, disp_str, canvas_width(canvas));
    uint8_t x_pos =
        (canvas_width(canvas) - canvas_string_width(canvas, furi_string_get_cstr(disp_str))) / 2;
    canvas_draw_str(canvas, x_pos, y, furi_string_get_cstr(disp_str));
    furi_string_free(disp_str);
}

static void hid_ptt_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    HidPushToTalkModel* model = context;

    // Header
    canvas_set_font(canvas, FontPrimary);
#ifdef HID_TRANSPORT_BLE
    if(model->connected) {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_connected_15x15);
    } else {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_disconnected_15x15);
    }
#endif

    // OS and App labels
    canvas_set_font(canvas, FontSecondary);
    hid_ptt_draw_text_centered(canvas, 73, model->app);
    hid_ptt_draw_text_centered(canvas, 84, model->os);

    // Help label
    canvas_draw_icon(canvas, 0, 88, &I_Help_top_64x17);
    canvas_draw_line(canvas, 4, 105, 4, 114);
    canvas_draw_line(canvas, 63, 105, 63, 114);
    canvas_draw_icon(canvas, 7, 107, &I_Hold_15x5);
    canvas_draw_icon(canvas, 24, 105, &I_BtnLeft_9x9);
    canvas_draw_icon(canvas, 34, 108, &I_for_help_27x5);
    canvas_draw_icon(canvas, 0, 115, &I_Help_exit_64x9);
    canvas_draw_icon(canvas, 24, 115, &I_BtnBackV_9x9);

    const uint8_t x_1 = 0;
    const uint8_t x_2 = x_1 + 19 + 4;
    const uint8_t x_3 = x_1 + 19 * 2 + 8;

    const uint8_t y_1 = 3;
    const uint8_t y_2 = y_1 + 19;
    const uint8_t y_3 = y_2 + 19;

    // Up
    canvas_draw_icon(canvas, x_2, y_1, &I_Button_18x18);
    if(model->up_pressed) {
        elements_slightly_rounded_box(canvas, x_2 + 3, y_1 + 2, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, x_2 + 5, y_1 + 5, &I_Volup_8x6);
    canvas_set_color(canvas, ColorBlack);

    // Down
    canvas_draw_icon(canvas, x_2, y_3, &I_Button_18x18);
    if(model->down_pressed) {
        elements_slightly_rounded_box(canvas, x_2 + 3, y_3 + 2, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, x_2 + 6, y_3 + 5, &I_Voldwn_6x6);
    canvas_set_color(canvas, ColorBlack);

    // Left / Help
    canvas_draw_icon(canvas, x_1, y_2, &I_Button_18x18);
    if(model->left_pressed) {
        elements_slightly_rounded_box(canvas, x_1 + 3, y_2 + 2, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    if(model->callback_trigger_hand) {
        canvas_draw_icon(canvas, x_1 + 4, y_2 + 3, &I_Hand_8x10);
    } else {
        canvas_draw_icon(canvas, x_1 + 2, y_2 + 1, &I_BrokenButton_15x15);
    }
    canvas_set_color(canvas, ColorBlack);

    // Right / Camera
    canvas_draw_icon(canvas, x_3, y_2, &I_Button_18x18);
    if(model->right_pressed) {
        elements_slightly_rounded_box(canvas, x_3 + 3, y_2 + 2, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    if(model->callback_trigger_camera) {
        hid_ptt_draw_camera(canvas, x_3 + 4, y_2 + 5);
    } else {
        canvas_draw_icon(canvas, x_3 + 2, y_2 + 1, &I_BrokenButton_15x15);
    }
    canvas_set_color(canvas, ColorBlack);

    // Back / Mic
    const uint8_t x_mic = x_3;
    canvas_draw_icon(canvas, x_mic, 0, &I_RoundButtonUnpressed_16x16);

    if(!(!model->muted || (model->ptt_pressed))) {
        // show muted
        if(model->mic_pressed) {
            // canvas_draw_icon(canvas, x_mic + 1, 0, &I_MicrophonePressedCrossed_15x15);
            canvas_draw_icon(canvas, x_mic, 0, &I_MicrophonePressedCrossedBtn_16x16);
        } else {
            canvas_draw_icon(canvas, x_mic, 0, &I_MicrophoneCrossed_16x16);
        }
    } else {
        // show unmuted
        if(model->mic_pressed) {
            // canvas_draw_icon(canvas, x_mic + 1, 0, &I_MicrophonePressed_15x15);
            canvas_draw_icon(canvas, x_mic, 0, &I_MicrophonePressedBtn_16x16);
        } else {
            canvas_draw_icon(canvas, x_mic + 5, 2, &I_Mic_7x11);
        }
    }

    // Ok / PTT
    const uint8_t x_ptt_margin = 4;
    const uint8_t x_ptt_width = 17;
    const uint8_t x_ptt = x_1 + 19;
    canvas_draw_icon(canvas, x_ptt, y_2, &I_BtnFrameLeft_3x18);
    canvas_draw_icon(canvas, x_ptt + x_ptt_width + 3 + x_ptt_margin, y_2, &I_BtnFrameRight_2x18);
    canvas_draw_line(canvas, x_ptt + 3, y_2, x_ptt + x_ptt_width + 2 + x_ptt_margin, y_2);
    canvas_draw_line(
        canvas, x_ptt + 3, y_2 + 16, x_ptt + x_ptt_width + 2 + x_ptt_margin, y_2 + 16);
    canvas_draw_line(
        canvas, x_ptt + 3, y_2 + 17, x_ptt + x_ptt_width + 2 + x_ptt_margin, y_2 + 17);

    if(model->ptt_pressed) {
        elements_slightly_rounded_box(canvas, x_ptt + 3, y_2 + 2, x_ptt_width + x_ptt_margin, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(
        canvas, x_ptt + 2 + x_ptt_margin / 2, y_2 + 13, AlignLeft, AlignBottom, "PTT");
    canvas_set_font(canvas, FontSecondary);
    canvas_set_color(canvas, ColorBlack);
}

static void hid_ptt_process(HidPushToTalk* hid_ptt, InputEvent* event) {
    with_view_model(
        hid_ptt->view,
        HidPushToTalkModel * model,
        {
            if(event->type == InputTypePress && !model->ptt_pressed) {
                if(event->key == InputKeyUp) {
                    model->up_pressed = true;
                    hid_hal_consumer_key_press(hid_ptt->hid, HID_CONSUMER_VOLUME_INCREMENT);
                } else if(event->key == InputKeyDown) {
                    model->down_pressed = true;
                    hid_hal_consumer_key_press(hid_ptt->hid, HID_CONSUMER_VOLUME_DECREMENT);
                } else if(event->key == InputKeyLeft) {
                    model->left_pressed = true;
                } else if(event->key == InputKeyRight) {
                    model->right_pressed = true;
                } else if(event->key == InputKeyOk) {
                    model->ptt_pressed = true;
                    if(!model->mic_pressed && model->muted) {
                        model->callback_start_ptt ? model->callback_start_ptt(hid_ptt) : 0;
                    }
                } else if(event->key == InputKeyBack) {
                    model->mic_pressed = true;
                }
            } else if(event->type == InputTypeRelease) {
                if(event->key == InputKeyUp) {
                    model->up_pressed = false;
                    if(!model->ptt_pressed) {
                        hid_hal_consumer_key_release(hid_ptt->hid, HID_CONSUMER_VOLUME_INCREMENT);
                    }
                } else if(event->key == InputKeyDown) {
                    model->down_pressed = false;
                    if(!model->ptt_pressed) {
                        hid_hal_consumer_key_release(hid_ptt->hid, HID_CONSUMER_VOLUME_DECREMENT);
                    }
                } else if(event->key == InputKeyLeft) {
                    model->left_pressed = false;
                } else if(event->key == InputKeyRight) {
                    model->right_pressed = false;

                } else if(event->key == InputKeyOk) {
                    model->ptt_pressed = false;
                    if(!model->mic_pressed) {
                        if(model->muted) {
                            model->callback_stop_ptt ? model->callback_stop_ptt(hid_ptt) : 0;
                        } else {
                            model->callback_trigger_mute ? model->callback_trigger_mute(hid_ptt) :
                                                           0;
                            model->muted = true;
                        }
                    }
                } else if(event->key == InputKeyBack) {
                    model->mic_pressed = false;
                }
            } else if(event->type == InputTypeShort && !model->ptt_pressed) {
                if(event->key == InputKeyBack) { // no changes if PTT is pressed
                    model->muted = !model->muted;
                    model->callback_trigger_mute ? model->callback_trigger_mute(hid_ptt) : 0;
                } else if(event->key == InputKeyRight) {
                    model->callback_trigger_camera ? model->callback_trigger_camera(hid_ptt) : 0;
                } else if(event->key == InputKeyLeft) {
                    model->callback_trigger_hand ? model->callback_trigger_hand(hid_ptt) : 0;
                }
            } else if(event->type == InputTypeLong && event->key == InputKeyRight) {
                model->muted = !model->muted;
                notification_message(hid_ptt->hid->notifications, &sequence_single_vibro);
            } else if(event->type == InputTypeLong && event->key == InputKeyLeft) {
                notification_message(hid_ptt->hid->notifications, &sequence_single_vibro);
                model->left_pressed = false;
                view_dispatcher_switch_to_view(
                    hid_ptt->hid->view_dispatcher, HidViewPushToTalkHelp);
            }
            //LED
            if(!model->muted || (model->ptt_pressed)) {
                notification_message(hid_ptt->hid->notifications, &sequence_set_red_255);
            } else {
                notification_message(hid_ptt->hid->notifications, &sequence_reset_red);
            }
        },
        true);
}

static bool hid_ptt_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    HidPushToTalk* hid_ptt = context;
    bool consumed = false;
    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        hid_hal_keyboard_release_all(hid_ptt->hid);
        notification_message(hid_ptt->hid->notifications, &sequence_double_vibro);
        widget_reset(hid_ptt->help);
    } else {
        consumed = true;
        hid_ptt_process(hid_ptt, event);
    }
    return consumed;
}

View* hid_ptt_get_view(HidPushToTalk* hid_ptt) {
    furi_assert(hid_ptt);
    return hid_ptt->view;
}

static uint32_t hid_ptt_view(void* context) {
    UNUSED(context);
    return HidViewPushToTalk;
}

HidPushToTalk* hid_ptt_alloc(Hid* hid) {
    HidPushToTalk* hid_ptt = malloc(sizeof(HidPushToTalk));
    hid_ptt->hid = hid;
    hid_ptt->view = view_alloc();
    view_set_context(hid_ptt->view, hid_ptt);
    view_allocate_model(hid_ptt->view, ViewModelTypeLocking, sizeof(HidPushToTalkModel));
    view_set_draw_callback(hid_ptt->view, hid_ptt_draw_callback);
    view_set_input_callback(hid_ptt->view, hid_ptt_input_callback);
    view_set_orientation(hid_ptt->view, ViewOrientationVerticalFlip);

    with_view_model(
        hid_ptt->view,
        HidPushToTalkModel * model,
        {
            model->muted = true; // assume we're muted
            model->os = furi_string_alloc();
            model->app = furi_string_alloc();
        },
        true);

    FURI_LOG_I(TAG, "Calling adding list");
    ptt_menu_add_list(hid->hid_ptt_menu, "macOS", HidPushToTalkMacOS);
    ptt_menu_add_list(hid->hid_ptt_menu, "Win/Linux", HidPushToTalkLinux);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkMacOS,
        "Google Meet",
        HidPushToTalkAppIndexGoogleMeet,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkLinux,
        "Google Meet",
        HidPushToTalkAppIndexGoogleMeet,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkMacOS,
        "Google Hangouts",
        HidPushToTalkAppIndexGoogleHangouts,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkLinux,
        "Google Hangouts",
        HidPushToTalkAppIndexGoogleHangouts,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkMacOS,
        "Discord",
        HidPushToTalkAppIndexDiscord,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkLinux,
        "Discord",
        HidPushToTalkAppIndexDiscord,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkMacOS,
        "FaceTime",
        HidPushToTalkAppIndexFaceTime,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkMacOS,
        "Jamulus",
        HidPushToTalkAppIndexJamulus,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkLinux,
        "Jamulus",
        HidPushToTalkAppIndexJamulus,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkMacOS,
        "Signal",
        HidPushToTalkAppIndexSignal,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkLinux,
        "Signal",
        HidPushToTalkAppIndexSignal,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkMacOS,
        "Skype",
        HidPushToTalkAppIndexSkype,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkLinux,
        "Skype",
        HidPushToTalkAppIndexSkype,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkMacOS,
        "Slack Call",
        HidPushToTalkAppIndexSlackCall,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkLinux,
        "Slack Call",
        HidPushToTalkAppIndexSlackCall,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkMacOS,
        "Slack Hubble",
        HidPushToTalkAppIndexSlackHubble,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkLinux,
        "Slack Hubble",
        HidPushToTalkAppIndexSlackHubble,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkMacOS,
        "TeamSpeak",
        HidPushToTalkAppIndexTeamSpeak,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkLinux,
        "TeamSpeak",
        HidPushToTalkAppIndexTeamSpeak,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkMacOS,
        "Teams",
        HidPushToTalkAppIndexTeams,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkLinux,
        "Teams",
        HidPushToTalkAppIndexTeams,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkMacOS,
        "Zoom",
        HidPushToTalkAppIndexZoom,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkLinux,
        "Zoom",
        HidPushToTalkAppIndexZoom,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkMacOS,
        "Webex",
        HidPushToTalkAppIndexWebex,
        hid_ptt_menu_callback,
        hid_ptt);
    ptt_menu_add_item_to_list(
        hid->hid_ptt_menu,
        HidPushToTalkLinux,
        "Webex",
        HidPushToTalkAppIndexWebex,
        hid_ptt_menu_callback,
        hid_ptt);

    hid_ptt->help = widget_alloc();
    view_set_previous_callback(widget_get_view(hid_ptt->help), hid_ptt_view);
    view_dispatcher_add_view(
        hid->view_dispatcher, HidViewPushToTalkHelp, widget_get_view(hid_ptt->help));
    return hid_ptt;
}

void hid_ptt_free(HidPushToTalk* hid_ptt) {
    furi_assert(hid_ptt);
    notification_message(hid_ptt->hid->notifications, &sequence_reset_red);
    with_view_model(
        hid_ptt->view,
        HidPushToTalkModel * model,
        {
            furi_string_free(model->os);
            furi_string_free(model->app);
        },
        true);
    view_dispatcher_remove_view(hid_ptt->hid->view_dispatcher, HidViewPushToTalkHelp);
    widget_free(hid_ptt->help);
    view_free(hid_ptt->view);
    free(hid_ptt);
}

void hid_ptt_set_connected_status(HidPushToTalk* hid_ptt, bool connected) {
    furi_assert(hid_ptt);
    with_view_model(
        hid_ptt->view,
        HidPushToTalkModel * model,
        {
            if(!connected && model->connected) {
                notification_message(hid_ptt->hid->notifications, &sequence_single_vibro);
            }
            model->connected = connected;
        },
        true);
}
