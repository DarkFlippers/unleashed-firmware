#include <dialogs/dialogs.h>
#include "js_thread.h"
#include <storage/storage.h>
#include "js_app_i.h"
#include <toolbox/path.h>
#include <assets_icons.h>

#define TAG "JS app"

typedef struct {
    JsThread* js_thread;
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Loading* loading;
    JsConsoleView* console_view;
} JsApp;

static uint32_t js_view_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

static void js_app_compact_trace(FuriString* trace_str) {
    // Keep only first line
    size_t line_end = furi_string_search_char(trace_str, '\n');
    if(line_end > 0) {
        furi_string_left(trace_str, line_end);
    }

    // Remove full path
    FuriString* file_name = furi_string_alloc();
    size_t filename_start = furi_string_search_rchar(trace_str, '/');
    if(filename_start > 0) {
        filename_start++;
        furi_string_set_n(
            file_name, trace_str, filename_start, furi_string_size(trace_str) - filename_start);
        furi_string_printf(trace_str, "at %s", furi_string_get_cstr(file_name));
    }

    furi_string_free(file_name);
}

static void js_callback(JsThreadEvent event, const char* msg, void* context) {
    JsApp* app = context;
    furi_assert(app);

    if(event == JsThreadEventDone) {
        FURI_LOG_I(TAG, "Script done");
        console_view_print(app->console_view, "--- DONE ---");
    } else if(event == JsThreadEventPrint) {
        console_view_print(app->console_view, msg);
    } else if(event == JsThreadEventError) {
        console_view_print(app->console_view, "--- ERROR ---");
        console_view_print(app->console_view, msg);
    } else if(event == JsThreadEventErrorTrace) {
        FuriString* compact_trace = furi_string_alloc_set_str(msg);
        js_app_compact_trace(compact_trace);
        console_view_print(app->console_view, furi_string_get_cstr(compact_trace));
        furi_string_free(compact_trace);
        console_view_print(app->console_view, "See logs for full trace");
    }
}

static JsApp* js_app_alloc(void) {
    JsApp* app = malloc(sizeof(JsApp));

    app->view_dispatcher = view_dispatcher_alloc();
    app->loading = loading_alloc();

    app->gui = furi_record_open("gui");
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_add_view(
        app->view_dispatcher, JsAppViewLoading, loading_get_view(app->loading));

    app->console_view = console_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, JsAppViewConsole, console_view_get_view(app->console_view));
    view_set_previous_callback(console_view_get_view(app->console_view), js_view_exit);
    view_dispatcher_switch_to_view(app->view_dispatcher, JsAppViewConsole);

    return app;
}

static void js_app_free(JsApp* app) {
    console_view_free(app->console_view);
    view_dispatcher_remove_view(app->view_dispatcher, JsAppViewConsole);
    loading_free(app->loading);
    view_dispatcher_remove_view(app->view_dispatcher, JsAppViewLoading);

    view_dispatcher_free(app->view_dispatcher);
    furi_record_close("gui");

    free(app);
}

int32_t js_app(void* arg) {
    JsApp* app = js_app_alloc();

    FuriString* script_path = furi_string_alloc_set(EXT_PATH("apps/Scripts"));
    do {
        if(arg != NULL && strlen(arg) > 0) {
            furi_string_set(script_path, (const char*)arg);
        } else {
            DialogsFileBrowserOptions browser_options;
            dialog_file_browser_set_basic_options(&browser_options, ".js", &I_js_script_10px);
            DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
            if(!dialog_file_browser_show(dialogs, script_path, script_path, &browser_options))
                break;
            furi_record_close(RECORD_DIALOGS);
        }
        FuriString* name = furi_string_alloc();
        path_extract_filename(script_path, name, false);
        FuriString* start_text =
            furi_string_alloc_printf("Running %s", furi_string_get_cstr(name));
        console_view_print(app->console_view, furi_string_get_cstr(start_text));
        console_view_print(app->console_view, "------------");
        furi_string_free(name);
        furi_string_free(start_text);

        app->js_thread = js_thread_run(furi_string_get_cstr(script_path), js_callback, app);
        view_dispatcher_run(app->view_dispatcher);

        js_thread_stop(app->js_thread);
    } while(0);

    furi_string_free(script_path);

    js_app_free(app);
    return 0;
} //-V773