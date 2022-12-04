#include <gui/icon.h>
#include "Dice_app_icons.h"

#define TAG "DiceApp"

#define DICE_TYPES 8

#define MAX_DICE_COUNT 10
#define MAX_COIN_FRAMES 9
#define MAX_DICE_FRAMES 4

#define DICE_X 45
#define DICE_Y 6
#define DICE_Y_T 0

#define DICE_GAP 44

#define SWIPE_DIST 11

const Icon* coin_heads_start[] = {&I_coin_1, &I_coin_2};
const Icon* coin_heads_end[] = {&I_coin_7, &I_coin_1};
const Icon* coin_tails_start[] = {&I_coin_5, &I_coin_6};
const Icon* coin_tails_end[] = {&I_coin_4, &I_coin_5};
const Icon* coin_frames[] = {
    &I_coin_1,
    &I_coin_2,
    &I_coin_3,
    &I_coin_4,
    &I_coin_5,
    &I_coin_6,
    &I_coin_3,
    &I_coin_7,
    &I_coin_1,
};

const Icon* dice_frames[] = {
    &I_d4_1,   &I_d4_2,   &I_d4_3,   &I_d4_1, // d4
    &I_d6_1,   &I_d6_2,   &I_d6_3,   &I_d6_4, // d6
    &I_d8_1,   &I_d8_2,   &I_d8_3,   &I_d8_4, // d8
    &I_d10_1,  &I_d10_2,  &I_d10_3,  &I_d10_4, // d10
    &I_d12_1,  &I_d12_2,  &I_d12_3,  &I_d12_4, // d12
    &I_d20_1,  &I_d20_2,  &I_d20_3,  &I_d20_4, // d20
    &I_d100_1, &I_d100_2, &I_d100_3, &I_d100_4, // d100
};

typedef struct {
    uint8_t type;
    int x;
    int y;
    char* name;
} Dice;

const uint8_t screen_pos[] = {};

static const Dice dice_types[] = {
    {2, 0, 0, "Coin"},
    {4, 0, 0, "d4"},
    {6, 0, 0, "d6"},
    {8, 0, 0, "d8"},
    {10, 0, 0, "d10"},
    {12, 0, 0, "d12"},
    {20, 0, 0, "d20"},
    {100, 0, 0, "d100"},
};

typedef enum { EventTypeTick, EventTypeKey } EventType;
typedef enum { SelectState, SwipeLeftState, SwipeRightState, AnimState, ResultState } AppState;

typedef struct {
    EventType type;
    InputEvent input;
} AppEvent;

typedef struct {
    AppState app_state;
    uint16_t roll_result;
    uint8_t rolled_dices[MAX_DICE_COUNT];
    uint8_t anim_frame;
    uint8_t dice_index;
    uint8_t dice_count;
    Dice dices[DICE_TYPES];
} State;

void init(State* const state) {
    state->app_state = SelectState;
    state->roll_result = 0;
    state->dice_index = 0;
    state->anim_frame = 0;
    state->dice_count = 1;

    for(uint8_t i = 0; i < DICE_TYPES; i++) {
        state->dices[i] = dice_types[i];
        state->dices[i].x = DICE_X + (i * DICE_GAP);
        state->dices[i].y = i == 0 ? DICE_Y_T : DICE_Y;
    }
}

void coin_set_start(uint16_t type) {
    if(type == 1) {
        coin_frames[0] = coin_heads_start[0];
        coin_frames[1] = coin_heads_start[1];
    } else {
        coin_frames[0] = coin_tails_start[0];
        coin_frames[1] = coin_tails_start[1];
    }
}

void coin_set_end(uint16_t type) {
    if(type == 1) {
        coin_frames[MAX_COIN_FRAMES - 2] = coin_heads_end[0];
        coin_frames[MAX_COIN_FRAMES - 1] = coin_heads_end[1];
    } else {
        coin_frames[MAX_COIN_FRAMES - 2] = coin_tails_end[0];
        coin_frames[MAX_COIN_FRAMES - 1] = coin_tails_end[1];
    }
}