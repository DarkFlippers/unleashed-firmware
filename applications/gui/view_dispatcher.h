#pragma once

#include "view.h"
#include "gui.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ViewDispatcher view_port placement
 */
typedef enum {
    ViewDispatcherTypeNone, /**< Special layer for internal use only */
    ViewDispatcherTypeWindow, /**< Main view_port layer, status bar is shown */
    ViewDispatcherTypeFullscreen /**< Fullscreen view_port layer */
} ViewDispatcherType;

typedef struct ViewDispatcher ViewDispatcher;

/**
 * @brief Allocate ViewDispatcher
 * @return pointer to ViewDispatcher instance
 */
ViewDispatcher* view_dispatcher_alloc();

/**
 * @brief Free ViewDispatcher
 * @param view_dispatcher pointer to ViewDispatcher
 */
void view_dispatcher_free(ViewDispatcher* view_dispatcher);

/**
 * @brief Add view to ViewDispatcher
 * @param view_dispatcher, ViewDispatcher instance
 * @param view_id View id to register
 * @param view View instance
 */
void view_dispatcher_add_view(ViewDispatcher* view_dispatcher, uint32_t view_id, View* view);

/**
 * @brief Remove view from ViewDispatcher
 * @param view_dispatcher ViewDispatcher instance
 * @param view_id View id to remove
 */
void view_dispatcher_remove_view(ViewDispatcher* view_dispatcher, uint32_t view_id);

/**
 * @brief Switch to View
 * @param view_dispatcher ViewDispatcher instance
 * @param view_id View id to register
 */
void view_dispatcher_switch_to_view(ViewDispatcher* view_dispatcher, uint32_t view_id);

/**
 * @brief Attach ViewDispatcher to GUI
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
