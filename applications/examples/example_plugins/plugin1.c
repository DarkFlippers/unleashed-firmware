/**
 * @file plugin1.c
 * @brief Plugin example 1.
 *
 * A simple plugin implementing example_plugins application's plugin interface
 */

#include "plugin_interface.h"

#include <flipper_application/flipper_application.h>

static int example_plugin1_method1(void) {
    return 42;
}

static int example_plugin1_method2(int arg1, int arg2) {
    return arg1 + arg2;
}

/* Actual implementation of app<>plugin interface */
static const ExamplePlugin example_plugin1 = {
    .name = "Demo App Plugin 1",
    .method1 = &example_plugin1_method1,
    .method2 = &example_plugin1_method2,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor example_plugin1_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &example_plugin1,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* example_plugin1_ep(void) {
    return &example_plugin1_descriptor;
}
