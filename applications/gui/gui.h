#pragma once

#include "widget.h"
#include "canvas.h"

typedef enum {
    WidgetLayerStatusBar,
    WidgetLayerMain,
    WidgetLayerFullscreen,
    WidgetLayerDialog
} WidgetLayer;

typedef struct Widget Widget;

typedef struct GuiApi GuiApi;
struct GuiApi {
    void (*add_widget)(GuiApi* gui_api, Widget* widget, WidgetLayer layer);
};
