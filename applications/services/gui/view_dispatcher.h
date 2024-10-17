/**
 * @file view_dispatcher.h
 * @brief GUI: ViewDispatcher API
 *
 * ViewDispatcher is used to connect several Views to a Gui instance, switch between them and handle various events.
 * This is useful in applications featuring an advanced graphical user interface.
 *
 * Internally, ViewDispatcher employs a FuriEventLoop instance together with two separate
 * message queues for input and custom event handling. See FuriEventLoop for more information.
 *
 * If no multi-view or complex event handling capabilities are required, consider using ViewHolder instead.
 *
 * @warning Views added to a ViewDispatcher MUST NOT be in a ViewStack at the same time.
 */

#pragma once

#include "view.h"
#include "gui.h"
#include "scene_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/** ViewDispatcher view_port placement */
typedef enum {
    ViewDispatcherTypeDesktop, /**< Desktop layer: fullscreen with status bar on top of it. For internal usage. */
    ViewDispatcherTypeWindow, /**< Window layer: with status bar  */
    ViewDispatcherTypeFullscreen /**< Fullscreen layer: without status bar */
} ViewDispatcherType;

typedef struct ViewDispatcher ViewDispatcher;

/** Prototype for custom event callback */
typedef bool (*ViewDispatcherCustomEventCallback)(void* context, uint32_t event);

/** Prototype for navigation event callback */
typedef bool (*ViewDispatcherNavigationEventCallback)(void* context);

/** Prototype for tick event callback */
typedef void (*ViewDispatcherTickEventCallback)(void* context);

/** Allocate ViewDispatcher instance
 *
 * @return     pointer to ViewDispatcher instance
 */
ViewDispatcher* view_dispatcher_alloc(void);

/** Allocate ViewDispatcher instance with an externally owned event loop. If
 * this constructor is used instead of `view_dispatcher_alloc`, the burden of
 * freeing the event loop is placed on the caller.
 *
 * @param loop pointer to FuriEventLoop instance
 * @return     pointer to ViewDispatcher instance
 */
ViewDispatcher* view_dispatcher_alloc_ex(FuriEventLoop* loop);

/** Free ViewDispatcher instance
 *
 * @warning All added views MUST be removed using view_dispatcher_remove_view()
 *          before calling this function.
 *
 * @param      view_dispatcher  pointer to ViewDispatcher
 */
void view_dispatcher_free(ViewDispatcher* view_dispatcher);

/** Enable queue support
 *
 * @deprecated Do NOT use in new code and remove all calls to it from existing code.
 *             The queue support is now always enabled during construction. If no queue support
 *             is required, consider using ViewHolder instead.
 *
 * @param      view_dispatcher  ViewDispatcher instance
 */
FURI_DEPRECATED void view_dispatcher_enable_queue(ViewDispatcher* view_dispatcher);

/** Send custom event
 *
 * @param      view_dispatcher  ViewDispatcher instance
 * @param[in]  event            The event
 */
void view_dispatcher_send_custom_event(ViewDispatcher* view_dispatcher, uint32_t event);

/** Set custom event handler
 *
 * Called on Custom Event, if it is not consumed by view
 *
 * @param      view_dispatcher  ViewDispatcher instance
 * @param      callback         ViewDispatcherCustomEventCallback instance
 */
void view_dispatcher_set_custom_event_callback(
    ViewDispatcher* view_dispatcher,
    ViewDispatcherCustomEventCallback callback);

/** Set navigation event handler
 *
 * Called on Input Short Back Event, if it is not consumed by view
 *
 * @param      view_dispatcher  ViewDispatcher instance
 * @param      callback         ViewDispatcherNavigationEventCallback instance
 */
void view_dispatcher_set_navigation_event_callback(
    ViewDispatcher* view_dispatcher,
    ViewDispatcherNavigationEventCallback callback);

/** Set tick event handler
 *
 * @warning Requires the event loop to be owned by the view dispatcher, i.e.
 * it should have been instantiated with `view_dispatcher_alloc`, not
 * `view_dispatcher_alloc_ex`.
 * 
 * @param      view_dispatcher  ViewDispatcher instance
 * @param      callback         ViewDispatcherTickEventCallback
 * @param      tick_period      callback call period
 */
void view_dispatcher_set_tick_event_callback(
    ViewDispatcher* view_dispatcher,
    ViewDispatcherTickEventCallback callback,
    uint32_t tick_period);

/** Set event callback context
 *
 * @param      view_dispatcher  ViewDispatcher instance
 * @param      context          pointer to context
 */
void view_dispatcher_set_event_callback_context(ViewDispatcher* view_dispatcher, void* context);

/** Get event_loop instance
 *
 * Use the return value to connect additional supported primitives (message queues, timers, etc)
 * to this ViewDispatcher instance's event loop.
 *
 * @warning Do NOT call furi_event_loop_run() on the returned instance, it is done internally
 *          in the view_dispatcher_run() call.
 *
 * @param      view_dispatcher  ViewDispatcher instance
 *
 * @return     The event_loop instance.
 */
FuriEventLoop* view_dispatcher_get_event_loop(ViewDispatcher* view_dispatcher);

/** Run ViewDispatcher
 *
 * This function will start the event loop and block until view_dispatcher_stop() is called
 * or the current thread receives a FuriSignalExit signal.
 *
 * @param      view_dispatcher  ViewDispatcher instance
 */
void view_dispatcher_run(ViewDispatcher* view_dispatcher);

/** Stop ViewDispatcher
 *
 * @param      view_dispatcher  ViewDispatcher instance
 */
void view_dispatcher_stop(ViewDispatcher* view_dispatcher);

/** Add view to ViewDispatcher
 *
 * @param      view_dispatcher  ViewDispatcher instance
 * @param      view_id          View id to register
 * @param      view             View instance
 */
void view_dispatcher_add_view(ViewDispatcher* view_dispatcher, uint32_t view_id, View* view);

/** Remove view from ViewDispatcher
 *
 * @param      view_dispatcher  ViewDispatcher instance
 * @param      view_id          View id to remove
 */
void view_dispatcher_remove_view(ViewDispatcher* view_dispatcher, uint32_t view_id);

/** Switch to View
 *
 * @param      view_dispatcher  ViewDispatcher instance
 * @param      view_id          View id to register
 * @warning    switching may be delayed till input events complementarity
 *             reached
 */
void view_dispatcher_switch_to_view(ViewDispatcher* view_dispatcher, uint32_t view_id);

/** Send ViewPort of this ViewDispatcher instance to front
 *
 * @param      view_dispatcher  ViewDispatcher instance
 */
void view_dispatcher_send_to_front(ViewDispatcher* view_dispatcher);

/** Send ViewPort of this ViewDispatcher instance to back
 *
 * @param      view_dispatcher  ViewDispatcher instance
 */
void view_dispatcher_send_to_back(ViewDispatcher* view_dispatcher);

/** Attach ViewDispatcher to GUI
 *
 * @param      view_dispatcher  ViewDispatcher instance
 * @param      gui              GUI instance to attach to
 * @param[in]  type             The type
 */
void view_dispatcher_attach_to_gui(
    ViewDispatcher* view_dispatcher,
    Gui* gui,
    ViewDispatcherType type);

#ifdef __cplusplus
}
#endif
