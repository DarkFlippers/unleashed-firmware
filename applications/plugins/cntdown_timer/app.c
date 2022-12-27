#include "views/countdown_view.h"
#include "app.h"

static void register_view(ViewDispatcher* dispatcher, View* view, uint32_t viewid);

int32_t app_main(void* p) {
    UNUSED(p);

    CountDownTimerApp* app = countdown_app_new();

    countdown_app_run(app);

    countdown_app_delete(app);

    return 0;
}

static uint32_t view_exit(void* ctx) {
    furi_assert(ctx);

    return VIEW_NONE;
}

CountDownTimerApp* countdown_app_new(void) {
    CountDownTimerApp* app = (CountDownTimerApp*)(malloc(sizeof(CountDownTimerApp)));

    // 1.1 open gui
    app->gui = furi_record_open(RECORD_GUI);

    // 2.1 setup view dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);

    // 2.2 attach view dispatcher to gui
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // 2.3 attach views to the dispatcher
    // helloworld view
    app->helloworld_view = countdown_timer_view_new();
    register_view(app->view_dispatcher, countdown_timer_view_get_view(app->helloworld_view), 0xff);

    // 2.5 switch to default view
    view_dispatcher_switch_to_view(app->view_dispatcher, 0xff);

    return app;
}

void countdown_app_delete(CountDownTimerApp* app) {
    furi_assert(app);

    // delete views
    view_dispatcher_remove_view(app->view_dispatcher, 0xff);
    countdown_timer_view_delete(app->helloworld_view); // hello world view

    // delete view dispatcher
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);

    // self
    free(app);
}

void countdown_app_run(CountDownTimerApp* app) {
    view_dispatcher_run(app->view_dispatcher);
}

static void register_view(ViewDispatcher* dispatcher, View* view, uint32_t viewid) {
    view_dispatcher_add_view(dispatcher, viewid, view);

    view_set_previous_callback(view, view_exit);
}
