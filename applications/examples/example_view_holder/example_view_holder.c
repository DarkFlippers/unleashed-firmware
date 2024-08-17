/**
 * @file example_view_holder.c
 * @brief Example application demonstrating the usage of the ViewHolder library.
 *
 * This application will display a text box with some scrollable text in it.
 * Press the Back key to exit the application.
 */

#include <gui/gui.h>
#include <gui/view_holder.h>
#include <gui/modules/text_box.h>

#include <api_lock.h>

// This function will be called when the user presses the Back button.
static void example_view_holder_back_callback(void* context) {
    FuriApiLock exit_lock = context;
    // Unlock the exit lock, thus enabling the app to exit.
    api_lock_unlock(exit_lock);
}

int32_t example_view_holder_app(void* arg) {
    UNUSED(arg);

    // Access the GUI API instance.
    Gui* gui = furi_record_open(RECORD_GUI);
    // Create a TextBox view. The Gui object only accepts
    // ViewPort instances, so we will need to address that later.
    TextBox* text_box = text_box_alloc();
    // Set some text so that the text box is not empty.
    text_box_set_text(
        text_box,
        "ViewHolder is being used\n"
        "to show this TextBox view.\n\n"
        "Scroll down to see more.\n\n\n"
        "Press \"Back\" to exit.");

    // Create a ViewHolder instance. It will serve as an adapter to convert
    // between the View type provided by the TextBox view and the ViewPort type
    // that the GUI can actually display.
    ViewHolder* view_holder = view_holder_alloc();
    // Let the GUI know about this ViewHolder instance.
    view_holder_attach_to_gui(view_holder, gui);
    // Set the view that we want to display.
    view_holder_set_view(view_holder, text_box_get_view(text_box));

    // The part below is not really related to this example, but is necessary for it to function.
    // We need to somehow stall the application thread so that the view stays on the screen (otherwise
    // the app will just exit and won't display anything) and at the same time we need a way to quit out
    // of the application.

    // In this example, a simple FuriApiLock instance is used. A real-world application is likely to have some
    // kind of event handling loop here instead. (see the ViewDispatcher example or one of FuriEventLoop
    // examples for that).

    // Create a pre-locked FuriApiLock instance.
    FuriApiLock exit_lock = api_lock_alloc_locked();
    // Set a Back event callback for the ViewHolder instance. It will be called when the user
    // presses the Back button. We pass the exit lock instance as the context to be able to access
    // it inside the callback function.
    view_holder_set_back_callback(view_holder, example_view_holder_back_callback, exit_lock);

    // This call will block the application thread from running until the exit lock gets unlocked somehow
    // (the only way it can happen in this example is via the back callback).
    api_lock_wait_unlock_and_free(exit_lock);

    // The back key has been pressed, which unlocked the exit lock. The application is about to exit.

    // The view must be removed from a ViewHolder instance before deleting it.
    view_holder_set_view(view_holder, NULL);
    // Delete everything to prevent memory leaks.
    view_holder_free(view_holder);
    text_box_free(text_box);
    // End access to the GUI API.
    furi_record_close(RECORD_GUI);

    return 0;
}
