/**
 * @file gui.h
 * GUI: main API
 */

#pragma once

#include "view_port.h"
#include "canvas.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Gui layers */
typedef enum {
    GuiLayerDesktop, /**< Desktop layer for internal use. Like fullscreen but with status bar */

    GuiLayerWindow, /**< Window layer, status bar is shown */

    GuiLayerStatusBarLeft, /**< Status bar left-side layer, auto-layout */
    GuiLayerStatusBarRight, /**< Status bar right-side layer, auto-layout */

    GuiLayerFullscreen, /**< Fullscreen layer, no status bar */

    GuiLayerMAX /**< Don't use or move, special value */
} GuiLayer;

/** Gui Canvas Commit Callback */
typedef void (*GuiCanvasCommitCallback)(uint8_t* data, size_t size, void* context);

typedef struct Gui Gui;

/** Add view_port to view_port tree
 *
 * @remark     thread safe
 *
 * @param      gui        Gui instance
 * @param      view_port  ViewPort instance
 * @param[in]  layer      GuiLayer where to place view_port
 */
void gui_add_view_port(Gui* gui, ViewPort* view_port, GuiLayer layer);

/** Remove view_port from rendering tree
 *
 * @remark     thread safe
 *
 * @param      gui        Gui instance
 * @param      view_port  ViewPort instance
 */
void gui_remove_view_port(Gui* gui, ViewPort* view_port);

/** Send ViewPort to the front
 *
 * Places selected ViewPort to the top of the drawing stack
 *
 * @param      gui        Gui instance
 * @param      view_port  ViewPort instance
 */
void gui_view_port_send_to_front(Gui* gui, ViewPort* view_port);

/** Send ViewPort to the back
 *
 * Places selected ViewPort to the bottom of the drawing stack
 *
 * @param      gui        Gui instance
 * @param      view_port  ViewPort instance
 */
void gui_view_port_send_to_back(Gui* gui, ViewPort* view_port);

/** Set gui canvas commit callback
 *
 * This callback will be called upon Canvas commit Callback dispatched from GUI
 * thread and is time critical
 *
 * @param      gui       Gui instance
 * @param      callback  GuiCanvasCommitCallback
 * @param      context   GuiCanvasCommitCallback context
 */
void gui_set_framebuffer_callback(Gui* gui, GuiCanvasCommitCallback callback, void* context);

#ifdef __cplusplus
}
#endif
