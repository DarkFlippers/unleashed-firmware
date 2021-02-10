#pragma once

#include "view_port.h"
#include "canvas.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GuiLayerNone, /* Special layer for internal use only */

    GuiLayerStatusBarLeft, /* Status bar left-side layer, auto-layout */
    GuiLayerStatusBarRight, /* Status bar right-side layer, auto-layout */
    GuiLayerMain, /* Main layer, status bar is shown */
    GuiLayerFullscreen, /* Fullscreen layer */

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

/* Send ViewPort to the front
 * Places selected ViewPort to the top of the drawing stack
 * @param gui, Gui instance
 * @param view_port, ViewPort instance
 */
void gui_send_view_port_front(Gui* gui, ViewPort* view_port);

/* Send ViewPort to the back
 * Places selected ViewPort to the bottom of the drawing stack
 * @param gui, Gui instance
 * @param view_port, ViewPort instance
 */
void gui_send_view_port_back(Gui* gui, ViewPort* view_port);

#ifdef __cplusplus
}
#endif
