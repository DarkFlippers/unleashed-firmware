#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <i2cTools_icons.h>
#include "../i2csniffer.h"

#define SNIFF_MENU_TEXT "Sniff"
#define SNIFF_MENU_X 75
#define SNIFF_MENU_Y 20

void draw_sniffer_view(Canvas* canvas, i2cSniffer* i2c_sniffer);