#include "view_navigator_i.h"

ViewNavigator* view_navigator_alloc(void* context) {
    furi_assert(context);

    ViewNavigator* view_navigator = furi_alloc(sizeof(ViewNavigator));
    view_navigator->context = context;
    ViewNavSceneArray_init(view_navigator->scene_array);

    return view_navigator;
}

void view_navigator_free(ViewNavigator* view_navigator) {
    furi_assert(view_navigator);
    ViewNavSceneArray_clear(view_navigator->scene_array);

    free(view_navigator);
}

bool view_navigator_handle_custom_event(ViewNavigator* view_navigator, uint32_t event) {
    AppScene* scene = *ViewNavSceneArray_back(view_navigator->scene_array);
    return scene->on_event(view_navigator->context, event);
}

bool view_navigator_handle_navigation_event(ViewNavigator* view_navigator, uint32_t event) {
    if(event == ViewNavigatorEventNext) {
        return view_navigator_next_scene(view_navigator);
    } else if(event == ViewNavigatorEventBack) {
        AppScene* scene = *ViewNavSceneArray_back(view_navigator->scene_array);
        if(scene->on_event(view_navigator->context, ViewNavigatorEventBack)) {
            return true;
        } else {
            return view_navigator_previous_scene(view_navigator);
        }
    }
    return false;
}

bool view_navigator_handle_back_search_scene_event(ViewNavigator* view_navigator, uint32_t event) {
    return view_navigator_search_previous_scene(view_navigator, event);
}

void view_navigator_add_next_scene(ViewNavigator* view_navigator, AppScene* scene) {
    furi_assert(view_navigator);
    furi_assert(scene);

    ViewNavSceneArray_push_back(view_navigator->scene_array, scene);
}

void view_navigator_start(ViewNavigator* view_navigator) {
    furi_assert(view_navigator);
    AppScene* scene = *ViewNavSceneArray_front(view_navigator->scene_array);
    furi_assert(scene);
    scene->on_enter(view_navigator->context);
}

bool view_navigator_next_scene(ViewNavigator* view_navigator) {
    ViewNavSceneArray_it_t scene_it;
    ViewNavSceneArray_it_last(scene_it, view_navigator->scene_array);
    ViewNavSceneArray_previous(scene_it);
    AppScene* current_scene = *ViewNavSceneArray_ref(scene_it);
    AppScene* next_scene = *ViewNavSceneArray_back(view_navigator->scene_array);
    if(current_scene && next_scene) {
        current_scene->on_exit(view_navigator->context);
        next_scene->on_enter(view_navigator->context);
        return true;
    }
    return false;
}

bool view_navigator_previous_scene(ViewNavigator* view_navigator) {
    AppScene* current_scene = NULL;
    ViewNavSceneArray_pop_back(&current_scene, view_navigator->scene_array);
    if(ViewNavSceneArray_size(view_navigator->scene_array) == 0) {
        // Handle exit from start scene separately
        current_scene->on_exit(view_navigator->context);
        return false;
    }
    AppScene* previous_scene = *ViewNavSceneArray_back(view_navigator->scene_array);
    if(current_scene && previous_scene) {
        current_scene->on_exit(view_navigator->context);
        previous_scene->on_enter(view_navigator->context);
        return true;
    }
    return false;
}

bool view_navigator_search_previous_scene(ViewNavigator* view_navigator, uint32_t scene_id) {
    AppScene* previous_scene = NULL;
    AppScene* current_scene = *ViewNavSceneArray_back(view_navigator->scene_array);
    ViewNavSceneArray_it_t scene_it;
    ViewNavSceneArray_it_last(scene_it, view_navigator->scene_array);
    bool scene_found = false;
    while(!scene_found) {
        ViewNavSceneArray_previous(scene_it);
        previous_scene = *ViewNavSceneArray_ref(scene_it);
        if(previous_scene == NULL) {
            return false;
        }
        if(previous_scene->id == scene_id) {
            scene_found = true;
        }
    }
    ViewNavSceneArray_next(scene_it);
    ViewNavSceneArray_pop_until(view_navigator->scene_array, scene_it);
    current_scene->on_exit(view_navigator->context);
    previous_scene->on_enter(view_navigator->context);
    return true;
}
