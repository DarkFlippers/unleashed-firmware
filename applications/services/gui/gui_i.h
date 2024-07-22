/**
 * @file gui_i.h
 * GUI: main API internals
 */

#pragma once

#include "gui.h"

#include <furi.h>
#include <furi_hal_rtc.h>
#include <m-array.h>
#include <stdio.h>

#include "canvas.h"
#include "canvas_i.h"
#include "view_port.h"
#include "view_port_i.h"

#define GUI_DISPLAY_WIDTH  128
#define GUI_DISPLAY_HEIGHT 64

#define GUI_STATUS_BAR_X               0
#define GUI_STATUS_BAR_Y               0
#define GUI_STATUS_BAR_WIDTH           GUI_DISPLAY_WIDTH
/* 0-1 pixels for upper thin frame
 * 2-9 pixels for icons (battery, sd card, etc)
 * 10-12 pixels for lower bold line */
#define GUI_STATUS_BAR_HEIGHT          13
/* icon itself area (battery, sd card, etc) excluding frame.
 * painted 2 pixels below GUI_STATUS_BAR_X.
 */
#define GUI_STATUS_BAR_WORKAREA_HEIGHT 8

#define GUI_WINDOW_X      0
#define GUI_WINDOW_Y      GUI_STATUS_BAR_HEIGHT
#define GUI_WINDOW_WIDTH  GUI_DISPLAY_WIDTH
#define GUI_WINDOW_HEIGHT (GUI_DISPLAY_HEIGHT - GUI_WINDOW_Y)

#define GUI_THREAD_FLAG_DRAW  (1 << 0)
#define GUI_THREAD_FLAG_INPUT (1 << 1)
#define GUI_THREAD_FLAG_ALL   (GUI_THREAD_FLAG_DRAW | GUI_THREAD_FLAG_INPUT)

ARRAY_DEF(ViewPortArray, ViewPort*, M_PTR_OPLIST);

/** Gui structure */
struct Gui {
    // Thread and lock
    FuriThreadId thread_id;
    FuriMutex* mutex;

    // Layers and Canvas
    bool lockdown;
    bool direct_draw;
    ViewPortArray_t layers[GuiLayerMAX];
    Canvas* canvas;

    // Input
    FuriMessageQueue* input_queue;
    FuriPubSub* input_events;
    uint8_t ongoing_input;
    ViewPort* ongoing_input_view_port;
};

/** Find enabled ViewPort in ViewPortArray
 *
 * @param[in]  array  The ViewPortArray instance
 *
 * @return     ViewPort instance or NULL
 */
ViewPort* gui_view_port_find_enabled(ViewPortArray_t array);

/** Update GUI, request redraw
 *
 * @param      gui   Gui instance
 */
void gui_update(Gui* gui);

/** Input event callback
 * 
 * Used to receive input from input service or to inject new input events
 *
 * @param[in]  value  The value pointer (InputEvent*)
 * @param      ctx    The context (Gui instance)
 */
void gui_input_events_callback(const void* value, void* ctx);

/** Get count of view ports in layer
 *
 * @param      gui        The Gui instance
 * @param[in]  layer      GuiLayer that we want to get count of view ports
 */
size_t gui_active_view_port_count(Gui* gui, GuiLayer layer);

/** Lock GUI
 *
 * @param      gui   The Gui instance
 */
void gui_lock(Gui* gui);

/** Unlock GUI
 *
 * @param      gui   The Gui instance
 */
void gui_unlock(Gui* gui);
