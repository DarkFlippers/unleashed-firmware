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
