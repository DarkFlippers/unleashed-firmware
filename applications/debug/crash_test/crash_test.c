#include <furi_hal.h>
#include <furi.h>

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>

#define TAG "CrashTest"

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
} CrashTest;

typedef enum {
    CrashTestViewSubmenu,
} CrashTestView;

typedef enum {
    CrashTestSubmenuCheck,
    CrashTestSubmenuCheckMessage,
    CrashTestSubmenuAssert,
    CrashTestSubmenuAssertMessage,
    CrashTestSubmenuCrash,
    CrashTestSubmenuHalt,
} CrashTestSubmenu;

static void crash_test_submenu_callback(void* context, uint32_t index) {
    CrashTest* instance = (CrashTest*)context;
    UNUSED(instance);

    switch(index) {
    case CrashTestSubmenuCheck:
        furi_check(false);
        break;
    case CrashTestSubmenuCheckMessage:
        furi_check(false, "Crash test: furi_check with message");
        break;
    case CrashTestSubmenuAssert:
        furi_assert(false);
        break;
    case CrashTestSubmenuAssertMessage:
        furi_assert(false, "Crash test: furi_assert with message");
        break;
    case CrashTestSubmenuCrash:
        furi_crash("Crash test: furi_crash");
        break;
    case CrashTestSubmenuHalt:
        furi_halt("Crash test: furi_halt");
        break;
    default:
        furi_crash();
    }
}

static uint32_t crash_test_exit_callback(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

CrashTest* crash_test_alloc(void) {
    CrashTest* instance = malloc(sizeof(CrashTest));

    View* view = NULL;

    instance->gui = furi_record_open(RECORD_GUI);
    instance->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(
        instance->view_dispatcher, instance->gui, ViewDispatcherTypeFullscreen);

    // Menu
    instance->submenu = submenu_alloc();
    view = submenu_get_view(instance->submenu);
    view_set_previous_callback(view, crash_test_exit_callback);
    view_dispatcher_add_view(instance->view_dispatcher, CrashTestViewSubmenu, view);
    submenu_add_item(
        instance->submenu, "Check", CrashTestSubmenuCheck, crash_test_submenu_callback, instance);
    submenu_add_item(
        instance->submenu,
        "Check with message",
        CrashTestSubmenuCheckMessage,
        crash_test_submenu_callback,
        instance);
    submenu_add_item(
        instance->submenu, "Assert", CrashTestSubmenuAssert, crash_test_submenu_callback, instance);
    submenu_add_item(
        instance->submenu,
        "Assert with message",
        CrashTestSubmenuAssertMessage,
        crash_test_submenu_callback,
        instance);
    submenu_add_item(
        instance->submenu, "Crash", CrashTestSubmenuCrash, crash_test_submenu_callback, instance);
    submenu_add_item(
        instance->submenu, "Halt", CrashTestSubmenuHalt, crash_test_submenu_callback, instance);

    return instance;
}

void crash_test_free(CrashTest* instance) {
    view_dispatcher_remove_view(instance->view_dispatcher, CrashTestViewSubmenu);
    submenu_free(instance->submenu);

    view_dispatcher_free(instance->view_dispatcher);
    furi_record_close(RECORD_GUI);

    free(instance);
}

int32_t crash_test_run(CrashTest* instance) {
    view_dispatcher_switch_to_view(instance->view_dispatcher, CrashTestViewSubmenu);
    view_dispatcher_run(instance->view_dispatcher);
    return 0;
}

int32_t crash_test_app(void* p) {
    UNUSED(p);

    CrashTest* instance = crash_test_alloc();

    int32_t ret = crash_test_run(instance);

    crash_test_free(instance);

    return ret;
}
