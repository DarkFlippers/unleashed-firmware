#pragma once

#include <gui/view.h>

typedef struct Hid Hid;
typedef struct HidPushToTalk HidPushToTalk;

HidPushToTalk* hid_ptt_alloc(Hid* bt_hid);

void hid_ptt_free(HidPushToTalk* hid_ptt);

View* hid_ptt_get_view(HidPushToTalk* hid_ptt);

void hid_ptt_set_connected_status(HidPushToTalk* hid_ptt, bool connected);

enum HidPushToTalkOSes {
    HidPushToTalkMacOS,
    HidPushToTalkLinux,
};
