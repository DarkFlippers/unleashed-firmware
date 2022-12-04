#pragma once

#include "gpio_app.h"
#include "gpio_item.h"
#include "scenes/gpio_scene.h"
#include "gpio_custom_event.h"
#include "usb_uart_bridge.h"

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <notification/notification_messages.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/widget.h>
#include "views/gpio_test.h"
#include "views/gpio_usb_uart.h"
#include "views/gpio_i2c_scanner.h"
#include "views/gpio_i2c_sfp.h"
#include <gpio_icons.h>

struct GpioApp {
    Gui* gui;
    NotificationApp* notifications;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    Widget* widget;

    VariableItemList* var_item_list;
    VariableItem* var_item_flow;
    GpioTest* gpio_test;
    GpioUsbUart* gpio_usb_uart;
    UsbUartBridge* usb_uart_bridge;
    GpioI2CScanner* gpio_i2c_scanner;
    GpioI2CSfp* gpio_i2c_sfp;
    UsbUartConfig* usb_uart_cfg;
};

typedef enum {
    GpioAppViewVarItemList,
    GpioAppViewGpioTest,
    GpioAppViewUsbUart,
    GpioAppViewUsbUartCfg,
    GpioAppViewUsbUartCloseRpc,
    GpioAppViewI2CScanner,
    GpioAppViewI2CSfp
} GpioAppView;
