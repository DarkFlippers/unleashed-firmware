/**
 * @file view_dispatcher.h
 * GUI: ViewDispatcher API
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
ViewDispatcher* view_dispatcher_alloc();

/** Free ViewDispatcher instance
 *
 * @param      view_dispatcher  pointer to ViewDispatcher
 */
void view_dispatcher_free(ViewDispatcher* view_dispatcher);

/** Enable queue support
 *
 * If queue enabled all input and custom events will be dispatched throw
 * internal queue
 *
 * @param      view_dispatcher  ViewDispatcher instance
 */
void view_dispatcher_enable_queue(ViewDispatcher* view_dispatcher);

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

/** Run ViewDispatcher
 *
 * Use only after queue enabled
 *
 * @param      view_dispatcher  ViewDispatcher instance
 */
void view_dispatcher_run(ViewDispatcher* view_dispatcher);

/** Stop ViewDispatcher
 *
 * Use only after queue enabled
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
