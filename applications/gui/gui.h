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
typedef struct Gui Gui;

typedef struct {
    void (*add_widget)(Gui* gui, Widget* widget, WidgetLayer layer);
    Gui* gui;
} GuiApi;
