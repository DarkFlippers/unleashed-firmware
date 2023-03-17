#pragma once

#include <furi.h>
#include <input/input.h>
#include <gui/elements.h>
#include <flipper_format/flipper_format.h>
#include <flipper_format/flipper_format_i.h>
#include "common/card.h"
#include "common/queue.h"
#include "common/menu.h"

#define APP_NAME "Blackjack"

#define CONF_ANIMATION_DURATION "AnimationDuration"
#define CONF_MESSAGE_DURATION "MessageDuration"
#define CONF_STARTING_MONEY "StartingMoney"
#define CONF_ROUND_PRICE "RoundPrice"
#define CONF_SOUND_EFFECTS "SoundEffects"

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    uint32_t animation_duration;
    uint32_t message_duration;
    uint32_t starting_money;
    uint32_t round_price;
    bool sound_effects;
} Settings;

typedef struct {
    EventType type;
    InputEvent input;
} AppEvent;

typedef enum {
    GameStateGameOver,
    GameStateStart,
    GameStatePlay,
    GameStateSettings,
    GameStateDealer,
} PlayState;

typedef enum {
    DirectionUp,
    DirectionDown,
    DirectionRight,
    DirectionLeft,
    Select,
    Back,
    None
} Direction;

typedef struct {
    FuriMutex* mutex;
    Card player_cards[21];
    Card dealer_cards[21];
    uint8_t player_card_count;
    uint8_t dealer_card_count;

    Direction selectDirection;
    Settings settings;

    uint32_t player_score;
    uint32_t bet;
    uint8_t selectedMenu;
    bool doubled;
    bool started;
    bool processing;
    Deck deck;
    PlayState state;
    QueueState queue_state;
    Menu* menu;
    unsigned int last_tick;
} GameState;
