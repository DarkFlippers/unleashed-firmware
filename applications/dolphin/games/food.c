#include <furi.h>
#include <gui/gui.h>
#include "dolphin/dolphin_state.h"

#define MAX_TRIES 3
#define DISHES_TOTAL 3
#define LID_POS_MAX 20
#define TRY_TIMEOUT 10

typedef enum {
    EventTypeTick,
    EventTypeKey,
    EventTypeDeed,
} EventType;

typedef struct {
    union {
        InputEvent input;
    } value;
    EventType type;
} AppEvent;

typedef enum {
    PlayerChoiceEvent,
    OpenLootEvent,
    WinEvent,
    LooseEvent,
    FinishedEvent,
    ExitGameEvent,
    GameEventTotal,
} GameEventType;

typedef enum {
    LootSkeleton,
    LootFish,
    LootShit,
    LootTotalNum,
} LootIdEnum;

typedef struct {
    GameEventType current_event;
    osMessageQueueId_t event_queue;
    LootIdEnum loot_list[DISHES_TOTAL];

    uint8_t cursor_pos;
    uint8_t lid_pos;
    uint8_t timeout;
    uint8_t try;

    bool selected;
    bool deed;

} GameState;

typedef struct {
    const Icon* f;
    const Icon* b;
} LootGfx;

static const Icon* letters[DISHES_TOTAL] = {&I_letterA_10x10, &I_letterB_10x10, &I_letterC_10x10};

static const LootGfx loot[LootTotalNum] = {
    [LootSkeleton] =
        {
            .f = &I_skeleton_25x17,
            .b = &I_blackskeleton_25x17,
        },
    [LootFish] =
        {
            .f = &I_fish_25x17,
            .b = &I_blackfish_25x17,
        },
    [LootShit] =
        {
            .f = &I_shit_25x17,
            .b = &I_blackshit_25x17,
        },
};

static void input_callback(InputEvent* input_event, void* ctx) {
    osMessageQueueId_t event_queue = ctx;
    AppEvent event;
    event.type = EventTypeKey;
    event.value.input = *input_event;
    osMessageQueuePut(event_queue, &event, 0, osWaitForever);
}

static void draw_dish(Canvas* canvas, GameState* state, uint8_t x, uint8_t y, uint8_t id) {
    bool active = state->cursor_pos == id;
    bool opened = state->current_event == OpenLootEvent && active;

    canvas_set_bitmap_mode(canvas, true);
    canvas_set_color(canvas, ColorBlack);

    if(active) {
        canvas_draw_icon(canvas, x, y, &I_active_plate_48x18);
    }

    if(opened) {
        state->lid_pos = CLAMP(state->lid_pos + 1, LID_POS_MAX, 0);
    }

    uint8_t lid_pos = (y - 17) - (opened ? state->lid_pos : 0);

    canvas_draw_icon(canvas, x + 3, y - 6, &I_plate_42x19);

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_icon(canvas, x + 11, y - 10, loot[state->loot_list[id]].b);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_icon(canvas, x + 11, y - 10, loot[state->loot_list[id]].f);

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_icon(canvas, x + 6, lid_pos, &I_blacklid_36x27);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_icon(canvas, x + 6, lid_pos, &I_lid_36x27);
    canvas_set_bitmap_mode(canvas, false);

    canvas_draw_icon(canvas, x + 19, y + 8, letters[id]);
}

static void draw_dishes_scene(Canvas* canvas, GameState* state) {
    uint8_t tries_left = MAX_TRIES - state->try;
    for(size_t i = 0; i < MAX_TRIES; i++) {
        if(i < tries_left) {
            canvas_draw_disc(canvas, 5 + i * 8, 5, 2);
        } else {
            canvas_draw_circle(canvas, 5 + i * 8, 5, 2);
        }
    }

    for(size_t i = 0; i < DISHES_TOTAL; i++) {
        draw_dish(canvas, state, i * 40, i % 2 ? 26 : 44, i);
    }
}

static void render_callback(Canvas* canvas, void* ctx) {
    GameState* state = (GameState*)acquire_mutex((ValueMutex*)ctx, 25);
    canvas_clear(canvas);

    switch(state->current_event) {
    case WinEvent:
        canvas_draw_str(canvas, 30, 30, "Dolphin_happy.png");
        break;
    case LooseEvent:
        canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignCenter, "Try again!");
        break;
    case ExitGameEvent:
        break;
    case FinishedEvent:
        break;
    default:
        draw_dishes_scene(canvas, state);
        break;
    }

    release_mutex((ValueMutex*)ctx, state);
}
static void reset_lid_pos(GameState* state) {
    state->selected = false;
    state->lid_pos = 0;
}

void dolphin_food_deed(GameState* state) {
    furi_assert(state);
    AppEvent event;
    event.type = EventTypeDeed;
    furi_check(osMessageQueuePut(state->event_queue, &event, 0, osWaitForever) == osOK);
}

static void reset_loot_array(GameState* state) {
    for(size_t i = 0; i < LootTotalNum; i++) {
        state->loot_list[i] = i;
    }

    for(size_t i = 0; i < LootTotalNum; i++) {
        int temp = state->loot_list[i];
        int r_idx = rand() % LootTotalNum;

        state->loot_list[i] = state->loot_list[r_idx];
        state->loot_list[r_idx] = temp;
    }
}

static bool selected_is_food(GameState* state) {
    return state->loot_list[state->cursor_pos] == LootFish;
}

static bool tries_exceed(GameState* state) {
    return state->try == MAX_TRIES;
}

static bool timeout_exceed(GameState* state) {
    return state->timeout == TRY_TIMEOUT;
}

static void gamestate_update(GameState* state, DolphinState* dolphin_state) {
    switch(state->current_event) {
    case PlayerChoiceEvent:
        if(state->selected) {
            state->current_event = OpenLootEvent;
        }
        break;
    case OpenLootEvent:
        state->timeout = CLAMP(state->timeout + 1, TRY_TIMEOUT, 0);
        if(timeout_exceed(state)) {
            state->timeout = 0;
            state->current_event = selected_is_food(state) ? WinEvent : LooseEvent;
            state->deed = selected_is_food(state);
        }
        break;
    case LooseEvent:
        state->timeout = CLAMP(state->timeout + 1, TRY_TIMEOUT, 0);
        if(timeout_exceed(state)) {
            state->timeout = 0;
            state->current_event = FinishedEvent;
        }
        break;
    case WinEvent:
        if(state->deed) {
            dolphin_food_deed(state);
        }
        break;
    case FinishedEvent:
        reset_lid_pos(state);
        reset_loot_array(state);

        state->try++;
        state->current_event = tries_exceed(state) ? ExitGameEvent : PlayerChoiceEvent;
        break;

    default:
        break;
    }
}

static void food_minigame_controls(GameState* state, AppEvent* event) {
    furi_assert(state);
    furi_assert(event);

    if(event->value.input.key == InputKeyRight) {
        if(state->current_event == PlayerChoiceEvent) {
            state->cursor_pos = CLAMP(state->cursor_pos + 1, DISHES_TOTAL - 1, 0);
        }
    } else if(event->value.input.key == InputKeyLeft) {
        if(state->current_event == PlayerChoiceEvent) {
            state->cursor_pos = CLAMP(state->cursor_pos - 1, DISHES_TOTAL - 1, 0);
        }
    } else if(event->value.input.key == InputKeyOk) {
        switch(state->current_event) {
        case PlayerChoiceEvent:
            state->selected = true;
            break;
        case WinEvent:
            state->current_event = FinishedEvent;
            break;
        default:
            break;
        }
    }
}

int32_t food_minigame_app(void* p) {
    GameState* state = furi_alloc(sizeof(GameState));
    DolphinState* dolphin_state = dolphin_state_alloc();
    dolphin_state_load(dolphin_state);

    ValueMutex state_mutex;

    state->event_queue = osMessageQueueNew(2, sizeof(AppEvent), NULL);

    furi_check(state->event_queue);

    if(!init_mutex(&state_mutex, state, sizeof(GameState*))) {
        printf("[Food minigame] cannot create mutex\r\n");
        return 0;
    }

    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, render_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, state->event_queue);

    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    reset_loot_array(state);

    AppEvent event;
    while(1) {
        osStatus_t event_status = osMessageQueueGet(state->event_queue, &event, NULL, 100);
        if(event_status == osOK) {
            if(event.type == EventTypeKey && event.value.input.type == InputTypeShort) {
                food_minigame_controls(state, &event);

                if(event.value.input.key == InputKeyBack) {
                    break;
                }
            } else if(event.type == EventTypeDeed) {
                dolphin_state_on_deed(dolphin_state, DolphinDeedIButtonRead);
                dolphin_state_save(dolphin_state);
                state->deed = false;
            }
        }

        if(state->current_event == ExitGameEvent) {
            break;
        }
        gamestate_update(state, dolphin_state);
        view_port_update(view_port);
    }

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close("gui");
    delete_mutex(&state_mutex);
    osMessageQueueDelete(state->event_queue);
    dolphin_state_free(dolphin_state);
    free(state);

    return 0;
}
