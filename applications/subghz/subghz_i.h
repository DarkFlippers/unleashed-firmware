#pragma once

#include "subghz.h"
#include "subghz_test_basic.h"
#include "subghz_test_packet.h"
#include "subghz_static.h"

#include <furi.h>
#include <api-hal.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>

typedef struct {
    uint32_t frequency;
    uint8_t path;
} SubGhzFrequency;

extern const SubGhzFrequency subghz_frequencies[];
extern const uint32_t subghz_frequencies_count;
extern const uint32_t subghz_frequencies_433_92;

struct SubGhz {
    Gui* gui;

    ViewDispatcher* view_dispatcher;

    Submenu* submenu;

    SubghzTestBasic* subghz_test_basic;

    SubghzTestPacket* subghz_test_packet;

    SubghzStatic* subghz_static;
};

typedef enum {
    SubGhzViewMenu,
    SubGhzViewTestBasic,
    SubGhzViewTestPacket,
    SubGhzViewStatic,
} SubGhzView;
