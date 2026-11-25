#pragma once

#include <furi.h>
#include <gui/canvas.h>
#include "../helpers/sim800_sms.h"

#define SIM800_SMS_PLUGIN_APP_ID      "sim800"
#define SIM800_SMS_PLUGIN_API_VERSION 2
#define SIM800_SMS_PLUGIN_PATH        APP_ASSETS_PATH("plugins/sim800_sms_plugin.fal")

typedef struct {
    const uint8_t* font;
    void (*decode_text)(const char* in, FuriString* out);
    void (*draw_inbox)(
        Canvas* canvas,
        const Sim800Inbox* inbox,
        size_t selected,
        const char* status_text);
    uint16_t (*draw_reader)(
        Canvas* canvas,
        const char* from,
        const char* date,
        const char* body,
        uint16_t scroll_line);
    uint16_t (*draw_text)(
        Canvas* canvas,
        const char* title,
        const char* text,
        uint16_t scroll_line);
} Sim800SmsPlugin;
