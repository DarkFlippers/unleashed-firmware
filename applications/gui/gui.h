#pragma once

#include "widget.h"

typedef enum {
    WidgetLayerStatusBar,
    WidgetLayerMain,
    WidgetLayerFullscreen,
    WidgetLayerDialog
} WidgetLayer;

typedef struct Widget Widget;
typedef struct GUI GUI;

typedef struct {
    void (*add_widget)(GUI* gui, Widget* widget, WidgetLayer layer);
    GUI* gui;
} GuiApi;
