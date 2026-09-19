/**
 * @file three_d.c
 * 3D main menu style.
 *
 * Ported by @apfxtech. Based on the 3D OLED carousel menu (MIT), by @upiir:
 * https://github.com/upiir/arduino_3d_menu_oled
 */
#include "menu_style_helpers.h"

#include <gui/gui.h>
#include <gui/icon_animation_i.h>
#include <gui/icon_i.h>

#define LCD_W 128
#define LCD_H 64

#define RING_SLOTS  5
#define SLOT_FRAMES 30
#define PATH_FRAMES (RING_SLOTS * SLOT_FRAMES)
#define HALF_PATH   (PATH_FRAMES / 2)
#define HALF_STEP   (SLOT_FRAMES / 2)

#define ZOOM_FRAMES 18
#define ZOOM_MIN    100
#define ZOOM_MAX    200
#define ICON_MAX    16

#define STEP_TICKS    320
#define FRAME_TICKS   32
#define RESTART_TICKS 1000

#define LABEL_MAX 24

static const uint8_t path[][2] = {
    {64, 37}, {64, 37}, {64, 37}, {63, 37}, {63, 37}, {62, 37}, {61, 37}, {60, 37}, {59, 37},
    {57, 37}, {55, 37}, {52, 37}, {49, 36}, {46, 35}, {43, 35}, {39, 33}, {36, 32}, {34, 31},
    {32, 30}, {31, 29}, {30, 28}, {29, 27}, {29, 26}, {28, 26}, {28, 26}, {28, 25}, {28, 25},
    {28, 25}, {28, 25}, {28, 25}, {28, 25}, {28, 25}, {28, 25}, {28, 24}, {28, 24}, {28, 24},
    {27, 24}, {27, 23}, {27, 23}, {27, 22}, {28, 22}, {28, 21}, {28, 20}, {29, 18}, {30, 17},
    {32, 16}, {33, 15}, {35, 14}, {37, 13}, {39, 12}, {40, 12}, {41, 11}, {42, 11}, {43, 11},
    {44, 10}, {44, 10}, {45, 10}, {45, 10}, {45, 10}, {45, 10}, {46, 10}, {46, 10}, {46, 10},
    {46, 10}, {46, 10}, {47, 10}, {47, 10}, {48, 9},  {49, 9},  {50, 9},  {51, 9},  {53, 9},
    {55, 8},  {58, 8},  {61, 8},  {64, 8},
};

_Static_assert(COUNT_OF(path) == HALF_PATH + 1, "path table must cover half a turn");

static struct {
    uint8_t* fb;
    bool flipped;
    uint32_t anim_start;
    uint32_t last_draw;
    FuriTimer* timer;
    Gui* gui;
    ViewPort* poke;
    size_t position;
    int32_t dir;
} ring;

static inline __attribute__((always_inline)) uint8_t* cell(int x, int y, uint8_t* mask) {
    if(ring.flipped) {
        x = LCD_W - 1 - x;
        y = LCD_H - 1 - y;
    }
    *mask = 1 << (y & 7);
    return &ring.fb[(y >> 3) * LCD_W + x];
}

static inline __attribute__((always_inline)) void
    draw_icon(Canvas* canvas, IconAnimation* icon, int cx, int cy, int percent) {
    int sw = icon->icon->width;
    int sh = icon->icon->height;
    int x = cx - sw / 2;
    int y = cy - sh / 2;
    canvas_draw_icon_animation(canvas, x, y, icon);
    if(percent <= ZOOM_MIN || sw > ICON_MAX || sh > ICON_MAX) return;

    uint8_t mask;
    uint32_t rows[ICON_MAX];
    for(int r = 0; r < sh; r++) {
        uint32_t bits = 0;
        for(int c = 0; c < sw; c++) {
            if((unsigned)(x + c) >= LCD_W || (unsigned)(y + r) >= LCD_H) continue;
            uint8_t* p = cell(x + c, y + r, &mask);
            if(*p & mask) bits |= 1u << c;
            *p &= ~mask;
        }
        rows[r] = bits;
    }

    int dw = sw * percent / 100;
    int dh = sh * percent / 100;
    x = cx - dw / 2;
    y = cy - dh / 2;
    for(int r = 0; r < dh; r++) {
        uint32_t bits = rows[r * sh / dh];
        int py = y + r;
        if((unsigned)py >= LCD_H) continue;
        for(int c = 0; c < dw; c++) {
            int px = x + c;
            if((unsigned)px >= LCD_W) continue;
            if(bits & (1u << (c * sw / dw))) {
                uint8_t* p = cell(px, py, &mask);
                *p |= mask;
            }
        }
    }
}

static void poke(void* context) {
    UNUSED(context);
    if(!ring.poke) {
        ring.gui = furi_record_open(RECORD_GUI);
        ring.poke = view_port_alloc();
        view_port_enabled_set(ring.poke, false);
        gui_add_view_port(ring.gui, ring.poke, GuiLayerDesktop);
    }
    gui_view_port_send_to_front(ring.gui, ring.poke);
}

static void draw(Canvas* canvas, MenuModel* model) {
    ring.fb = canvas_get_buffer(canvas);
    ring.flipped = canvas_get_orientation(canvas) == CanvasOrientationHorizontalFlip;

    uint32_t now = furi_get_tick();
    uint32_t step = STEP_TICKS;
    if(now - ring.last_draw > RESTART_TICKS) {
        ring.position = model->position;
        ring.anim_start = now - step;
    } else if(model->position != ring.position) {
        ring.position = model->position;
        ring.anim_start = now;
    }
    ring.last_draw = now;

    uint32_t elapsed = now - ring.anim_start;
    int32_t phase = 0;
    if(elapsed < step) {
        phase = ring.dir * (int32_t)(SLOT_FRAMES * (step - elapsed) / step);
        if(!ring.timer) {
            ring.timer = furi_timer_alloc(poke, FuriTimerTypeOnce, NULL);
        }
        furi_timer_start(ring.timer, FRAME_TICKS);
    }

    int count = model->count;
    int slots = MIN(count, RING_SLOTS);
    for(int d = -(slots / 2); d < slots - slots / 2; d++) {
        int frame = (PATH_FRAMES - SLOT_FRAMES * d - phase) % PATH_FRAMES;
        int dist = frame < HALF_PATH ? frame : PATH_FRAMES - frame;
        int mirror = frame > HALF_PATH;
        const uint8_t* point = path[mirror ? PATH_FRAMES - frame : frame];
        int percent = ZOOM_MIN;
        if(dist < ZOOM_FRAMES) {
            percent += (ZOOM_MAX - ZOOM_MIN) * (ZOOM_FRAMES - dist) / ZOOM_FRAMES;
        }
        draw_icon(
            canvas,
            model->items[((int)model->position + count + d) % count].icon,
            mirror ? LCD_W - point[0] : point[0],
            point[1],
            percent);
    }

    uint32_t away = phase < 0 ? -phase : phase;
    int index = (int)model->position;
    if(away > HALF_STEP) {
        index = (index + count - ring.dir) % count;
        away -= HALF_STEP;
    } else {
        away = HALF_STEP - away;
    }
    const char* label = model->items[index].label;
    size_t len = 0;
    while(label[len])
        len++;
    len = len * away / HALF_STEP;
    if(len > (size_t)(LABEL_MAX - 1)) len = LABEL_MAX - 1;
    char text[LABEL_MAX];
    for(size_t i = 0; i < len; i++)
        text[i] = label[i];
    text[len] = '\0';
    canvas_draw_str_aligned(canvas, LCD_W / 2, LCD_H - 1, AlignCenter, AlignBottom, text);
}

static size_t nav(MenuModel* model, InputKey key) {
    bool back = key == InputKeyUp || key == InputKeyLeft;
    ring.dir = back ? -1 : 1;
    if(back) return model->position ? model->position - 1 : model->count - 1;
    return (model->position + 1) % model->count;
}

static const MenuStyle style = {
    .draw = draw,
    .navigate = nav,
};

MENU_STYLE_PLUGIN(style, menu_style_3d_ep)
