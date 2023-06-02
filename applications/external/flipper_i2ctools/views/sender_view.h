#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <i2ctools_icons.h>
#include "../i2csender.h"

#define SEND_TEXT "SEND"

void draw_sender_view(Canvas* canvas, i2cSender* i2c_sender);