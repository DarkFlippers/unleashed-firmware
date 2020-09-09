#pragma once

extern "C" {
    #include "main.h"
    #include "cmsis_os.h"
    #include "u8g2_support.h"
    #include "u8g2/u8g2.h"
}

#include "events.h"

typedef struct {
    void* item;
    const char* name;
} MenuItem;

#include "vendor.h"

typedef enum {
    LayoutActionUp,
    LayoutActionDown,
    LayoutActionLeft,
    LayoutActionRight,
    LayoutActionOk,
    LayoutActionBack,
    LayoutActionTimeout,
    LayoutActionUsbConnect,
    LayoutActionUsbDisconnect,
    LayoutActionEndOfCycle
} LayoutAction;

typedef struct {
    FlipperComponent* item;
    LayoutAction action;
} ActionItem;

void draw_text(const char* text, u8g2_t* u8g2, ScreenArea area);
void draw_bitmap(const char* bitmap, u8g2_t* u8g2, ScreenArea area);

class LayoutComponent: FlipperComponent {
public:
    LayoutComponent(void (*draw_fn)(const char* text, u8g2_t* u8g2, ScreenArea area), ActionItem* actions, size_t actions_size, uint32_t timeout, const char* data) {
        this->data = data;
        this->actions = actions;
        this->actions_size = actions_size;
        this->timeout = timeout;
        this->draw_fn = draw_fn;

        this->dirty = true;

        this->wait_time = 0;
    }

    virtual void handle(Event* event, struct _Store* store, u8g2_t* u8g2, ScreenArea area);

private:
    const char* data;
    ActionItem* actions;
    size_t actions_size;
    uint32_t timeout;

    void (*draw_fn)(const char* text, u8g2_t* u8g2, ScreenArea area);

    uint32_t wait_time;

    bool dirty;
};

typedef struct {
    uint32_t on_time;
    Color on_color;
    uint32_t off_time;
    Color off_color;
} BlinkerComponentConfig;

class BlinkerComponent: FlipperComponent {
public:
    BlinkerComponent(
        void (*draw_fn)(const char* text, u8g2_t* u8g2, ScreenArea area),
        BlinkerComponentConfig config,
        const char* data
    ) {
        this->data = data;
        this->draw_fn = draw_fn;
        this->config = config;

        this->dirty = true;

        this->wait_time = 0;
        this->is_on = true;
        this->active = false;
        this->prev = NULL;
    }

    virtual void handle(Event* event, struct _Store* store, u8g2_t* u8g2, ScreenArea area);

private:
    const char* data;
    BlinkerComponentConfig config;

    void (*draw_fn)(const char* text, u8g2_t* u8g2, ScreenArea area);

    uint32_t wait_time;

    bool is_on;
    bool active;

    bool dirty;
    FlipperComponent* prev;
};

typedef struct {
    uint32_t on_time;
    Color on_color;
    uint32_t off_time;
    Color off_color;
} BlinkerComponentOnBtnConfig;

class BlinkerComponentOnBtn: FlipperComponent {
public:
    BlinkerComponentOnBtn(
        void (*draw_fn)(const char* text, u8g2_t* u8g2, ScreenArea area),
        BlinkerComponentOnBtnConfig config,
        const char* data
    ) {
        this->data = data;
        this->draw_fn = draw_fn;
        this->config = config;

        this->dirty = true;

        this->wait_time = 0;
        this->is_on = true;
        this->active = false;
        this->prev = NULL;
    }

    virtual void handle(Event* event, struct _Store* store, u8g2_t* u8g2, ScreenArea area);

private:
    const char* data;
    BlinkerComponentOnBtnConfig config;

    void (*draw_fn)(const char* text, u8g2_t* u8g2, ScreenArea area);

    uint32_t wait_time;

    bool is_on;
    bool active;

    bool dirty;
    FlipperComponent* prev;
};



typedef enum {
    MenuEventUp,
    MenuEventDown
} MenuEvent;

class MenuCtx {
public:
    size_t size;
    size_t current;
    uint8_t cursor;
    MenuItem* list;

    void handle(MenuEvent event);
    void reset();
};

void draw_menu(MenuCtx* ctx, u8g2_t* u8g2, ScreenArea area);

class MenuComponent: FlipperComponent {
public:
    MenuComponent(MenuItem* list, size_t size, const char* name) {
        this->ctx.size = size;
        this->ctx.current = 0;
        this->ctx.cursor = 0;

        this->ctx.list = list;
        
        this->name = name;
        this->prev = NULL;

        this->dirty = true;
    }

    
    const char* name;
    FlipperComponent* prev;
    MenuCtx ctx;
    

    bool dirty;

    virtual void handle(Event* event, struct _Store* store, u8g2_t* u8g2, ScreenArea area);
};

typedef struct {
    uint8_t x;
    uint8_t y;
} Point;

typedef struct {
    uint8_t width;
    uint8_t height;
    uint8_t margin_left;
    uint8_t margin_top;
    uint8_t padding_left;
    uint8_t padding_top;
    uint8_t background;
    uint8_t color;
    uint8_t* font;
} Block;

Point draw_block(u8g2_t* u8g2, const char* text, Block layout, uint8_t x, uint8_t y);
