#include <furi.h>
#include "dolphin_scene/dolphin_scene.h"

void dolphin_scene_redraw(Canvas* canvas, void* ctx) {
    furi_assert(canvas);
    furi_assert(ctx);

    SceneState* state = (SceneState*)acquire_mutex((ValueMutex*)ctx, 25);
    if(state == NULL) return; // redraw fail
    uint32_t t = xTaskGetTickCount();

    canvas_clear(canvas);

    dolphin_scene_render(state, canvas, t);

    dolphin_scene_render_dolphin_state(state, canvas);

    release_mutex((ValueMutex*)ctx, state);
}

void dolphin_scene_handle_input(SceneState* state, InputEvent* input) {
    // printf("[kb] event: %02x %s\n", input->key, input->state ? "pressed" : "released");
    dolphin_scene_handle_user_input(state, input);
}

void dolphin_scene_tick_handler(SceneState* state, uint32_t t, uint32_t dt) {
    // printf("t: %d, dt: %d\n", t, dt);

    dolphin_scene_coordinates(state, dt);
    dolphin_scene_update_dolphin_state(state, t, dt);
}
