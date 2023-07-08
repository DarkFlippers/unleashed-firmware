#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <i2ctools_icons.h>
#include "../i2cscanner.h"

#define SCAN_TEXT "SCAN"

void draw_scanner_view(Canvas* canvas, i2cScanner* i2c_scanner);