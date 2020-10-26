#pragma once

#include "widget.h"
#include "canvas.h"

typedef enum {
    GuiLayerNone, /* Special layer for internal use only */

    GuiLayerStatusBar, /* Status bar widget layer */
    GuiLayerMain, /* Main widget layer, status bar is shown */
    GuiLayerFullscreen, /* Fullscreen widget layer */

    GuiLayerMAX /* Don't use or move, special value */
} GuiLayer;

typedef struct GuiApi GuiApi;
struct GuiApi {
    void (*add_widget)(GuiApi* gui_api, Widget* widget, GuiLayer layer);
    void (*remove_widget)(GuiApi* gui_api, Widget* widget);
};
