#pragma once
#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LoaderMenu LoaderMenu;

LoaderMenu* loader_menu_alloc(void (*closed_cb)(void*), void* context);

void loader_menu_free(LoaderMenu* loader_menu);

#ifdef __cplusplus
}
#endif
