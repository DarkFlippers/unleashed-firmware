#pragma once

#include "subghz.h"
#include "subghz_test_basic.h"
#include "subghz_test_packet.h"

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>

static const uint32_t subghz_frequencies[] = {
    301000000,
    315000000,
    346000000,
    385000000,
    433920000,
    438900000,
    463000000,
    781000000,
    868000000,
    915000000,
    925000000,
};

static const uint32_t subghz_frequencies_count = sizeof(subghz_frequencies) / sizeof(uint32_t);

struct SubGhz {
    Gui* gui;

    ViewDispatcher* view_dispatcher;

    Submenu* submenu;

    SubghzTestBasic* subghz_test_basic;

    SubghzTestPacket* subghz_test_packet;
};

typedef enum {
    SubGhzViewMenu,
    SubGhzViewTestBasic,
    SubGhzViewTestPacket,
} SubGhzView;
