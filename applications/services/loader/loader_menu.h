#pragma once
#include <furi.h>
#include <gui/modules/menu.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LoaderMenu LoaderMenu;

LoaderMenu* loader_menu_alloc(void (*closed_cb)(void*), void* context, const MenuStyle* style);

void loader_menu_free(LoaderMenu* loader_menu);

void loader_menu_set_style(LoaderMenu* loader_menu, const MenuStyle* style);

#ifdef __cplusplus
}
#endif
