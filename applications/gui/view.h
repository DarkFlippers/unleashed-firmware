#pragma once

#include <input/input.h>
#include "canvas.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Hides drawing widget */
#define VIEW_NONE 0xFFFFFFFF
/* Ignore navigation event */
#define VIEW_IGNORE 0xFFFFFFFE
/* Deatch from gui, deallocate Views and ViewDispatcher
 * BE SUPER CAREFUL, deallocation happens automatically on GUI thread
 * You ARE NOT owning ViewDispatcher and Views instances
 */
#define VIEW_DESTROY 0xFFFFFFFA

/* View Draw callback
 * @param canvas, pointer to canvas
 * @param view_model, pointer to context
 * @warning called from GUI thread
 */
typedef void (*ViewDrawCallback)(Canvas* canvas, void* model);

/* View Input callback
 * @param event, pointer to input event data
 * @param context, pointer to context
 * @return true if event handled, false if event ignored
 * @warning called from GUI thread
 */
typedef bool (*ViewInputCallback)(InputEvent* event, void* context);

/* View navigation callback
 * @param context, pointer to context
 * @return next view id
 * @warning called from GUI thread
 */
typedef uint32_t (*ViewNavigationCallback)(void* context);

/* View model types */
typedef enum {
    /* Model is not allocated */
    ViewModelTypeNone,
    /* Model consist of atomic types and/or partial update is not critical for rendering.
     * Lock free.
     */
    ViewModelTypeLockFree,
    /* Model access is guarded with mutex.
     * Locking gui thread.
     */
    ViewModelTypeLocking,
} ViewModelType;

typedef struct View View;

/* Allocate and init View
 * @return pointer to View
 */
View* view_alloc();

/* Free View
 * @param pointer to View
 */
void view_free(View* view);

/* Set View Draw callback
 * @param view, pointer to View
 * @param callback, draw callback
 */
void view_set_draw_callback(View* view, ViewDrawCallback callback);

/* Set View Draw callback
 * @param view, pointer to View
 * @param callback, input callback
 */
void view_set_input_callback(View* view, ViewInputCallback callback);

/* Set Navigation Previous callback
 * @param view, pointer to View
 * @param callback, input callback
 */
void view_set_previous_callback(View* view, ViewNavigationCallback callback);

/* Set Navigation Next callback
 * @param view, pointer to View
 * @param callback, input callback
 */
void view_set_next_callback(View* view, ViewNavigationCallback callback);

/* Set View Draw callback
 * @param view, pointer to View
 * @param context, context for callbacks
 */
void view_set_context(View* view, void* context);

/* Allocate view model.
 * @param view, pointer to View
 * @param type, View Model Type
 * @param size, size
 */
void view_allocate_model(View* view, ViewModelType type, size_t size);

/* Free view model data memory.
 * @param view, pointer to View
 */
void view_free_model(View* view);

/* Get view model data
 * @param view, pointer to View
 * @return pointer to model data
 * @warning Don't forget to commit model changes
 */
void* view_get_model(View* view);

/* Commit view model
 * @param view, pointer to View
 */
void view_commit_model(View* view);

/* 
 * With clause for view model
 * @param view, View instance pointer
 * @param function_body a (){} lambda declaration,
 * executed within you parent function context.
 */
#define with_view_model(view, function_body)        \
    {                                               \
        void* p = view_get_model(view);             \
        ({ void __fn__ function_body __fn__; })(p); \
        view_commit_model(view);                    \
    }

#ifdef __cplusplus
}
#endif
