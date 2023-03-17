#pragma once

#include <gui/view.h>

typedef struct Hid Hid;
typedef struct HidTikTok HidTikTok;

HidTikTok* hid_tiktok_alloc(Hid* bt_hid);

void hid_tiktok_free(HidTikTok* hid_tiktok);

View* hid_tiktok_get_view(HidTikTok* hid_tiktok);

void hid_tiktok_set_connected_status(HidTikTok* hid_tiktok, bool connected);
