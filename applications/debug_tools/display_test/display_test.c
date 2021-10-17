#include "display_test.h"

#include <furi-hal.h>
#include <furi.h>

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/variable-item-list.h>

#include "view_display_test.h"

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    ViewDisplayTest* view_display_test;
    VariableItemList* variable_item_list;
    Submenu* submenu;
} DisplayTest;

typedef enum {
    DisplayTestViewSubmenu,
    DisplayTestViewConfigure,
    DisplayTestViewDisplayTest,
} DisplayTestView;

static void display_test_submenu_callback(void* context, uint32_t index) {
    DisplayTest* instance = (DisplayTest*)context;

    view_dispatcher_switch_to_view(instance->view_dispatcher, index);
}

static uint32_t display_test_previous_callback(void* context) {
    return DisplayTestViewSubmenu;
}

static uint32_t display_test_exit_callback(void* context) {
    return VIEW_NONE;
}

DisplayTest* display_test_alloc() {
    DisplayTest* instance = furi_alloc(sizeof(DisplayTest));

    View* view = NULL;

    instance->gui = furi_record_open("gui");
    instance->view_dispatcher = view_dispatcher_alloc();

    instance->view_display_test = view_display_test_alloc();
    view_dispatcher_enable_queue(instance->view_dispatcher);
    view_dispatcher_attach_to_gui(
        instance->view_dispatcher, instance->gui, ViewDispatcherTypeFullscreen);
    view = view_display_test_get_view(instance->view_display_test);
    view_set_previous_callback(view, display_test_previous_callback);
    view_dispatcher_add_view(instance->view_dispatcher, DisplayTestViewDisplayTest, view);

    instance->submenu = submenu_alloc();
    view = submenu_get_view(instance->submenu);
    view_set_previous_callback(view, display_test_exit_callback);
    view_dispatcher_add_view(instance->view_dispatcher, DisplayTestViewSubmenu, view);
    submenu_add_item(
        instance->submenu,
        "Test",
        DisplayTestViewDisplayTest,
        display_test_submenu_callback,
        instance);
    // submenu_add_item(instance->submenu, "Configure", DisplayTestViewConfigure, display_test_submenu_callback, instance);

    return instance;
}

void display_test_free(DisplayTest* instance) {
    view_dispatcher_remove_view(instance->view_dispatcher, DisplayTestViewSubmenu);
    submenu_free(instance->submenu);

    view_dispatcher_remove_view(instance->view_dispatcher, DisplayTestViewDisplayTest);
    view_display_test_free(instance->view_display_test);

    view_dispatcher_free(instance->view_dispatcher);
    furi_record_close("gui");

    free(instance);
}

int32_t display_test_run(DisplayTest* instance) {
    view_dispatcher_switch_to_view(instance->view_dispatcher, DisplayTestViewSubmenu);
    view_dispatcher_run(instance->view_dispatcher);

    return 0;
}

int32_t display_test_app(void* p) {
    DisplayTest* instance = display_test_alloc();

    int32_t ret = display_test_run(instance);

    display_test_free(instance);

    return ret;
}