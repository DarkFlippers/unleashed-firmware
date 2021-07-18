#include "loader.h"

#include <furi.h>
#include <api-hal.h>
#include <cli/cli.h>
#include <menu/menu.h>
#include <menu/menu_item.h>
#include <applications.h>
#include <assets_icons.h>

#define LOADER_LOG_TAG "loader"

struct Loader {
    FuriThread* thread;
    const FlipperApplication* current_app;
    string_t args;
    Cli* cli;
    ValueMutex* menu_vm;
    size_t free_heap_size;
    osMutexId_t mutex;
    volatile uint8_t lock_semaphore;
};
