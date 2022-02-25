/**
  * @file infrared_app_view_manager.h
  * Infrared: Scene events description
  */
#pragma once
#include <gui/modules/button_menu.h>
#include <gui/modules/text_input.h>
#include <gui/view_stack.h>
#include <gui/modules/button_panel.h>
#include <furi.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include <gui/modules/loading.h>

#include "infrared_app_event.h"
#include "view/infrared_progress_view.h"

/** Infrared View manager class */
class InfraredAppViewManager {
public:
    /** Infrared View Id enum, it is used
     * to identify added views */
    enum class ViewId : uint8_t {
        DialogEx,
        TextInput,
        Submenu,
        ButtonMenu,
        UniversalRemote,
        Popup,
    };

    /** Class constructor */
    InfraredAppViewManager();
    /** Class destructor */
    ~InfraredAppViewManager();

    /** Switch to another view
     *
     * @param id - view id to switch to
     */
    void switch_to(ViewId id);

    /** Receive event from queue
     *
     * @param event - received event
     */
    void receive_event(InfraredAppEvent* event);

    /** Send event to queue
     *
     * @param event - event to send
     */
    void send_event(InfraredAppEvent* event);

    /** Clear events that already in queue
     *
     * @param event - event to send
     */
    void clear_events();

    /** Get dialog_ex view module
     *
     * @retval dialog_ex view module
     */
    DialogEx* get_dialog_ex();

    /** Get submenu view module
     *
     * @retval submenu view module
     */
    Submenu* get_submenu();

    /** Get popup view module
     *
     * @retval popup view module
     */
    Popup* get_popup();

    /** Get text_input view module
     *
     * @retval text_input view module
     */
    TextInput* get_text_input();

    /** Get button_menu view module
     *
     * @retval button_menu view module
     */
    ButtonMenu* get_button_menu();

    /** Get button_panel view module
     *
     * @retval button_panel view module
     */
    ButtonPanel* get_button_panel();

    /** Get view_stack view module used in universal remote
     *
     * @retval view_stack view module
     */
    ViewStack* get_universal_view_stack();

    /** Get progress view module
     *
     * @retval progress view module
     */
    InfraredProgressView* get_progress();

    /** Get loading view module
     *
     * @retval loading view module
     */
    Loading* get_loading();

    /** Get event queue
     *
     * @retval event queue
     */
    osMessageQueueId_t get_event_queue();

    /** Callback to handle back button
     *
     * @param context - context to pass to callback
     * @retval always returns VIEW_IGNORE
     */
    uint32_t previous_view_callback(void* context);

private:
    /** View Dispatcher instance.
     * It handles view switching */
    ViewDispatcher* view_dispatcher;
    /** Gui instance */
    Gui* gui;
    /** Text input view module instance */
    TextInput* text_input;
    /** DialogEx view module instance */
    DialogEx* dialog_ex;
    /** Submenu view module instance */
    Submenu* submenu;
    /** Popup view module instance */
    Popup* popup;
    /** ButtonMenu view module instance */
    ButtonMenu* button_menu;
    /** ButtonPanel view module instance */
    ButtonPanel* button_panel;
    /** ViewStack view module instance */
    ViewStack* universal_view_stack;
    /** ProgressView view module instance */
    InfraredProgressView* progress_view;
    /** Loading view module instance */
    Loading* loading_view;

    /** Queue to handle events, which are processed in scenes */
    osMessageQueueId_t event_queue;

    /** Add View to pull of views
     *
     * @param view_id - id to identify view
     * @param view - view to add
     */
    void add_view(ViewId view_id, View* view);
};
