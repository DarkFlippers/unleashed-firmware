/**
 * @file plugin2.c
 * @brief Plugin example 2.
 *
 * Second plugin implementing example_plugins application's plugin interface
 */

#include "plugin_interface.h"

#include <flipper_application/flipper_application.h>

static int example_plugin2_method1(void) {
    return 1337;
}

static int example_plugin2_method2(int arg1, int arg2) {
    return arg1 - arg2;
}

/* Actual implementation of app<>plugin interface */
static const ExamplePlugin example_plugin2 = {
    .name = "Demo App Plugin 2",
    .method1 = &example_plugin2_method1,
    .method2 = &example_plugin2_method2,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor example_plugin2_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &example_plugin2,
};

/* Plugin entry point - must return a pointer to const descriptor */
const FlipperAppPluginDescriptor* example_plugin2_ep(void) {
    return &example_plugin2_descriptor;
}
