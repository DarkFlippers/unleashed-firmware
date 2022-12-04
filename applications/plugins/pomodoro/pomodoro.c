#include "pomodoro.h"
#include <notification/notification_messages.h>

#define TAG "PomodoroApp"

enum PomodoroDebugSubmenuIndex {
    PomodoroSubmenuIndex10,
    PomodoroSubmenuIndex25,
    PomodoroSubmenuIndex50,
};

void pomodoro_submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    Pomodoro* app = context;
    if(index == PomodoroSubmenuIndex10) {
        app->view_id = PomodoroView10;
        view_dispatcher_switch_to_view(app->view_dispatcher, PomodoroView10);
    }
    if(index == PomodoroSubmenuIndex25) {
        app->view_id = PomodoroView25;
        view_dispatcher_switch_to_view(app->view_dispatcher, PomodoroView25);
    }
    if(index == PomodoroSubmenuIndex50) {
        app->view_id = PomodoroView50;
        view_dispatcher_switch_to_view(app->view_dispatcher, PomodoroView50);
    }
}

void pomodoro_dialog_callback(DialogExResult result, void* context) {
    furi_assert(context);
    Pomodoro* app = context;
    if(result == DialogExResultLeft) {
        view_dispatcher_stop(app->view_dispatcher);
    } else if(result == DialogExResultRight) {
        view_dispatcher_switch_to_view(app->view_dispatcher, app->view_id); // Show last view
    } else if(result == DialogExResultCenter) {
        view_dispatcher_switch_to_view(app->view_dispatcher, PomodoroViewSubmenu);
    }
}

uint32_t pomodoro_exit_confirm_view(void* context) {
    UNUSED(context);
    return PomodoroViewExitConfirm;
}

uint32_t pomodoro_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

Pomodoro* pomodoro_app_alloc() {
    Pomodoro* app = malloc(sizeof(Pomodoro));

    // Gui
    app->gui = furi_record_open(RECORD_GUI);

    // Notifications
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    // View dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Submenu view
    app->submenu = submenu_alloc();
    submenu_add_item(
        app->submenu,
        "Classic: 25 work 5 rest",
        PomodoroSubmenuIndex25,
        pomodoro_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Long: 50 work 10 rest",
        PomodoroSubmenuIndex50,
        pomodoro_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Sprint: 10 work 2 rest",
        PomodoroSubmenuIndex10,
        pomodoro_submenu_callback,
        app);
    view_set_previous_callback(submenu_get_view(app->submenu), pomodoro_exit);
    view_dispatcher_add_view(
        app->view_dispatcher, PomodoroViewSubmenu, submenu_get_view(app->submenu));

    // Dialog view
    app->dialog = dialog_ex_alloc();
    dialog_ex_set_result_callback(app->dialog, pomodoro_dialog_callback);
    dialog_ex_set_context(app->dialog, app);
    dialog_ex_set_left_button_text(app->dialog, "Exit");
    dialog_ex_set_right_button_text(app->dialog, "Stay");
    dialog_ex_set_center_button_text(app->dialog, "Menu");
    dialog_ex_set_header(app->dialog, "Close Current App?", 16, 12, AlignLeft, AlignTop);
    view_dispatcher_add_view(
        app->view_dispatcher, PomodoroViewExitConfirm, dialog_ex_get_view(app->dialog));

    // 25 minutes view
    app->pomodoro_25 = pomodoro_25_alloc();
    view_set_previous_callback(pomodoro_25_get_view(app->pomodoro_25), pomodoro_exit_confirm_view);
    view_dispatcher_add_view(
        app->view_dispatcher, PomodoroView25, pomodoro_25_get_view(app->pomodoro_25));

    // 50 minutes view
    app->pomodoro_50 = pomodoro_50_alloc();
    view_set_previous_callback(pomodoro_50_get_view(app->pomodoro_50), pomodoro_exit_confirm_view);
    view_dispatcher_add_view(
        app->view_dispatcher, PomodoroView50, pomodoro_50_get_view(app->pomodoro_50));

    // 10 minutes view
    app->pomodoro_10 = pomodoro_10_alloc();
    view_set_previous_callback(pomodoro_10_get_view(app->pomodoro_10), pomodoro_exit_confirm_view);
    view_dispatcher_add_view(
        app->view_dispatcher, PomodoroView10, pomodoro_10_get_view(app->pomodoro_10));

    // TODO switch to menu after Media is done
    app->view_id = PomodoroViewSubmenu;
    view_dispatcher_switch_to_view(app->view_dispatcher, app->view_id);

    return app;
}

void pomodoro_app_free(Pomodoro* app) {
    furi_assert(app);

    // Reset notification
    notification_internal_message(app->notifications, &sequence_reset_blue);

    // Free views
    view_dispatcher_remove_view(app->view_dispatcher, PomodoroViewSubmenu);
    submenu_free(app->submenu);
    view_dispatcher_remove_view(app->view_dispatcher, PomodoroViewExitConfirm);
    dialog_ex_free(app->dialog);
    view_dispatcher_remove_view(app->view_dispatcher, PomodoroView25);
    pomodoro_25_free(app->pomodoro_25);
    view_dispatcher_remove_view(app->view_dispatcher, PomodoroView50);
    pomodoro_50_free(app->pomodoro_50);
    view_dispatcher_remove_view(app->view_dispatcher, PomodoroView10);
    pomodoro_10_free(app->pomodoro_10);
    view_dispatcher_free(app->view_dispatcher);

    // Close records
    furi_record_close(RECORD_GUI);
    app->gui = NULL;
    furi_record_close(RECORD_NOTIFICATION);
    app->notifications = NULL;

    // Free rest
    free(app);
}

int32_t pomodoro_app(void* p) {
    UNUSED(p);
    // Switch profile to Hid
    Pomodoro* app = pomodoro_app_alloc();

    view_dispatcher_run(app->view_dispatcher);

    pomodoro_app_free(app);

    return 0;
}
