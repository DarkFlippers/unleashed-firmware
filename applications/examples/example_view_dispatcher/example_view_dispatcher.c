/**
 * @file example_view_dispatcher.c
 * @brief Example application demonstrating the usage of the ViewDispatcher library.
 *
 * This application can display one of two views: either a Widget or a Submenu.
 * Each view has its own way of switching to another one:
 *
 * - A center button in the Widget view.
 * - A submenu item in the Submenu view
 *
 * Press either to switch to a different view. Press Back to exit the application.
 *
 */

#include <gui/gui.h>
#include <gui/view_dispatcher.h>

#include <gui/modules/widget.h>
#include <gui/modules/submenu.h>

// Enumeration of the view indexes.
typedef enum {
    ViewIndexWidget,
    ViewIndexSubmenu,
    ViewIndexCount,
} ViewIndex;

// Enumeration of submenu items.
typedef enum {
    SubmenuIndexNothing,
    SubmenuIndexSwitchView,
} SubmenuIndex;

// Main application structure.
typedef struct {
    ViewDispatcher* view_dispatcher;
    Widget* widget;
    Submenu* submenu;
} ExampleViewDispatcherApp;

// This function is called when the user has pressed the Back key.
static bool example_view_dispatcher_app_navigation_callback(void* context) {
    furi_assert(context);
    ExampleViewDispatcherApp* app = context;
    // Back means exit the application, which can be done by stopping the ViewDispatcher.
    view_dispatcher_stop(app->view_dispatcher);
    return true;
}

// This function is called when there are custom events to process.
static bool example_view_dispatcher_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    ExampleViewDispatcherApp* app = context;
    // The event numerical value can mean different things (the application is responsible to uphold its chosen convention)
    // In this example, the only possible meaning is the view index to switch to.
    furi_assert(event < ViewIndexCount);
    // Switch to the requested view.
    view_dispatcher_switch_to_view(app->view_dispatcher, event);

    return true;
}

// This function is called when the user presses the "Switch View" button on the Widget view.
static void example_view_dispatcher_app_button_callback(
    GuiButtonType button_type,
    InputType input_type,
    void* context) {
    furi_assert(context);
    ExampleViewDispatcherApp* app = context;
    // Only request the view switch if the user short-presses the Center button.
    if(button_type == GuiButtonTypeCenter && input_type == InputTypeShort) {
        // Request switch to the Submenu view via the custom event queue.
        view_dispatcher_send_custom_event(app->view_dispatcher, ViewIndexSubmenu);
    }
}

// This function is called when the user activates the "Switch View" submenu item.
static void example_view_dispatcher_app_submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    ExampleViewDispatcherApp* app = context;
    // Only request the view switch if the user activates the "Switch View" item.
    if(index == SubmenuIndexSwitchView) {
        // Request switch to the Widget view via the custom event queue.
        view_dispatcher_send_custom_event(app->view_dispatcher, ViewIndexWidget);
    }
}

// Application constructor function.
static ExampleViewDispatcherApp* example_view_dispatcher_app_alloc() {
    ExampleViewDispatcherApp* app = malloc(sizeof(ExampleViewDispatcherApp));
    // Access the GUI API instance.
    Gui* gui = furi_record_open(RECORD_GUI);
    // Create and initialize the Widget view.
    app->widget = widget_alloc();
    widget_add_string_multiline_element(
        app->widget, 64, 32, AlignCenter, AlignCenter, FontSecondary, "Press the Button below");
    widget_add_button_element(
        app->widget,
        GuiButtonTypeCenter,
        "Switch View",
        example_view_dispatcher_app_button_callback,
        app);
    // Create and initialize the Submenu view.
    app->submenu = submenu_alloc();
    submenu_add_item(app->submenu, "Do Nothing", SubmenuIndexNothing, NULL, NULL);
    submenu_add_item(
        app->submenu,
        "Switch View",
        SubmenuIndexSwitchView,
        example_view_dispatcher_app_submenu_callback,
        app);
    // Create the ViewDispatcher instance.
    app->view_dispatcher = view_dispatcher_alloc();
    // Let the GUI know about this ViewDispatcher instance.
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    // Register the views within the ViewDispatcher instance. This alone will not show any of them on the screen.
    // Each view must have its own index to refer to it later (it is best done via an enumeration as shown here).
    view_dispatcher_add_view(app->view_dispatcher, ViewIndexWidget, widget_get_view(app->widget));
    view_dispatcher_add_view(
        app->view_dispatcher, ViewIndexSubmenu, submenu_get_view(app->submenu));
    // Set the custom event callback. It will be called each time a custom event is scheduled
    // using the view_dispatcher_send_custom_callback() function.
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, example_view_dispatcher_app_custom_event_callback);
    // Set the navigation, or back button callback. It will be called if the user pressed the Back button
    // and the event was not handled in the currently displayed view.
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, example_view_dispatcher_app_navigation_callback);
    // The context will be passed to the callbacks as a parameter, so we have access to our application object.
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    return app;
}

// Application destructor function.
static void example_view_dispatcher_app_free(ExampleViewDispatcherApp* app) {
    // All views must be un-registered (removed) from a ViewDispatcher instance
    // before deleting it. Failure to do so will result in a crash.
    view_dispatcher_remove_view(app->view_dispatcher, ViewIndexWidget);
    view_dispatcher_remove_view(app->view_dispatcher, ViewIndexSubmenu);
    // Now it is safe to delete the ViewDispatcher instance.
    view_dispatcher_free(app->view_dispatcher);
    // Delete the views
    widget_free(app->widget);
    submenu_free(app->submenu);
    // End access to hte the GUI API.
    furi_record_close(RECORD_GUI);
    // Free the remaining memory.
    free(app);
}

static void example_view_dispatcher_app_run(ExampleViewDispatcherApp* app) {
    // Display the Widget view on the screen.
    view_dispatcher_switch_to_view(app->view_dispatcher, ViewIndexWidget);
    // This function will block until view_dispatcher_stop() is called.
    // Internally, it uses a FuriEventLoop (see FuriEventLoop examples for more info on this).
    view_dispatcher_run(app->view_dispatcher);
}

/*******************************************************************
 *                     vvv START HERE vvv
 *
 * The application's entry point - referenced in application.fam
 *******************************************************************/
int32_t example_view_dispatcher_app(void* arg) {
    UNUSED(arg);

    ExampleViewDispatcherApp* app = example_view_dispatcher_app_alloc();
    example_view_dispatcher_app_run(app);
    example_view_dispatcher_app_free(app);

    return 0;
}
