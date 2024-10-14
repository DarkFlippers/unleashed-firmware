#include "../test.h" // IWYU pragma: keep

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_random.h>

#include <storage/storage.h>
#include <applications/system/js_app/js_thread.h>

#include <stdint.h>

#define JS_SCRIPT_PATH(name) EXT_PATH("unit_tests/js/" name ".js")

typedef enum {
    JsTestsFinished = 1,
    JsTestsError = 2,
} JsTestFlag;

typedef struct {
    FuriEventFlag* event_flags;
    FuriString* error_string;
} JsTestCallbackContext;

static void js_test_callback(JsThreadEvent event, const char* msg, void* param) {
    JsTestCallbackContext* context = param;
    if(event == JsThreadEventPrint) {
        FURI_LOG_I("js_test", "%s", msg);
    } else if(event == JsThreadEventError || event == JsThreadEventErrorTrace) {
        context->error_string = furi_string_alloc_set_str(msg);
        furi_event_flag_set(context->event_flags, JsTestsFinished | JsTestsError);
    } else if(event == JsThreadEventDone) {
        furi_event_flag_set(context->event_flags, JsTestsFinished);
    }
}

static void js_test_run(const char* script_path) {
    JsTestCallbackContext* context = malloc(sizeof(JsTestCallbackContext));
    context->event_flags = furi_event_flag_alloc();

    JsThread* thread = js_thread_run(script_path, js_test_callback, context);
    uint32_t flags = furi_event_flag_wait(
        context->event_flags, JsTestsFinished, FuriFlagWaitAny, FuriWaitForever);
    if(flags & FuriFlagError) {
        // getting the flags themselves should not fail
        furi_crash();
    }

    FuriString* error_string = context->error_string;

    js_thread_stop(thread);
    furi_event_flag_free(context->event_flags);
    free(context);

    if(flags & JsTestsError) {
        // memory leak: not freeing the FuriString if the tests fail,
        // because mu_fail executes a return
        //
        // who cares tho?
        mu_fail(furi_string_get_cstr(error_string));
    }
}

MU_TEST(js_test_basic) {
    js_test_run(JS_SCRIPT_PATH("basic"));
}
MU_TEST(js_test_math) {
    js_test_run(JS_SCRIPT_PATH("math"));
}
MU_TEST(js_test_event_loop) {
    js_test_run(JS_SCRIPT_PATH("event_loop"));
}
MU_TEST(js_test_storage) {
    js_test_run(JS_SCRIPT_PATH("storage"));
}

MU_TEST_SUITE(test_js) {
    MU_RUN_TEST(js_test_basic);
    MU_RUN_TEST(js_test_math);
    MU_RUN_TEST(js_test_event_loop);
    MU_RUN_TEST(js_test_storage);
}

int run_minunit_test_js(void) {
    MU_RUN_SUITE(test_js);
    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_js)
