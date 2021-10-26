#pragma once

#include "gpio_app.h"
#include "gpio_item.h"
#include "scenes/gpio_scene.h"

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <notification/notification-messages.h>
#include <gui/modules/variable-item-list.h>
#include "views/gpio_test.h"

#define GPIO_SCENE_START_CUSTOM_EVENT_OTG_OFF (0UL)
#define GPIO_SCENE_START_CUSTOM_EVENT_OTG_ON (1UL)
#define GPIO_SCENE_START_CUSTOM_EVENT_TEST (2UL)
#define GPIO_SCENE_START_CUSTOM_EVENT_USB_UART (3UL)

#define GPIO_SCENE_USB_UART_CUSTOM_EVENT_ENABLE (4UL)
#define GPIO_SCENE_USB_UART_CUSTOM_EVENT_DISABLE (5UL)

struct GpioApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    NotificationApp* notifications;

    VariableItemList* var_item_list;
    GpioTest* gpio_test;
};

typedef enum {
    GpioAppViewVarItemList,
    GpioAppViewGpioTest,
    GpioAppViewUsbUart,
} GpioAppView;
