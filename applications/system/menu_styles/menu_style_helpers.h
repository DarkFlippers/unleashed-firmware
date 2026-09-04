/**
 * @file menu_style_helpers.h
 * Shared helpers for main menu style plugins.
 *
 * The layouts in this directory are ported from Momentum Firmware's built-in menu styles
 * (https://github.com/Next-Flip/Momentum-Firmware, GPL-3.0), reworked to ship as plugins.
 */

#pragma once

#include <furi.h>
#include <gui/elements.h>
#include <gui/modules/menu.h>
#include <flipper_application/flipper_application.h>

#define MENU_STYLE_PLUGIN(menu_style, menu_style_ep)                         \
    static const FlipperAppPluginDescriptor menu_style_plugin_descriptor = { \
        .appid = MENU_STYLE_PLUGIN_APP_ID,                                   \
        .ep_api_version = MENU_STYLE_PLUGIN_API_VERSION,                     \
        .entry_point = &(menu_style),                                        \
    };                                                                       \
    const FlipperAppPluginDescriptor* menu_style_ep(void) {                  \
        return &menu_style_plugin_descriptor;                                \
    }

/** Item label, optionally shortened for a layout too narrow to scroll it comfortably.
 * Only the stock names that do not fit are special-cased; anything else is returned as-is. These
 * are the name= fields of applications/main/lfrfid and .../subghz - rename either and the
 * shortening silently stops matching.
 */
static inline const char* menu_style_label(const MenuItem* item, bool shorter) {
    if(shorter) {
        if(strcmp(item->label, "125 kHz RFID") == 0) return "RFID";
        if(strcmp(item->label, "Sub-GHz") == 0) return "SubGHz";
    }
    return item->label;
}

static inline size_t menu_style_scroll(const MenuModel* model, bool selected) {
    return (selected && model->scroll_counter) ? model->scroll_counter - 1 : 0;
}

static inline void menu_style_icon_centered(
    Canvas* canvas,
    IconAnimation* icon,
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height) {
    canvas_draw_icon_animation(
        canvas,
        x + (width - icon_animation_get_width(icon)) / 2,
        y + (height - icon_animation_get_height(icon)) / 2,
        icon);
}

static inline void menu_style_scrollbar_horizontal(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    size_t pos,
    size_t total) {
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, x, y - 3, width, 3);
    canvas_set_color(canvas, ColorBlack);
    for(size_t i = x; i < width + x; i += 2) {
        canvas_draw_dot(canvas, i, y - 2);
    }
    if(total) {
        size_t block = width / total;
        canvas_draw_box(canvas, x + (width * pos) / total, y - 3, block ? block : 1, 3);
    }
}

static inline size_t menu_style_navigate_wrap(const MenuModel* model, InputKey key) {
    if(key == InputKeyLeft) return model->position ? model->position - 1 : model->count - 1;
    if(key == InputKeyRight) return (model->position + 1) % model->count;
    return model->position;
}

static inline size_t menu_style_navigate_list(const MenuModel* model, InputKey key) {
    if(key == InputKeyUp) return model->position ? model->position - 1 : model->count - 1;
    if(key == InputKeyDown) return (model->position + 1) % model->count;
    return model->position;
}

/** Jump to the other column of a two-column page. The right column of the last page can be empty,
 * and then there is nowhere to go sideways at all; otherwise clamp into it.
 */
static inline size_t menu_style_navigate_columns(const MenuModel* model, size_t page) {
    size_t position = model->position;
    size_t half = page / 2;
    if(position - (position % page) + half >= model->count) return position;
    size_t target = (position % page) < half ? position + half : position - half;
    return MIN(target, model->count - 1);
}
