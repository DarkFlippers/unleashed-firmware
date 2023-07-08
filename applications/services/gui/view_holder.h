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
 * @warning comes from GUI thread
 */
typedef void (*BackCallback)(void* back_context);

/**
 * @brief Allocate ViewHolder
 * @return pointer to ViewHolder instance
 */
ViewHolder* view_holder_alloc();

/**
 * @brief Free ViewHolder and call Free callback
 * @param view_holder pointer to ViewHolder
 */
void view_holder_free(ViewHolder* view_holder);

/**
 * @brief Set view for ViewHolder
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
 * @brief Free callback context getter. Useful if your Free callback is a module destructor, so you can get an instance of the module using this method.
 * 
 * @param view_holder ViewHolder instance
 * @return void* free callback context
 */
void* view_holder_get_free_context(ViewHolder* view_holder);

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
 * @brief Enable view processing
 * 
 * @param view_holder 
 */
void view_holder_start(ViewHolder* view_holder);

/**
 * @brief Disable view processing
 * 
 * @param view_holder 
 */
void view_holder_stop(ViewHolder* view_holder);

/** View Update Handler
 * @param view, View Instance
 * @param context, ViewHolder instance
 */
void view_holder_update(View* view, void* context);

#ifdef __cplusplus
}
#endif
