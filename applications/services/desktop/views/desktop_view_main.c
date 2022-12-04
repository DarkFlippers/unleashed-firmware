#include <gui/gui_i.h>
#include <gui/view.h>
#include <gui/elements.h>
#include <gui/canvas.h>
#include <furi.h>
#include <input/input.h>
#include <dolphin/dolphin.h>

#include "../desktop_i.h"
#include "desktop_view_main.h"
#include "applications/settings/desktop_settings/desktop_settings_app.h"

struct DesktopMainView {
    View* view;
    DesktopMainViewCallback callback;
    void* context;
    TimerHandle_t poweroff_timer;
    bool is_gamemode;
    bool dummy_mode;
};

#define DESKTOP_MAIN_VIEW_POWEROFF_TIMEOUT 2000

static void desktop_main_poweroff_timer_callback(TimerHandle_t timer) {
    DesktopMainView* main_view = pvTimerGetTimerID(timer);
    main_view->callback(DesktopMainEventOpenPowerOff, main_view->context);
}

void desktop_main_set_callback(
    DesktopMainView* main_view,
    DesktopMainViewCallback callback,
    void* context) {
    furi_assert(main_view);
    furi_assert(callback);
    main_view->callback = callback;
    main_view->context = context;
}

View* desktop_main_get_view(DesktopMainView* main_view) {
    furi_assert(main_view);
    return main_view->view;
}

void desktop_main_set_dummy_mode_state(DesktopMainView* main_view, bool dummy_mode) {
    furi_assert(main_view);
    main_view->dummy_mode = dummy_mode;
}

bool desktop_main_input_callback(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    DesktopMainView* main_view = context;

    // change to only check for game mode setting on keypress
    if(event->type == InputTypeShort || event->type == InputTypeLong) {
        main_view->is_gamemode = false;
        DesktopSettings* desktop_settings = malloc(sizeof(DesktopSettings));
        DESKTOP_SETTINGS_LOAD(desktop_settings);
        if(desktop_settings->is_dumbmode) main_view->is_gamemode = true;
        free(desktop_settings);
    }

    if(main_view->is_gamemode == false && main_view->dummy_mode == false) {
        if(event->type == InputTypeShort) {
            if(event->key == InputKeyOk) {
                main_view->callback(DesktopMainEventOpenMenu, main_view->context);
            } else if(event->key == InputKeyUp) {
                main_view->callback(DesktopMainEventOpenLockMenu, main_view->context);
            } else if(event->key == InputKeyDown) {
                main_view->callback(DesktopMainEventOpenArchive, main_view->context);
            } else if(event->key == InputKeyLeft) {
                main_view->callback(DesktopMainEventOpenClock, main_view->context); // OPENS Clock
            } else if(event->key == InputKeyRight) {
                // Right key is handled by animation manager
                // GOES TO PASSPORT NO MATTER WHAT
                // THIS DOESNT WORK, PASSPORT WILL ONLY OPEN ON REGULAR RIGHT, NOTHING CAN GET ASSIGNED HERE
                main_view->callback(DesktopMainEventOpenPassport, main_view->context);
            }
        } else if(event->type == InputTypeLong) {
            if(event->key == InputKeyOk) {
                main_view->callback(DesktopAnimationEventNewIdleAnimation, main_view->context);
            } else if(event->key == InputKeyUp) {
                main_view->callback(DesktopMainEventOpenFavoritePrimary, main_view->context);
            } else if(event->key == InputKeyDown) {
                main_view->callback(DesktopMainEventOpenFavoriteSecondary, main_view->context);
            } else if(event->key == InputKeyLeft) {
                main_view->callback(
                    DesktopMainEventOpenSubRemote, main_view->context); // OPENS SUBGHZ REMOTE
            }
        }
    } else if(main_view->is_gamemode == true) {
        if(event->type == InputTypeShort) {
            if(event->key == InputKeyOk) {
                main_view->callback(DesktopMainEventOpenDice, main_view->context); // OPENS Dice
            } else if(event->key == InputKeyUp) {
                main_view->callback(DesktopMainEventOpenSnake, main_view->context); // OPENS SNAKE
            } else if(event->key == InputKeyDown) {
                // PREFER TO OPEN GAMES MENU
                main_view->callback(DesktopMainEventOpen2048, main_view->context); // OPENS 2048
            } else if(event->key == InputKeyLeft) {
                main_view->callback(
                    DesktopMainEventOpenTetris, main_view->context); // OPENS TETRIS
            }
        } else if(event->type == InputTypeLong) {
            if(event->key == InputKeyOk) {
                main_view->callback(DesktopAnimationEventNewIdleAnimation, main_view->context);
            } else if(event->key == InputKeyUp) {
                main_view->callback(DesktopMainEventOpenDOOM, main_view->context); // OPENS DOOM
            } else if(event->key == InputKeyDown) {
                main_view->callback(
                    DesktopMainEventOpenZombiez, main_view->context); // OPENS Zombiez
            } else if(event->key == InputKeyLeft) {
                main_view->callback(DesktopMainEventOpenClock, main_view->context); // OPENS CLOCK
            }
        }
    } else {
        if(event->type == InputTypeShort) {
            if(event->key == InputKeyOk) {
                main_view->callback(DesktopMainEventOpenSnake, main_view->context); // OPENS SNAKE
            } else if(event->key == InputKeyUp) {
                main_view->callback(DesktopMainEventOpenLockMenu, main_view->context);
            } else if(event->key == InputKeyDown) {
                main_view->callback(
                    DesktopMainEventOpenTetris, main_view->context); // OPENS Tetris
            } else if(event->key == InputKeyLeft) {
                main_view->callback(
                    DesktopMainEventOpenArkanoid, main_view->context); // OPENS Arkanoid
            }
        } else if(event->type == InputTypeLong) {
            if(event->key == InputKeyOk) {
                main_view->callback(DesktopAnimationEventNewIdleAnimation, main_view->context);
            } else if(event->key == InputKeyUp) {
                main_view->callback(DesktopMainEventOpenDOOM, main_view->context); // OPENS DOOM
            } else if(event->key == InputKeyDown) {
                main_view->callback(
                    DesktopMainEventOpenZombiez, main_view->context); // OPENS Zombiez
            } else if(event->key == InputKeyLeft) {
                main_view->callback(
                    DesktopMainEventOpenHeap, main_view->context); // OPENS Heap Defence
            }
        }
    }

    if(event->key == InputKeyBack) {
        if(event->type == InputTypePress) {
            xTimerChangePeriod(
                main_view->poweroff_timer,
                pdMS_TO_TICKS(DESKTOP_MAIN_VIEW_POWEROFF_TIMEOUT),
                portMAX_DELAY);
        } else if(event->type == InputTypeRelease) {
            xTimerStop(main_view->poweroff_timer, portMAX_DELAY);
        }
    }

    return true;
}

DesktopMainView* desktop_main_alloc() {
    DesktopMainView* main_view = malloc(sizeof(DesktopMainView));
    main_view->is_gamemode = false;
    DesktopSettings* desktop_settings = malloc(sizeof(DesktopSettings));
    DESKTOP_SETTINGS_LOAD(desktop_settings);
    if(desktop_settings->is_dumbmode) main_view->is_gamemode = true;
    free(desktop_settings);

    main_view->view = view_alloc();
    view_allocate_model(main_view->view, ViewModelTypeLockFree, 1);
    view_set_context(main_view->view, main_view);
    view_set_input_callback(main_view->view, desktop_main_input_callback);

    main_view->poweroff_timer = xTimerCreate(
        NULL,
        pdMS_TO_TICKS(DESKTOP_MAIN_VIEW_POWEROFF_TIMEOUT),
        pdFALSE,
        main_view,
        desktop_main_poweroff_timer_callback);

    return main_view;
}

void desktop_main_free(DesktopMainView* main_view) {
    furi_assert(main_view);
    view_free(main_view->view);
    furi_timer_free(main_view->poweroff_timer);
    free(main_view);
}