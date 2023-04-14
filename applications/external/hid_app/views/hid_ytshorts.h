#pragma once

#include <gui/view.h>

typedef struct Hid Hid;
typedef struct HidYTShorts HidYTShorts;

HidYTShorts* hid_ytshorts_alloc(Hid* bt_hid);

void hid_ytshorts_free(HidYTShorts* hid_ytshorts);

View* hid_ytshorts_get_view(HidYTShorts* hid_ytshorts);

void hid_ytshorts_set_connected_status(HidYTShorts* hid_ytshorts, bool connected);
