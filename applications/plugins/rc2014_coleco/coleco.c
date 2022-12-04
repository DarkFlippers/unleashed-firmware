#include <furi.h>
#include <furi_hal_gpio.h>
#include <furi_hal_power.h>
#include <gui/gui.h>
#include "RC2014_Coleco_icons.h"

#define CODE_0 0x0A
#define CODE_1 0x0D
#define CODE_2 0x07
#define CODE_3 0x0C
#define CODE_4 0x02
#define CODE_5 0x03
#define CODE_6 0x0E
#define CODE_7 0x05
#define CODE_8 0x01
#define CODE_9 0x0B
#define CODE_H 0x06
#define CODE_S 0x09
#define CODE_N 0x0F

const GpioPin* const pin_up = &gpio_ext_pa6;
const GpioPin* const pin_down = &gpio_ext_pc0;
const GpioPin* const pin_right = &gpio_ext_pb2;
const GpioPin* const pin_left = &gpio_ext_pc3;
const GpioPin* const pin_code0 = &gpio_ext_pa7;
const GpioPin* const pin_code1 = &gpio_ext_pa4;
const GpioPin* const pin_code2 = &ibutton_gpio;
const GpioPin* const pin_code3 = &gpio_ext_pc1;
const GpioPin* const pin_fire = &gpio_ext_pb3;
const GpioPin* const pin_alt = &gpio_usart_tx;

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef struct {
    bool dpad;
    int row;
    int column;
} Coleco;

static void render_callback(Canvas* const canvas, void* context) {
    Coleco* coleco = acquire_mutex((ValueMutex*)context, 25);
    if(coleco == NULL) {
        return;
    }

    if(coleco->dpad) {
        canvas_draw_icon(canvas, 4, 16, &I_ColecoJoystick_sel_33x33);
        canvas_draw_icon(canvas, 27, 52, &I_ColecoFire_sel_18x9);
    } else {
        const bool hvr = coleco->row == 0 && coleco->column < 2;
        canvas_draw_icon(
            canvas, 4, 16, hvr ? &I_ColecoJoystick_hvr_33x33 : &I_ColecoJoystick_33x33);
        canvas_draw_icon(canvas, 27, 52, hvr ? &I_ColecoFire_hvr_18x9 : &I_ColecoFire_18x9);
    }

    canvas_draw_icon(
        canvas,
        27,
        4,
        (coleco->row == 0 && coleco->column == 2) ? &I_ColecoAlt_hvr_18x9 : &I_ColecoAlt_18x9);
    canvas_draw_icon(
        canvas,
        49,
        44,
        (coleco->row == 1 && coleco->column == 0) ? &I_Coleco1_hvr_17x17 : &I_Coleco1_17x17);
    canvas_draw_icon(
        canvas,
        49,
        24,
        (coleco->row == 1 && coleco->column == 1) ? &I_Coleco2_hvr_17x17 : &I_Coleco2_17x17);
    canvas_draw_icon(
        canvas,
        49,
        4,
        (coleco->row == 1 && coleco->column == 2) ? &I_Coleco3_hvr_17x17 : &I_Coleco3_17x17);
    canvas_draw_icon(
        canvas,
        69,
        44,
        (coleco->row == 2 && coleco->column == 0) ? &I_Coleco4_hvr_17x17 : &I_Coleco4_17x17);
    canvas_draw_icon(
        canvas,
        69,
        24,
        (coleco->row == 2 && coleco->column == 1) ? &I_Coleco5_hvr_17x17 : &I_Coleco5_17x17);
    canvas_draw_icon(
        canvas,
        69,
        4,
        (coleco->row == 2 && coleco->column == 2) ? &I_Coleco6_hvr_17x17 : &I_Coleco6_17x17);
    canvas_draw_icon(
        canvas,
        89,
        44,
        (coleco->row == 3 && coleco->column == 0) ? &I_Coleco7_hvr_17x17 : &I_Coleco7_17x17);
    canvas_draw_icon(
        canvas,
        89,
        24,
        (coleco->row == 3 && coleco->column == 1) ? &I_Coleco8_hvr_17x17 : &I_Coleco8_17x17);
    canvas_draw_icon(
        canvas,
        89,
        4,
        (coleco->row == 3 && coleco->column == 2) ? &I_Coleco9_hvr_17x17 : &I_Coleco9_17x17);
    canvas_draw_icon(
        canvas,
        109,
        44,
        (coleco->row == 4 && coleco->column == 0) ? &I_ColecoStar_hvr_17x17 : &I_ColecoStar_17x17);
    canvas_draw_icon(
        canvas,
        109,
        24,
        (coleco->row == 4 && coleco->column == 1) ? &I_Coleco0_hvr_17x17 : &I_Coleco0_17x17);
    canvas_draw_icon(
        canvas,
        109,
        4,
        (coleco->row == 4 && coleco->column == 2) ? &I_ColecoPound_hvr_17x17 :
                                                    &I_ColecoPound_17x17);

    release_mutex((ValueMutex*)context, coleco);
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void coleco_write_code(uint8_t code) {
    furi_hal_gpio_write(pin_code0, (code & 1));
    furi_hal_gpio_write(pin_code1, (code & 2));
    furi_hal_gpio_write(pin_code2, (code & 4));
    furi_hal_gpio_write(pin_code3, (code & 8));
}

static void coleco_gpio_init() {
    // configure output pins
    furi_hal_gpio_init(pin_up, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init(pin_down, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init(pin_right, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init(pin_left, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init(pin_code0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init(pin_code1, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init(pin_code2, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init(pin_code3, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init(pin_fire, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init(pin_alt, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);

    furi_hal_gpio_write(pin_up, true);
    furi_hal_gpio_write(pin_down, true);
    furi_hal_gpio_write(pin_right, true);
    furi_hal_gpio_write(pin_left, true);
    furi_hal_gpio_write(pin_fire, true);
    furi_hal_gpio_write(pin_alt, true);

    coleco_write_code(CODE_N);
}

static Coleco* coleco_alloc() {
    Coleco* coleco = malloc(sizeof(Coleco));

    coleco->dpad = false;
    coleco->row = 0;
    coleco->column = 1;

    return coleco;
}

static void coleco_free(Coleco* coleco) {
    furi_assert(coleco);

    free(coleco);
}

int32_t coleco_app(void* p) {
    UNUSED(p);

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));

    Coleco* coleco = coleco_alloc();

    ValueMutex coleco_mutex;
    if(!init_mutex(&coleco_mutex, coleco, sizeof(Coleco))) {
        FURI_LOG_E("Coleco", "cannot create mutex\r\n");
        coleco_free(coleco);
        return 255;
    }

    // set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, &coleco_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    coleco_gpio_init();
    furi_hal_power_enable_otg();

    PluginEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);

        Coleco* coleco = (Coleco*)acquire_mutex_block(&coleco_mutex);

        if(event_status == FuriStatusOk) {
            // press events
            if(event.type == EventTypeKey) {
                switch(event.input.key) {
                case InputKeyUp:
                    if(coleco->dpad) {
                        if(event.input.type == InputTypePress) {
                            furi_hal_gpio_write(pin_up, false);
                        } else if(event.input.type == InputTypeRelease) {
                            furi_hal_gpio_write(pin_up, true);
                        }
                    } else {
                        if(event.input.type == InputTypePress && coleco->column < 2) {
                            coleco->column++;
                            coleco_write_code(CODE_N);
                        }
                    }
                    break;
                case InputKeyDown:
                    if(coleco->dpad) {
                        if(event.input.type == InputTypePress) {
                            furi_hal_gpio_write(pin_down, false);
                        } else if(event.input.type == InputTypeRelease) {
                            furi_hal_gpio_write(pin_down, true);
                        }
                    } else {
                        if(event.input.type == InputTypePress && coleco->column > 0) {
                            coleco->column--;
                            coleco_write_code(CODE_N);
                        }
                    }
                    break;
                case InputKeyRight:
                    if(coleco->dpad) {
                        if(event.input.type == InputTypePress) {
                            furi_hal_gpio_write(pin_right, false);
                        } else if(event.input.type == InputTypeRelease) {
                            furi_hal_gpio_write(pin_right, true);
                        }
                    } else {
                        if(event.input.type == InputTypePress && coleco->row < 4) {
                            coleco->row++;
                            coleco_write_code(CODE_N);
                        }
                    }
                    break;
                case InputKeyLeft:
                    if(coleco->dpad) {
                        if(event.input.type == InputTypePress) {
                            furi_hal_gpio_write(pin_left, false);
                        } else if(event.input.type == InputTypeRelease) {
                            furi_hal_gpio_write(pin_left, true);
                        }
                    } else {
                        if(event.input.type == InputTypePress && coleco->row > 0) {
                            coleco->row--;
                            coleco_write_code(CODE_N);
                        }
                    }
                    break;
                case InputKeyOk:
                    if(coleco->dpad) {
                        if(event.input.type == InputTypePress) {
                            furi_hal_gpio_write(pin_fire, false);
                        } else if(event.input.type == InputTypeRelease) {
                            furi_hal_gpio_write(pin_fire, true);
                        }
                    } else {
                        if(event.input.type == InputTypePress) {
                            if(coleco->row == 0) {
                                if(coleco->column == 2) {
                                    furi_hal_gpio_write(pin_alt, false);
                                } else {
                                    coleco->dpad = true;
                                }
                            } else if(coleco->row == 1) {
                                if(coleco->column == 0) {
                                    coleco_write_code(CODE_1);
                                } else if(coleco->column == 1) {
                                    coleco_write_code(CODE_2);
                                } else {
                                    coleco_write_code(CODE_3);
                                }
                            } else if(coleco->row == 2) {
                                if(coleco->column == 0) {
                                    coleco_write_code(CODE_4);
                                } else if(coleco->column == 1) {
                                    coleco_write_code(CODE_5);
                                } else {
                                    coleco_write_code(CODE_6);
                                }
                            } else if(coleco->row == 3) {
                                if(coleco->column == 0) {
                                    coleco_write_code(CODE_7);
                                } else if(coleco->column == 1) {
                                    coleco_write_code(CODE_8);
                                } else {
                                    coleco_write_code(CODE_9);
                                }
                            } else if(coleco->row == 4) {
                                if(coleco->column == 0) {
                                    coleco_write_code(CODE_S);
                                } else if(coleco->column == 1) {
                                    coleco_write_code(CODE_0);
                                } else {
                                    coleco_write_code(CODE_H);
                                }
                            }
                        }
                        if(event.input.type == InputTypeRelease) {
                            furi_hal_gpio_write(pin_alt, true);
                            coleco_write_code(CODE_N);
                        }
                    }
                    break;
                case InputKeyBack:
                    if(event.input.type == InputTypePress) {
                        if(coleco->dpad) {
                            coleco->dpad = false;
                        } else {
                            processing = false;
                        }
                    }
                    break;
                default:
                    break;
                }

                view_port_update(view_port);
            }
        } else {
            FURI_LOG_D("Coleco", "FuriMessageQueue: event timeout");
        }

        release_mutex(&coleco_mutex, coleco);
    }

    furi_hal_power_disable_otg();

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    delete_mutex(&coleco_mutex);
    coleco_free(coleco);
    return 0;
}
