#pragma once
#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LoaderMenu LoaderMenu;

LoaderMenu* loader_menu_alloc();

void loader_menu_free(LoaderMenu* loader_menu);

void loader_menu_start(LoaderMenu* loader_menu);

void loader_menu_stop(LoaderMenu* loader_menu);

void loader_menu_set_closed_callback(
    LoaderMenu* loader_menu,
    void (*callback)(void*),
    void* context);

void loader_menu_set_click_callback(
    LoaderMenu* loader_menu,
    void (*callback)(const char*, void*),
    void* context);

#ifdef __cplusplus
}
#endif