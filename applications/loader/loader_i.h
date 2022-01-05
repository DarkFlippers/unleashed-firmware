#include "loader.h"

#include <furi.h>
#include <furi_hal.h>
#include <furi/pubsub.h>
#include <cli/cli.h>
#include <lib/toolbox/args.h>

#include <gui/view_dispatcher.h>

#include <gui/modules/menu.h>
#include <gui/modules/submenu.h>

#include <applications.h>
#include <assets_icons.h>

struct Loader {
    osThreadId_t loader_thread;
    FuriThread* thread;
    const FlipperApplication* current_app;
    string_t args;
    Cli* cli;
    Gui* gui;

    ViewDispatcher* view_dispatcher;
    Menu* primary_menu;
    Submenu* plugins_menu;
    Submenu* debug_menu;
    Submenu* settings_menu;

    size_t free_heap_size;
    osMutexId_t mutex;
    volatile uint8_t lock_semaphore;

    FuriPubSub* pubsub;
};

typedef enum {
    LoaderMenuViewPrimary,
    LoaderMenuViewPlugins,
    LoaderMenuViewDebug,
    LoaderMenuViewSettings,
} LoaderMenuView;
