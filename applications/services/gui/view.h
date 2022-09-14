/**
 * @file view.h
 * GUI: View API
 */

#pragma once

#include <input/input.h>

#include "icon_animation.h"
#include "canvas.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Hides drawing view_port */
#define VIEW_NONE 0xFFFFFFFF

/** Ignore navigation event */
#define VIEW_IGNORE 0xFFFFFFFE

typedef enum {
    ViewOrientationHorizontal,
    ViewOrientationVertical,
} ViewOrientation;

/** View, anonymous type */
typedef struct View View;

/** View Draw callback
 * @param      canvas,      pointer to canvas
 * @param      view_model,  pointer to context
 * @warning    called from GUI thread
 */
typedef void (*ViewDrawCallback)(Canvas* canvas, void* model);

/** View Input callback
 * @param      event,    pointer to input event data
 * @param      context,  pointer to context
 * @return     true if event handled, false if event ignored
 * @warning    called from GUI thread
 */
typedef bool (*ViewInputCallback)(InputEvent* event, void* context);

/** View Custom callback
 * @param      event,    number of custom event
 * @param      context,  pointer to context
 * @return     true if event handled, false if event ignored
 */
typedef bool (*ViewCustomCallback)(uint32_t event, void* context);

/** View navigation callback
 * @param      context,  pointer to context
 * @return     next view id
 * @warning    called from GUI thread
 */
typedef uint32_t (*ViewNavigationCallback)(void* context);

/** View callback
 * @param      context,  pointer to context
 * @warning    called from GUI thread
 */
typedef void (*ViewCallback)(void* context);

/** View Update Callback Called upon model change, need to be propagated to GUI
 * throw ViewPort update
 * @param      view,     pointer to view
 * @param      context,  pointer to context
 * @warning    called from GUI thread
 */
typedef void (*ViewUpdateCallback)(View* view, void* context);

/** View model types */
typedef enum {
    /** Model is not allocated */
    ViewModelTypeNone,
    /** Model consist of atomic types and/or partial update is not critical for rendering.
     * Lock free.
     */
    ViewModelTypeLockFree,
    /** Model access is guarded with mutex.
     * Locking gui thread.
     */
    ViewModelTypeLocking,
} ViewModelType;

/** Allocate and init View
 * @return View instance
 */
View* view_alloc();

/** Free View
 *
 * @param      view  instance
 */
void view_free(View* view);

/** Tie IconAnimation with View
 *
 * @param      view            View instance
 * @param      icon_animation  IconAnimation instance
 */
void view_tie_icon_animation(View* view, IconAnimation* icon_animation);

/** Set View Draw callback
 *
 * @param      view      View instance
 * @param      callback  draw callback
 */
void view_set_draw_callback(View* view, ViewDrawCallback callback);

/** Set View Input callback
 *
 * @param      view      View instance
 * @param      callback  input callback
 */
void view_set_input_callback(View* view, ViewInputCallback callback);

/** Set View Custom callback
 *
 * @param      view      View instance
 * @param      callback  input callback
 */
void view_set_custom_callback(View* view, ViewCustomCallback callback);

/** Set Navigation Previous callback
 *
 * @param      view      View instance
 * @param      callback  input callback
 */
void view_set_previous_callback(View* view, ViewNavigationCallback callback);

/** Set Enter callback
 *
 * @param      view      View instance
 * @param      callback  callback
 */
void view_set_enter_callback(View* view, ViewCallback callback);

/** Set Exit callback
 *
 * @param      view      View instance
 * @param      callback  callback
 */
void view_set_exit_callback(View* view, ViewCallback callback);

/** Set Update callback
 *
 * @param      view      View instance
 * @param      callback  callback
 */
void view_set_update_callback(View* view, ViewUpdateCallback callback);

/** Set View Draw callback
 *
 * @param      view     View instance
 * @param      context  context for callbacks
 */
void view_set_update_callback_context(View* view, void* context);

/** Set View Draw callback
 *
 * @param      view     View instance
 * @param      context  context for callbacks
 */
void view_set_context(View* view, void* context);

/** Set View Orientation
 *
 * @param      view         View instance
 * @param      orientation  either vertical or horizontal
 */
void view_set_orientation(View* view, ViewOrientation orientation);

/** Allocate view model.
 *
 * @param      view  View instance
 * @param      type  View Model Type
 * @param      size  size
 */
void view_allocate_model(View* view, ViewModelType type, size_t size);

/** Free view model data memory.
 *
 * @param      view  View instance
 */
void view_free_model(View* view);

/** Get view model data
 *
 * @param      view  View instance
 *
 * @return     pointer to model data
 * @warning    Don't forget to commit model changes
 */
void* view_get_model(View* view);

/** Commit view model
 *
 * @param      view    View instance
 * @param      update  true if you want to emit view update, false otherwise
 */
void view_commit_model(View* view, bool update);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#define with_view_model_cpp(view, type, var, function_body) \
    {                                                       \
        type* p = static_cast<type*>(view_get_model(view)); \
        bool update = [&](type * var) function_body(p);     \
        view_commit_model(view, update);                    \
    }
#else
/** With clause for view model
 *
 * @param      view           View instance pointer
 * @param      function_body  a (){} lambda declaration, executed within you
 *                            parent function context
 *
 * @return     true if you want to emit view update, false otherwise
 */
#define with_view_model(view, function_body)                      \
    {                                                             \
        void* p = view_get_model(view);                           \
        bool update = ({ bool __fn__ function_body __fn__; })(p); \
        view_commit_model(view, update);                          \
    }
#endif
