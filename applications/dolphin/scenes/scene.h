#pragma once

#include <furi.h>
#include <gui/gui_i.h>
#include <u8g2/u8g2.h>

#ifndef ARRSIZE
#define ARRSIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef CLAMP
#define CLAMP(x, upper, lower) (MIN(upper, MAX(x, lower)))
#endif

// global
#define SCALE 32
// screen

#define SCREEN_WIDTH GUI_DISPLAY_WIDTH
#define SCREEN_HEIGHT GUI_DISPLAY_HEIGHT
#define BONDARIES_X_LEFT 40
#define BONDARIES_X_RIGHT 88

// player
#define DOLPHIN_WIDTH 32
#define DOLPHIN_HEIGHT 32
#define DOLPHIN_CENTER (SCREEN_WIDTH / 2 - DOLPHIN_WIDTH / 2)
#define SPEED_X 2
#define ACTIONS_NUM 5
#define DOLPHIN_DEFAULT_Y 20
// world
#define WORLD_WIDTH 2048
#define WORLD_HEIGHT 64

#define LAYERS 8
#define SCENE_ZOOM 9
#define DOLPHIN_LAYER 6
#define PARALLAX_MOD 7
#define PARALLAX(layer) layer / PARALLAX_MOD - layer

#define DIALOG_PROGRESS 250

enum Actions { SLEEP = 0, IDLE, WALK, EMOTE, INTERACT, MINDCONTROL };

static const uint16_t default_timeout[] =
    {[SLEEP] = 300, [IDLE] = 100, [WALK] = 100, [EMOTE] = 50, [INTERACT] = 10, [MINDCONTROL] = 50};

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    union {
        InputEvent input;
    } value;
    EventType type;
} AppEvent;

typedef struct {
    int32_t x;
    int32_t y;
} Vec2;

typedef struct {
    osMessageQueueId_t mqueue;
    Gui* gui;
    ViewPort* view_port;
    osTimerId_t* timer;
} SceneAppGui;

typedef struct {
    uint8_t layer;
    uint16_t timeout;
    int32_t x;
    int32_t y;
    IconName icon;
    char action_name[16];
    void (*draw)(Canvas* canvas, void* model);
    void (*callback)(Canvas* canvas, void* model);
} Item;

typedef struct {
    ///
    Vec2 player;
    Vec2 player_global;
    Vec2 player_v;
    Vec2 screen;

    IconName dolphin_gfx;
    IconName dolphin_gfx_b; // temp

    bool player_flipped;
    bool use_pending;
    // dolphin_scene_debug
    bool debug;

    uint8_t player_anim;
    uint8_t scene_id;

    uint8_t emote_id;
    uint8_t previous_emote;

    uint8_t dialogue_id;
    uint8_t previous_dialogue;

    uint32_t action_timeout;
    uint8_t poi;

    uint8_t action;
    uint8_t next_action;
    uint8_t prev_action;

    int8_t zoom_v;
    uint8_t scene_zoom;
    uint8_t dialog_progress;

    FuriThread* scene_app_thread;
} SceneState;

void dolphin_scene_render(SceneState* state, Canvas* canvas, uint32_t t);
void dolphin_scene_render_dolphin(SceneState* state, Canvas* canvas);
void dolphin_scene_handle_user_input(SceneState* state, InputEvent* input);
void dolphin_scene_coordinates(SceneState* state, uint32_t dt);

void dolphin_scene_render_state(SceneState* state, Canvas* canvas);
void dolphin_scene_update_state(SceneState* state, uint32_t t, uint32_t dt);

void dolphin_scene_redraw(Canvas* canvas, void* ctx);
void dolphin_scene_tick_handler(SceneState* state, uint32_t t, uint32_t dt);
void dolphin_scene_handle_input(SceneState* state, InputEvent* input);
