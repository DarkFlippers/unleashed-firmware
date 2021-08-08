#pragma once

#include "subghz.h"
#include "views/subghz_capture.h"
#include "views/subghz_test_basic.h"
#include "views/subghz_test_packet.h"
#include "views/subghz_static.h"

#include <furi.h>
#include <furi-hal.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>

extern const uint32_t subghz_frequencies[];
extern const uint32_t subghz_frequencies_count;
extern const uint32_t subghz_frequencies_433_92;

struct SubGhz {
    Gui* gui;

    ViewDispatcher* view_dispatcher;

    Submenu* submenu;

    SubghzCapture* subghz_capture;

    SubghzTestBasic* subghz_test_basic;

    SubghzTestPacket* subghz_test_packet;

    SubghzStatic* subghz_static;
};

typedef enum {
    SubGhzViewMenu,
    SubGhzViewCapture,
    SubGhzViewTestBasic,
    SubGhzViewTestPacket,
    SubGhzViewStatic,
} SubGhzView;
