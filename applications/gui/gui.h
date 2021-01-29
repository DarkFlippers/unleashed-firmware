#pragma once

#include "view_port.h"
#include "canvas.h"

#ifdef __cplusplus
extern "C" {
#endif

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

    GuiLayerStatusBarLeft, /* Status bar left-side view_port layer, auto-layout */
    GuiLayerStatusBarRight, /* Status bar right-side view_port layer, auto-layout */
    GuiLayerMain, /* Main view_port layer, status bar is shown */
    GuiLayerFullscreen, /* Fullscreen view_port layer */

    GuiLayerMAX /* Don't use or move, special value */
} GuiLayer;

typedef struct Gui Gui;

/*
 * Add view_port to view_port tree
 * @remarks thread safe
 */
void gui_add_view_port(Gui* gui, ViewPort* view_port, GuiLayer layer);

/*
 * Remove view_port from rendering tree
 * @remarks thread safe
 */
void gui_remove_view_port(Gui* gui, ViewPort* view_port);

#ifdef __cplusplus
}
#endif
