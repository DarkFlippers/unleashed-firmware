#include "dolphin_i.h"
#include <stdlib.h>
#include "applications.h"

static void
dolphin_switch_to_interactive_scene(Dolphin* dolphin, const FlipperApplication* flipper_app) {
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
void dolphin_scene_handler_set_scene(Dolphin* dolphin, IconName icon) {
    with_view_model(
        dolphin->idle_view_main, (DolphinViewMainModel * model) {
            model->animation = assets_icons_get(icon);
            icon_start_animation(model->animation);
            return true;
        });
}

void dolphin_scene_handler_switch_scene(Dolphin* dolphin) {
    with_view_model(
        dolphin->idle_view_main, (DolphinViewMainModel * model) {
            if(icon_is_last_frame(model->animation)) {
                model->animation = assets_icons_get(idle_scenes[model->scene_num]);
                icon_start_animation(model->animation);
                model->scene_num = random() % sizeof(idle_scenes);
            }
            return true;
        });
}

bool dolphin_view_first_start_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Dolphin* dolphin = context;
    if(event->type == InputTypeShort) {
        if(event->key == InputKeyLeft) {
            with_view_model(
                dolphin->idle_view_first_start, (DolphinViewFirstStartModel * model) {
                    if(model->page > 0) model->page--;
                    return true;
                });
        } else if(event->key == InputKeyRight) {
            uint32_t page;
            with_view_model(
                dolphin->idle_view_first_start, (DolphinViewFirstStartModel * model) {
                    page = ++model->page;
                    return true;
                });
            if(page > 8) {
                dolphin_save(dolphin);
                view_dispatcher_switch_to_view(dolphin->idle_view_dispatcher, DolphinViewIdleMain);
            }
        }
    }
    // All events consumed
    return true;
}

bool dolphin_view_idle_main_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Dolphin* dolphin = context;

    if(event->type == InputTypeShort) {
        if(!dolphin->locked) {
            if(event->key == InputKeyOk) {
                with_value_mutex(
                    dolphin->menu_vm, (Menu * menu) { menu_ok(menu); });
            } else if(event->key == InputKeyUp) {
                view_dispatcher_switch_to_view(dolphin->idle_view_dispatcher, DolphinViewLockMenu);
            } else if(event->key == InputKeyLeft) {
                view_dispatcher_switch_to_view(dolphin->idle_view_dispatcher, DolphinViewIdleUp);
            } else if(event->key == InputKeyRight) {
                dolphin_switch_to_interactive_scene(dolphin, &FLIPPER_SCENES);
            } else if(event->key == InputKeyDown) {
                view_dispatcher_switch_to_view(dolphin->idle_view_dispatcher, DolphinViewIdleDown);
            }
            if(event->key == InputKeyBack) {
                view_dispatcher_switch_to_view(dolphin->idle_view_dispatcher, DolphinViewIdleMain);
            }
        } else {
            if(event->key == InputKeyBack) {
                dolphin->lock_count++;
                if(dolphin->lock_count == 3) {
                    dolphin->locked = false;
                    dolphin->lock_count = 0;
                    view_dispatcher_switch_to_view(
                        dolphin->idle_view_dispatcher, DolphinViewIdleMain);
                    view_port_enabled_set(dolphin->lock_viewport, false);
                }
            }
        }
        dolphin_scene_handler_switch_scene(dolphin);
    }

    // All events consumed
    return true;
}

bool dolphin_view_idle_up_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Dolphin* dolphin = context;

    if(event->type != InputTypeShort) return false;

    if(event->key == InputKeyLeft) {
        dolphin_deed(dolphin, DolphinDeedWrong);
    } else if(event->key == InputKeyRight) {
        dolphin_deed(dolphin, DolphinDeedIButtonRead);
    } else if(event->key == InputKeyOk) {
        dolphin_save(dolphin);
    } else {
        return false;
    }

    return true;
}

static void lock_menu_callback(void* context, uint8_t index) {
    Dolphin* dolphin = context;
    switch(index) {
    case 0:
        dolphin->locked = true;
        view_dispatcher_switch_to_view(dolphin->idle_view_dispatcher, DolphinViewIdleMain);
        view_port_enabled_set(dolphin->lock_viewport, true);
        break;
    default:
        break;
    }
}

static void lock_icon_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    Dolphin* dolphin = context;
    canvas_draw_icon(canvas, 0, 0, dolphin->lock_icon);
}

bool dolphin_view_lockmenu_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Dolphin* dolphin = context;

    if(event->type != InputTypeShort) return false;

    if(event->key == InputKeyUp) {
        with_view_model(
            dolphin->view_lockmenu, (DolphinViewMenuModel * model) {
                if(model->idx <= 0)
                    model->idx = 0;
                else
                    --model->idx;
                return true;
            });
    } else if(event->key == InputKeyDown) {
        with_view_model(
            dolphin->view_lockmenu, (DolphinViewMenuModel * model) {
                if(model->idx >= 2)
                    model->idx = 2;
                else
                    ++model->idx;
                return true;
            });
    } else if(event->key == InputKeyOk) {
        with_view_model(
            dolphin->view_lockmenu, (DolphinViewMenuModel * model) {
                lock_menu_callback(context, model->idx);
                return true;
            });
    } else if(event->key == InputKeyBack) {
        with_view_model(
            dolphin->view_lockmenu, (DolphinViewMenuModel * model) {
                model->idx = 0;
                return true;
            });
        view_dispatcher_switch_to_view(dolphin->idle_view_dispatcher, DolphinViewIdleMain);

        if(random() % 100 > 50)
            dolphin_scene_handler_set_scene(dolphin, idle_scenes[random() % sizeof(idle_scenes)]);
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
        dolphin->idle_view_main, ViewModelTypeLockFree, sizeof(DolphinViewIdleUpModel));

    view_set_draw_callback(dolphin->idle_view_main, dolphin_view_idle_main_draw);
    view_set_input_callback(dolphin->idle_view_main, dolphin_view_idle_main_input);
    view_dispatcher_add_view(
        dolphin->idle_view_dispatcher, DolphinViewIdleMain, dolphin->idle_view_main);

    // Stats Idle View
    dolphin->idle_view_up = view_alloc();
    view_set_context(dolphin->idle_view_up, dolphin);

    view_allocate_model(
        dolphin->idle_view_up, ViewModelTypeLockFree, sizeof(DolphinViewMainModel));
    view_set_draw_callback(dolphin->idle_view_up, dolphin_view_idle_up_draw);
    view_set_input_callback(dolphin->idle_view_up, dolphin_view_idle_up_input);
    view_set_previous_callback(dolphin->idle_view_up, dolphin_view_idle_back);
    view_dispatcher_add_view(
        dolphin->idle_view_dispatcher, DolphinViewIdleUp, dolphin->idle_view_up);

    // Lock Menu View
    dolphin->view_lockmenu = view_alloc();
    view_set_context(dolphin->view_lockmenu, dolphin);
    view_allocate_model(
        dolphin->view_lockmenu, ViewModelTypeLockFree, sizeof(DolphinViewMenuModel));
    view_set_draw_callback(dolphin->view_lockmenu, dolphin_view_lockmenu_draw);
    view_set_input_callback(dolphin->view_lockmenu, dolphin_view_lockmenu_input);
    view_set_previous_callback(dolphin->view_lockmenu, dolphin_view_idle_back);
    view_dispatcher_add_view(
        dolphin->idle_view_dispatcher, DolphinViewLockMenu, dolphin->view_lockmenu);

    // Down Idle View
    dolphin->idle_view_down = view_alloc();
    view_set_draw_callback(dolphin->idle_view_down, dolphin_view_idle_down_draw);
    view_set_previous_callback(dolphin->idle_view_down, dolphin_view_idle_back);
    view_dispatcher_add_view(
        dolphin->idle_view_dispatcher, DolphinViewIdleDown, dolphin->idle_view_down);

    // HW Mismatch
    dolphin->view_hw_mismatch = view_alloc();
    view_set_draw_callback(dolphin->view_hw_mismatch, dolphin_view_hw_mismatch_draw);
    view_set_previous_callback(dolphin->view_hw_mismatch, dolphin_view_idle_back);
    view_dispatcher_add_view(
        dolphin->idle_view_dispatcher, DolphinViewHwMismatch, dolphin->view_hw_mismatch);

    // Lock icon
    dolphin->lock_icon = assets_icons_get(I_Lock_8x8);
    dolphin->lock_viewport = view_port_alloc();
    view_port_set_width(dolphin->lock_viewport, icon_get_width(dolphin->lock_icon));
    view_port_draw_callback_set(dolphin->lock_viewport, lock_icon_callback, dolphin);
    view_port_enabled_set(dolphin->lock_viewport, false);

    // Main screen animation
    dolphin_scene_handler_set_scene(dolphin, idle_scenes[random() % sizeof(idle_scenes)]);

    view_dispatcher_attach_to_gui(
        dolphin->idle_view_dispatcher, dolphin->gui, ViewDispatcherTypeWindow);
    gui_add_view_port(dolphin->gui, dolphin->lock_viewport, GuiLayerStatusBarLeft);

    return dolphin;
}

void dolphin_free(Dolphin* dolphin) {
    furi_assert(dolphin);

    gui_remove_view_port(dolphin->gui, dolphin->lock_viewport);
    view_port_free(dolphin->lock_viewport);
    icon_free(dolphin->lock_icon);

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
        dolphin->idle_view_up, (DolphinViewIdleUpModel * model) {
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
        if(event.type == DolphinEventTypeDeed) {
            dolphin_state_on_deed(dolphin->state, event.deed);
            with_view_model(
                dolphin->idle_view_up, (DolphinViewIdleUpModel * model) {
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
