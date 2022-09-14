#pragma once

#include <gui/view.h>

typedef struct BtHidMedia BtHidMedia;

BtHidMedia* bt_hid_media_alloc();

void bt_hid_media_free(BtHidMedia* bt_hid_media);

View* bt_hid_media_get_view(BtHidMedia* bt_hid_media);

void bt_hid_media_set_connected_status(BtHidMedia* bt_hid_media, bool connected);
