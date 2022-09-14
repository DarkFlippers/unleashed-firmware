#include "../usb_uart_bridge.h"
#include "../gpio_app_i.h"
#include "furi_hal.h"
#include <gui/elements.h>

struct GpioUsbUart {
    View* view;
    GpioUsbUartCallback callback;
    void* context;
};

typedef struct {
    uint32_t baudrate;
    uint32_t tx_cnt;
    uint32_t rx_cnt;
    uint8_t vcp_port;
    uint8_t tx_pin;
    uint8_t rx_pin;
    bool tx_active;
    bool rx_active;
} GpioUsbUartModel;

static void gpio_usb_uart_draw_callback(Canvas* canvas, void* _model) {
    GpioUsbUartModel* model = _model;
    char temp_str[18];
    elements_button_left(canvas, "Config");
    canvas_draw_line(canvas, 2, 10, 125, 10);
    canvas_draw_line(canvas, 44, 52, 123, 52);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 9, "USB Serial");
    canvas_draw_str(canvas, 3, 25, "TX:");
    canvas_draw_str(canvas, 3, 42, "RX:");

    canvas_set_font(canvas, FontSecondary);
    snprintf(temp_str, 18, "COM PORT:%u", model->vcp_port);
    canvas_draw_str_aligned(canvas, 126, 8, AlignRight, AlignBottom, temp_str);
    snprintf(temp_str, 18, "Pin %u", model->tx_pin);
    canvas_draw_str(canvas, 22, 25, temp_str);
    snprintf(temp_str, 18, "Pin %u", model->rx_pin);
    canvas_draw_str(canvas, 22, 42, temp_str);

    if(model->baudrate == 0)
        snprintf(temp_str, 18, "Baud: ????");
    else
        snprintf(temp_str, 18, "Baud: %lu", model->baudrate);
    canvas_draw_str(canvas, 45, 62, temp_str);

    if(model->tx_cnt < 100000000) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 127, 24, AlignRight, AlignBottom, "B.");
        canvas_set_font(canvas, FontKeyboard);
        snprintf(temp_str, 18, "%lu", model->tx_cnt);
        canvas_draw_str_aligned(canvas, 116, 24, AlignRight, AlignBottom, temp_str);
    } else {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 127, 24, AlignRight, AlignBottom, "KB.");
        canvas_set_font(canvas, FontKeyboard);
        snprintf(temp_str, 18, "%lu", model->tx_cnt / 1024);
        canvas_draw_str_aligned(canvas, 111, 24, AlignRight, AlignBottom, temp_str);
    }

    if(model->rx_cnt < 100000000) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 127, 41, AlignRight, AlignBottom, "B.");
        canvas_set_font(canvas, FontKeyboard);
        snprintf(temp_str, 18, "%lu", model->rx_cnt);
        canvas_draw_str_aligned(canvas, 116, 41, AlignRight, AlignBottom, temp_str);
    } else {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 127, 41, AlignRight, AlignBottom, "KB.");
        canvas_set_font(canvas, FontKeyboard);
        snprintf(temp_str, 18, "%lu", model->rx_cnt / 1024);
        canvas_draw_str_aligned(canvas, 111, 41, AlignRight, AlignBottom, temp_str);
    }

    if(model->tx_active)
        canvas_draw_icon(canvas, 48, 14, &I_ArrowUpFilled_14x15);
    else
        canvas_draw_icon(canvas, 48, 14, &I_ArrowUpEmpty_14x15);

    if(model->rx_active)
        canvas_draw_icon(canvas, 48, 34, &I_ArrowDownFilled_14x15);
    else
        canvas_draw_icon(canvas, 48, 34, &I_ArrowDownEmpty_14x15);
}

static bool gpio_usb_uart_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    GpioUsbUart* usb_uart = context;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyLeft) {
            consumed = true;
            furi_assert(usb_uart->callback);
            usb_uart->callback(GpioUsbUartEventConfig, usb_uart->context);
        }
    }

    return consumed;
}

GpioUsbUart* gpio_usb_uart_alloc() {
    GpioUsbUart* usb_uart = malloc(sizeof(GpioUsbUart));

    usb_uart->view = view_alloc();
    view_allocate_model(usb_uart->view, ViewModelTypeLocking, sizeof(GpioUsbUartModel));
    view_set_context(usb_uart->view, usb_uart);
    view_set_draw_callback(usb_uart->view, gpio_usb_uart_draw_callback);
    view_set_input_callback(usb_uart->view, gpio_usb_uart_input_callback);

    return usb_uart;
}

void gpio_usb_uart_free(GpioUsbUart* usb_uart) {
    furi_assert(usb_uart);
    view_free(usb_uart->view);
    free(usb_uart);
}

View* gpio_usb_uart_get_view(GpioUsbUart* usb_uart) {
    furi_assert(usb_uart);
    return usb_uart->view;
}

void gpio_usb_uart_set_callback(GpioUsbUart* usb_uart, GpioUsbUartCallback callback, void* context) {
    furi_assert(usb_uart);
    furi_assert(callback);

    with_view_model(
        usb_uart->view, (GpioUsbUartModel * model) {
            UNUSED(model);
            usb_uart->callback = callback;
            usb_uart->context = context;
            return false;
        });
}

void gpio_usb_uart_update_state(GpioUsbUart* instance, UsbUartConfig* cfg, UsbUartState* st) {
    furi_assert(instance);
    furi_assert(cfg);
    furi_assert(st);

    with_view_model(
        instance->view, (GpioUsbUartModel * model) {
            model->baudrate = st->baudrate_cur;
            model->vcp_port = cfg->vcp_ch;
            model->tx_pin = (cfg->uart_ch == 0) ? (13) : (15);
            model->rx_pin = (cfg->uart_ch == 0) ? (14) : (16);
            model->tx_active = (model->tx_cnt != st->tx_cnt);
            model->rx_active = (model->rx_cnt != st->rx_cnt);
            model->tx_cnt = st->tx_cnt;
            model->rx_cnt = st->rx_cnt;
            return true;
        });
}
