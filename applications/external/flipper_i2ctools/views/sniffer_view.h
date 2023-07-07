#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <i2ctools_icons.h>
#include "../i2csniffer.h"

#define SNIFF_TEXT "SNIFF"

void draw_sniffer_view(Canvas* canvas, i2cSniffer* i2c_sniffer);