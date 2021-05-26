#include <furi.h>
#include <cli/cli.h>
#include "menu/menu.h"
#include "menu/menu_item.h"
#include "applications.h"
#include <assets_icons.h>
#include <api-hal.h>

/**
 * Start application
 * @param name - application name
 * @param args - application arguments
 * @retval true on success
 */
bool app_loader_start(const char* name, const char* args);
