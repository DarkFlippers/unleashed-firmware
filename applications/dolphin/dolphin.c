#include "dolphin_i.h"
#include <stdlib.h>
#include "applications.h"

const Icon* idle_scenes[] = {&A_Wink_128x64, &A_WatchingTV_128x64};

static void dolphin_switch_to_app(Dolphin* dolphin, const FlipperApplication* flipper_app) {
    furi_assert(dolphin);
    furi_assert(flipper_app);
    furi_assert(flipper_app->app);
    furi_assert(flipper_app->name);

    furi_thread_set_name(dolphin->scene_thread, flipper_app->name);
    furi_thread_set_stack_size(dolphin->scene_thread, flipper_app->stack_size);
    furi_thread_set_callback(dolphin->scene_thread, flipper_app->app);
    furi_thread_start(dolphin->scene_thread);
}

// temporary main screen animation managment
void dolphin_scene_handler_set_scene(Dolphin* dolphin, const Icon* icon_data) {
    with_view_model(
        dolphin->idle_view_main, (DolphinViewMainModel * model) {
            if(model->animation) icon_animation_free(model->animation);
            model->animation = icon_animation_alloc(icon_data);
            icon_animation_start(model->animation);
            return true;
        });
}

void dolphin_scene_handler_switch_scene(Dolphin* dolphin) {
    with_view_model(
        dolphin->idle_view_main, (DolphinViewMainModel * model) {
            if(icon_animation_is_last_frame(model->animation)) {
                if(model->animation) icon_animation_free(model->animation);
                model->animation = icon_animation_alloc(idle_scenes[model->scene_num]);
                icon_animation_start(model->animation);
                model->scene_num = random() % COUNT_OF(idle_scenes);
            }
            return true;
        });
}

bool dolphin_view_first_start_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Dolphin* dolphin = context;
    if(event->type == InputTypeShort) {
        DolphinViewFirstStartModel* model = view_get_model(dolphin->idle_view_first_start);
        if(event->key == InputKeyLeft) {
            if(model->page > 0) model->page--;
        } else if(event->key == InputKeyRight) {
            uint32_t page = ++model->page;
            if(page > 8) {
                dolphin_save(dolphin);
                view_dispatcher_switch_to_view(dolphin->idle_view_dispatcher, DolphinViewIdleMain);
            }
        }
        view_commit_model(dolphin->idle_view_first_start, true);
    }
    // All evennts cosumed
    return true;
}

void dolphin_lock_handler(InputEvent* event, Dolphin* dolphin) {
    furi_assert(event);
    furi_assert(dolphin);

    with_view_model(
        dolphin->idle_view_main, (DolphinViewMainModel * model) {
            model->hint_timeout = HINT_TIMEOUT_L;
            return true;
        });

    if(event->key == InputKeyBack && event->type == InputTypeShort) {
        uint32_t press_time = HAL_GetTick();

        // check if pressed sequentially
        if(press_time - dolphin->lock_lastpress > UNLOCK_RST_TIMEOUT) {
            dolphin->lock_lastpress = press_time;
            dolphin->lock_count = 0;
        } else if(press_time - dolphin->lock_lastpress < UNLOCK_RST_TIMEOUT) {
            dolphin->lock_lastpress = press_time;
            dolphin->lock_count++;
        }

        if(dolphin->lock_count == 2) {
            dolphin->locked = false;
            dolphin->lock_count = 0;

            with_view_model(
                dolphin->view_lockmenu, (DolphinViewLockMenuModel * model) {
                    model->locked = false;
                    model->door_left_x = -57; // move doors to default pos
                    model->door_right_x = 115;
                    return true;
                });

            with_view_model(
                dolphin->idle_view_main, (DolphinViewMainModel * model) {
                    model->hint_timeout = HINT_TIMEOUT_L; // "unlocked" hint timeout
                    model->locked = false;
                    return true;
                });

            view_port_enabled_set(dolphin->lock_viewport, false);
        }
    }
}

bool dolphin_view_idle_main_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Dolphin* dolphin = context;
    // unlocked
    if(!dolphin->locked) {
        if(event->key == InputKeyOk && event->type == InputTypeShort) {
            with_value_mutex(
                dolphin->menu_vm, (Menu * menu) { menu_ok(menu); });
        } else if(event->key == InputKeyUp && event->type == InputTypeShort) {
            osTimerStart(dolphin->timeout_timer, 40);
            view_dispatcher_switch_to_view(dolphin->idle_view_dispatcher, DolphinViewLockMenu);
        } else if(event->key == InputKeyLeft && event->type == InputTypeShort) {
#if 0
            dolphin_switch_to_app(dolphin, &FAV_APP);
#endif
        } else if(event->key == InputKeyRight && event->type == InputTypeShort) {
            dolphin_switch_to_app(dolphin, &FLIPPER_SCENE);
        } else if(event->key == InputKeyDown && event->type == InputTypeShort) {
            dolphin_switch_to_app(dolphin, &FLIPPER_ARCHIVE);
        } else if(event->key == InputKeyDown && event->type == InputTypeLong) {
            view_dispatcher_switch_to_view(dolphin->idle_view_dispatcher, DolphinViewStats);
        } else if(event->key == InputKeyBack && event->type == InputTypeShort) {
            view_dispatcher_switch_to_view(dolphin->idle_view_dispatcher, DolphinViewIdleMain);
        }

        with_view_model(
            dolphin->idle_view_main, (DolphinViewMainModel * model) {
                model->hint_timeout = 0; // clear hint timeout
                return true;
            });

    } else {
        // locked

        dolphin_lock_handler(event, dolphin);
        dolphin_scene_handler_switch_scene(dolphin);
    }
    // All events consumed
    return true;
}

void lock_menu_refresh_handler(void* p) {
    osMessageQueueId_t event_queue = p;
    DolphinEvent event;
    event.type = DolphinEventTypeTick;
    // Some tick events may lost and we don't care.
    osMessageQueuePut(event_queue, &event, 0, 0);
}

static void lock_menu_callback(void* context, uint8_t index) {
    furi_assert(context);
    Dolphin* dolphin = context;
    switch(index) {
    // lock
    case 0:
        dolphin->locked = true;

        with_view_model(
            dolphin->view_lockmenu, (DolphinViewLockMenuModel * model) {
                model->locked = true;
                model->exit_timeout = HINT_TIMEOUT_H;
                return true;
            });

        with_view_model(
            dolphin->idle_view_main, (DolphinViewMainModel * model) {
                model->locked = true;
                return true;
            });
        break;

    default:
        break;
    }
}

static void lock_icon_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    Dolphin* dolphin = context;
    canvas_draw_icon_animation(canvas, 0, 0, dolphin->lock_icon);
}

bool dolphin_view_lockmenu_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Dolphin* dolphin = context;

    if(event->type != InputTypeShort) return false;

    DolphinViewLockMenuModel* model = view_get_model(dolphin->view_lockmenu);

    if(event->key == InputKeyUp) {
        model->idx = CLAMP(model->idx - 1, 2, 0);
    } else if(event->key == InputKeyDown) {
        model->idx = CLAMP(model->idx + 1, 2, 0);
    } else if(event->key == InputKeyOk) {
        lock_menu_callback(context, model->idx);
    } else if(event->key == InputKeyBack) {
        model->idx = 0;
        view_dispatcher_switch_to_view(dolphin->idle_view_dispatcher, DolphinViewIdleMain);

        if(random() % 100 > 50)
            dolphin_scene_handler_set_scene(
                dolphin, idle_scenes[random() % COUNT_OF(idle_scenes)]);
    }

    view_commit_model(dolphin->view_lockmenu, true);

    return true;
}

bool dolphin_view_idle_down_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Dolphin* dolphin = context;
    DolphinViewStatsScreens current;

    if(event->type != InputTypeShort) return false;

    DolphinViewStatsModel* model = view_get_model(dolphin->idle_view_dolphin_stats);

    current = model->screen;

    if(event->key == InputKeyDown) {
        model->screen = (model->screen + 1) % DolphinViewStatsTotalCount;
    } else if(event->key == InputKeyUp) {
        model->screen =
            ((model->screen - 1) + DolphinViewStatsTotalCount) % DolphinViewStatsTotalCount;
    }

    view_commit_model(dolphin->idle_view_dolphin_stats, true);

    if(current == DolphinViewStatsMeta) {
        if(event->key == InputKeyLeft) {
            dolphin_deed(dolphin, DolphinDeedWrong);
        } else if(event->key == InputKeyRight) {
            dolphin_deed(dolphin, DolphinDeedIButtonRead);
        } else if(event->key == InputKeyOk) {
            dolphin_save(dolphin);
        } else {
            return false;
        }
    }

    if(event->key == InputKeyBack) {
        view_dispatcher_switch_to_view(dolphin->idle_view_dispatcher, DolphinViewIdleMain);
    }

    return true;
}

Dolphin* dolphin_alloc() {
    Dolphin* dolphin = furi_alloc(sizeof(Dolphin));
    // Message queue
    dolphin->event_queue = osMessageQueueNew(8, sizeof(DolphinEvent), NULL);
    furi_check(dolphin->event_queue);
    // State
    dolphin->state = dolphin_state_alloc();
    // Menu
    dolphin->menu_vm = furi_record_open("menu");
    // Scene thread
    dolphin->scene_thread = furi_thread_alloc();
    // GUI
    dolphin->gui = furi_record_open("gui");
    // Dispatcher
    dolphin->idle_view_dispatcher = view_dispatcher_alloc();

    // First start View
    dolphin->idle_view_first_start = view_alloc();
    view_allocate_model(
        dolphin->idle_view_first_start, ViewModelTypeLockFree, sizeof(DolphinViewFirstStartModel));
    view_set_context(dolphin->idle_view_first_start, dolphin);
    view_set_draw_callback(dolphin->idle_view_first_start, dolphin_view_first_start_draw);
    view_set_input_callback(dolphin->idle_view_first_start, dolphin_view_first_start_input);
    view_dispatcher_add_view(
        dolphin->idle_view_dispatcher, DolphinViewFirstStart, dolphin->idle_view_first_start);

    // Main Idle View
    dolphin->idle_view_main = view_alloc();
    view_set_context(dolphin->idle_view_main, dolphin);
    view_allocate_model(
        dolphin->idle_view_main, ViewModelTypeLockFree, sizeof(DolphinViewMainModel));

    view_set_draw_callback(dolphin->idle_view_main, dolphin_view_idle_main_draw);
    view_set_input_callback(dolphin->idle_view_main, dolphin_view_idle_main_input);
    view_dispatcher_add_view(
        dolphin->idle_view_dispatcher, DolphinViewIdleMain, dolphin->idle_view_main);

    // Lock Menu View
    dolphin->view_lockmenu = view_alloc();
    view_set_context(dolphin->view_lockmenu, dolphin);
    view_allocate_model(
        dolphin->view_lockmenu, ViewModelTypeLockFree, sizeof(DolphinViewLockMenuModel));
    view_set_draw_callback(dolphin->view_lockmenu, dolphin_view_lockmenu_draw);
    view_set_input_callback(dolphin->view_lockmenu, dolphin_view_lockmenu_input);
    view_set_previous_callback(dolphin->view_lockmenu, dolphin_view_idle_back);
    view_dispatcher_add_view(
        dolphin->idle_view_dispatcher, DolphinViewLockMenu, dolphin->view_lockmenu);

    // default doors xpos
    with_view_model(
        dolphin->view_lockmenu, (DolphinViewLockMenuModel * model) {
            model->door_left_x = -57; // defaults
            model->door_right_x = 115; // defaults
            return true;
        });

    dolphin->timeout_timer =
        osTimerNew(lock_menu_refresh_handler, osTimerPeriodic, dolphin->event_queue, NULL);

    // Stats Idle View
    dolphin->idle_view_dolphin_stats = view_alloc();
    view_set_context(dolphin->idle_view_dolphin_stats, dolphin);
    view_allocate_model(
        dolphin->idle_view_dolphin_stats, ViewModelTypeLockFree, sizeof(DolphinViewStatsModel));
    view_set_draw_callback(dolphin->idle_view_dolphin_stats, dolphin_view_idle_down_draw);
    view_set_input_callback(dolphin->idle_view_dolphin_stats, dolphin_view_idle_down_input);
    view_set_previous_callback(dolphin->idle_view_dolphin_stats, dolphin_view_idle_back);
    view_dispatcher_add_view(
        dolphin->idle_view_dispatcher, DolphinViewStats, dolphin->idle_view_dolphin_stats);
    // HW Mismatch
    dolphin->view_hw_mismatch = view_alloc();
    view_set_draw_callback(dolphin->view_hw_mismatch, dolphin_view_hw_mismatch_draw);
    view_set_previous_callback(dolphin->view_hw_mismatch, dolphin_view_idle_back);
    view_dispatcher_add_view(
        dolphin->idle_view_dispatcher, DolphinViewHwMismatch, dolphin->view_hw_mismatch);

    // Lock icon
    dolphin->lock_icon = icon_animation_alloc(&I_Lock_8x8);
    dolphin->lock_viewport = view_port_alloc();
    view_port_set_width(dolphin->lock_viewport, icon_animation_get_width(dolphin->lock_icon));
    view_port_draw_callback_set(dolphin->lock_viewport, lock_icon_callback, dolphin);
    view_port_enabled_set(dolphin->lock_viewport, false);

    // Main screen animation
    dolphin_scene_handler_set_scene(dolphin, idle_scenes[random() % COUNT_OF(idle_scenes)]);

    view_dispatcher_attach_to_gui(
        dolphin->idle_view_dispatcher, dolphin->gui, ViewDispatcherTypeWindow);
    gui_add_view_port(dolphin->gui, dolphin->lock_viewport, GuiLayerStatusBarLeft);

    return dolphin;
}

void dolphin_free(Dolphin* dolphin) {
    furi_assert(dolphin);

    gui_remove_view_port(dolphin->gui, dolphin->lock_viewport);
    view_port_free(dolphin->lock_viewport);
    icon_animation_free(dolphin->lock_icon);

    osTimerDelete(dolphin->timeout_timer);

    view_dispatcher_free(dolphin->idle_view_dispatcher);

    furi_record_close("gui");
    dolphin->gui = NULL;

    furi_thread_free(dolphin->scene_thread);

    furi_record_close("menu");
    dolphin->menu_vm = NULL;

    dolphin_state_free(dolphin->state);

    osMessageQueueDelete(dolphin->event_queue);

    free(dolphin);
}

void dolphin_save(Dolphin* dolphin) {
    furi_assert(dolphin);
    DolphinEvent event;
    event.type = DolphinEventTypeSave;
    furi_check(osMessageQueuePut(dolphin->event_queue, &event, 0, osWaitForever) == osOK);
}

void dolphin_deed(Dolphin* dolphin, DolphinDeed deed) {
    furi_assert(dolphin);
    DolphinEvent event;
    event.type = DolphinEventTypeDeed;
    event.deed = deed;
    furi_check(osMessageQueuePut(dolphin->event_queue, &event, 0, osWaitForever) == osOK);
}

int32_t dolphin_task() {
    Dolphin* dolphin = dolphin_alloc();

    if(dolphin_state_load(dolphin->state)) {
        view_dispatcher_switch_to_view(dolphin->idle_view_dispatcher, DolphinViewIdleMain);
    } else {
        view_dispatcher_switch_to_view(dolphin->idle_view_dispatcher, DolphinViewFirstStart);
    }

    with_view_model(
        dolphin->idle_view_dolphin_stats, (DolphinViewStatsModel * model) {
            model->icounter = dolphin_state_get_icounter(dolphin->state);
            model->butthurt = dolphin_state_get_butthurt(dolphin->state);
            return true;
        });

    furi_record_create("dolphin", dolphin);

    if(!api_hal_version_do_i_belong_here()) {
        view_dispatcher_switch_to_view(dolphin->idle_view_dispatcher, DolphinViewHwMismatch);
    }

    DolphinEvent event;
    while(1) {
        furi_check(osMessageQueueGet(dolphin->event_queue, &event, NULL, osWaitForever) == osOK);

        DolphinViewLockMenuModel* lock_model = view_get_model(dolphin->view_lockmenu);

        if(lock_model->locked && lock_model->exit_timeout == 0 &&
           osTimerIsRunning(dolphin->timeout_timer)) {
            osTimerStop(dolphin->timeout_timer);
            osDelay(1); // smol enterprise delay
            view_dispatcher_switch_to_view(dolphin->idle_view_dispatcher, DolphinViewIdleMain);
        }

        if(event.type == DolphinEventTypeTick) {
            view_commit_model(dolphin->view_lockmenu, true);

        } else if(event.type == DolphinEventTypeDeed) {
            dolphin_state_on_deed(dolphin->state, event.deed);
            with_view_model(
                dolphin->idle_view_dolphin_stats, (DolphinViewStatsModel * model) {
                    model->icounter = dolphin_state_get_icounter(dolphin->state);
                    model->butthurt = dolphin_state_get_butthurt(dolphin->state);
                    return true;
                });
        } else if(event.type == DolphinEventTypeSave) {
            dolphin_state_save(dolphin->state);
        }
    }
    dolphin_free(dolphin);
    return 0;
}
