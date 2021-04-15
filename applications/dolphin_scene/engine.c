#include <furi.h>
#include <api-hal.h>
#include "dolphin_scene/dolphin_scene.h"

static SceneAppGui* scene_app_gui = NULL;
static ValueMutex* scene_state_mutex = NULL;

static void dolphin_engine_tick_callback(void* p) {
    osMessageQueueId_t event_queue = p;
    AppEvent event;
    event.type = EventTypeTick;
    osMessageQueuePut(event_queue, (void*)&event, 0, 0);
}

static void dolphin_engine_input_callback(InputEvent* input_event, void* ctx) {
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
    scene_app_gui->mqueue = osMessageQueueNew(2, sizeof(AppEvent), NULL);
    scene_app_gui->gui = furi_record_open("gui");
    scene_app_gui->view_port = view_port_alloc();
    scene_app_gui->timer =
        osTimerNew(dolphin_engine_tick_callback, osTimerPeriodic, scene_app_gui->mqueue, NULL);
    printf("scene_alloc: timer %p\r\n", scene_app_gui->timer);
    // Scene State
    SceneState* scene_state = furi_alloc(sizeof(SceneState));
    scene_state->player.y = DOLPHIN_DEFAULT_Y;
    scene_state->player.x = DOLPHIN_CENTER;
    scene_state->player_global.x = random() % WORLD_WIDTH / 4;
    scene_state->screen.x = scene_state->player.x;
    scene_state->screen.y = scene_state->player.y;

    scene_state_mutex = furi_alloc(sizeof(ValueMutex));
    furi_check(init_mutex(scene_state_mutex, scene_state, sizeof(SceneState)));

    // Open GUI and register fullscreen view_port
    view_port_draw_callback_set(scene_app_gui->view_port, dolphin_scene_redraw, scene_state_mutex);
    view_port_input_callback_set(
        scene_app_gui->view_port, dolphin_engine_input_callback, scene_app_gui->mqueue);
    gui_add_view_port(scene_app_gui->gui, scene_app_gui->view_port, GuiLayerMain);
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

int32_t dolphin_scene(void* p) {
    api_hal_power_insomnia_enter();
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
    scene_free();
    api_hal_power_insomnia_exit();
    return 0;
}
