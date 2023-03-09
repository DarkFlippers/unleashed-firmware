#pragma once
#include <furi.h>
#include <input/input.h>
#include <gui/elements.h>
#include <flipper_format/flipper_format.h>
#include <flipper_format/flipper_format_i.h>
#include "common/card.h"
#include "common/queue.h"

#define APP_NAME "Solitaire"

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} AppEvent;

typedef enum { GameStateGameOver, GameStateStart, GameStatePlay, GameStateAnimate } PlayState;

typedef struct {
    uint8_t* buffer;
    Card card;
    int8_t deck;
    int indexes[4];
    float x;
    float y;
    float vx;
    float vy;
    bool started;
} CardAnimation;

typedef struct {
    Deck deck;
    Hand bottom_columns[7];
    Card top_cards[4];
    bool dragging_deck;
    uint8_t dragging_column;
    Hand dragging_hand;

    InputKey input;

    bool started;
    bool processing;
    bool longPress;
    PlayState state;
    unsigned int last_tick;
    uint8_t selectRow;
    uint8_t selectColumn;
    int8_t selected_card;
    CardAnimation animation;
    uint8_t* buffer;
    FuriMutex* mutex;
} GameState;