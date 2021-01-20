#pragma once

#include <furi.h>
#include <assets_icons.h>

typedef void (*FlipperApplication)(void*);

typedef struct {
    const FlipperApplication app;
    const char* name;
    const IconName icon;
} FuriApplication;

extern const FuriApplication FLIPPER_SERVICES[];
size_t FLIPPER_SERVICES_size();

extern const FuriApplication FLIPPER_APPS[];
size_t FLIPPER_APPS_size();

extern const FuriApplication FLIPPER_PLUGINS[];
size_t FLIPPER_PLUGINS_size();
