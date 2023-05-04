#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/menu.h>
#include <gui/modules/submenu.h>
#include <assets_icons.h>
#include <applications.h>

#include "loader_menu.h"

#define TAG "LoaderMenu"

struct LoaderMenu {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Menu* primary_menu;
    Submenu* settings_menu;

    void (*closed_callback)(void*);
    void* closed_callback_context;

    void (*click_callback)(const char*, void*);
    void* click_callback_context;

    FuriThread* thread;
};

typedef enum {
    LoaderMenuViewPrimary,
    LoaderMenuViewSettings,
} LoaderMenuView;

static int32_t loader_menu_thread(void* p);

LoaderMenu* loader_menu_alloc() {
    LoaderMenu* loader_menu = malloc(sizeof(LoaderMenu));
    loader_menu->gui = furi_record_open(RECORD_GUI);
    loader_menu->view_dispatcher = view_dispatcher_alloc();
    loader_menu->primary_menu = menu_alloc();
    loader_menu->settings_menu = submenu_alloc();
    loader_menu->thread = NULL;
    return loader_menu;
}

void loader_menu_free(LoaderMenu* loader_menu) {
    furi_assert(loader_menu);
    // check if thread is running
    furi_assert(!loader_menu->thread);

    submenu_free(loader_menu->settings_menu);
    menu_free(loader_menu->primary_menu);
    view_dispatcher_free(loader_menu->view_dispatcher);
    furi_record_close(RECORD_GUI);
    free(loader_menu);
}

void loader_menu_start(LoaderMenu* loader_menu) {
    furi_assert(loader_menu);
    furi_assert(!loader_menu->thread);
    loader_menu->thread = furi_thread_alloc_ex(TAG, 1024, loader_menu_thread, loader_menu);
    furi_thread_start(loader_menu->thread);
}

void loader_menu_stop(LoaderMenu* loader_menu) {
    furi_assert(loader_menu);
    furi_assert(loader_menu->thread);
    view_dispatcher_stop(loader_menu->view_dispatcher);
    furi_thread_join(loader_menu->thread);
    furi_thread_free(loader_menu->thread);
    loader_menu->thread = NULL;
}

void loader_menu_set_closed_callback(
    LoaderMenu* loader_menu,
    void (*callback)(void*),
    void* context) {
    loader_menu->closed_callback = callback;
    loader_menu->closed_callback_context = context;
}

void loader_menu_set_click_callback(
    LoaderMenu* loader_menu,
    void (*callback)(const char*, void*),
    void* context) {
    loader_menu->click_callback = callback;
    loader_menu->click_callback_context = context;
}

static void loader_menu_callback(void* context, uint32_t index) {
    LoaderMenu* loader_menu = context;
    const char* name = FLIPPER_APPS[index].name;
    if(loader_menu->click_callback) {
        loader_menu->click_callback(name, loader_menu->click_callback_context);
    }
}

static void loader_menu_settings_menu_callback(void* context, uint32_t index) {
    LoaderMenu* loader_menu = context;
    const char* name = FLIPPER_SETTINGS_APPS[index].name;
    if(loader_menu->click_callback) {
        loader_menu->click_callback(name, loader_menu->click_callback_context);
    }
}

static void loader_menu_switch_to_settings(void* context, uint32_t index) {
    UNUSED(index);
    LoaderMenu* loader_menu = context;
    view_dispatcher_switch_to_view(loader_menu->view_dispatcher, LoaderMenuViewSettings);
}

static uint32_t loader_menu_switch_to_primary(void* context) {
    UNUSED(context);
    return LoaderMenuViewPrimary;
}

static uint32_t loader_menu_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

static void loader_menu_build_menu(LoaderMenu* loader_menu) {
    size_t i;
    for(i = 0; i < FLIPPER_APPS_COUNT; i++) {
        menu_add_item(
            loader_menu->primary_menu,
            FLIPPER_APPS[i].name,
            FLIPPER_APPS[i].icon,
            i,
            loader_menu_callback,
            (void*)loader_menu);
    }
    menu_add_item(
        loader_menu->primary_menu,
        "Settings",
        &A_Settings_14,
        i++,
        loader_menu_switch_to_settings,
        loader_menu);
};

static void loader_menu_build_submenu(LoaderMenu* loader_menu) {
    for(size_t i = 0; i < FLIPPER_SETTINGS_APPS_COUNT; i++) {
        submenu_add_item(
            loader_menu->settings_menu,
            FLIPPER_SETTINGS_APPS[i].name,
            i,
            loader_menu_settings_menu_callback,
            loader_menu);
    }
}

static int32_t loader_menu_thread(void* p) {
    LoaderMenu* loader_menu = p;
    furi_assert(loader_menu);

    loader_menu_build_menu(loader_menu);
    loader_menu_build_submenu(loader_menu);

    view_dispatcher_attach_to_gui(
        loader_menu->view_dispatcher, loader_menu->gui, ViewDispatcherTypeFullscreen);

    // Primary menu
    View* primary_view = menu_get_view(loader_menu->primary_menu);
    view_set_context(primary_view, loader_menu->primary_menu);
    view_set_previous_callback(primary_view, loader_menu_exit);
    view_dispatcher_add_view(loader_menu->view_dispatcher, LoaderMenuViewPrimary, primary_view);

    // Settings menu
    View* settings_view = submenu_get_view(loader_menu->settings_menu);
    view_set_context(settings_view, loader_menu->settings_menu);
    view_set_previous_callback(settings_view, loader_menu_switch_to_primary);
    view_dispatcher_add_view(loader_menu->view_dispatcher, LoaderMenuViewSettings, settings_view);

    view_dispatcher_enable_queue(loader_menu->view_dispatcher);
    view_dispatcher_switch_to_view(loader_menu->view_dispatcher, LoaderMenuViewPrimary);

    // run view dispatcher
    view_dispatcher_run(loader_menu->view_dispatcher);

    view_dispatcher_remove_view(loader_menu->view_dispatcher, LoaderMenuViewPrimary);
    view_dispatcher_remove_view(loader_menu->view_dispatcher, LoaderMenuViewSettings);

    if(loader_menu->closed_callback) {
        loader_menu->closed_callback(loader_menu->closed_callback_context);
    }

    return 0;
}