#pragma once

#include <gui/view.h>
#include "../gpio_custom_event.h"
#include "../usb_uart_bridge.h"

typedef struct GpioUsbUart GpioUsbUart;
typedef void (*GpioUsbUartCallback)(GpioCustomEvent event, void* context);

GpioUsbUart* gpio_usb_uart_alloc(void);

void gpio_usb_uart_free(GpioUsbUart* usb_uart);

View* gpio_usb_uart_get_view(GpioUsbUart* usb_uart);

void gpio_usb_uart_set_callback(GpioUsbUart* usb_uart, GpioUsbUartCallback callback, void* context);

void gpio_usb_uart_update_state(GpioUsbUart* instance, UsbUartConfig* cfg, UsbUartState* st);
