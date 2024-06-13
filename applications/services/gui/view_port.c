#include "view_port_i.h"

#include <furi.h>
#include <furi_hal.h>

#include "gui.h"
#include "gui_i.h"

#define TAG "ViewPort"

_Static_assert(ViewPortOrientationMAX == 4, "Incorrect ViewPortOrientation count");
_Static_assert(
    (ViewPortOrientationHorizontal == 0 && ViewPortOrientationHorizontalFlip == 1 &&
     ViewPortOrientationVertical == 2 && ViewPortOrientationVerticalFlip == 3),
    "Incorrect ViewPortOrientation order");
_Static_assert(InputKeyMAX == 6, "Incorrect InputKey count");
_Static_assert(
    (InputKeyUp == 0 && InputKeyDown == 1 && InputKeyRight == 2 && InputKeyLeft == 3 &&
     InputKeyOk == 4 && InputKeyBack == 5),
    "Incorrect InputKey order");

/** InputKey directional keys mappings for different screen orientations
* 
*/
static const InputKey view_port_input_mapping[ViewPortOrientationMAX][InputKeyMAX] = {
    {InputKeyUp,
     InputKeyDown,
     InputKeyRight,
     InputKeyLeft,
     InputKeyOk,
     InputKeyBack}, //ViewPortOrientationHorizontal
    {InputKeyDown,
     InputKeyUp,
     InputKeyLeft,
     InputKeyRight,
     InputKeyOk,
     InputKeyBack}, //ViewPortOrientationHorizontalFlip
    {InputKeyRight,
     InputKeyLeft,
     InputKeyDown,
     InputKeyUp,
     InputKeyOk,
     InputKeyBack}, //ViewPortOrientationVertical
    {InputKeyLeft,
     InputKeyRight,
     InputKeyUp,
     InputKeyDown,
     InputKeyOk,
     InputKeyBack}, //ViewPortOrientationVerticalFlip
};

static const InputKey view_port_left_hand_input_mapping[InputKeyMAX] =
    {InputKeyDown, InputKeyUp, InputKeyLeft, InputKeyRight, InputKeyOk, InputKeyBack};

static const CanvasOrientation view_port_orientation_mapping[ViewPortOrientationMAX] = {
    [ViewPortOrientationHorizontal] = CanvasOrientationHorizontal,
    [ViewPortOrientationHorizontalFlip] = CanvasOrientationHorizontalFlip,
    [ViewPortOrientationVertical] = CanvasOrientationVertical,
    [ViewPortOrientationVerticalFlip] = CanvasOrientationVerticalFlip,
};

// Remaps directional pad buttons on Flipper based on ViewPort orientation
static void view_port_map_input(InputEvent* event, ViewPortOrientation orientation) {
    furi_check(orientation < ViewPortOrientationMAX && event->key < InputKeyMAX);

    if(event->sequence_source != INPUT_SEQUENCE_SOURCE_HARDWARE) {
        return;
    }

    if(orientation == ViewPortOrientationHorizontal ||
       orientation == ViewPortOrientationHorizontalFlip) {
        if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagHandOrient)) {
            event->key = view_port_left_hand_input_mapping[event->key];
        }
    }
    event->key = view_port_input_mapping[orientation][event->key];
}

static void view_port_setup_canvas_orientation(const ViewPort* view_port, Canvas* canvas) {
    CanvasOrientation orientation = view_port_orientation_mapping[view_port->orientation];

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagHandOrient)) {
        if(orientation == CanvasOrientationHorizontal) {
            orientation = CanvasOrientationHorizontalFlip;
        } else if(orientation == CanvasOrientationHorizontalFlip) {
            orientation = CanvasOrientationHorizontal;
        }
    }

    canvas_set_orientation(canvas, orientation);
}

ViewPort* view_port_alloc(void) {
    ViewPort* view_port = malloc(sizeof(ViewPort));
    view_port->orientation = ViewPortOrientationHorizontal;
    view_port->is_enabled = true;
    view_port->mutex = furi_mutex_alloc(FuriMutexTypeRecursive);
    return view_port;
}

void view_port_free(ViewPort* view_port) {
    furi_check(view_port);
    furi_check(furi_mutex_acquire(view_port->mutex, FuriWaitForever) == FuriStatusOk);
    furi_check(view_port->gui == NULL);
    furi_check(furi_mutex_release(view_port->mutex) == FuriStatusOk);
    furi_mutex_free(view_port->mutex);
    free(view_port);
}

void view_port_set_width(ViewPort* view_port, uint8_t width) {
    furi_check(view_port);
    furi_check(furi_mutex_acquire(view_port->mutex, FuriWaitForever) == FuriStatusOk);
    view_port->width = width;
    furi_check(furi_mutex_release(view_port->mutex) == FuriStatusOk);
}

uint8_t view_port_get_width(const ViewPort* view_port) {
    furi_check(view_port);
    furi_check(furi_mutex_acquire(view_port->mutex, FuriWaitForever) == FuriStatusOk);
    uint8_t width = view_port->width;
    furi_check(furi_mutex_release(view_port->mutex) == FuriStatusOk);
    return width;
}

void view_port_set_height(ViewPort* view_port, uint8_t height) {
    furi_check(view_port);
    furi_check(furi_mutex_acquire(view_port->mutex, FuriWaitForever) == FuriStatusOk);
    view_port->height = height;
    furi_check(furi_mutex_release(view_port->mutex) == FuriStatusOk);
}

uint8_t view_port_get_height(const ViewPort* view_port) {
    furi_check(view_port);
    furi_check(furi_mutex_acquire(view_port->mutex, FuriWaitForever) == FuriStatusOk);
    uint8_t height = view_port->height;
    furi_check(furi_mutex_release(view_port->mutex) == FuriStatusOk);
    return height;
}

void view_port_enabled_set(ViewPort* view_port, bool enabled) {
    furi_check(view_port);

    // We are not going to lockup system, but will notify you instead
    // Make sure that you don't call viewport methods inside of another mutex, especially one that is used in draw call
    if(furi_mutex_acquire(view_port->mutex, 2) != FuriStatusOk) {
        FURI_LOG_W(TAG, "ViewPort lockup: see %s:%d", __FILE__, __LINE__ - 3);
    }
    if(view_port->is_enabled != enabled) {
        view_port->is_enabled = enabled;
        if(view_port->gui) gui_update(view_port->gui);
    }
    furi_mutex_release(view_port->mutex);
}

bool view_port_is_enabled(const ViewPort* view_port) {
    furi_check(view_port);
    furi_check(furi_mutex_acquire(view_port->mutex, FuriWaitForever) == FuriStatusOk);
    bool is_enabled = view_port->is_enabled;
    furi_check(furi_mutex_release(view_port->mutex) == FuriStatusOk);
    return is_enabled;
}

void view_port_draw_callback_set(ViewPort* view_port, ViewPortDrawCallback callback, void* context) {
    furi_check(view_port);
    furi_check(furi_mutex_acquire(view_port->mutex, FuriWaitForever) == FuriStatusOk);
    view_port->draw_callback = callback;
    view_port->draw_callback_context = context;
    furi_check(furi_mutex_release(view_port->mutex) == FuriStatusOk);
}

void view_port_input_callback_set(
    ViewPort* view_port,
    ViewPortInputCallback callback,
    void* context) {
    furi_check(view_port);
    furi_check(furi_mutex_acquire(view_port->mutex, FuriWaitForever) == FuriStatusOk);
    view_port->input_callback = callback;
    view_port->input_callback_context = context;
    furi_check(furi_mutex_release(view_port->mutex) == FuriStatusOk);
}

void view_port_update(ViewPort* view_port) {
    furi_check(view_port);

    // We are not going to lockup system, but will notify you instead
    // Make sure that you don't call viewport methods inside of another mutex, especially one that is used in draw call
    if(furi_mutex_acquire(view_port->mutex, 2) != FuriStatusOk) {
        FURI_LOG_W(TAG, "ViewPort lockup: see %s:%d", __FILE__, __LINE__ - 3);
    }

    if(view_port->gui && view_port->is_enabled) gui_update(view_port->gui);
    furi_mutex_release(view_port->mutex);
}

void view_port_gui_set(ViewPort* view_port, Gui* gui) {
    furi_check(view_port);
    furi_check(furi_mutex_acquire(view_port->mutex, FuriWaitForever) == FuriStatusOk);
    view_port->gui = gui;
    furi_check(furi_mutex_release(view_port->mutex) == FuriStatusOk);
}

void view_port_draw(ViewPort* view_port, Canvas* canvas) {
    furi_check(view_port);
    furi_check(canvas);

    // We are not going to lockup system, but will notify you instead
    // Make sure that you don't call viewport methods inside of another mutex, especially one that is used in draw call
    if(furi_mutex_acquire(view_port->mutex, 2) != FuriStatusOk) {
        FURI_LOG_W(TAG, "ViewPort lockup: see %s:%d", __FILE__, __LINE__ - 3);
    }

    furi_check(view_port->gui);

    if(view_port->draw_callback) {
        view_port_setup_canvas_orientation(view_port, canvas);
        view_port->draw_callback(canvas, view_port->draw_callback_context);
    }

    furi_mutex_release(view_port->mutex);
}

void view_port_input(ViewPort* view_port, InputEvent* event) {
    furi_check(view_port);
    furi_check(event);
    furi_check(furi_mutex_acquire(view_port->mutex, FuriWaitForever) == FuriStatusOk);
    furi_check(view_port->gui);

    if(view_port->input_callback) {
        ViewPortOrientation orientation = view_port_get_orientation(view_port);
        view_port_map_input(event, orientation);
        view_port->input_callback(event, view_port->input_callback_context);
    }
    furi_check(furi_mutex_release(view_port->mutex) == FuriStatusOk);
}

void view_port_set_orientation(ViewPort* view_port, ViewPortOrientation orientation) {
    furi_check(view_port);
    furi_check(furi_mutex_acquire(view_port->mutex, FuriWaitForever) == FuriStatusOk);
    view_port->orientation = orientation;
    furi_check(furi_mutex_release(view_port->mutex) == FuriStatusOk);
}

ViewPortOrientation view_port_get_orientation(const ViewPort* view_port) {
    furi_check(view_port);
    // We are not going to lockup system, but will notify you instead
    // Make sure that you don't call viewport methods inside of another mutex, especially one that is used in draw call
    if(furi_mutex_acquire(view_port->mutex, 2) != FuriStatusOk) {
        FURI_LOG_W(TAG, "ViewPort lockup: see %s:%d", __FILE__, __LINE__ - 3);
    }
    ViewPortOrientation orientation = view_port->orientation;
    furi_mutex_release(view_port->mutex);
    return orientation;
}
