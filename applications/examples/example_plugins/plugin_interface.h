#pragma once

/* Common interface between a plugin and host application */

#define PLUGIN_APP_ID "example_plugins"
#define PLUGIN_API_VERSION 1

typedef struct {
    const char* name;
    int (*method1)();
    int (*method2)(int, int);
} ExamplePlugin;
