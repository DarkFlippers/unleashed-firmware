#pragma once

#include "gpio_app.h"
#include "gpio_item.h"
#include "scenes/gpio_scene.h"

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <notification/notification-messages.h>

#include <gui/modules/variable-item-list.h>
#include "views/gpio_test.h"

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
} GpioAppView;
