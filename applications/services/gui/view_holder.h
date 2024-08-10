/**
 * @file view_holder.h
 * @brief GUI: ViewHolder API
 *
 * ViewHolder is used to connect a single View to a Gui instance. This is useful in smaller applications
 * with a simple user interface. If advanced view switching capabilites are required, consider using ViewDispatcher instead.
 *
 * @warning Views added to a ViewHolder MUST NOT be in a ViewStack at the same time.
 */
#pragma once

#include <gui/view.h>
#include <gui/gui.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ViewHolder ViewHolder;

/** 
 * @brief Free callback type
 */
typedef void (*FreeCallback)(void* free_context);

/** 
 * @brief Back callback type
 *
 * @warning Will be called from the GUI thread
 */
typedef void (*BackCallback)(void* back_context);

/**
 * @brief Allocate ViewHolder
 * @return pointer to ViewHolder instance
 */
ViewHolder* view_holder_alloc(void);

/**
 * @brief Free ViewHolder and call Free callback
 *
 * @warning The current view must be unset prior to freeing a ViewHolder instance.
 *
 * @param view_holder pointer to ViewHolder
 */
void view_holder_free(ViewHolder* view_holder);

/**
 * @brief Set view for ViewHolder
 *
 * Pass NULL as the view parameter to unset the current view.
 * 
 * @param view_holder ViewHolder instance
 * @param view View instance
 */
void view_holder_set_view(ViewHolder* view_holder, View* view);

/**
 * @brief Set Free callback
 * 
 * @param view_holder ViewHolder instance
 * @param free_callback callback pointer
 * @param free_context callback context
 */
void view_holder_set_free_callback(
    ViewHolder* view_holder,
    FreeCallback free_callback,
    void* free_context);

/**
 * @brief Free callback context getter.
 *
 * Useful if your Free callback is a module destructor, so you can get an instance of the module using this method.
 * 
 * @param view_holder ViewHolder instance
 * @return void* free callback context
 */
void* view_holder_get_free_context(ViewHolder* view_holder);

/**
 * @brief Set the back key callback.
 *
 * The callback function will be called if the user has pressed the Back key
 * and the current view did not handle this event.
 *
 * @param view_holder ViewHolder instance
 * @param back_callback pointer to the callback function
 * @param back_context pointer to a user-specific object, can be NULL
 */
void view_holder_set_back_callback(
    ViewHolder* view_holder,
    BackCallback back_callback,
    void* back_context);

/**
 * @brief Attach ViewHolder to GUI
 * 
 * @param view_holder ViewHolder instance
 * @param gui GUI instance to attach to
 */
void view_holder_attach_to_gui(ViewHolder* view_holder, Gui* gui);

/**
 * @brief View Update Handler
 *
 * @param view View Instance
 * @param context ViewHolder instance
 */
void view_holder_update(View* view, void* context);

/**
 * @brief Send ViewPort of this ViewHolder instance to front
 *
 * @param view_holder ViewHolder instance
 */
void view_holder_send_to_front(ViewHolder* view_holder);

/**
 * @brief Send ViewPort of this ViewHolder instance to back
 *
 * @param view_holder ViewHolder instance
 */
void view_holder_send_to_back(ViewHolder* view_holder);

#ifdef __cplusplus
}
#endif
