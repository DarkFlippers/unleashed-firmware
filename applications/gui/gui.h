#pragma once

#include "widget.h"
#include "canvas.h"

#define GUI_DISPLAY_WIDTH 128
#define GUI_DISPLAY_HEIGHT 64

#define GUI_STATUS_BAR_X 0
#define GUI_STATUS_BAR_Y 0
#define GUI_STATUS_BAR_WIDTH GUI_DISPLAY_WIDTH
#define GUI_STATUS_BAR_HEIGHT 8

#define GUI_MAIN_X 0
#define GUI_MAIN_Y 9
#define GUI_MAIN_WIDTH GUI_DISPLAY_WIDTH
#define GUI_MAIN_HEIGHT (GUI_DISPLAY_HEIGHT - GUI_MAIN_Y)

typedef enum {
    GuiLayerNone, /* Special layer for internal use only */

    GuiLayerStatusBarLeft, /* Status bar left-side widget layer, auto-layout */
    GuiLayerStatusBarRight, /* Status bar right-side widget layer, auto-layout */
    GuiLayerMain, /* Main widget layer, status bar is shown */
    GuiLayerFullscreen, /* Fullscreen widget layer */

    GuiLayerMAX /* Don't use or move, special value */
} GuiLayer;

typedef struct GuiApi GuiApi;
struct GuiApi {
    void (*add_widget)(GuiApi* gui_api, Widget* widget, GuiLayer layer);
    void (*remove_widget)(GuiApi* gui_api, Widget* widget);
};
