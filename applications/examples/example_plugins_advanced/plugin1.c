/**
 * @file plugin1.c
 * @brief Plugin example 1.
 *
 * This plugin uses both firmware's API interface and private application headers.
 * It can be loaded by a plugin manager that uses CompoundApiInterface,
 * which combines both interfaces.
 */

#include "app_api.h"
#include "plugin_interface.h"

#include <flipper_application/flipper_application.h>
#include <furi.h>

static void advanced_plugin1_method1(int arg1) {
    /* This function is implemented inside host application */
    app_api_accumulator_add(arg1);
}

static void advanced_plugin1_method2(void) {
    /* Accumulator value is stored inside host application */
    FURI_LOG_I("TEST", "Plugin 1, accumulator: %lu", app_api_accumulator_get());
}

/* Actual implementation of app<>plugin interface */
static const AdvancedPlugin advanced_plugin1 = {
    .name = "Advanced Plugin 1",
    .method1 = &advanced_plugin1_method1,
    .method2 = &advanced_plugin1_method2,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor advanced_plugin1_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &advanced_plugin1,
};

/* Plugin entry point - must return a pointer to const descriptor */
const FlipperAppPluginDescriptor* advanced_plugin1_ep(void) {
    return &advanced_plugin1_descriptor;
}
