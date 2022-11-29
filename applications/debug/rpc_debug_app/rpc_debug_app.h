#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>

#include <gui/modules/widget.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_box.h>
#include <gui/modules/text_input.h>
#include <gui/modules/byte_input.h>

#include <rpc/rpc_app.h>
#include <notification/notification_messages.h>

#include "scenes/rpc_debug_app_scene.h"

#define DATA_STORE_SIZE 64U
#define TEXT_STORE_SIZE 64U

typedef struct {
    Gui* gui;
    RpcAppSystem* rpc;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    NotificationApp* notifications;

    Widget* widget;
    Submenu* submenu;
    TextBox* text_box;
    TextInput* text_input;
    ByteInput* byte_input;

    char text_store[TEXT_STORE_SIZE];
    uint8_t data_store[DATA_STORE_SIZE];
} RpcDebugApp;

typedef enum {
    RpcDebugAppViewWidget,
    RpcDebugAppViewSubmenu,
    RpcDebugAppViewTextBox,
    RpcDebugAppViewTextInput,
    RpcDebugAppViewByteInput,
} RpcDebugAppView;

typedef enum {
    // Reserve first 100 events for button types and indexes, starting from 0
    RpcDebugAppCustomEventInputErrorCode = 100,
    RpcDebugAppCustomEventInputErrorText,
    RpcDebugAppCustomEventInputDataExchange,
    RpcDebugAppCustomEventRpcDataExchange,
} RpcDebugAppCustomEvent;
