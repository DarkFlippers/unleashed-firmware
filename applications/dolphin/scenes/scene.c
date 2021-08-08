#include <furi.h>
#include <furi-hal.h>
#include "scene.h"

static SceneAppGui* scene_app_gui = NULL;
static ValueMutex* scene_state_mutex = NULL;

void dolphin_scene_redraw(Canvas* canvas, void* ctx) {
    furi_assert(canvas);
    furi_assert(ctx);

    SceneState* state = (SceneState*)acquire_mutex((ValueMutex*)ctx, 25);
    if(state == NULL) return; // redraw fail
    uint32_t t = xTaskGetTickCount();

    canvas_clear(canvas);
    dolphin_scene_render(state, canvas, t);
    dolphin_scene_render_state(state, canvas);
    release_mutex((ValueMutex*)ctx, state);
}

void dolphin_scene_handle_input(SceneState* state, InputEvent* input) {
    // printf("[kb] event: %02x %s\n", input->key, input->state ? "pressed" : "released");
    dolphin_scene_handle_user_input(state, input);
}

void dolphin_scene_tick_handler(SceneState* state, uint32_t t, uint32_t dt) {
    // printf("t: %d, dt: %d\n", t, dt);
    dolphin_scene_coordinates(state, dt);
    dolphin_scene_update_state(state, t, dt);
}

static void scene_engine_tick_callback(void* p) {
    osMessageQueueId_t event_queue = p;
    AppEvent event;
    event.type = EventTypeTick;
    osMessageQueuePut(event_queue, (void*)&event, 0, 0);
}

static void scene_engine_input_callback(InputEvent* input_event, void* ctx) {
    osMessageQueueId_t event_queue = ctx;
    AppEvent event;
    event.type = EventTypeKey;
    event.value.input = *input_event;
    osMessageQueuePut(event_queue, (void*)&event, 0, osWaitForever);
}

void scene_alloc() {
    printf("scene_alloc: start\r\n");
    furi_assert(scene_app_gui == NULL);
    furi_assert(scene_state_mutex == NULL);

    // SceneAppGui
    scene_app_gui = furi_alloc(sizeof(SceneAppGui));
    scene_app_gui->mqueue = osMessageQueueNew(8, sizeof(AppEvent), NULL);
    scene_app_gui->gui = furi_record_open("gui");
    scene_app_gui->view_port = view_port_alloc();
    scene_app_gui->timer =
        osTimerNew(scene_engine_tick_callback, osTimerPeriodic, scene_app_gui->mqueue, NULL);
    printf("scene_alloc: timer %p\r\n", scene_app_gui->timer);
    // Scene State
    SceneState* scene_state = furi_alloc(sizeof(SceneState));
    scene_state->player.y = DOLPHIN_DEFAULT_Y;
    scene_state->player.x = DOLPHIN_CENTER;

    scene_state->player_global.x = 160;
    scene_state->player_global.y = WORLD_HEIGHT;

    scene_state->frame_group = DirRight;
    scene_state->frame_type = DirRight;
    scene_state->frame_pending = DirRight;
    scene_state->last_group = DirRight;

    scene_state->screen.x = scene_state->player.x;
    scene_state->screen.y = scene_state->player.y;
    // scene_state->debug = true;
    scene_state_mutex = furi_alloc(sizeof(ValueMutex));
    furi_check(init_mutex(scene_state_mutex, scene_state, sizeof(SceneState)));

    // Open GUI and register fullscreen view_port
    view_port_draw_callback_set(scene_app_gui->view_port, dolphin_scene_redraw, scene_state_mutex);
    view_port_input_callback_set(
        scene_app_gui->view_port, scene_engine_input_callback, scene_app_gui->mqueue);
    gui_add_view_port(scene_app_gui->gui, scene_app_gui->view_port, GuiLayerFullscreen);
    view_port_enabled_set(scene_app_gui->view_port, true);
    printf("scene_alloc: complete\r\n");
}

void scene_free() {
    printf("scene_free: start\r\n");
    view_port_enabled_set(scene_app_gui->view_port, false);
    gui_remove_view_port(scene_app_gui->gui, scene_app_gui->view_port);

    SceneState* scene_state = (SceneState*)acquire_mutex_block(scene_state_mutex);
    furi_assert(scene_state);
    free(scene_state);
    release_mutex(scene_state_mutex, scene_state);
    delete_mutex(scene_state_mutex);
    free(scene_state_mutex);
    scene_state_mutex = NULL;

    furi_check(osTimerDelete(scene_app_gui->timer) == osOK);
    furi_record_close("gui");
    view_port_free(scene_app_gui->view_port);
    furi_check(osMessageQueueDelete(scene_app_gui->mqueue) == osOK);
    free(scene_app_gui);
    scene_app_gui = NULL;
    printf("scene_free: complete\r\n");
}

int32_t scene_app(void* p) {
    furi_hal_power_insomnia_enter();
    scene_alloc();

    osTimerStart(scene_app_gui->timer, 40);

    uint32_t t = xTaskGetTickCount();
    uint32_t prev_t = 0;

    while(1) {
        AppEvent event;
        if(osMessageQueueGet(scene_app_gui->mqueue, &event, 0, osWaitForever) == osOK) {
            SceneState* scene_state = (SceneState*)acquire_mutex_block(scene_state_mutex);
            if(event.type == EventTypeTick) {
                t = xTaskGetTickCount();
                dolphin_scene_tick_handler(scene_state, t, (t - prev_t) % 1024);
                prev_t = t;
            } else if(event.type == EventTypeKey) {
                if(event.value.input.key == InputKeyBack &&
                   event.value.input.type == InputTypeShort) {
                    release_mutex(scene_state_mutex, scene_state);
                    break;

                } else {
                    dolphin_scene_handle_input(scene_state, &event.value.input);
                }
            }
            release_mutex(scene_state_mutex, scene_state);
            view_port_update(scene_app_gui->view_port);
        }
    }

    osTimerStop(scene_app_gui->timer);

    // CMSIS + FreeRTOS = Enterprise
    osDelay(15);

    scene_free();
    furi_hal_power_insomnia_exit();
    return 0;
}