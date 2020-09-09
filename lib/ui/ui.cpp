#include <stdio.h>

extern "C" {
    #include "main.h"
    #include "cmsis_os.h"
    #include "u8g2_support.h"
    #include "u8g2/u8g2.h"
}

#include "ui.h"
#include "events.h"

// function draw basic layout -- single bmp
void draw_bitmap(const char* bitmap, u8g2_t* u8g2, ScreenArea area) {
    if(bitmap == NULL) {
        printf("[basic layout] no content\n");
        u8g2_SetFont(u8g2, u8g2_font_6x10_mf);
        u8g2_SetDrawColor(u8g2, 1);
        u8g2_SetFontMode(u8g2, 1);
        u8g2_DrawStr(u8g2, 2, 12, "no content");
    } else {
        u8g2_SetDrawColor(u8g2, 1);
        u8g2_DrawXBM(u8g2, 0, 0, area.x + area.width, area.y + area.height, (unsigned char*)bitmap);
    }
}

void draw_text(const char* text, u8g2_t* u8g2, ScreenArea area) {
    // TODO proper cleanup statusbar
    u8g2_SetDrawColor(u8g2, 0);
    u8g2_DrawBox(u8g2, 0, 0, area.x + area.width, area.y + area.height);

    Block text_block = Block {
        width: area.width,
        height: area.height,
        margin_left: 0,
        margin_top: 0,
        padding_left: 3,
        padding_top: 7,
        background: 0,
        color: 1,
        font: (uint8_t*)u8g2_font_6x10_mf,
    };

    draw_block(u8g2, text, text_block, area.x, area.y);
}

// draw layout and switch between ui item by button and timer
void LayoutComponent::handle(Event* event, Store* store, u8g2_t* u8g2, ScreenArea area) {
    switch(event->type) {
        // get button event
        case EventTypeButton:
            if(event->value.button.state) {
                for(size_t i = 0; i < this->actions_size; i++) {
                    FlipperComponent* next = NULL;

                    switch(this->actions[i].action) {
                        case LayoutActionUp:
                            if(event->value.button.id == ButtonsUp) {
                                next = this->actions[i].item;
                            }
                        break;
                        case LayoutActionDown:
                            if(event->value.button.id == ButtonsDown) {
                                next = this->actions[i].item;
                            }
                        break;
                        case LayoutActionLeft:
                            if(event->value.button.id == ButtonsLeft) {
                                next = this->actions[i].item;
                            }
                        break;
                        case LayoutActionRight:
                            if(event->value.button.id == ButtonsRight) {
                                next = this->actions[i].item;
                            }
                        break;
                        case LayoutActionOk:
                            if(event->value.button.id == ButtonsOk) {
                                next = this->actions[i].item;
                            }
                        break;
                        case LayoutActionBack:
                            if(event->value.button.id == ButtonsBack) {
                                next = this->actions[i].item;
                            }
                        break;

                        // stub action
                        case LayoutActionUsbDisconnect:
                            if(event->value.button.id == ButtonsLeft) {
                                next = this->actions[i].item;
                            }
                        break;

                        case LayoutActionUsbConnect:
                            if(event->value.button.id == ButtonsRight) {
                                next = this->actions[i].item;
                            }
                        break;

                        default: break;
                    }

                    if(next) {
                        printf("[layout view] go to next item\n");
                        Event send_event;
                        send_event.type = EventTypeUiNext;
                        next->handle(
                            &send_event,
                            store,
                            u8g2,
                            area
                        );
                    }
                }
            }
        break;

        case EventTypeUsb: {
            printf("get usb event\n");
            
            FlipperComponent* next = NULL;

            if(event->value.usb == UsbEventConnect) {
                for(size_t i = 0; i < this->actions_size; i++) {
                    if(this->actions[i].action == LayoutActionUsbConnect) {
                        next = this->actions[i].item;
                    }
                }
            }

            if(event->value.usb == UsbEventDisconnect) {
                for(size_t i = 0; i < this->actions_size; i++) {
                    if(this->actions[i].action == LayoutActionUsbDisconnect) {
                        next = this->actions[i].item;
                    }
                }
            }

            if(next) {
                printf("[layout view] go to next item\n");
                Event send_event;
                send_event.type = EventTypeUiNext;
                next->handle(
                    &send_event,
                    store,
                    u8g2,
                    area
                );
            }
        } break;

        // start component from prev
        case EventTypeUiNext:
            printf("[layout view] start component %lX\n", (uint32_t)this);

            if(this->timeout > 0) {
                // TODO start timer
            }

            // set current item to self
            store->current_component = this;

            this->wait_time = 0;

            // render layout
            this->dirty = true;
        break;

        case EventTypeTick:
            this->wait_time += event->value.tick_delta;

            if(this->wait_time > this->timeout) {
                for(size_t i = 0; i < this->actions_size; i++) {
                    if(this->actions[i].action == LayoutActionTimeout ||
                        this->actions[i].action == LayoutActionEndOfCycle
                    ) {
                        if(this->actions[i].item != NULL) {
                            printf("[layout view] go to next item\n");
                            Event send_event;
                            send_event.type = EventTypeUiNext;
                            this->actions[i].item->handle(
                                &send_event,
                                store,
                                u8g2,
                                area
                            );

                            return;
                        }
                    }
                }
            }

            if(this->dirty) {
                this->draw_fn(this->data, u8g2, area);

                store->dirty_screen = true;

                this->dirty = false;
            }
        break;

        default: break;
    }
}

void BlinkerComponent::handle(Event* event, Store* store, u8g2_t* u8g2, ScreenArea area) {
    switch(event->type) {
        // get button event
        case EventTypeButton:
            if(event->value.button.state && event->value.button.id == ButtonsBack) {
                if(this->prev && this->prev != this) {
                    printf("[blinker view] go back\n");

                    Event send_event;
                    send_event.type = EventTypeUiNext;
                    this->prev->handle(
                        &send_event,
                        store,
                        u8g2,
                        area
                    );

                    this->prev = NULL;

                    store->led = ColorBlack;
                    this->wait_time = 0;
                    this->is_on = true;
                    this->active = false;
                } else {
                    printf("[blinker view] no back/loop\n");
                }
            }

            if(event->value.button.state && event->value.button.id != ButtonsBack) {
                this->active = false;
            }

            if(!event->value.button.state && event->value.button.id != ButtonsBack) {
                this->active = true;
            }
        break;

        // start component from prev
        case EventTypeUiNext:
            printf("[blinker view] start component %lX\n", (uint32_t)this);

            if(this->prev == NULL) {
                this->prev = store->current_component;
            }

            // set current item to self
            store->current_component = this;

            this->dirty = true;
            this->wait_time = 0;
            this->is_on = true;
            this->active = false;
        break;

        case EventTypeTick:
            if(this->active) {
                this->wait_time += event->value.tick_delta;

                if(this->is_on) {
                    if(this->wait_time > this->config.on_time) {
                        this->wait_time = 0;
                        this->is_on = false;
                    }
                } else {
                    if(this->wait_time > this->config.off_time) {
                        this->wait_time = 0;
                        this->is_on = true;
                    }
                }

                store->led = this->is_on ? this->config.on_color : this->config.off_color;
            } else {
                store->led = ColorBlack;
                this->wait_time = 0;
                this->is_on = true;
            }

            if(this->dirty) {
                this->draw_fn(this->data, u8g2, area);

                store->dirty_screen = true;

                this->dirty = false;
            }
        break;

        default: break;
    }
}
void BlinkerComponentOnBtn::handle(Event* event, Store* store, u8g2_t* u8g2, ScreenArea area) {
    switch(event->type) {
        // get button event
        case EventTypeButton:
            if(event->value.button.state && event->value.button.id == ButtonsBack) {
                if(this->prev && this->prev != this) {
                    printf("[blinker view] go back\n");

                    Event send_event;
                    send_event.type = EventTypeUiNext;
                    this->prev->handle(
                        &send_event,
                        store,
                        u8g2,
                        area
                    );

                    this->prev = NULL;

                    store->led = ColorBlack;
                    this->wait_time = 0;
                    this->is_on = true;
                    this->active = false;
                } else {
                    printf("[blinker view] no back/loop\n");
                }
            }

            if(event->value.button.state && event->value.button.id != ButtonsBack) {
                this->active = true;
            }

            if(!event->value.button.state && event->value.button.id != ButtonsBack) {
                this->active = false;
            }
        break;

        // start component from prev
        case EventTypeUiNext:
            printf("[blinker view] start component %lX\n", (uint32_t)this);

            if(this->prev == NULL) {
                this->prev = store->current_component;
            }

            // set current item to self
            store->current_component = this;

            this->dirty = true;
            this->wait_time = 0;
            this->is_on = true;
            this->active = false;
        break;

        case EventTypeTick:
            if(this->active) {
                this->wait_time += event->value.tick_delta;

                if(this->is_on) {
                    if(this->wait_time > this->config.on_time) {
                        this->wait_time = 0;
                        this->is_on = false;
                    }
                } else {
                    if(this->wait_time > this->config.off_time) {
                        this->wait_time = 0;
                        this->is_on = true;
                    }
                }

                store->led = this->is_on ? this->config.on_color : this->config.off_color;
            } else {
                store->led = ColorBlack;
                this->wait_time = 0;
                this->is_on = true;
            }

            if(this->dirty) {
                this->draw_fn(this->data, u8g2, area);

                store->dirty_screen = true;

                this->dirty = false;
            }
        break;

        default: break;
    }
}

#define MENU_DRAW_LINES 4

Point draw_block(u8g2_t* u8g2, const char* text, Block layout, uint8_t x, uint8_t y) {
    u8g2_SetDrawColor(u8g2, layout.background);
    u8g2_DrawBox(u8g2,
        x + layout.margin_left,
        y + layout.margin_top,
        layout.width, layout.height
    );

    u8g2_SetDrawColor(u8g2, layout.color);
    u8g2_SetFont(u8g2, layout.font);
    if(text != NULL) {
        u8g2_DrawStr(u8g2,
            x + layout.margin_left + layout.padding_left,
            y + layout.margin_top + layout.padding_top,
            text
        );
    }

    return {
        x: x + layout.margin_left + layout.width,
        y: y + layout.margin_top + layout.height
    };
}

void draw_menu(MenuCtx* ctx, u8g2_t* u8g2, ScreenArea area) {
    // u8g2_ClearBuffer(u8g2);
    // clear area
    u8g2_SetDrawColor(u8g2, 0);
    u8g2_DrawBox(u8g2, area.x, area.y, area.width, area.height);

    u8g2_SetFontMode(u8g2, 1);

    uint8_t list_start = ctx->current - ctx->cursor;
    uint8_t list_size = ctx->size > MENU_DRAW_LINES ? MENU_DRAW_LINES : ctx->size;

    // draw header
    /*
    Point next = draw_block(u8g2, (const char*)data->name, Block {
        width: 128,
        height: 14,
        margin_left: 0,
        margin_top: 0,
        padding_left: 4,
        padding_top: 13,
        background: 1,
        color: 0,
        font: (uint8_t*)u8g2_font_helvB14_tf,
    }, area.x, area.y);
    */

    Point next = {area.x, area.y};

    for(size_t i = 0; i < list_size; i++) {
        next = draw_block(u8g2, (const char*)ctx->list[list_start + i].name, Block {
            width: 128,
            height: 15,
            margin_left: 0,
            margin_top: i == 0 ? 2 : 0,
            padding_left: 2,
            padding_top: 12,
            background: i == ctx->cursor ? 1 : 0,
            color: i == ctx->cursor ? 0 : 1,
            font: (uint8_t*)u8g2_font_7x14_tf,
        }, area.x, next.y);
    }

    // u8g2_font_7x14_tf
    // u8g2_font_profont12_tf smallerbut cute
    // u8g2_font_samim_12_t_all орочий
}

void MenuCtx::handle(MenuEvent event) {
    uint8_t menu_size = this->size > MENU_DRAW_LINES ? MENU_DRAW_LINES : this->size;

    switch(event) {
        case MenuEventDown: {
            if(this->current < (this->size - 1)) {
                this->current++;

                if(this->cursor < menu_size - 2 || this->current == this->size - 1) {
                    this->cursor++;
                }
            } else {
                this->current = 0;
                this->cursor = 0;
            }
        } break;

        case MenuEventUp: {
            if(this->current > 0) {
                this->current--;

                if(this->cursor > 1 || this->current == 0) {
                    this->cursor--;
                }

            } else {
                this->current = this->size - 1;
                this->cursor = menu_size - 1;
            }
        } break;
    }
}

void MenuCtx::reset() {
    this->current = 0;
    this->cursor = 0;
}

// draw numenu and handle navigation
void MenuComponent::handle(Event* event, Store* store, u8g2_t* u8g2, ScreenArea area) {
    switch(event->type) {
        // get button event
        case EventTypeButton: {
            if(event->value.button.id == ButtonsOk && event->value.button.state) {
                if(this->ctx.current < this->ctx.size) {
                    FlipperComponent* next_item = (FlipperComponent*)this->ctx.list[this->ctx.current].item;

                    if(next_item) {
                        store->is_fullscreen = false;

                        printf("[layout view] go to %d item\n", this->ctx.current);
                        Event send_event;
                        send_event.type = EventTypeUiNext;

                        next_item->handle(
                            &send_event,
                            store,
                            u8g2,
                            area
                        );
                    } else {
                        printf("[menu view] no item at %d\n", this->ctx.current);
                    }
                }
            }

            if(event->value.button.id == ButtonsDown && event->value.button.state) {
                this->ctx.handle(MenuEventDown);
                this->dirty = true;
            }

            if(event->value.button.id == ButtonsUp && event->value.button.state) {
                this->ctx.handle(MenuEventUp);
                this->dirty = true;
            }

            // go back item
            if(event->value.button.id == ButtonsBack && event->value.button.state) {
                if(this->prev && this->prev != this) {
                    store->is_fullscreen = false;

                    printf("[menu view] go back\n");

                    this->ctx.reset();

                    Event send_event;
                    send_event.type = EventTypeUiNext;
                    this->prev->handle(
                        &send_event,
                        store,
                        u8g2,
                        area
                    );

                    this->prev = NULL;
                } else {
                    printf("[menu view] no back/loop\n");
                }
            }
        } break;

        // start component from prev
        case EventTypeUiNext:
            printf("[menu view] start component %lX (size %d)\n", (uint32_t)this, this->ctx.size);

            // set prev item
            if(this->prev == NULL) {
                printf("[menu view] set prev element to %lX\n", (uint32_t)store->current_component);
                this->prev = store->current_component;
            }
            // set current item to self
            store->current_component = this;

            store->is_fullscreen = true;

            // render menu
            this->dirty = true;
        break;

        case EventTypeTick:
            if(this->dirty) {
                draw_menu(&this->ctx, u8g2, area);
                store->dirty_screen = true;
                this->dirty = false;
            }
        break;

        default: break;
    }
}
