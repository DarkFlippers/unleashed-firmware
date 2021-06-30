#pragma once

#include "app_scene.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ViewNavigatorEventNext = 0x00000000UL,
    ViewNavigatorEventBack = 0xFFFFFFFFUL,
} ViewNavigatorEvent;

typedef struct ViewNavigator ViewNavigator;

ViewNavigator* view_navigator_alloc(void* context);
void view_navigator_free(ViewNavigator* view_navigator);

bool view_navigator_handle_custom_event(ViewNavigator* view_navigator, uint32_t event);
bool view_navigator_handle_navigation_event(ViewNavigator* view_navigator, uint32_t event);
bool view_navigator_handle_back_search_scene_event(ViewNavigator* view_navigator, uint32_t event);

void view_navigator_add_next_scene(ViewNavigator* view_navigator, AppScene* scene);
void view_navigator_start(ViewNavigator* view_navigator);

#ifdef __cplusplus
}
#endif
