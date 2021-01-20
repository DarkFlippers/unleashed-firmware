#pragma once

#include "view.h"
#include "gui.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ViewDispatcher widget placement */
typedef enum {
    ViewDispatcherTypeNone, /* Special layer for internal use only */
    ViewDispatcherTypeWindow, /* Main widget layer, status bar is shown */
    ViewDispatcherTypeFullscreen /* Fullscreen widget layer */
} ViewDispatcherType;

typedef struct ViewDispatcher ViewDispatcher;

/* Allocate ViewDispatcher
 * @return pointer to ViewDispatcher instance
 */
ViewDispatcher* view_dispatcher_alloc();

/* Free ViewDispatcher
 * @param pointer to View
 */
void view_dispatcher_free(ViewDispatcher* view_dispatcher);

/* Add view to ViewDispatcher
 * @param view_dispatcher, ViewDispatcher instance
 * @param view_id, View id to register
 * @param view, View instance
 */
void view_dispatcher_add_view(ViewDispatcher* view_dispatcher, uint32_t view_id, View* view);

/* Switch to View
 * @param view_dispatcher, ViewDispatcher instance
 * @param view_id, View id to register
 */
void view_dispatcher_switch_to_view(ViewDispatcher* view_dispatcher, uint32_t view_id);

/* Attach ViewDispatcher to GUI
 * @param view_dispatcher, ViewDispatcher instance
 * @param gui, GUI instance to attach to
 */
void view_dispatcher_attach_to_gui(
    ViewDispatcher* view_dispatcher,
    Gui* gui,
    ViewDispatcherType type);

#ifdef __cplusplus
}
#endif
