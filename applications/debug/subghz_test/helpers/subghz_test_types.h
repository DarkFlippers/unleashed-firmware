#pragma once

#include <furi.h>
#include <furi_hal.h>

#define SUBGHZ_TEST_VERSION_APP "0.1"
#define SUBGHZ_TEST_DEVELOPED "SkorP"
#define SUBGHZ_TEST_GITHUB "https://github.com/flipperdevices/flipperzero-firmware"

typedef enum {
    SubGhzTestViewVariableItemList,
    SubGhzTestViewSubmenu,
    SubGhzTestViewStatic,
    SubGhzTestViewCarrier,
    SubGhzTestViewPacket,
    SubGhzTestViewWidget,
    SubGhzTestViewPopup,
} SubGhzTestView;
