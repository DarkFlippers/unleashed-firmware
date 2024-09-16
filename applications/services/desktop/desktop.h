#pragma once

#include <furi.h>

#include "desktop_settings.h"

#define RECORD_DESKTOP "desktop"

typedef struct Desktop Desktop;

typedef struct {
    bool locked;
} DesktopStatus;

bool desktop_api_is_locked(Desktop* instance);

void desktop_api_unlock(Desktop* instance);

FuriPubSub* desktop_api_get_status_pubsub(Desktop* instance);

void desktop_api_get_settings(Desktop* instance, DesktopSettings* settings);

void desktop_api_set_settings(Desktop* instance, const DesktopSettings* settings);

/**
 * @brief Adds 1 to the count of active external autolock inhibitors
 * 
 * Autolocking will not get triggered while there's at least 1 inhibitor
 */
void desktop_api_add_external_inhibitor(Desktop* instance);

/**
 * @brief Removes 1 from the count of active external autolock inhibitors
 * 
 * Autolocking will not get triggered while there's at least 1 inhibitor
 */
void desktop_api_remove_external_inhibitor(Desktop* instance);
