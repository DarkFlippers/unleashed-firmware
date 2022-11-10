#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <i2cTools_icons.h>
#include "../i2cscanner.h"

#define SCAN_MENU_TEXT "Scan"
#define SCAN_MENU_X 75
#define SCAN_MENU_Y 6

void draw_scanner_view(Canvas* canvas, i2cScanner* i2c_scanner);