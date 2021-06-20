#pragma once

#include "view.h"
#include "gui.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Prototype for custom event callback
 */
typedef void (*CustomEventCallback)(uint32_t custom_event, void* context);

/** ViewDispatcher view_port placement
 */
typedef enum {
    ViewDispatcherTypeNone, /**< Special layer for internal use only */
    ViewDispatcherTypeWindow, /**< Main view_port layer, status bar is shown */
    ViewDispatcherTypeFullscreen /**< Fullscreen view_port layer */
} ViewDispatcherType;

typedef struct ViewDispatcher ViewDispatcher;

/** Allocate ViewDispatcher instance
 * @return pointer to ViewDispatcher instance
 */
ViewDispatcher* view_dispatcher_alloc();

/** Free ViewDispatcher instance
 * @param view_dispatcher pointer to ViewDispatcher
 */
void view_dispatcher_free(ViewDispatcher* view_dispatcher);

/** Enable queue support
 * If queue enabled all input and custom events will be dispatched throw internal queue
 * @param view_dispatcher ViewDispatcher instance
 */
void view_dispatcher_enable_queue(ViewDispatcher* view_dispatcher);

/** Set custom event callback
 * Custom callback is called when custom event in internal queue received
 */
void view_dispatcher_set_custom_callback(
    ViewDispatcher* view_dispatcher,
    CustomEventCallback callback,
    void* context);

/** Send custom event
 */
void view_dispatcher_send_custom_event(ViewDispatcher* view_dispatcher, uint32_t event);

/** Run ViewDispatcher
 * Use only after queue enabled
 * @param view_dispatcher ViewDispatcher instance
 */
void view_dispatcher_run(ViewDispatcher* view_dispatcher);

/** Stop ViewDispatcher
 * Use only after queue enabled
 * @param view_dispatcher ViewDispatcher instance
 */
void view_dispatcher_stop(ViewDispatcher* view_dispatcher);

/** Add view to ViewDispatcher
 * @param view_dispatcher, ViewDispatcher instance
 * @param view_id View id to register
 * @param view View instance
 */
void view_dispatcher_add_view(ViewDispatcher* view_dispatcher, uint32_t view_id, View* view);

/** Remove view from ViewDispatcher
 * @param view_dispatcher ViewDispatcher instance
 * @param view_id View id to remove
 */
void view_dispatcher_remove_view(ViewDispatcher* view_dispatcher, uint32_t view_id);

/** Switch to View
 * @param view_dispatcher ViewDispatcher instance
 * @param view_id View id to register
 */
void view_dispatcher_switch_to_view(ViewDispatcher* view_dispatcher, uint32_t view_id);

/** Attach ViewDispatcher to GUI
 * @param view_dispatcher ViewDispatcher instance
 * @param gui GUI instance to attach to
 */
void view_dispatcher_attach_to_gui(
    ViewDispatcher* view_dispatcher,
    Gui* gui,
    ViewDispatcherType type);

#ifdef __cplusplus
}
#endif
