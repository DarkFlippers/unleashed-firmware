#pragma once

#include "view_navigator.h"
#include <furi.h>
#include <m-array.h>

ARRAY_DEF(ViewNavSceneArray, AppScene*, M_PTR_OPLIST);

struct ViewNavigator {
    ViewNavSceneArray_t scene_array;
    void* context;
};

bool view_navigator_next_scene(ViewNavigator* view_navigator);
bool view_navigator_previous_scene(ViewNavigator* view_navigator);
bool view_navigator_search_previous_scene(ViewNavigator* view_navigator, uint32_t scene_id);
