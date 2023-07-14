#pragma once

#include <gui/view.h>
#include "../helpers/subrem_custom_event.h"
#include "../helpers/subrem_presets.h"

typedef struct SubRemViewEditMenu SubRemViewEditMenu;

typedef void (*SubRemViewEditMenuCallback)(SubRemCustomEvent event, void* context);

void subrem_view_edit_menu_set_callback(
    SubRemViewEditMenu* subrem_view_edit_menu,
    SubRemViewEditMenuCallback callback,
    void* context);

SubRemViewEditMenu* subrem_view_edit_menu_alloc();

void subrem_view_edit_menu_free(SubRemViewEditMenu* subrem_view_edit_menu);

View* subrem_view_edit_menu_get_view(SubRemViewEditMenu* subrem_view_edit_menu);

void subrem_view_edit_menu_add_data_to_show(
    SubRemViewEditMenu* subrem_view_edit_remote,
    uint8_t index,
    FuriString* label,
    FuriString* path,
    SubRemLoadSubState state);

uint8_t subrem_view_edit_menu_get_index(SubRemViewEditMenu* subrem_view_edit_remote);