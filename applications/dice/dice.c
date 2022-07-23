#include <furi.h>
#include <furi_hal.h>
#include "furi_hal_random.h"
#include <gui/elements.h>
#include <gui/gui.h>
#include <input/input.h>

#define TAG "Dice Roller"

uint8_t diceSelect = 20;
uint8_t diceQty = 1;
uint8_t diceRoll = 0;
uint8_t playerOneScore = 0;
uint8_t playerTwoScore = 0;
char rollTime[1][12];
char diceType[1][8];
char strings[5][45];
char theScores[1][45];
bool letsRoll = false;

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef struct {
    FuriHalRtcDateTime datetime;
} ClockState;

static void dice_input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);
    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void dice_render_callback(Canvas* const canvas, void* ctx) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    if(diceSelect < 229) {
        if(diceQty == 1) {
            elements_button_left(canvas, "x1");
        } else if(diceQty == 2) {
            elements_button_left(canvas, "x2");
        } else if(diceQty == 3) {
            elements_button_left(canvas, "x3");
        } else if(diceQty == 4) {
            elements_button_left(canvas, "x4");
        } else if(diceQty == 5) {
            elements_button_left(canvas, "x5");
        } else if(diceQty == 6) {
            elements_button_left(canvas, "x6");
        }
    }
    ClockState* state = (ClockState*)acquire_mutex((ValueMutex*)ctx, 25);
    if(letsRoll) {
        static bool rand_generator_inited = false;
        if(!rand_generator_inited) {
            srand(furi_get_tick());
            rand_generator_inited = true;
        }
        sprintf(
            rollTime[0],
            "%.2d:%.2d:%.2d",
            state->datetime.hour,
            state->datetime.minute,
            state->datetime.second);
        if(diceSelect == 229) {
            const char* eightBall[] = {
                "It is certain",
                "Without a doubt",
                "You may rely on it",
                "Yes definitely",
                "It is decidedly so",
                "As I see it, yes",
                "Most likely",
                "Yes",
                "Outlook good",
                "Signs point to yes",
                "Reply hazy try again",
                "Better not tell you now",
                "Ask again later",
                "Cannot predict now",
                "Concentrate and ask again",
                "Don't count on it",
                "Outlook not so good",
                "My sources say no",
                "Very doubtful",
                "My reply is no"};
            diceRoll = ((rand() % diceSelect) + 1); // JUST TO GET IT GOING? AND FIX BUG
            sprintf(diceType[0], "%s", "8BALL");
            sprintf(strings[0], "%s at %s", diceType[0], rollTime[0]);
            uint8_t d1_i = rand() % COUNT_OF(eightBall);
            sprintf(strings[1], "%s", eightBall[d1_i]);
        } else if(diceSelect == 230) {
            const char* diceOne[] = {
                "Nibble",
                "Massage",
                "Touch",
                "Caress",
                "Pet",
                "Fondle",
                "Suck",
                "Lick",
                "Blow",
                "Kiss",
                "???"};
            const char* diceTwo[] = {
                "Navel",
                "Ears",
                "Lips",
                "Neck",
                "Hand",
                "Thigh",
                "Nipple",
                "Breasts",
                "???",
                "Genitals"};
            diceRoll = ((rand() % diceSelect) + 1); // JUST TO GET IT GOING? AND FIX BUG
            sprintf(diceType[0], "%s", "SEX?");
            sprintf(strings[0], "%s at %s", diceType[0], rollTime[0]);
            uint8_t d1_i = rand() % COUNT_OF(diceOne);
            uint8_t d2_i = rand() % COUNT_OF(diceTwo);
            sprintf(strings[1], "%s %s", diceOne[d1_i], diceTwo[d2_i]);
        } else if(diceSelect == 231) {
            const char* deckOne[] = {"2H", "2C", "2D", "2S", "3H", "3C",  "3D",  "3S",  "4H",
                                     "4C", "4D", "4S", "5H", "5C", "5D",  "5S",  "6H",  "6C",
                                     "6D", "6S", "7H", "7C", "7D", "7S",  "8H",  "8C",  "8D",
                                     "8S", "9H", "9C", "9D", "9S", "10H", "10C", "10D", "10S",
                                     "JH", "JC", "JD", "JS", "QH", "QC",  "QD",  "QS",  "KH",
                                     "KC", "KD", "KS", "AH", "AC", "AD",  "AS"};
            char* deckTwo[] = {"2H", "2C", "2D", "2S", "3H", "3C",  "3D",  "3S",  "4H",
                               "4C", "4D", "4S", "5H", "5C", "5D",  "5S",  "6H",  "6C",
                               "6D", "6S", "7H", "7C", "7D", "7S",  "8H",  "8C",  "8D",
                               "8S", "9H", "9C", "9D", "9S", "10H", "10C", "10D", "10S",
                               "JH", "JC", "JD", "JS", "QH", "QC",  "QD",  "QS",  "KH",
                               "KC", "KD", "KS", "AH", "AC", "AD"}; // ONE LESS SINCE ONE WILL BE REMOVED
            diceRoll = ((rand() % diceSelect) + 1); // JUST TO GET IT GOING? AND FIX BUG
            sprintf(diceType[0], "%s", "WAR!");
            sprintf(strings[0], "%s at %s", diceType[0], rollTime[0]);
            uint8_t d1_i = rand() % COUNT_OF(deckOne);
            // INITIALIZE WITH PLACEHOLDERS TO AVOID MAYBE UNINITIALIZED ERROR
            for(int i = 0; i < COUNT_OF(deckOne); i++) {
                if(i < d1_i) {
                    sprintf(deckTwo[i], "%s", deckOne[i]);
                } else if(i > d1_i) {
                    sprintf(deckTwo[i - 1], "%s", deckOne[i]);
                }
            }
            uint8_t d2_i = rand() % COUNT_OF(deckTwo);
            if(d1_i > d2_i) {
                playerOneScore++;
                sprintf(strings[1], "%s > %s", deckOne[d1_i], deckTwo[d2_i]);
            } else {
                playerTwoScore++;
                sprintf(strings[1], "%s < %s", deckOne[d1_i], deckTwo[d2_i]);
            }
        } else if(diceSelect == 232) {
            const char* diceOne[] = {
                "You", "You choose", "Nobody", "Everyone", "Nose goes", "Player to your right"};
            const char* diceTwo[] = {
                "take a tiny toke",
                "just chill",
                "take 2 tokes",
                "take a huge hit",
                "bogart it",
                "take a puff"};
            const char* diceThree[] = {
                "while humming a tune",
                "with your eyes closed",
                "on your knees",
                "while holding your nose",
                "while spinning in a circle",
                "in slow motion"};
            const char* diceFour[] = {
                "twice",
                "then tell a joke",
                "then laugh as hard as you can",
                "with the player to your left",
                "then sing a song",
                "then do a dance"};
            diceRoll = ((rand() % diceSelect) + 1); // JUST TO GET IT GOING? AND FIX BUG
            sprintf(diceType[0], "%s", "WEED!");
            sprintf(strings[0], "%s at %s", diceType[0], rollTime[0]);
            uint8_t d1_i = rand() % COUNT_OF(diceOne);
            uint8_t d2_i = rand() % COUNT_OF(diceTwo);
            uint8_t d3_i = rand() % COUNT_OF(diceThree);
            uint8_t d4_i = rand() % COUNT_OF(diceFour);
            sprintf(strings[1], "%s", diceOne[d1_i]);
            sprintf(strings[2], "%s", diceTwo[d2_i]);
            sprintf(strings[3], "%s", diceThree[d3_i]);
            sprintf(strings[4], "%s", diceFour[d4_i]);
        } else {
            diceRoll = ((rand() % diceSelect) + 1);
            sprintf(diceType[0], "%s%d", "d", diceSelect);
            sprintf(strings[0], "%d%s at %s", diceQty, diceType[0], rollTime[0]);
            if(diceQty == 1) {
                sprintf(strings[1], "%d", diceRoll);
            } else if(diceQty == 2) {
                sprintf(strings[1], "%d %d", diceRoll, ((rand() % diceSelect) + 1));
            } else if(diceQty == 3) {
                sprintf(
                    strings[1],
                    "%d %d %d",
                    diceRoll,
                    ((rand() % diceSelect) + 1),
                    ((rand() % diceSelect) + 1));
            } else if(diceQty == 4) {
                sprintf(
                    strings[1],
                    "%d %d %d %d",
                    diceRoll,
                    ((rand() % diceSelect) + 1),
                    ((rand() % diceSelect) + 1),
                    ((rand() % diceSelect) + 1));
            } else if(diceQty == 5) {
                sprintf(
                    strings[1],
                    "%d %d %d %d %d",
                    diceRoll,
                    ((rand() % diceSelect) + 1),
                    ((rand() % diceSelect) + 1),
                    ((rand() % diceSelect) + 1),
                    ((rand() % diceSelect) + 1));
            } else if(diceQty == 6) {
                sprintf(
                    strings[1],
                    "%d %d %d %d %d %d",
                    diceRoll,
                    ((rand() % diceSelect) + 1),
                    ((rand() % diceSelect) + 1),
                    ((rand() % diceSelect) + 1),
                    ((rand() % diceSelect) + 1),
                    ((rand() % diceSelect) + 1));
            }
        }
        letsRoll = false;
    }
    release_mutex((ValueMutex*)ctx, state);
    if(diceRoll != 0) {
        if(diceSelect == 232) {
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str_aligned(canvas, 64, 8, AlignCenter, AlignCenter, strings[0]);
            canvas_draw_str_aligned(canvas, 64, 18, AlignCenter, AlignCenter, strings[1]);
            canvas_draw_str_aligned(canvas, 64, 26, AlignCenter, AlignCenter, strings[2]);
            canvas_draw_str_aligned(canvas, 64, 34, AlignCenter, AlignCenter, strings[3]);
            canvas_draw_str_aligned(canvas, 64, 42, AlignCenter, AlignCenter, strings[4]);
        } else {
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str_aligned(canvas, 64, 20, AlignCenter, AlignCenter, strings[1]);
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str_aligned(canvas, 64, 8, AlignCenter, AlignCenter, strings[0]);
        }
        if(diceSelect == 231 && !(playerOneScore == 0 && playerTwoScore == 0)) {
            canvas_set_font(canvas, FontSecondary);
            sprintf(
                theScores[0],
                "%d                                   %d",
                playerOneScore,
                playerTwoScore);
            canvas_draw_str_aligned(canvas, 64, 34, AlignCenter, AlignCenter, theScores[0]);
        }
    }
    if(diceSelect == 229) {
        elements_button_center(canvas, "Shake");
    } else if(diceSelect == 231) {
        elements_button_center(canvas, "Draw");
    } else {
        elements_button_center(canvas, "Roll");
    }
    if(diceSelect == 2) {
        elements_button_right(canvas, "d2");
    } else if(diceSelect == 3) {
        elements_button_right(canvas, "d3");
    } else if(diceSelect == 4) {
        elements_button_right(canvas, "d4");
    } else if(diceSelect == 6) {
        elements_button_right(canvas, "d6");
    } else if(diceSelect == 8) {
        elements_button_right(canvas, "d8");
    } else if(diceSelect == 10) {
        elements_button_right(canvas, "d10");
    } else if(diceSelect == 12) {
        elements_button_right(canvas, "d12");
    } else if(diceSelect == 20) {
        elements_button_right(canvas, "d20");
    } else if(diceSelect == 59) {
        elements_button_right(canvas, "d59");
    } else if(diceSelect == 69) {
        elements_button_right(canvas, "d69");
    } else if(diceSelect == 100) {
        elements_button_right(canvas, "d100");
    } else if(diceSelect == 229) {
        elements_button_right(canvas, "8BALL");
    } else if(diceSelect == 230) {
        elements_button_right(canvas, "SEX");
    } else if(diceSelect == 231) {
        elements_button_right(canvas, "WAR");
    } else if(diceSelect == 232) {
        elements_button_right(canvas, "WEED");
    }
}

static void diceclock_state_init(ClockState* const state) {
    furi_hal_rtc_get_datetime(&state->datetime);
}

static void dice_tick(void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    PluginEvent event = {.type = EventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
}

int32_t dice_app(void* p) {
    UNUSED(p);
    letsRoll = false;
    diceSelect = 20;
    diceQty = 1;
    diceRoll = 0;
    playerOneScore = 0;
    playerTwoScore = 0;
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));
    ClockState* plugin_state = malloc(sizeof(ClockState));
    diceclock_state_init(plugin_state);
    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, plugin_state, sizeof(ClockState))) {
        FURI_LOG_E(TAG, "cannot create mutex\r\n");
        free(plugin_state);
        return 255;
    }
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, dice_render_callback, &state_mutex);
    view_port_input_callback_set(view_port, dice_input_callback, event_queue);
    FuriTimer* timer = furi_timer_alloc(dice_tick, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_kernel_get_tick_frequency());
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    PluginEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        ClockState* plugin_state = (ClockState*)acquire_mutex_block(&state_mutex);
        if(event_status == FuriStatusOk) {
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypeShort || event.input.type == InputTypeRepeat) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        break;
                    case InputKeyDown:
                        break;
                    case InputKeyRight:
                        if(diceSelect == 2) {
                            diceSelect = 3;
                        } else if(diceSelect == 3) {
                            diceSelect = 4;
                        } else if(diceSelect == 4) {
                            diceSelect = 6;
                        } else if(diceSelect == 6) {
                            diceSelect = 8;
                        } else if(diceSelect == 8) {
                            diceSelect = 10;
                        } else if(diceSelect == 10) {
                            diceSelect = 12;
                        } else if(diceSelect == 12) {
                            diceSelect = 20;
                        } else if(diceSelect == 20) {
                            diceSelect = 100;
                        } else if(diceSelect == 100) {
                            diceSelect = 230;
                        } else if(diceSelect == 230) {
                            playerOneScore = 0;
                            playerTwoScore = 0;
                            diceSelect = 231;
                        } else if(diceSelect == 231) {
                            diceSelect = 229;
                        } else if(diceSelect == 229) {
                            diceSelect = 232;
                        } else if(diceSelect == 232) {
                            diceSelect = 59;
                        } else if(diceSelect == 59) {
                            diceSelect = 69;
                        } else {
                            diceSelect = 2;
                        }
                        break;
                    case InputKeyLeft:
                        if(diceQty <= 5) {
                            diceQty = diceQty + 1;
                        } else {
                            diceQty = 1;
                        }
                        break;
                    case InputKeyOk:
                        letsRoll = true;
                        break;
                    case InputKeyBack:
                        processing = false;
                        break;
                    }
                }
            } else if(event.type == EventTypeTick) {
                furi_hal_rtc_get_datetime(&plugin_state->datetime);
            }
        } else {
            FURI_LOG_D(TAG, "osMessageQueue: event timeout");
        }
        view_port_update(view_port);
        release_mutex(&state_mutex, plugin_state);
    }
    furi_timer_free(timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    return 0;
}