#include <furi.h>
#include "dolphin_scene/dolphin_scene.h"

void dolphin_engine_tick_cb(void* p) {
    osMessageQueueId_t event_queue = p;
    AppEvent tick_event;
    tick_event.type = EventTypeTick;
    osMessageQueuePut(event_queue, (void*)&tick_event, 0, 0);
}

static void dolphin_engine_event_cb(InputEvent* input_event, void* ctx) {
    osMessageQueueId_t event_queue = ctx;

    AppEvent event;
    event.type = EventTypeKey;
    event.value.input = *input_event;
    osMessageQueuePut(event_queue, (void*)&event, 0, osWaitForever);
}

ValueMutex* scene_init() {
    SceneState* scene_state = furi_alloc(sizeof(SceneState));
    scene_state->ui.mqueue = osMessageQueueNew(2, sizeof(AppEvent), NULL);

    scene_state->player.y = DOLPHIN_DEFAULT_Y;
    scene_state->player.x = DOLPHIN_CENTER;

    //randomize position
    scene_state->player_global.x = random() % WORLD_WIDTH / 4;

    scene_state->screen.x = scene_state->player.x;
    scene_state->screen.y = scene_state->player.y;

    ValueMutex* scene_state_mutex = furi_alloc(sizeof(ValueMutex));
    if(scene_state_mutex == NULL ||
       !init_mutex(scene_state_mutex, scene_state, sizeof(SceneState))) {
        printf("[menu_task] cannot create menu mutex\r\n");
        furi_check(0);
    }

    // Open GUI and register view_port
    scene_state->ui.gui = furi_record_open("gui");

    // Allocate and configure view_port
    scene_state->ui.view_port = view_port_alloc();

    // Open GUI and register fullscreen view_port
    gui_add_view_port(scene_state->ui.gui, scene_state->ui.view_port, GuiLayerMain);
    view_port_draw_callback_set(
        scene_state->ui.view_port, dolphin_scene_redraw, scene_state_mutex);
    view_port_input_callback_set(
        scene_state->ui.view_port, dolphin_engine_event_cb, scene_state->ui.mqueue);
    view_port_enabled_set(scene_state->ui.view_port, true);

    scene_state->ui.timer =
        osTimerNew(dolphin_engine_tick_cb, osTimerPeriodic, scene_state->ui.mqueue, NULL);

    return scene_state_mutex;
}

void scene_free(ValueMutex* scene_state_mutex) {
    furi_assert(scene_state_mutex);

    SceneState* scene_state = (SceneState*)acquire_mutex_block(scene_state_mutex);

    osTimerDelete(scene_state->ui.timer);
    gui_remove_view_port(scene_state->ui.gui, scene_state->ui.view_port);
    view_port_free(scene_state->ui.view_port);
    furi_record_close("gui");
    osMessageQueueDelete(scene_state->ui.mqueue);
    free(scene_state);

    release_mutex(scene_state_mutex, scene_state);
    delete_mutex(scene_state_mutex);
    free(scene_state_mutex);
}

int32_t dolphin_scene(void* p) {
    ValueMutex* scene_state_mutex = scene_init();

    furi_record_create("scene", scene_state_mutex);

    SceneState* _state = (SceneState*)acquire_mutex_block(scene_state_mutex);
    osTimerStart(_state->ui.timer, 40);
    uint32_t t = xTaskGetTickCount();
    uint32_t prev_t = 0;
    osMessageQueueId_t q = _state->ui.mqueue;
    release_mutex(scene_state_mutex, _state);

    while(1) {
        AppEvent event;
        if(osMessageQueueGet(q, &event, 0, osWaitForever) == osOK) {
            SceneState* _state = (SceneState*)acquire_mutex_block(scene_state_mutex);
            if(event.type == EventTypeTick) {
                t = xTaskGetTickCount();
                dolphin_scene_tick_handler(_state, t, (t - prev_t) % 1024);
                prev_t = t;
            } else if(event.type == EventTypeKey) {
                if(event.value.input.key == InputKeyBack &&
                   event.value.input.type == InputTypeShort) {
                    release_mutex(scene_state_mutex, _state);
                    break;

                } else {
                    dolphin_scene_handle_input(_state, &event.value.input);
                }
            }
            release_mutex(scene_state_mutex, _state);
            view_port_update(_state->ui.view_port);
        }
    }

    scene_free(scene_state_mutex);

    return 0;
}
