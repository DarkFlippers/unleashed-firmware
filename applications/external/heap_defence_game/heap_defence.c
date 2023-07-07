//
// Created by moh on 30.11.2021.
//
// Ported to latest firmware by @xMasterX - 18 Oct 2022
//

#include <string.h>

#include "hede_assets.h"
#include "heap_defence_icons.h"

#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <dolphin/dolphin.h>

#define Y_FIELD_SIZE 6
#define Y_LAST (Y_FIELD_SIZE - 1)
#define X_FIELD_SIZE 12
#define X_LAST (X_FIELD_SIZE - 1)

#define DRAW_X_OFFSET 4

#define TAG "HeDe"

#define BOX_HEIGHT 10
#define BOX_WIDTH 10
#define TIMER_UPDATE_FREQ 8
#define BOX_GENERATION_RATE 15

static IconAnimation* BOX_DESTROYED;
static const Icon* boxes[] = {
    (Icon*)&A_HD_BoxDestroyed_10x10,
    &I_Box1_10x10,
    &I_Box2_10x10,
    &I_Box3_10x10,
    &I_Box4_10x10,
    &I_Box5_10x10};

static uint8_t BOX_TEXTURE_COUNT = sizeof(boxes) / sizeof(Icon*);

typedef enum {
    AnimationGameOver = 0,
    AnimationPause,
    AnimationLeft,
    AnimationRight,
} Animations;

static IconAnimation* animations[4];

typedef u_int8_t byte;

typedef enum {
    GameStatusVibro = 1 << 0,
    GameStatusInProgress = 1 << 1,
} GameStatuses;

typedef struct {
    uint8_t x;
    uint8_t y;
} Position;

typedef enum { PlayerRising = 1, PlayerFalling = -1, PlayerNothing = 0 } PlayerStates;

typedef struct {
    Position p;
    int8_t x_direction;
    int8_t j_tick;
    int8_t h_tick;
    int8_t states;
    bool right_frame;
} Person;

typedef struct {
    uint8_t offset : 4;
    uint8_t box_id : 3;
    uint8_t exists : 1;
} Box;

static const uint8_t ROW_BYTE_SIZE = sizeof(Box) * X_FIELD_SIZE;

typedef struct {
    Box** field;
    Person* person;
    Animations animation;
    GameStatuses game_status;
    FuriMutex* mutex;
} GameState;

typedef Box** Field;

typedef enum { EventGameTick, EventKeyPress } EventType;

typedef struct {
    EventType type;
    InputEvent input;
} GameEvent;

/**
 * #Construct / Destroy
 */

static void game_reset_field_and_player(GameState* game) {
    ///Reset field
    bzero(game->field[0], X_FIELD_SIZE * Y_FIELD_SIZE * sizeof(Box));

    ///Reset person
    bzero(game->person, sizeof(Person));
    game->person->p.x = X_FIELD_SIZE / 2;
    game->person->p.y = Y_LAST;
}

static GameState* allocGameState() {
    GameState* game = malloc(sizeof(GameState));

    game->person = malloc(sizeof(Person));

    game->field = malloc(Y_FIELD_SIZE * sizeof(Box*));
    game->field[0] = malloc(X_FIELD_SIZE * Y_FIELD_SIZE * sizeof(Box));
    for(int y = 1; y < Y_FIELD_SIZE; ++y) {
        game->field[y] = game->field[0] + (y * X_FIELD_SIZE);
    }
    game_reset_field_and_player(game);

    game->game_status = GameStatusInProgress;
    return game;
}

static void game_destroy(GameState* game) {
    furi_assert(game);
    free(game->field[0]);
    free(game->field);
    free(game);
}

static void assets_load() {
    /// Init animations
    animations[AnimationPause] = icon_animation_alloc(&A_HD_start_128x64);
    animations[AnimationGameOver] = icon_animation_alloc(&A_HD_game_over_128x64);
    animations[AnimationLeft] = icon_animation_alloc(&A_HD_person_left_10x20);
    animations[AnimationRight] = icon_animation_alloc(&A_HD_person_right_10x20);

    BOX_DESTROYED = icon_animation_alloc(&A_HD_BoxDestroyed_10x10);

    icon_animation_start(animations[AnimationLeft]);
    icon_animation_start(animations[AnimationRight]);
}

static void assets_clear() {
    for(int i = 0; i < 4; ++i) {
        icon_animation_stop(animations[i]);
        icon_animation_free(animations[i]);
    }
    icon_animation_free(BOX_DESTROYED);
}

/**
 * Box utils
 */

static inline bool is_empty(Box* box) {
    return !box->exists;
}

static inline bool has_dropped(Box* box) {
    return box->offset == 0;
}

static Box* get_upper_box(Field field, Position current) {
    return (&field[current.y - 1][current.x]);
}

static Box* get_lower_box(Field field, Position current) {
    return (&field[current.y + 1][current.x]);
}

static Box* get_next_box(Field field, Position current, int x_direction) {
    return (&field[current.y][current.x + x_direction]);
}

static inline void decrement_y_offset_to_zero(Box* n) {
    if(n->offset) --n->offset;
}

static inline void heap_swap(Box* first, Box* second) {
    Box temp = *first;

    *first = *second;
    *second = temp;
}

/**
 * #Box logic
 */

static void generate_box(GameState const* game) {
    furi_assert(game);

    static byte tick_count = BOX_GENERATION_RATE;
    if(tick_count++ != BOX_GENERATION_RATE) {
        return;
    }
    tick_count = 0;

    int x_offset = rand() % X_FIELD_SIZE;
    while(game->field[1][x_offset].exists) {
        x_offset = rand() % X_FIELD_SIZE;
    }

    game->field[1][x_offset].exists = true;
    game->field[1][x_offset].offset = BOX_HEIGHT;
    game->field[1][x_offset].box_id = (rand() % (BOX_TEXTURE_COUNT - 1)) + 1;
}

static void drop_box(GameState* game) {
    furi_assert(game);

    for(int y = Y_LAST; y > 0; y--) {
        for(int x = 0; x < X_FIELD_SIZE; x++) {
            Box* current_box = game->field[y] + x;
            Box* upper_box = game->field[y - 1] + x;

            if(y == Y_LAST) {
                decrement_y_offset_to_zero(current_box);
            }

            decrement_y_offset_to_zero(upper_box);

            if(is_empty(current_box) && !is_empty(upper_box) && has_dropped(upper_box)) {
                upper_box->offset = BOX_HEIGHT;
                heap_swap(current_box, upper_box);
            }
        }
    }
}

static bool clear_rows(Box** field) {
    for(int x = 0; x < X_FIELD_SIZE; ++x) {
        if(is_empty(field[Y_LAST] + x) || !has_dropped(field[Y_LAST] + x)) {
            return false;
        }
    }

    memset(field[Y_LAST], 128, ROW_BYTE_SIZE);
    return true;
}

/**
 * Input Handling
 */

static inline bool on_ground(Person* person, Field field) {
    return person->p.y == Y_LAST || field[person->p.y + 1][person->p.x].exists;
}

static void handle_key_presses(Person* person, InputEvent* input, GameState* game) {
    switch(input->key) {
    case InputKeyUp:
        if(person->states == PlayerNothing && on_ground(person, game->field)) {
            person->states = PlayerRising;
            person->j_tick = 0;
        }
        break;
    case InputKeyLeft:
        person->right_frame = false;
        if(person->h_tick == 0) {
            person->h_tick = 1;
            person->x_direction = -1;
        }
        break;
    case InputKeyRight:
        person->right_frame = true;
        if(person->h_tick == 0) {
            person->h_tick = 1;
            person->x_direction = 1;
        }
        break;
    case InputKeyOk:
        game->game_status &= ~GameStatusInProgress;
        game->animation = AnimationPause;
        icon_animation_start(animations[AnimationPause]);
    default:
        break;
    }
}

/**
 * #Person logic
 */

static inline bool ground_box_check(Field field, Position new_position) {
    Box* lower_box = get_lower_box(field, new_position);

    bool ground_box_dropped =
        (new_position.y == Y_LAST || //Eсли мы и так в самом низу
         is_empty(lower_box) || // Ecли снизу пустота
         has_dropped(lower_box)); //Eсли бокс снизу допадал
    return ground_box_dropped;
}

static inline bool is_movable(Field field, Position box_pos, int x_direction) {
    //TODO::Moжет и не двух, предположение
    bool out_of_bounds = box_pos.x == 0 || box_pos.x == X_LAST;
    if(out_of_bounds) return false;
    bool box_on_top = box_pos.y < 1 || get_upper_box(field, box_pos)->exists;
    if(box_on_top) return false;
    bool has_next_box = get_next_box(field, box_pos, x_direction)->exists;
    if(has_next_box) return false;

    return true;
}

static bool horizontal_move(Person* person, Field field) {
    Position new_position = person->p;

    if(!person->x_direction) return false;

    new_position.x += person->x_direction;

    bool on_edge_column = new_position.x > X_LAST;
    if(on_edge_column) return false;

    if(is_empty(&field[new_position.y][new_position.x])) {
        bool ground_box_dropped = ground_box_check(field, new_position);
        if(ground_box_dropped) {
            person->p = new_position;
            return true;
        }
    } else if(is_movable(field, new_position, person->x_direction)) {
        *get_next_box(field, new_position, person->x_direction) =
            field[new_position.y][new_position.x];

        field[new_position.y][new_position.x] = (Box){0};
        person->p = new_position;
        return true;
    }
    return false;
}

void hd_person_set_state(Person* person, PlayerStates state) {
    person->states = state;
    person->j_tick = 0;
}

static void person_move(Person* person, Field field) {
    /// Left-right logic
    FURI_LOG_W(TAG, "[JUMP]func:[%s] line: %d", __FUNCTION__, __LINE__);

    if(person->states == PlayerNothing) {
        if(!on_ground(person, field)) {
            hd_person_set_state(person, PlayerFalling);
        }
    } else if(person->states == PlayerRising) {
        if(person->j_tick++ == 0) {
            person->p.y--;
        } else if(person->j_tick == 6) {
            hd_person_set_state(person, PlayerNothing);
        }

        /// Destroy upper box
        get_upper_box(field, person->p)->box_id = 0;
        field[person->p.y][person->p.x].box_id = 0;

    } else if(person->states == PlayerFalling) {
        if(person->j_tick++ == 0) {
            if(on_ground(person, field)) { // TODO: Test the bugfix
                hd_person_set_state(person, PlayerNothing);
            } else {
                person->p.y++;
            }
        } else if(person->j_tick == 5) {
            if(on_ground(person, field)) {
                hd_person_set_state(person, PlayerNothing);
            } else {
                hd_person_set_state(person, PlayerFalling);
            }
        }
    }

    switch(person->h_tick) {
    case 0:
        break;
    case 1:
        person->h_tick++;
        FURI_LOG_W(TAG, "[JUMP]func:[%s] line: %d", __FUNCTION__, __LINE__);
        bool moved = horizontal_move(person, field);
        if(!moved) {
            person->h_tick = 0;
            person->x_direction = 0;
        }
        break;
    case 5:
        FURI_LOG_W(TAG, "[JUMP]func:[%s] line: %d", __FUNCTION__, __LINE__);
        person->h_tick = 0;
        person->x_direction = 0;
        break;
    default:
        FURI_LOG_W(TAG, "[JUMP]func:[%s] line: %d", __FUNCTION__, __LINE__);
        person->h_tick++;
    }
}

static inline bool is_person_dead(Person* person, Box** field) {
    return get_upper_box(field, person->p)->box_id != 0;
}

/**
 * #Callback
 */

static void draw_box(Canvas* canvas, Box* box, int x, int y) {
    if(is_empty(box)) {
        return;
    }
    byte y_screen = y * BOX_HEIGHT - box->offset;
    byte x_screen = x * BOX_WIDTH + DRAW_X_OFFSET;

    if(box->box_id == 0) {
        canvas_set_bitmap_mode(canvas, true);
        icon_animation_start(BOX_DESTROYED);
        canvas_draw_icon_animation(canvas, x_screen, y_screen, BOX_DESTROYED);
        if(icon_animation_is_last_frame(BOX_DESTROYED)) {
            *box = (Box){0};
            icon_animation_stop(BOX_DESTROYED);
        }
        canvas_set_bitmap_mode(canvas, false);
    } else {
        canvas_draw_icon(canvas, x_screen, y_screen, boxes[box->box_id]);
    }
}

static void heap_defense_render_callback(Canvas* const canvas, void* mutex) {
    furi_assert(mutex);
    const GameState* game = mutex;
    furi_mutex_acquire(game->mutex, FuriWaitForever);

    ///Draw GameOver or Pause
    if(!(game->game_status & GameStatusInProgress)) {
        FURI_LOG_W(TAG, "[DAED_DRAW]func: [%s] line: %d ", __FUNCTION__, __LINE__);

        canvas_draw_icon_animation(canvas, 0, 0, animations[game->animation]);
        furi_mutex_release(game->mutex);
        return;
    }

    ///Draw field
    canvas_draw_icon(canvas, 0, 0, &I_Background_128x64);

    ///Draw Person
    const Person* person = game->person;
    IconAnimation* player_animation = person->right_frame ? animations[AnimationRight] :
                                                            animations[AnimationLeft];

    uint8_t x_screen = person->p.x * BOX_WIDTH + DRAW_X_OFFSET;
    if(person->h_tick && person->h_tick != 1) {
        if(person->right_frame) {
            x_screen += (person->h_tick) * 2 - BOX_WIDTH;
        } else {
            x_screen -= (person->h_tick) * 2 - BOX_WIDTH;
        }
    }

    uint8_t y_screen = (person->p.y - 1) * BOX_HEIGHT;
    if(person->j_tick) {
        if(person->states == PlayerRising) {
            y_screen += BOX_HEIGHT - (person->j_tick) * 2;
        } else if(person->states == PlayerFalling) {
            y_screen -= BOX_HEIGHT - (person->j_tick) * 2;
        }
    }

    canvas_draw_icon_animation(canvas, x_screen, y_screen, player_animation);

    ///Draw Boxes
    canvas_set_color(canvas, ColorBlack);
    for(int y = 1; y < Y_FIELD_SIZE; ++y) {
        for(int x = 0; x < X_FIELD_SIZE; ++x) {
            draw_box(canvas, &(game->field[y][x]), x, y);
        }
    }

    furi_mutex_release(game->mutex);
}

static void heap_defense_input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    if(input_event->type != InputTypePress && input_event->type != InputTypeLong) return;

    furi_assert(event_queue);
    GameEvent event = {.type = EventKeyPress, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void heap_defense_timer_callback(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    GameEvent event;
    event.type = EventGameTick;
    event.input = (InputEvent){0};
    furi_message_queue_put(event_queue, &event, 0);
}

int32_t heap_defence_app(void* p) {
    UNUSED(p);

    //FURI_LOG_W(TAG, "Heap defence start %d", __LINE__);
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(GameEvent));
    GameState* game = allocGameState();

    game->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    if(!game->mutex) {
        game_destroy(game);
        return 1;
    }

    assets_load();
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, heap_defense_render_callback, game);
    view_port_input_callback_set(view_port, heap_defense_input_callback, event_queue);

    FuriTimer* timer =
        furi_timer_alloc(heap_defense_timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_kernel_get_tick_frequency() / TIMER_UPDATE_FREQ);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);

    memset(game->field[Y_LAST], 128, ROW_BYTE_SIZE);
    game->person->p.y -= 2;
    game->game_status = 0;
    game->animation = AnimationPause;

    // Call dolphin deed on game start
    dolphin_deed(DolphinDeedPluginGameStart);

    GameEvent event = {0};
    while(event.input.key != InputKeyBack) {
        if(furi_message_queue_get(event_queue, &event, 100) != FuriStatusOk) {
            continue;
        }

        furi_mutex_acquire(game->mutex, FuriWaitForever);

        //unset vibration
        if(game->game_status & GameStatusVibro) {
            notification_message(notification, &sequence_reset_vibro);
            game->game_status &= ~GameStatusVibro;
            icon_animation_stop(BOX_DESTROYED);
            memset(game->field[Y_LAST], 0, ROW_BYTE_SIZE);
        }

        if(!(game->game_status & GameStatusInProgress)) {
            if(event.type == EventKeyPress && event.input.key == InputKeyOk) {
                game->game_status |= GameStatusInProgress;
                icon_animation_stop(animations[game->animation]);
            }

        } else if(event.type == EventKeyPress) {
            handle_key_presses(game->person, &(event.input), game);
        } else { // EventGameTick

            drop_box(game);
            generate_box(game);
            if(clear_rows(game->field)) {
                notification_message(notification, &sequence_set_vibro_on);
                icon_animation_start(BOX_DESTROYED);
                game->game_status |= GameStatusVibro;
            }
            person_move(game->person, game->field);

            if(is_person_dead(game->person, game->field)) {
                game->game_status &= ~GameStatusInProgress;
                game->animation = AnimationGameOver;
                icon_animation_start(animations[AnimationGameOver]);
                game_reset_field_and_player(game);
                notification_message(notification, &sequence_error);
            }
        }
        furi_mutex_release(game->mutex);
        view_port_update(view_port);
    }

    furi_timer_free(timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_message_queue_free(event_queue);
    assets_clear();
    furi_mutex_free(game->mutex);
    game_destroy(game);

    return 0;
}
