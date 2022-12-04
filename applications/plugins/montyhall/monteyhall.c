#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <gui/icon_i.h>

#include <input/input.h>
#include <stdlib.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//AUTHOR: https://github.com/DevMilanIan
//I_DoorClosed_22x35 sourced from VideoPoker/poker.c -> I_CardBack22x35
//PRs for syntax, formatting, etc can get you listed as a contributor :)

// CONCEPT: one of three doors will have a car while the other two have only a goat
// randomize a winning door each round, let the player choose a first selection
// reveal a goat door and allow the player to keep or switch their selection
// based on the Monty Hall problem from Let's Make a Deal

//void draw_goat(Canvas* canvas, int x, int y) { TODO }

void draw_car(Canvas* canvas, int x, int y) {
    // x -> leftmost pixel, y -> topmost pixel
    // could be in another file or a pixel array but idk how to so feel free to PR

    canvas_draw_dot(canvas, x + 1, y + 4);
    canvas_draw_dot(canvas, x + 1, y + 5);
    canvas_draw_dot(canvas, x + 2, y + 3);
    canvas_draw_dot(canvas, x + 2, y + 6);
    canvas_draw_dot(canvas, x + 3, y + 3);
    canvas_draw_dot(canvas, x + 3, y + 6);

    canvas_draw_dot(canvas, x + 4, y + 2);
    canvas_draw_dot(canvas, x + 4, y + 3);
    canvas_draw_dot(canvas, x + 4, y + 6);
    canvas_draw_dot(canvas, x + 4, y + 7);

    canvas_draw_dot(canvas, x + 5, y + 1);
    canvas_draw_dot(canvas, x + 5, y + 2);
    canvas_draw_dot(canvas, x + 5, y + 3);
    canvas_draw_dot(canvas, x + 5, y + 5);
    canvas_draw_dot(canvas, x + 5, y + 8);

    canvas_draw_dot(canvas, x + 6, y);
    canvas_draw_dot(canvas, x + 6, y + 1);
    canvas_draw_dot(canvas, x + 6, y + 3);
    canvas_draw_dot(canvas, x + 6, y + 5);
    canvas_draw_dot(canvas, x + 6, y + 8);

    canvas_draw_dot(canvas, x + 7, y);
    canvas_draw_dot(canvas, x + 7, y + 3);
    canvas_draw_dot(canvas, x + 7, y + 6);
    canvas_draw_dot(canvas, x + 7, y + 7);

    canvas_draw_dot(canvas, x + 8, y);
    canvas_draw_dot(canvas, x + 8, y + 3);
    canvas_draw_dot(canvas, x + 8, y + 6);

    canvas_draw_dot(canvas, x + 9, y);
    canvas_draw_dot(canvas, x + 9, y + 3);
    canvas_draw_dot(canvas, x + 9, y + 6);

    canvas_draw_dot(canvas, x + 10, y);
    canvas_draw_dot(canvas, x + 10, y + 3);
    canvas_draw_dot(canvas, x + 10, y + 6);

    canvas_draw_dot(canvas, x + 11, y);
    canvas_draw_dot(canvas, x + 11, y + 1);
    canvas_draw_dot(canvas, x + 11, y + 3);
    canvas_draw_dot(canvas, x + 11, y + 6);

    canvas_draw_dot(canvas, x + 12, y + 1);
    canvas_draw_dot(canvas, x + 12, y + 2);
    canvas_draw_dot(canvas, x + 12, y + 3);
    canvas_draw_dot(canvas, x + 12, y + 6);
    canvas_draw_dot(canvas, x + 12, y + 7);

    canvas_draw_dot(canvas, x + 13, y + 2);
    canvas_draw_dot(canvas, x + 13, y + 3);
    canvas_draw_dot(canvas, x + 13, y + 5);
    canvas_draw_dot(canvas, x + 13, y + 8);

    canvas_draw_dot(canvas, x + 14, y + 1);
    canvas_draw_dot(canvas, x + 14, y + 2);
    canvas_draw_dot(canvas, x + 14, y + 5);
    canvas_draw_dot(canvas, x + 14, y + 8);

    canvas_draw_dot(canvas, x + 15, y);
    canvas_draw_dot(canvas, x + 15, y + 1);
    canvas_draw_dot(canvas, x + 15, y + 6);
    canvas_draw_dot(canvas, x + 15, y + 7);

    canvas_draw_dot(canvas, x + 16, y);
    canvas_draw_dot(canvas, x + 16, y + 1);
    canvas_draw_dot(canvas, x + 16, y + 2);
    canvas_draw_dot(canvas, x + 16, y + 3);
    canvas_draw_dot(canvas, x + 16, y + 4);
    canvas_draw_dot(canvas, x + 16, y + 5);
}

const uint8_t _I_DoorClosed_22x35_0[] = {
    0x01, 0x00, 0x23, 0x00, 0xfe, 0x7f, 0xe1, 0xf0, 0x28, 0x04, 0x43, 0xe3, 0xff,
    0x91, 0xea, 0x75, 0x52, 0x6a, 0xad, 0x56, 0x5b, 0xad, 0xd5, 0x4a, 0x80, 0xbe,
    0x05, 0xf0, 0x2f, 0x81, 0x7c, 0x0b, 0x45, 0x32, 0x2c, 0x91, 0x7c, 0x8c, 0xa4,
};
const uint8_t* _I_DoorClosed_22x35[] = {_I_DoorClosed_22x35_0};
const Icon I_DoorClosed_22x35 =
    {.width = 22, .height = 35, .frame_count = 1, .frame_rate = 0, .frames = _I_DoorClosed_22x35};

typedef struct {
    bool isOpen;
    bool isSelected; // picked in RoundOne, RoundThree
    bool isWinningDoor; // randomized in RoundOne
} Door;

typedef struct {
    Door doors[3];
    bool didSelect; // false in RoundOne -> RoundTwo when true
    bool didSwitch; // determined in RoundFour
} DoorState;

typedef enum {
    RoundOne, // all doors closed, player selects a door when ready
    RoundTwo, // door selected, reveal one of the remaining two (can go straight to GameOver)
    RoundThree, // player can keep or switch their selection
    RoundFour, // reveal all doors
    GameOver // score has been updated, allow restart
} GameState;

typedef struct {
    GameState game_state;
    DoorState door_state;
    uint16_t score;
} MontyState;

static void montyhall_game_init_state(MontyState* monty_state) {
    if(!monty_state->score) {
        monty_state->score = 0;
    }
    monty_state->door_state.didSelect = false;

    for(int i = 0; i < 3; i++) {
        monty_state->door_state.doors[i].isOpen = false;
        monty_state->door_state.doors[i].isSelected = false;
        monty_state->door_state.doors[i].isWinningDoor = false;
    }

    monty_state->game_state = RoundOne;
    int doorIndex = random() % 3;
    monty_state->door_state.doors[doorIndex].isWinningDoor = true;
}

void selectDoor(MontyState* monty_state, int doorIndex) {
    if(monty_state->game_state == RoundOne) {
        monty_state->door_state.doors[doorIndex].isSelected = true;
        if(monty_state->door_state.doors[doorIndex].isSelected) {
            monty_state->door_state.didSelect = true;
            monty_state->game_state = RoundTwo;
        }
    } else if(monty_state->game_state == RoundThree) {
        for(int i = 0; i < 3; i++) {
            monty_state->door_state.doors[i].isSelected = false;
        }

        monty_state->door_state.doors[doorIndex].isSelected = true;
    }
}

int getRandomDoorIndex() {
    int randomDoorIndex = random() % 3;
    return randomDoorIndex;
}

void revealBadDoor(MontyState* monty_state) {
    int doorToReveal = getRandomDoorIndex();
    while(!monty_state->door_state.doors[doorToReveal].isOpen) {
        if(!(monty_state->door_state.doors[doorToReveal].isSelected ||
             monty_state->door_state.doors[doorToReveal].isWinningDoor)) {
            monty_state->door_state.doors[doorToReveal].isOpen = true;
        } else {
            doorToReveal = getRandomDoorIndex();
        }
    }
}

void revealDoors_updateScore(MontyState* monty_state) {
    for(int i = 0; i < 3; i++) {
        monty_state->door_state.doors[i].isOpen = true;

        if(monty_state->door_state.doors[i].isWinningDoor &&
           monty_state->door_state.doors[i].isSelected) {
            monty_state->score++;
        }
    }
}

static void draw_top(Canvas* canvas, const MontyState* monty_state) {
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "Cars: %u", monty_state->score);
    canvas_draw_str_aligned(canvas, 2, 8, AlignLeft, AlignBottom, buffer);

    if(monty_state->game_state == RoundThree) {
        canvas_draw_str_aligned(
            canvas, SCREEN_WIDTH - 5, 8, AlignRight, AlignBottom, "Opened a decoy door");
    }
}

static void draw_doors(Canvas* canvas, const MontyState* monty_state) {
    // {| 16 | <22> | 15 | <22> | 15 | <22> | 16 |} = SCREEN_WIDTH
    if(monty_state->door_state.doors[0].isOpen) {
        if(monty_state->door_state.doors[0].isWinningDoor) {
            canvas_draw_frame(canvas, 16, 12, 22, 35);
            draw_car(canvas, 18, 26);
        } else {
            canvas_draw_frame(canvas, 16, 12, 22, 35);
            canvas_draw_str(canvas, 18, 34, "Goat");
        }
    } else {
        canvas_draw_icon(canvas, 16, 12, &I_DoorClosed_22x35);
    }

    if(monty_state->door_state.doors[1].isOpen) {
        if(monty_state->door_state.doors[1].isWinningDoor) {
            canvas_draw_frame(canvas, 53, 12, 22, 35);
            draw_car(canvas, 55, 26);
        } else {
            canvas_draw_frame(canvas, 53, 12, 22, 35);
            canvas_draw_str(canvas, 55, 34, "Goat");
        }
    } else {
        canvas_draw_icon(canvas, 53, 12, &I_DoorClosed_22x35);
    }

    if(monty_state->door_state.doors[2].isOpen) {
        if(monty_state->door_state.doors[2].isWinningDoor) {
            canvas_draw_frame(canvas, 90, 12, 22, 35);
            draw_car(canvas, 92, 26);
        } else {
            canvas_draw_frame(canvas, 90, 12, 22, 35);
            canvas_draw_str(canvas, 92, 34, "Goat");
        }
    } else {
        canvas_draw_icon(canvas, 90, 12, &I_DoorClosed_22x35);
    }
}

static void draw_bottom(Canvas* canvas, const MontyState* monty_state) {
    if(monty_state->game_state == RoundOne) {
        elements_button_left(canvas, "Left");
        elements_button_center(canvas, "Center");
        elements_button_right(canvas, "Right");
    }

    if(monty_state->game_state == RoundThree) {
        if(monty_state->door_state.doors[0].isSelected) {
            elements_button_left(canvas, "Keep");
            if(!monty_state->door_state.doors[1].isOpen) {
                elements_button_center(canvas, "Switch");
            } else {
                elements_button_right(canvas, "Switch");
            }
        } else if(monty_state->door_state.doors[1].isSelected) {
            elements_button_center(canvas, "Keep");
            if(!monty_state->door_state.doors[0].isOpen) {
                elements_button_left(canvas, "Switch");
            } else {
                elements_button_right(canvas, "Switch");
            }
        } else if(monty_state->door_state.doors[2].isSelected) {
            elements_button_right(canvas, "Keep");
            if(!monty_state->door_state.doors[0].isOpen) {
                elements_button_left(canvas, "Switch");
            } else {
                elements_button_center(canvas, "Switch");
            }
        }
    }

    if(monty_state->game_state == RoundFour) {
        elements_button_center(canvas, "Reveal");
    }

    if(monty_state->game_state == GameOver) {
        canvas_draw_str(canvas, 16, SCREEN_HEIGHT - 5, "Hold center to restart");
    }
}

static void montyhall_render_callback(Canvas* const canvas, void* ctx) {
    const MontyState* monty_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(monty_state == NULL) {
        return;
    }

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    draw_top(canvas, monty_state);
    draw_doors(canvas, monty_state);
    draw_bottom(canvas, monty_state);

    release_mutex((ValueMutex*)ctx, monty_state);
}

static void montyhall_input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t montyhall_game_app(void* p) {
    UNUSED(p);
    int32_t return_code = 0;

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    MontyState* monty_state = malloc(sizeof(MontyState));

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, monty_state, sizeof(MontyState))) {
        return_code = 255;
        goto free_and_exit;
    }

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, montyhall_render_callback, &state_mutex);
    view_port_input_callback_set(view_port, montyhall_input_callback, event_queue);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Start the game
    montyhall_game_init_state(monty_state);

    InputEvent event;
    for(bool loop = true; loop;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        MontyState* monty_state = (MontyState*)acquire_mutex_block(&state_mutex);

        if(event_status == FuriStatusOk) {
            if(event.type == InputTypeShort) {
                switch(event.key) {
                case InputKeyUp: /* <debug>
                    if(monty_state->game_state == RoundOne) {
                        monty_state->score++;
                    } else if(monty_state->game_state == RoundTwo) {
                        monty_state->score += 2;
                    } else if(monty_state->game_state == RoundThree) {
                        monty_state->score += 3;
                    } else if(monty_state->game_state == RoundFour) {
                        monty_state->score += 4;
                    } else if(monty_state->game_state == GameOver) {
                        monty_state->score += 5;
                    } </debug> */
                    break;
                case InputKeyDown: /* <debug>
                    if(monty_state->game_state == RoundOne) {
                        monty_state->score--;
                    } else if(monty_state->game_state == RoundTwo) {
                        monty_state->score -= 2;
                    } else if(monty_state->game_state == RoundThree) {
                        monty_state->score -= 3;
                    } else if(monty_state->game_state == RoundFour) {
                        monty_state->score -= 4;
                    } else if(monty_state->game_state == GameOver) {
                        monty_state->score -= 5;
                    } </dubug> */
                    break;
                case InputKeyLeft:
                    if(monty_state->game_state == RoundOne) {
                        selectDoor(monty_state, 0);
                        if(monty_state->game_state == RoundTwo) {
                            revealBadDoor(monty_state);
                            monty_state->game_state = RoundThree;
                        }
                    } else if(monty_state->game_state == RoundThree) {
                        if(monty_state->door_state.doors[0].isSelected) {
                            monty_state->door_state.didSwitch = false;
                        } else if(!monty_state->door_state.doors[0].isOpen) {
                            monty_state->door_state.didSwitch = true;
                            selectDoor(monty_state, 0);
                        }
                        monty_state->game_state = RoundFour;
                    }
                    break;
                case InputKeyOk:
                    if(monty_state->game_state == RoundOne) {
                        selectDoor(monty_state, 1);
                        if(monty_state->game_state == RoundTwo) {
                            revealBadDoor(monty_state);
                            monty_state->game_state = RoundThree;
                        }
                    } else if(monty_state->game_state == RoundThree) {
                        if(monty_state->door_state.doors[1].isSelected) {
                            monty_state->door_state.didSwitch = false;
                        } else if(!monty_state->door_state.doors[1].isOpen) {
                            monty_state->door_state.didSwitch = true;
                            selectDoor(monty_state, 1);
                        }
                        monty_state->game_state = RoundFour;
                    } else if(monty_state->game_state == RoundFour) {
                        revealDoors_updateScore(monty_state);
                        monty_state->game_state = GameOver;
                    }
                    break;
                case InputKeyRight:
                    if(monty_state->game_state == RoundOne) {
                        selectDoor(monty_state, 2);
                        if(monty_state->game_state == RoundTwo) {
                            revealBadDoor(monty_state);
                            monty_state->game_state = RoundThree;
                        }
                    } else if(monty_state->game_state == RoundThree) {
                        if(monty_state->door_state.doors[2].isSelected) {
                            monty_state->door_state.didSwitch = false;
                        } else if(!monty_state->door_state.doors[2].isOpen) {
                            monty_state->door_state.didSwitch = true;
                            selectDoor(monty_state, 2);
                        }
                        monty_state->game_state = RoundFour;
                    }
                    break;
                case InputKeyBack:
                    loop = false;
                    break;
                default:
                    break;
                }
            }
        } else if(event.type == InputTypeLong) {
            if(event.key == InputKeyOk && monty_state->game_state == GameOver) {
                montyhall_game_init_state(monty_state);
            }
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, monty_state);
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    delete_mutex(&state_mutex);

free_and_exit:
    free(monty_state);
    furi_message_queue_free(event_queue);

    return return_code;
}