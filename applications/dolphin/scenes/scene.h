#pragma once

#include <furi.h>
#include <gui/gui_i.h>
#include <u8g2/u8g2.h>

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
#define DOLPHIN_CENTER (SCREEN_WIDTH / 2 - DOLPHIN_WIDTH)
#define SPEED_X 4
#define SPEED_Y 4
#define ACTIONS_NUM 4
#define DOLPHIN_DEFAULT_Y 2
#define MAX_FRAMES 3

// world
#define WORLD_WIDTH 256
#define WORLD_HEIGHT 192

#define LAYERS 8
#define DOLPHIN_LAYER 6
#define PARALLAX_MOD 7
#define PARALLAX(layer) layer / PARALLAX_MOD - layer

#define DIALOG_PROGRESS 250

enum Actions { IDLE = 0, EMOTE, INTERACT, MINDCONTROL };

static const uint16_t default_timeout[] =
    {[IDLE] = 100, [EMOTE] = 50, [INTERACT] = 10, [MINDCONTROL] = 50};

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
    Vec2 pos;

    uint8_t width;
    uint8_t height;

    void (*draw)(Canvas* canvas, void* model);
    void (*callback)(Canvas* canvas, void* model);
} Item;

typedef enum {
    DirUp = 0,
    DirRight,
    DirDown,
    DirLeft,
} FrameDirectionEnum;

typedef struct {
    const Icon* f;
    const Icon* b;
} DolphinGfxAsset;

typedef struct {
    const DolphinGfxAsset frames[MAX_FRAMES];
    const uint8_t total;
} DolphinFrame;

typedef struct {
    Vec2 player;
    Vec2 player_global;
    Vec2 player_v;
    Vec2 screen;

    FrameDirectionEnum frame_group;
    FrameDirectionEnum last_group;
    FrameDirectionEnum frame_pending;
    FrameDirectionEnum frame_type;

    const DolphinFrame* current_frame;

    bool transition;
    bool transition_pending;
    bool use_pending;
    bool debug;

    uint8_t player_anim;
    uint8_t frame_idx;

    uint8_t scene_id;
    uint8_t emote_id;
    uint8_t previous_emote;

    uint8_t action;
    uint8_t prev_action;
    uint8_t action_timeout;
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
