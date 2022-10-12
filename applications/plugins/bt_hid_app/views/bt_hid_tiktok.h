#pragma once

#include <gui/view.h>

typedef struct BtHidTikTok BtHidTikTok;

BtHidTikTok* bt_hid_tiktok_alloc();

void bt_hid_tiktok_free(BtHidTikTok* bt_hid_tiktok);

View* bt_hid_tiktok_get_view(BtHidTikTok* bt_hid_tiktok);

void bt_hid_tiktok_set_connected_status(BtHidTikTok* bt_hid_tiktok, bool connected);
