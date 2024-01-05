#pragma once

#include <gui/view.h>

typedef struct Hid Hid;
typedef struct HidTikShorts HidTikShorts;

HidTikShorts* hid_tikshorts_alloc(Hid* bt_hid);

void hid_tikshorts_free(HidTikShorts* hid_tikshorts);

View* hid_tikshorts_get_view(HidTikShorts* hid_tikshorts);

void hid_tikshorts_set_connected_status(HidTikShorts* hid_tikshorts, bool connected);
