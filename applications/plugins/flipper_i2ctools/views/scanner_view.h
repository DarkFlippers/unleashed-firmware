#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <I2C_Tools_icons.h>
#include "../i2cscanner.h"

#define SCAN_TEXT "SCAN"

void draw_scanner_view(Canvas* canvas, i2cScanner* i2c_scanner);