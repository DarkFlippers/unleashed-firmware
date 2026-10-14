/**
 * @file macintosh.c
 * Macintosh main menu style.
 *
 * Ported by @apfxtech. Based on the classic Mac OS desktop of Ardutosh (MIT),
 * by @jhhoward: https://github.com/jhhoward/Ardutosh
 */
#include "menu_style_helpers.h"

#include <furi_hal_version.h>
#include <gui/gui.h>
#include <gui/icon_animation_i.h>
#include <gui/icon_i.h>

#define LCD_W 128
#define LCD_H 64

#define GLYPH_W 4
#define GLYPH_H 6

#define WINDOW_BAR_H 8
#define WIN_X        0
#define WIN_Y        0
#define WIN_W        128
#define WIN_H        64

#define GRID_X    16
#define GRID_Y    13
#define GRID_DX   36
#define GRID_DY   24
#define GRID_COLS 3
#define GRID_ROWS 2
#define ICON_W    14
#define ICON_H    14
#define LABEL_MAX 8

#define ARROW_W 9
#define ARROW_H 11

#define HILITE_TICKS 250

#define ANIM_FRAMES   8
#define FRAME_TICKS   48
#define RESTART_TICKS 1000

#define hline(x, y, w) fill((x), (y), (w), 1, true)
#define vline(x, y, h) fill((x), (y), 1, (h), true)

static const uint8_t font4x6[96][2] = {
    {0x00, 0x00}, {0x49, 0x08}, {0xb4, 0x00}, {0xbe, 0xf6}, {0x7b, 0x7a}, {0xa5, 0x94},
    {0x55, 0xb8}, {0x48, 0x00}, {0x29, 0x44}, {0x44, 0x2a}, {0x15, 0xa0}, {0x0b, 0x42},
    {0x00, 0x50}, {0x03, 0x02}, {0x00, 0x08}, {0x25, 0x90}, {0x76, 0xba}, {0x59, 0x5c},
    {0xc5, 0x9e}, {0xc5, 0x38}, {0x92, 0xe6}, {0xf3, 0x3a}, {0x73, 0xba}, {0xe5, 0x90},
    {0x77, 0xba}, {0x77, 0x3a}, {0x08, 0x40}, {0x08, 0x50}, {0x2a, 0x44}, {0x1c, 0xe0},
    {0x88, 0x52}, {0xe5, 0x08}, {0x56, 0x8e}, {0x77, 0xb6}, {0x77, 0xb8}, {0x72, 0x8c},
    {0xd6, 0xba}, {0x73, 0x9e}, {0x73, 0x92}, {0x72, 0xae}, {0xb7, 0xb6}, {0xe9, 0x5c},
    {0x64, 0xaa}, {0xb7, 0xb4}, {0x92, 0x9c}, {0xbe, 0xb6}, {0xd6, 0xb6}, {0x56, 0xaa},
    {0xd7, 0x92}, {0x76, 0xee}, {0x77, 0xb4}, {0x71, 0x38}, {0xe9, 0x48}, {0xb6, 0xae},
    {0xb6, 0xaa}, {0xb6, 0xf6}, {0xb5, 0xb4}, {0xb5, 0x48}, {0xe5, 0x9c}, {0x69, 0x4c},
    {0x91, 0x24}, {0x64, 0x2e}, {0x54, 0x00}, {0x00, 0x1c}, {0x44, 0x00}, {0x0e, 0xae},
    {0x9a, 0xba}, {0x0e, 0x8c}, {0x2e, 0xae}, {0x0e, 0xce}, {0x56, 0xd0}, {0x55, 0x3B},
    {0x93, 0xb4}, {0x41, 0x44}, {0x41, 0x51}, {0x97, 0xb4}, {0x49, 0x44}, {0x17, 0xb6},
    {0x1a, 0xb6}, {0x0a, 0xaa}, {0xd6, 0xd3}, {0x76, 0x67}, {0x17, 0x90}, {0x0f, 0x38},
    {0x9a, 0x8c}, {0x16, 0xae}, {0x16, 0xba}, {0x16, 0xf6}, {0x15, 0xb4}, {0xb5, 0x2b},
    {0x1c, 0x5e}, {0x6b, 0x4c}, {0x49, 0x48}, {0xc9, 0x5a}, {0x54, 0x00}, {0x56, 0xe2},
};

enum {
    SpriteUp,
    SpriteDown,
};

static const uint8_t sprites[][ARROW_W * 2] = {
    {0xcf,
     0xd7,
     0x1b,
     0xfd,
     0xfe,
     0xfd,
     0x1b,
     0xd7,
     0xcf,
     0x3,
     0x3,
     0x2,
     0x2,
     0x2,
     0x2,
     0x2,
     0x3,
     0x3},
    {0x9e,
     0x5e,
     0xc2,
     0xfa,
     0xfa,
     0xfa,
     0xc2,
     0x5e,
     0x9e,
     0x7,
     0x7,
     0x6,
     0x5,
     0x3,
     0x5,
     0x6,
     0x7,
     0x7},
};

static struct {
    uint8_t* fb;
    bool flipped;
    uint32_t anim_start;
    uint32_t last_draw;
    FuriTimer* timer;
    uint32_t key_time;
    uint8_t key_dir;
    Gui* gui;
    ViewPort* poke;
} mac;

static inline __attribute__((always_inline)) size_t text_len(const char* s) {
    size_t len = 0;
    while(s[len])
        len++;
    return len;
}

static void plot(int x, int y, bool black) {
    if((unsigned)x >= LCD_W || (unsigned)y >= LCD_H) return;
    if(mac.flipped) {
        x = LCD_W - 1 - x;
        y = LCD_H - 1 - y;
    }
    uint8_t* p = mac.fb + (y >> 3) * LCD_W + x;
    if(black) {
        *p |= 1 << (y & 7);
    } else {
        *p &= ~(1 << (y & 7));
    }
}

static void fill(int x, int y, int w, int h, bool black) {
    if(x < 0) {
        w += x;
        x = 0;
    }
    if(y < 0) {
        h += y;
        y = 0;
    }
    if(x + w > LCD_W) w = LCD_W - x;
    if(y + h > LCD_H) h = LCD_H - y;
    if(w <= 0 || h <= 0) return;
    if(mac.flipped) {
        x = LCD_W - x - w;
        y = LCD_H - y - h;
    }
    for(int row = y; row < y + h; row++) {
        uint8_t* p = mac.fb + (row >> 3) * LCD_W + x;
        uint8_t mask = 1 << (row & 7);
        for(int i = 0; i < w; i++) {
            if(black) {
                p[i] |= mask;
            } else {
                p[i] &= ~mask;
            }
        }
    }
}

static void rect(int x, int y, int w, int h) {
    hline(x, y, w);
    hline(x, y + h - 1, w);
    vline(x, y, h);
    vline(x + w - 1, y, h);
}

static void arrow_at(int x, int y, const uint8_t* sprite, bool invert) {
    for(int page = 0; page < 2; page++) {
        int rows = page ? ARROW_H - 8 : 8;
        for(int col = 0; col < ARROW_W; col++) {
            uint8_t image = sprite[page * ARROW_W + col];
            for(int bit = 0; bit < rows; bit++) {
                plot(x + col, y + page * 8 + bit, !(image & (1 << bit)) != invert);
            }
        }
    }
}

static void text_at(int x, int y, const char* s, size_t len, bool black) {
    for(size_t i = 0; i < len; i++) {
        unsigned index = (uint8_t)s[i] - 32;
        if(index >= 96) continue;
        uint8_t d1 = font4x6[index][0];
        uint8_t d2 = font4x6[index][1];
        uint16_t glyph = (d1 << 7) | ((d2 & 2) << 5) | ((d2 >> 2) & 0x3f);
        int top = (d2 & 1) ? y + 1 : y;
        uint16_t bit = 0x4000;
        for(int r = 0; r < 5; r++) {
            for(int c = 0; c < 3; c++, bit >>= 1) {
                if(glyph & bit) plot(x + c, top + r, black);
            }
        }
        x += GLYPH_W;
    }
}

static void tick(void* context) {
    UNUSED(context);
    if(!mac.poke) {
        mac.gui = furi_record_open(RECORD_GUI);
        mac.poke = view_port_alloc();
        view_port_enabled_set(mac.poke, false);
        gui_add_view_port(mac.gui, mac.poke, GuiLayerDesktop);
    }
    gui_view_port_send_to_front(mac.gui, mac.poke);
}

static inline __attribute__((always_inline)) void schedule(uint32_t ticks) {
    if(!mac.timer) mac.timer = furi_timer_alloc(tick, FuriTimerTypeOnce, NULL);
    furi_timer_start(mac.timer, ticks);
}

static inline __attribute__((always_inline)) void draw_window(const char* title, size_t len) {
    rect(WIN_X, WIN_Y, WIN_W, WIN_H);
    hline(WIN_X, WIN_Y + WINDOW_BAR_H, WIN_W);
    for(int j = 2; j < WINDOW_BAR_H - 1; j += 2) {
        hline(WIN_X + 2, WIN_Y + j, WIN_W - 4);
    }
    int tl = len * GLYPH_W;
    fill(WIN_X + WIN_W / 2 - tl / 2 - 2, WIN_Y + 1, tl + 4, WINDOW_BAR_H - 2, false);
    text_at(WIN_X + WIN_W / 2 - tl / 2, WIN_Y + 2, title, len, true);
    fill(WIN_X + 4, WIN_Y + 1, 7, WINDOW_BAR_H - 1, false);
    rect(WIN_X + 5, WIN_Y + 2, 5, 5);
}

static inline __attribute__((always_inline)) void
    draw_scrollbar(size_t current, size_t max, uint8_t hilite) {
    const int bar_x = WIN_X + WIN_W - 10;
    const int y1 = WIN_Y + 9;
    const int len = WIN_H - 10;
    const int y2 = WIN_Y + WIN_H - 1;
    for(int j = 0; j < len; j += 2) {
        int offset = 0;
        for(int i = 0; i < 10; i += 2) {
            plot(bar_x + i, y1 + j + offset, true);
            offset = !offset;
        }
    }
    vline(bar_x - 1, y1, len);
    arrow_at(bar_x, y1, sprites[SpriteUp], hilite == 1);
    arrow_at(bar_x, y2 - ARROW_H, sprites[SpriteDown], hilite == 2);
    if(max) {
        int pos = y1 + 11 + (current * (len - 22 - 9)) / max;
        fill(bar_x, pos, 9, 9, false);
        rect(bar_x, pos, 9, 9);
    }
}

static inline __attribute__((always_inline)) void
    draw_items(Canvas* canvas, MenuModel* model, size_t first_row) {
    for(size_t r = 0; r < GRID_ROWS; r++) {
        for(size_t c = 0; c < GRID_COLS; c++) {
            size_t i = (first_row + r) * GRID_COLS + c;
            if(i >= model->count) return;
            const MenuItem* item = &model->items[i];
            bool selected = i == model->position;
            int x = WIN_X + GRID_X + c * GRID_DX;
            int y = WIN_Y + GRID_Y + r * GRID_DY;
            const char* label = menu_style_label(item, true);
            size_t len = text_len(label);
            if(len > LABEL_MAX) {
                label += menu_style_scroll(model, selected) % (len - LABEL_MAX + 1);
                len = LABEL_MAX;
            }
            canvas_draw_icon_animation(
                canvas,
                x + (ICON_W - item->icon->icon->width) / 2,
                y + (ICON_H - item->icon->icon->height) / 2,
                item->icon);
            int label_w = len * GLYPH_W + 2;
            int label_x = x + ICON_W / 2 - label_w / 2;
            int label_y = y + ICON_H + 1;
            fill(label_x, label_y, label_w, GLYPH_H + 2, selected);
            text_at(label_x + 1, label_y + 1, label, len, !selected);
        }
    }
}

static void draw(Canvas* canvas, MenuModel* model) {
    mac.fb = canvas_get_buffer(canvas);
    mac.flipped = canvas_get_orientation(canvas) == CanvasOrientationHorizontalFlip;
    uint32_t now = furi_get_tick();
    if(now - mac.last_draw > RESTART_TICKS) mac.anim_start = now;
    mac.last_draw = now;
    uint32_t frame = (now - mac.anim_start) / FRAME_TICKS;
    if(frame < ANIM_FRAMES) {
        int w = WIN_W * (frame + 1) / ANIM_FRAMES;
        int h = WIN_H * (frame + 1) / ANIM_FRAMES;
        rect((LCD_W - w) / 2, (LCD_H - h) / 2, w, h);
        schedule(FRAME_TICKS);
        return;
    }

    const char* name = furi_hal_version_get_device_name_ptr();
    draw_window(name, text_len(name));

    size_t rows = (model->count + GRID_COLS - 1) / GRID_COLS;
    size_t max_scroll = rows > GRID_ROWS ? rows - GRID_ROWS : 0;
    size_t row = model->position / GRID_COLS;
    size_t offset = MIN(model->offset, max_scroll);
    if(offset > row) {
        offset = row;
    } else if(offset + GRID_ROWS <= row) {
        offset = row - GRID_ROWS + 1;
    }
    model->offset = offset;
    draw_items(canvas, model, offset);

    if(now - mac.key_time > HILITE_TICKS) mac.key_dir = 0;
    draw_scrollbar(offset, max_scroll, mac.key_dir);
}

static size_t nav(MenuModel* model, InputKey key) {
    size_t position = model->position;
    size_t count = model->count;
    if(key == InputKeyUp || key == InputKeyDown) {
        mac.key_dir = key == InputKeyUp ? 1 : 2;
        mac.key_time = furi_get_tick();
    }
    switch(key) {
    case InputKeyDown:
        return position + GRID_COLS < count ? position + GRID_COLS : position % GRID_COLS;
    case InputKeyUp:
        if(position >= GRID_COLS) return position - GRID_COLS;
        while(position + GRID_COLS < count) {
            position += GRID_COLS;
        }
        return position;
    default:
        return menu_style_navigate_wrap(model, key);
    }
}

static const MenuStyle style = {
    .draw = draw,
    .navigate = nav,
};

MENU_STYLE_PLUGIN(style, menu_style_macintosh_ep)
