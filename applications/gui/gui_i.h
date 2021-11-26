/**
 * @file gui_i.h
 * GUI: main API internals
 */

#pragma once

#include "gui.h"

#include <furi.h>
#include <m-array.h>
#include <stdio.h>

#include <cli/cli.h>
#include "canvas.h"
#include "canvas_i.h"
#include "view_port.h"
#include "view_port_i.h"

#define GUI_DISPLAY_WIDTH 128
#define GUI_DISPLAY_HEIGHT 64

#define GUI_STATUS_BAR_X 0
#define GUI_STATUS_BAR_Y 0
#define GUI_STATUS_BAR_WIDTH GUI_DISPLAY_WIDTH
/* 0-1 pixels for upper thin frame
 * 2-9 pixels for icons (battery, sd card, etc)
 * 10-12 pixels for lower bold line */
#define GUI_STATUS_BAR_HEIGHT 13
/* icon itself area (battery, sd card, etc) excluding frame.
 * painted 2 pixels below GUI_STATUS_BAR_X.
 */
#define GUI_STATUS_BAR_WORKAREA_HEIGHT 8

#define GUI_WINDOW_X 0
#define GUI_WINDOW_Y GUI_STATUS_BAR_HEIGHT
#define GUI_WINDOW_WIDTH GUI_DISPLAY_WIDTH
#define GUI_WINDOW_HEIGHT (GUI_DISPLAY_HEIGHT - GUI_WINDOW_Y)

#define GUI_THREAD_FLAG_DRAW (1 << 0)
#define GUI_THREAD_FLAG_INPUT (1 << 1)
#define GUI_THREAD_FLAG_ALL (GUI_THREAD_FLAG_DRAW | GUI_THREAD_FLAG_INPUT)

ARRAY_DEF(ViewPortArray, ViewPort*, M_PTR_OPLIST);

/** Gui structure */
struct Gui {
    // Thread and lock
    osThreadId_t thread;
    osMutexId_t mutex;

    // Layers and Canvas
    ViewPortArray_t layers[GuiLayerMAX];
    Canvas* canvas;
    GuiCanvasCommitCallback canvas_callback;
    void* canvas_callback_context;

    // Input
    osMessageQueueId_t input_queue;
    FuriPubSub* input_events;
    uint8_t ongoing_input;
    ViewPort* ongoing_input_view_port;

    // Cli
    Cli* cli;
};

ViewPort* gui_view_port_find_enabled(ViewPortArray_t array);

/** Update GUI, request redraw
 *
 * @param      gui   Gui instance
 */
void gui_update(Gui* gui);

void gui_input_events_callback(const void* value, void* ctx);

void gui_lock(Gui* gui);

void gui_unlock(Gui* gui);

void gui_cli_screen_stream_callback(uint8_t* data, size_t size, void* context);

void gui_cli_screen_stream(Cli* cli, string_t args, void* context);
