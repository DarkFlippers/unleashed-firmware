#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <dolphin/dolphin.h>

//ORIGINAL REPO: https://github.com/Dooskington/flipperzero-zombiez
//AUTHORS: https://github.com/Dooskington | https://github.com/DevMilanIan

#include "zombiez.h"

#define ZOMBIES_MAX 3
#define ZOMBIES_WIDTH 5
#define ZOMBIES_HEIGHT 8
#define PROJECTILES_MAX 10

#define MIN_Y 5
#define MAX_Y 58
#define WALL_X 16
#define PLAYER_START_X 8
#define PLAYER_START_Y (MAX_Y - MIN_Y) / 2

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef enum { GameStatePlaying, GameStateGameOver } GameState;

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point position;
    int hp;
} Player;

typedef struct {
    Point position;
    int hp;
} Zombie;

typedef struct {
    Point position;
} Projectile;

typedef struct {
    GameState game_state;
    Player player;

    size_t zombies_count;
    Zombie* zombies[ZOMBIES_MAX];

    size_t projectiles_count;
    Projectile* projectiles[PROJECTILES_MAX];

    uint16_t score;
    bool input_shoot;
} PluginState;

static void render_callback(Canvas* const canvas, void* ctx) {
    const PluginState* plugin_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(plugin_state == NULL) {
        return;
    }

    canvas_draw_frame(canvas, 0, 0, 128, 64);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(
        canvas,
        plugin_state->player.position.x,
        plugin_state->player.position.y,
        AlignCenter,
        AlignCenter,
        "@");

    canvas_draw_line(canvas, WALL_X, 0, WALL_X, 64);
    canvas_draw_line(canvas, WALL_X + 2, 4, WALL_X + 2, 59);

    for(int i = 0; i < PROJECTILES_MAX; ++i) {
        Projectile* p = plugin_state->projectiles[i];
        if(p != NULL) {
            canvas_draw_disc(canvas, p->position.x, p->position.y, 3);
        }
    }

    for(int i = 0; i < ZOMBIES_MAX; ++i) {
        Zombie* z = plugin_state->zombies[i];
        if(z != NULL) {
            for(int h = 0; h < ZOMBIES_HEIGHT; h++) {
                for(int w = 0; w < ZOMBIES_WIDTH; w++) {
                    // Switch animation
                    int zIdx = 0;
                    if(z->position.x % 2 == 0) {
                        zIdx = 1;
                    }

                    // Draw zombie pixels
                    if(zombie_array[zIdx][h][w] == 1) {
                        int x = z->position.x + w;
                        int y = z->position.y + h;

                        canvas_draw_dot(canvas, x, y);
                    }
                }
            }
        }
    }

    int heart;
    if((plugin_state->player.hp - 10) > 5) { // 16, 17, 18, 19, 20
        heart = 0;
    } else if((plugin_state->player.hp - 5) > 5) { // 11, 12, 13, 14, 15
        heart = 1;
    } else if((plugin_state->player.hp - 3) > 2) { // 6, 7, 8, 9, 10
        heart = 2;
    } else if(plugin_state->player.hp > 0) { // 1, 2, 3, 4, 5
        heart = 3;
    } else { // 0
        heart = 4;
    }
    // visual representation of health
    for(int h = 0; h < 5; h++) {
        for(int w = 0; w < 5; w++) {
            if(heart_array[heart][h][w] == 1) {
                int x = 124 - w;
                int y = 56 + h;

                canvas_draw_dot(canvas, x, y);
            }
        }
    }

    // buffer hp + score
    char hpBuffer[8];
    char scoreBuffer[14];

    if(plugin_state->game_state == GameStatePlaying) {
        // display ammo / reload
        if(plugin_state->projectiles_count >= PROJECTILES_MAX) {
            canvas_draw_str_aligned(canvas, 24, 10, AlignLeft, AlignCenter, "RELOAD");
        } else {
            for(uint8_t i = 0; i < (PROJECTILES_MAX - plugin_state->projectiles_count); i++) {
                canvas_draw_box(canvas, 24 + (4 * i), 6, 2, 4);
            }
        }
        // display hp + score
        snprintf(hpBuffer, sizeof(hpBuffer), "%u", plugin_state->player.hp);
        canvas_draw_str_aligned(canvas, 118, 62, AlignRight, AlignBottom, hpBuffer);

        snprintf(scoreBuffer, sizeof(scoreBuffer), "%u", plugin_state->score);
        canvas_draw_str_aligned(canvas, 126, 10, AlignRight, AlignBottom, scoreBuffer);
    }
    // Game Over banner
    if(plugin_state->game_state == GameStateGameOver) {
        // Screen is 128x64 px
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 34, 20, 62, 24);

        canvas_set_color(canvas, ColorBlack);
        canvas_draw_frame(canvas, 34, 20, 62, 24);

        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 37, 31, "Game Over");

        canvas_set_font(canvas, FontSecondary);
        snprintf(scoreBuffer, sizeof(scoreBuffer), "Score: %u", plugin_state->score);
        canvas_draw_str_aligned(canvas, 64, 41, AlignCenter, AlignBottom, scoreBuffer);
    }

    //char* info = (char*)malloc(16 * sizeof(char));
    //asprintf(&info, "%d, %d", plugin_state->x, plugin_state->y);
    //canvas_draw_str_aligned(canvas, 32, 16, AlignLeft, AlignBottom, info);
    //free(info);

    release_mutex((ValueMutex*)ctx, plugin_state);
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);
    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void tick(PluginState* const plugin_state) {
    if(plugin_state->input_shoot && (plugin_state->projectiles_count < PROJECTILES_MAX)) {
        Projectile* p = (Projectile*)malloc(sizeof(Projectile));
        p->position.x = plugin_state->player.position.x;
        p->position.y = plugin_state->player.position.y;

        size_t idx = plugin_state->projectiles_count;
        plugin_state->projectiles[idx] = p;
        plugin_state->projectiles_count += 1;
    }

    for(int i = 0; i < ZOMBIES_MAX; ++i) {
        if(!plugin_state->zombies[i]) {
            Zombie* z = (Zombie*)malloc(sizeof(Zombie));
            //z->hp = 20;
            z->position.x = 126;
            z->position.y = MIN_Y + (rand() % (MAX_Y - MIN_Y));

            plugin_state->zombies[i] = z;
            plugin_state->zombies_count += 1;
        }
    }

    for(int i = 0; i < PROJECTILES_MAX; ++i) {
        Projectile* p = plugin_state->projectiles[i];
        if(p != NULL) {
            p->position.x += 2;

            for(int i = 0; i < ZOMBIES_MAX; ++i) {
                Zombie* z = plugin_state->zombies[i];
                if(z != NULL) {
                    if( // projectile close enough to zombie
                        (((z->position.x - p->position.x) <= 2) &&
                         ((z->position.y - p->position.y) <= 4)) &&
                        (((p->position.x - z->position.x) <= 2) &&
                         ((p->position.y - z->position.y) <= 6))) {
                        //z->hp -= 5;
                        //if(z->hp <= 0) {
                        plugin_state->zombies_count -= 1;
                        free(z);
                        plugin_state->zombies[i] = NULL;
                        plugin_state->score++;
                        //if(plugin_state->score % 15 == 0) DOLPHIN_DEED(getRandomDeed());
                        //}
                    } else if(z->position.x <= WALL_X && z->position.x > 0) { // zombie got to the wall
                        plugin_state->zombies_count -= 1;
                        free(z);
                        plugin_state->zombies[i] = NULL;
                        if(plugin_state->player.hp > 0) {
                            plugin_state->player.hp--;
                        } else {
                            plugin_state->game_state = GameStateGameOver;
                        }
                    } else {
                        if(furi_get_tick() % 2 == 0) z->position.x--;
                    }
                }
            }

            if(p->position.x >= 128) {
                free(p);
                plugin_state->projectiles[i] = NULL;
            }
        }
    }

    plugin_state->input_shoot = false;
}

static void timer_callback(void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    PluginEvent event = {.type = EventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
}

static void zombiez_state_init(PluginState* const plugin_state) {
    plugin_state->player.position.x = PLAYER_START_X;
    plugin_state->player.position.y = PLAYER_START_Y;
    plugin_state->player.hp = 20;

    plugin_state->projectiles_count = 0;
    plugin_state->zombies_count = 0;
    plugin_state->score = 0;

    for(int i = 0; i < PROJECTILES_MAX; i++) {
        plugin_state->projectiles[i] = NULL;
    }

    for(int i = 0; i < ZOMBIES_MAX; i++) {
        plugin_state->zombies[i] = NULL;
    }

    plugin_state->game_state = GameStatePlaying;
    plugin_state->input_shoot = false;
}

int32_t zombiez_game_app(void* p) {
    UNUSED(p);
    uint32_t return_code = 0;
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));

    PluginState* plugin_state = malloc(sizeof(PluginState));
    zombiez_state_init(plugin_state);

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, plugin_state, sizeof(PluginState))) {
        FURI_LOG_E("Zombiez", "cannot create mutex\r\n");
        return_code = 255;
        goto free_and_exit;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    FuriTimer* timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_kernel_get_tick_frequency() / 22);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Call dolphin deed on game start
    DOLPHIN_DEED(DolphinDeedPluginGameStart);

    PluginEvent event;
    bool isRunning = true;
    while(isRunning) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        PluginState* plugin_state = (PluginState*)acquire_mutex_block(&state_mutex);
        if(event_status == FuriStatusOk) {
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        if(plugin_state->player.position.y > MIN_Y &&
                           plugin_state->game_state == GameStatePlaying) {
                            plugin_state->player.position.y--;
                        }
                        break;
                    case InputKeyDown:
                        if(plugin_state->player.position.y < MAX_Y &&
                           plugin_state->game_state == GameStatePlaying) {
                            plugin_state->player.position.y++;
                        }
                        break;
                    case InputKeyOk:
                        if(plugin_state->projectiles_count < PROJECTILES_MAX &&
                           plugin_state->game_state == GameStatePlaying) {
                            plugin_state->input_shoot = true;
                        }
                        break;
                    case InputKeyBack:
                        break;
                    default:
                        break;
                    }
                } else if(
                    event.input.type == InputTypeRepeat &&
                    plugin_state->game_state == GameStatePlaying) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        if(plugin_state->player.position.y > (MIN_Y + 1)) {
                            plugin_state->player.position.y -= 4;
                        }
                        break;
                    case InputKeyDown:
                        if(plugin_state->player.position.y < (MAX_Y - 1)) {
                            plugin_state->player.position.y += 4;
                        }
                        break;
                    default:
                        break;
                    }
                } else if(event.input.type == InputTypeLong) {
                    if(event.input.key == InputKeyOk) {
                        if(plugin_state->game_state == GameStateGameOver) {
                            zombiez_state_init(plugin_state);
                        } else if(plugin_state->projectiles_count >= PROJECTILES_MAX) {
                            plugin_state->projectiles_count = 0;
                            plugin_state->player.hp++;
                        }
                    } else if(event.input.key == InputKeyBack) {
                        isRunning = false;
                    }
                }
            } else if(event.type == EventTypeTick) {
                tick(plugin_state);
            }
        } else {
            // event timeout
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, plugin_state);
    }

    furi_timer_free(timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    delete_mutex(&state_mutex);

free_and_exit:
    free(plugin_state);
    furi_message_queue_free(event_queue);

    return return_code;
}