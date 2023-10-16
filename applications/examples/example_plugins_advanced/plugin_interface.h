#pragma once

/* Common interface between a plugin and host application */

#define PLUGIN_APP_ID "example_plugins_advanced"
#define PLUGIN_API_VERSION 1

typedef struct {
    const char* name;
    void (*method1)(int);
    void (*method2)();
} AdvancedPlugin;
