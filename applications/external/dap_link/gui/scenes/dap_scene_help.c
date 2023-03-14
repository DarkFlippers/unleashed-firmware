#include "../dap_gui_i.h"

void dap_scene_help_on_enter(void* context) {
    DapGuiApp* app = context;
    DapConfig* config = dap_app_get_config(app->dap_app);
    FuriString* string = furi_string_alloc();

    furi_string_cat(string, "CMSIS DAP/DAP Link v2\r\n");
    furi_string_cat_printf(string, "Serial: %s\r\n", dap_app_get_serial(app->dap_app));
    furi_string_cat(
        string,
        "Pinout:\r\n"
        "\e#SWD:\r\n");

    switch(config->swd_pins) {
    case DapSwdPinsPA7PA6:
        furi_string_cat(
            string,
            "    SWC: 2 [A7]\r\n"
            "    SWD: 3 [A6]\r\n");
        break;
    case DapSwdPinsPA14PA13:
        furi_string_cat(
            string,
            "    SWC: 10 [SWC]\r\n"
            "    SWD: 12 [SIO]\r\n");
        break;
    default:
        break;
    }

    furi_string_cat(string, "\e#JTAG:\r\n");
    switch(config->swd_pins) {
    case DapSwdPinsPA7PA6:
        furi_string_cat(
            string,
            "    TCK: 2 [A7]\r\n"
            "    TMS: 3 [A6]\r\n"
            "    RST: 4 [A4]\r\n"
            "    TDO: 5 [B3]\r\n"
            "    TDI: 6 [B2]\r\n");
        break;
    case DapSwdPinsPA14PA13:
        furi_string_cat(
            string,
            "    RST: 4 [A4]\r\n"
            "    TDO: 5 [B3]\r\n"
            "    TDI: 6 [B2]\r\n"
            "    TCK: 10 [SWC]\r\n"
            "    TMS: 12 [SIO]\r\n");
        break;
    default:
        break;
    }

    furi_string_cat(string, "\e#UART:\r\n");
    switch(config->uart_pins) {
    case DapUartTypeUSART1:
        if(config->uart_swap == DapUartTXRXNormal) {
            furi_string_cat(
                string,
                "    TX: 13 [TX]\r\n"
                "    RX: 14 [RX]\r\n");
        } else {
            furi_string_cat(
                string,
                "    RX: 13 [TX]\r\n"
                "    TX: 14 [RX]\r\n");
        }
        break;
    case DapUartTypeLPUART1:
        if(config->uart_swap == DapUartTXRXNormal) {
            furi_string_cat(
                string,
                "    TX: 15 [C1]\r\n"
                "    RX: 16 [C0]\r\n");
        } else {
            furi_string_cat(
                string,
                "    RX: 15 [C1]\r\n"
                "    TX: 16 [C0]\r\n");
        }
        break;
    default:
        break;
    }

    widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, furi_string_get_cstr(string));
    furi_string_free(string);
    view_dispatcher_switch_to_view(app->view_dispatcher, DapGuiAppViewWidget);
}

bool dap_scene_help_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void dap_scene_help_on_exit(void* context) {
    DapGuiApp* app = context;
    widget_reset(app->widget);
}