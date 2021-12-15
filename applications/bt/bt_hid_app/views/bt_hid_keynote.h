#pragma once

#include <gui/view.h>

typedef struct BtHidKeynote BtHidKeynote;

BtHidKeynote* bt_hid_keynote_alloc();

void bt_hid_keynote_free(BtHidKeynote* bt_hid_keynote);

View* bt_hid_keynote_get_view(BtHidKeynote* bt_hid_keynote);

void bt_hid_keynote_set_connected_status(BtHidKeynote* bt_hid_keynote, bool connected);
