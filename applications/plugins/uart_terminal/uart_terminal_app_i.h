#pragma once

#include "uart_terminal_app.h"
#include "scenes/uart_terminal_scene.h"
#include "uart_terminal_custom_event.h"
#include "uart_terminal_uart.h"

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/text_box.h>
#include <gui/modules/variable_item_list.h>
#include "uart_text_input.h"

#define NUM_MENU_ITEMS (4)

#define UART_TERMINAL_TEXT_BOX_STORE_SIZE (4096)
#define UART_TERMINAL_TEXT_INPUT_STORE_SIZE (512)
#define UART_CH (FuriHalUartIdUSART1)

struct UART_TerminalApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;

    char text_input_store[UART_TERMINAL_TEXT_INPUT_STORE_SIZE + 1];
    FuriString* text_box_store;
    size_t text_box_store_strlen;
    TextBox* text_box;
    UART_TextInput* text_input;

    VariableItemList* var_item_list;

    UART_TerminalUart* uart;
    int selected_menu_index;
    int selected_option_index[NUM_MENU_ITEMS];
    const char* selected_tx_string;
    bool is_command;
    bool is_custom_tx_string;
    bool focus_console_start;
    bool show_stopscan_tip;
    int BAUDRATE;
};

typedef enum {
    UART_TerminalAppViewVarItemList,
    UART_TerminalAppViewConsoleOutput,
    UART_TerminalAppViewTextInput,
} UART_TerminalAppView;
