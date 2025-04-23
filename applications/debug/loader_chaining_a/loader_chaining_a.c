#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <loader/loader.h>
#include <dialogs/dialogs.h>

#define TAG             "LoaderChainingA"
#define CHAINING_TEST_B "/ext/apps/Debug/loader_chaining_b.fap"
#define NONEXISTENT_APP "Some nonexistent app"

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;

    Loader* loader;

    DialogsApp* dialogs;
} LoaderChainingA;

typedef enum {
    LoaderChainingASubmenuLaunchB,
    LoaderChainingASubmenuLaunchBThenA,
    LoaderChainingASubmenuLaunchNonexistentSilent,
    LoaderChainingASubmenuLaunchNonexistentGui,
    LoaderChainingASubmenuLaunchNonexistentGuiThenA,
} LoaderChainingASubmenu;

static void loader_chaining_a_submenu_callback(void* context, uint32_t index) {
    LoaderChainingA* app = context;

    FuriString* self_path = furi_string_alloc();
    furi_check(loader_get_application_launch_path(app->loader, self_path));

    switch(index) {
    case LoaderChainingASubmenuLaunchB:
        loader_enqueue_launch(app->loader, CHAINING_TEST_B, "Hello", LoaderDeferredLaunchFlagGui);
        view_dispatcher_stop(app->view_dispatcher);
        break;

    case LoaderChainingASubmenuLaunchBThenA:
        loader_enqueue_launch(app->loader, CHAINING_TEST_B, "Hello", LoaderDeferredLaunchFlagGui);
        loader_enqueue_launch(
            app->loader,
            furi_string_get_cstr(self_path),
            "Hello to you from the future",
            LoaderDeferredLaunchFlagGui);

        break;

    case LoaderChainingASubmenuLaunchNonexistentSilent:
        loader_enqueue_launch(app->loader, NONEXISTENT_APP, NULL, LoaderDeferredLaunchFlagNone);
        break;

    case LoaderChainingASubmenuLaunchNonexistentGui:
        loader_enqueue_launch(app->loader, NONEXISTENT_APP, NULL, LoaderDeferredLaunchFlagGui);
        break;

    case LoaderChainingASubmenuLaunchNonexistentGuiThenA:
        loader_enqueue_launch(app->loader, NONEXISTENT_APP, NULL, LoaderDeferredLaunchFlagGui);
        loader_enqueue_launch(
            app->loader,
            furi_string_get_cstr(self_path),
            "Hello to you from the future",
            LoaderDeferredLaunchFlagGui);
        break;
    }

    furi_string_free(self_path);
    view_dispatcher_stop(app->view_dispatcher);
}

static bool loader_chaining_a_nav_callback(void* context) {
    LoaderChainingA* app = context;
    view_dispatcher_stop(app->view_dispatcher);
    return true;
}

LoaderChainingA* loader_chaining_a_alloc(void) {
    LoaderChainingA* app = malloc(sizeof(LoaderChainingA));
    app->gui = furi_record_open(RECORD_GUI);
    app->loader = furi_record_open(RECORD_LOADER);
    app->dialogs = furi_record_open(RECORD_DIALOGS);
    app->view_dispatcher = view_dispatcher_alloc();
    app->submenu = submenu_alloc();

    submenu_add_item(
        app->submenu,
        "Launch B",
        LoaderChainingASubmenuLaunchB,
        loader_chaining_a_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Launch B, then A",
        LoaderChainingASubmenuLaunchBThenA,
        loader_chaining_a_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Trigger error: silent",
        LoaderChainingASubmenuLaunchNonexistentSilent,
        loader_chaining_a_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Trigger error: GUI",
        LoaderChainingASubmenuLaunchNonexistentGui,
        loader_chaining_a_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Error, then launch A",
        LoaderChainingASubmenuLaunchNonexistentGuiThenA,
        loader_chaining_a_submenu_callback,
        app);

    view_dispatcher_add_view(app->view_dispatcher, 0, submenu_get_view(app->submenu));
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, loader_chaining_a_nav_callback);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_switch_to_view(app->view_dispatcher, 0);

    return app;
}

void loader_chaining_a_free(LoaderChainingA* app) {
    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_LOADER);
    furi_record_close(RECORD_GUI);
    view_dispatcher_remove_view(app->view_dispatcher, 0);
    submenu_free(app->submenu);
    view_dispatcher_free(app->view_dispatcher);
    free(app);
}

int32_t chaining_test_app_a(const char* arg) {
    LoaderChainingA* app = loader_chaining_a_alloc();

    if(arg) {
        if(strlen(arg)) {
            DialogMessage* message = dialog_message_alloc();
            FuriString* text;

            dialog_message_set_header(message, "Hi, I am A", 64, 0, AlignCenter, AlignTop);
            text = furi_string_alloc_printf("Me from the past says:\n%s", arg);
            dialog_message_set_buttons(message, NULL, "ok!", NULL);

            dialog_message_set_text(
                message, furi_string_get_cstr(text), 64, 32, AlignCenter, AlignCenter);
            dialog_message_show(app->dialogs, message);
            dialog_message_free(message);
            furi_string_free(text);
        }
    }

    view_dispatcher_run(app->view_dispatcher);

    loader_chaining_a_free(app);
    return 0;
}
