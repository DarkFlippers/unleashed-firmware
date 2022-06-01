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

    const FlipperApplication* application;
    FuriThread* application_thread;
    char* application_arguments;

    Cli* cli;
    Gui* gui;

    ViewDispatcher* view_dispatcher;
    Menu* primary_menu;
    Submenu* plugins_menu;
    Submenu* games_menu;
    Submenu* debug_menu;
    Submenu* settings_menu;

    volatile uint8_t lock_count;

    FuriPubSub* pubsub;
};

typedef enum {
    LoaderMenuViewPrimary,
    LoaderMenuViewPlugins,
    LoaderMenuViewGames,
    LoaderMenuViewDebug,
    LoaderMenuViewSettings,
} LoaderMenuView;
