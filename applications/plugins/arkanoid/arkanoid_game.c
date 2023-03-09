#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <gui/view.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <dolphin/dolphin.h>

#define TAG "Arkanoid"

#define FLIPPER_LCD_WIDTH 128
#define FLIPPER_LCD_HEIGHT 64
#define MAX_SPEED 3

typedef enum { EventTypeTick, EventTypeKey } EventType;

typedef struct {
    //Brick Bounds used in collision detection
    int leftBrick;
    int rightBrick;
    int topBrick;
    int bottomBrick;
    bool isHit[4][13]; //Array of if bricks are hit or not
} BrickState;

typedef struct {
    int dx; //Initial movement of ball
    int dy; //Initial movement of ball
    int xb; //Balls starting possition
    int yb; //Balls starting possition
    bool released; //If the ball has been released by the player
    //Ball Bounds used in collision detection
    int leftBall;
    int rightBall;
    int topBall;
    int bottomBall;
} BallState;

typedef struct {
    FuriMutex* mutex;
    BallState ball_state;
    BrickState brick_state;
    NotificationApp* notify;
    unsigned int COLUMNS; //Columns of bricks
    unsigned int ROWS; //Rows of bricks
    bool initialDraw; //If the inital draw has happened
    int xPaddle; //X position of paddle
    char text[16]; //General string buffer
    bool bounced; //Used to fix double bounce glitch
    int lives; //Amount of lives
    int level; //Current level
    unsigned int score; //Score for the game
    unsigned int brickCount; //Amount of bricks hit
    int tick; //Tick counter
    bool gameStarted; // Did the game start?
    int speed; // Ball speed
} ArkanoidState;

typedef struct {
    EventType type;
    InputEvent input;
} GameEvent;

static const NotificationSequence sequence_short_sound = {
    &message_note_c5,
    &message_delay_50,
    &message_sound_off,
    NULL,
};

// generate number in range [min,max)
int rand_range(int min, int max) {
    return min + rand() % (max - min);
}

void move_ball(Canvas* canvas, ArkanoidState* st) {
    st->tick++;

    int current_speed = abs(st->speed - 1 - MAX_SPEED);
    if(st->tick % current_speed != 0 && st->tick % (current_speed + 1) != 0) {
        return;
    }

    if(st->ball_state.released) {
        //Move ball
        if(abs(st->ball_state.dx) == 2) {
            st->ball_state.xb += st->ball_state.dx / 2;
            // 2x speed is really 1.5 speed
            if((st->tick / current_speed) % 2 == 0) st->ball_state.xb += st->ball_state.dx / 2;
        } else {
            st->ball_state.xb += st->ball_state.dx;
        }
        st->ball_state.yb = st->ball_state.yb + st->ball_state.dy;

        //Set bounds
        st->ball_state.leftBall = st->ball_state.xb;
        st->ball_state.rightBall = st->ball_state.xb + 2;
        st->ball_state.topBall = st->ball_state.yb;
        st->ball_state.bottomBall = st->ball_state.yb + 2;

        //Bounce off top edge
        if(st->ball_state.yb <= 0) {
            st->ball_state.yb = 2;
            st->ball_state.dy = -st->ball_state.dy;
        }

        //Lose a life if bottom edge hit
        if(st->ball_state.yb >= FLIPPER_LCD_HEIGHT) {
            canvas_draw_frame(canvas, st->xPaddle, FLIPPER_LCD_HEIGHT - 1, 11, 1);
            st->xPaddle = 54;
            st->ball_state.yb = 60;
            st->ball_state.released = false;
            st->lives--;
            st->gameStarted = false;

            if(rand_range(0, 2) == 0) {
                st->ball_state.dx = 1;
            } else {
                st->ball_state.dx = -1;
            }
        }

        //Bounce off left side
        if(st->ball_state.xb <= 0) {
            st->ball_state.xb = 2;
            st->ball_state.dx = -st->ball_state.dx;
        }

        //Bounce off right side
        if(st->ball_state.xb >= FLIPPER_LCD_WIDTH - 2) {
            st->ball_state.xb = FLIPPER_LCD_WIDTH - 4;
            st->ball_state.dx = -st->ball_state.dx;
            // arduboy.tunes.tone(523, 250);
        }

        //Bounce off paddle
        if(st->ball_state.xb + 1 >= st->xPaddle && st->ball_state.xb <= st->xPaddle + 12 &&
           st->ball_state.yb + 2 >= FLIPPER_LCD_HEIGHT - 1 &&
           st->ball_state.yb <= FLIPPER_LCD_HEIGHT) {
            st->ball_state.dy = -st->ball_state.dy;
            st->ball_state.dx =
                ((st->ball_state.xb - (st->xPaddle + 6)) / 3); //Applies spin on the ball
            // prevent straight bounce, but not prevent roguuemaster from stealing
            if(st->ball_state.dx == 0) {
                st->ball_state.dx = (rand_range(0, 2) == 1) ? 1 : -1;
            }
        }

        //Bounce off Bricks
        for(unsigned int row = 0; row < st->ROWS; row++) {
            for(unsigned int column = 0; column < st->COLUMNS; column++) {
                if(!st->brick_state.isHit[row][column]) {
                    //Sets Brick bounds
                    st->brick_state.leftBrick = 10 * column;
                    st->brick_state.rightBrick = 10 * column + 10;
                    st->brick_state.topBrick = 6 * row + 1;
                    st->brick_state.bottomBrick = 6 * row + 7;

                    //If A collison has occured
                    if(st->ball_state.topBall <= st->brick_state.bottomBrick &&
                       st->ball_state.bottomBall >= st->brick_state.topBrick &&
                       st->ball_state.leftBall <= st->brick_state.rightBrick &&
                       st->ball_state.rightBall >= st->brick_state.leftBrick) {
                        st->score += st->level;
                        // Blink led when we hit some brick
                        notification_message(st->notify, &sequence_short_sound);
                        //notification_message(st->notify, &sequence_blink_white_100);

                        st->brickCount++;
                        st->brick_state.isHit[row][column] = true;
                        canvas_draw_frame(canvas, 10 * column, 2 + 6 * row, 8, 4);

                        //Vertical collision
                        if(st->ball_state.bottomBall > st->brick_state.bottomBrick ||
                           st->ball_state.topBall < st->brick_state.topBrick) {
                            //Only bounce once each ball move
                            if(!st->bounced) {
                                st->ball_state.dy = -st->ball_state.dy;
                                st->ball_state.yb += st->ball_state.dy;
                                st->bounced = true;
                            }
                        }

                        //Hoizontal collision
                        if(st->ball_state.leftBall < st->brick_state.leftBrick ||
                           st->ball_state.rightBall > st->brick_state.rightBrick) {
                            //Only bounce once brick each ball move
                            if(!st->bounced) {
                                st->ball_state.dx = -st->ball_state.dx;
                                st->ball_state.xb += st->ball_state.dx;
                                st->bounced = true;
                            }
                        }
                    }
                }
            }
        }

        //Reset Bounce
        st->bounced = false;
    } else {
        //Ball follows paddle
        st->ball_state.xb = st->xPaddle + 5;
    }
}

void draw_lives(Canvas* canvas, ArkanoidState* arkanoid_state) {
    if(arkanoid_state->lives == 3) {
        canvas_draw_dot(canvas, 4, FLIPPER_LCD_HEIGHT - 7);
        canvas_draw_dot(canvas, 3, FLIPPER_LCD_HEIGHT - 7);
        canvas_draw_dot(canvas, 4, FLIPPER_LCD_HEIGHT - 8);
        canvas_draw_dot(canvas, 3, FLIPPER_LCD_HEIGHT - 8);

        canvas_draw_dot(canvas, 4, FLIPPER_LCD_HEIGHT - 11);
        canvas_draw_dot(canvas, 3, FLIPPER_LCD_HEIGHT - 11);
        canvas_draw_dot(canvas, 4, FLIPPER_LCD_HEIGHT - 12);
        canvas_draw_dot(canvas, 3, FLIPPER_LCD_HEIGHT - 12);

        canvas_draw_dot(canvas, 4, FLIPPER_LCD_HEIGHT - 15);
        canvas_draw_dot(canvas, 3, FLIPPER_LCD_HEIGHT - 15);
        canvas_draw_dot(canvas, 4, FLIPPER_LCD_HEIGHT - 16);
        canvas_draw_dot(canvas, 3, FLIPPER_LCD_HEIGHT - 16);
    } else if(arkanoid_state->lives == 2) {
        canvas_draw_dot(canvas, 4, FLIPPER_LCD_HEIGHT - 7);
        canvas_draw_dot(canvas, 3, FLIPPER_LCD_HEIGHT - 7);
        canvas_draw_dot(canvas, 4, FLIPPER_LCD_HEIGHT - 8);
        canvas_draw_dot(canvas, 3, FLIPPER_LCD_HEIGHT - 8);

        canvas_draw_dot(canvas, 4, FLIPPER_LCD_HEIGHT - 11);
        canvas_draw_dot(canvas, 3, FLIPPER_LCD_HEIGHT - 11);
        canvas_draw_dot(canvas, 4, FLIPPER_LCD_HEIGHT - 12);
        canvas_draw_dot(canvas, 3, FLIPPER_LCD_HEIGHT - 12);
    } else {
        canvas_draw_dot(canvas, 4, FLIPPER_LCD_HEIGHT - 7);
        canvas_draw_dot(canvas, 3, FLIPPER_LCD_HEIGHT - 7);
        canvas_draw_dot(canvas, 4, FLIPPER_LCD_HEIGHT - 8);
        canvas_draw_dot(canvas, 3, FLIPPER_LCD_HEIGHT - 8);
    }
}

void draw_score(Canvas* canvas, ArkanoidState* arkanoid_state) {
    snprintf(arkanoid_state->text, sizeof(arkanoid_state->text), "%u", arkanoid_state->score);
    canvas_draw_str_aligned(
        canvas,
        FLIPPER_LCD_WIDTH - 2,
        FLIPPER_LCD_HEIGHT - 6,
        AlignRight,
        AlignBottom,
        arkanoid_state->text);
}

void draw_ball(Canvas* canvas, ArkanoidState* ast) {
    canvas_draw_dot(canvas, ast->ball_state.xb, ast->ball_state.yb);
    canvas_draw_dot(canvas, ast->ball_state.xb + 1, ast->ball_state.yb);
    canvas_draw_dot(canvas, ast->ball_state.xb, ast->ball_state.yb + 1);
    canvas_draw_dot(canvas, ast->ball_state.xb + 1, ast->ball_state.yb + 1);

    move_ball(canvas, ast);
}

void draw_paddle(Canvas* canvas, ArkanoidState* arkanoid_state) {
    canvas_draw_frame(canvas, arkanoid_state->xPaddle, FLIPPER_LCD_HEIGHT - 1, 11, 1);
}

void reset_level(Canvas* canvas, ArkanoidState* arkanoid_state) {
    //Undraw paddle
    canvas_draw_frame(canvas, arkanoid_state->xPaddle, FLIPPER_LCD_HEIGHT - 1, 11, 1);

    //Undraw ball
    canvas_draw_dot(canvas, arkanoid_state->ball_state.xb, arkanoid_state->ball_state.yb);
    canvas_draw_dot(canvas, arkanoid_state->ball_state.xb + 1, arkanoid_state->ball_state.yb);
    canvas_draw_dot(canvas, arkanoid_state->ball_state.xb, arkanoid_state->ball_state.yb + 1);
    canvas_draw_dot(canvas, arkanoid_state->ball_state.xb + 1, arkanoid_state->ball_state.yb + 1);

    //Alter various variables to reset the game
    arkanoid_state->xPaddle = 54;
    arkanoid_state->ball_state.yb = 60;
    arkanoid_state->brickCount = 0;
    arkanoid_state->ball_state.released = false;
    arkanoid_state->gameStarted = false;

    // Reset all brick hit states
    for(unsigned int row = 0; row < arkanoid_state->ROWS; row++) {
        for(unsigned int column = 0; column < arkanoid_state->COLUMNS; column++) {
            arkanoid_state->brick_state.isHit[row][column] = false;
        }
    }
}

static void arkanoid_state_init(ArkanoidState* arkanoid_state) {
    // Init notification
    arkanoid_state->notify = furi_record_open(RECORD_NOTIFICATION);

    // Set the initial game state
    arkanoid_state->COLUMNS = 13;
    arkanoid_state->ROWS = 4;
    arkanoid_state->ball_state.dx = -1;
    arkanoid_state->ball_state.dy = -1;
    arkanoid_state->speed = 2;
    arkanoid_state->bounced = false;
    arkanoid_state->lives = 3;
    arkanoid_state->level = 1;
    arkanoid_state->score = 0;
    arkanoid_state->COLUMNS = 13;
    arkanoid_state->COLUMNS = 13;

    // Reset initial state
    arkanoid_state->initialDraw = false;
    arkanoid_state->gameStarted = false;
}

static void arkanoid_draw_callback(Canvas* const canvas, void* ctx) {
    furi_assert(ctx);
    ArkanoidState* arkanoid_state = ctx;
    furi_mutex_acquire(arkanoid_state->mutex, FuriWaitForever);

    //Initial level draw
    if(!arkanoid_state->initialDraw) {
        arkanoid_state->initialDraw = true;

        // Set default font for text
        canvas_set_font(canvas, FontSecondary);

        //Draws the new level
        reset_level(canvas, arkanoid_state);
    }

    //Draws new bricks and resets their values
    for(unsigned int row = 0; row < arkanoid_state->ROWS; row++) {
        for(unsigned int column = 0; column < arkanoid_state->COLUMNS; column++) {
            if(!arkanoid_state->brick_state.isHit[row][column]) {
                canvas_draw_frame(canvas, 10 * column, 2 + 6 * row, 8, 4);
            }
        }
    }

    if(arkanoid_state->lives > 0) {
        draw_paddle(canvas, arkanoid_state);
        draw_ball(canvas, arkanoid_state);
        draw_score(canvas, arkanoid_state);
        draw_lives(canvas, arkanoid_state);

        if(arkanoid_state->brickCount == arkanoid_state->ROWS * arkanoid_state->COLUMNS) {
            arkanoid_state->level++;
            reset_level(canvas, arkanoid_state);
        }
    } else {
        reset_level(canvas, arkanoid_state);
        arkanoid_state->initialDraw = false;
        arkanoid_state->lives = 3;
        arkanoid_state->score = 0;
    }

    furi_mutex_release(arkanoid_state->mutex);
}

static void arkanoid_input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    GameEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void arkanoid_update_timer_callback(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    GameEvent event = {.type = EventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
}

int32_t arkanoid_game_app(void* p) {
    UNUSED(p);
    int32_t return_code = 0;

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(GameEvent));

    ArkanoidState* arkanoid_state = malloc(sizeof(ArkanoidState));
    arkanoid_state_init(arkanoid_state);

    arkanoid_state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    if(!arkanoid_state->mutex) {
        FURI_LOG_E(TAG, "Cannot create mutex\r\n");
        return_code = 255;
        goto free_and_exit;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, arkanoid_draw_callback, arkanoid_state);
    view_port_input_callback_set(view_port, arkanoid_input_callback, event_queue);

    FuriTimer* timer =
        furi_timer_alloc(arkanoid_update_timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_kernel_get_tick_frequency() / 22);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Call dolphin deed on game start
    DOLPHIN_DEED(DolphinDeedPluginGameStart);

    GameEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        furi_mutex_acquire(arkanoid_state->mutex, FuriWaitForever);

        if(event_status == FuriStatusOk) {
            // Key events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress || event.input.type == InputTypeLong ||
                   event.input.type == InputTypeRepeat) {
                    switch(event.input.key) {
                    case InputKeyBack:
                        processing = false;
                        break;
                    case InputKeyRight:
                        if(arkanoid_state->xPaddle < FLIPPER_LCD_WIDTH - 12) {
                            arkanoid_state->xPaddle += 8;
                        }
                        break;
                    case InputKeyLeft:
                        if(arkanoid_state->xPaddle > 0) {
                            arkanoid_state->xPaddle -= 8;
                        }
                        break;
                    case InputKeyUp:
                        if(arkanoid_state->speed < MAX_SPEED) {
                            arkanoid_state->speed++;
                        }
                        break;
                    case InputKeyDown:
                        if(arkanoid_state->speed > 1) {
                            arkanoid_state->speed--;
                        }
                        break;
                    case InputKeyOk:
                        if(arkanoid_state->gameStarted == false) {
                            //Release ball if FIRE pressed
                            arkanoid_state->ball_state.released = true;

                            //Apply random direction to ball on release
                            if(rand_range(0, 2) == 0) {
                                arkanoid_state->ball_state.dx = 1;
                            } else {
                                arkanoid_state->ball_state.dx = -1;
                            }

                            //Makes sure the ball heads upwards
                            arkanoid_state->ball_state.dy = -1;
                            //start the game flag
                            arkanoid_state->gameStarted = true;
                        }
                        break;
                    default:
                        break;
                    }
                }
            }
        }

        view_port_update(view_port);
        furi_mutex_release(arkanoid_state->mutex);
    }
    furi_timer_free(timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    view_port_free(view_port);
    furi_mutex_free(arkanoid_state->mutex);

free_and_exit:
    free(arkanoid_state);
    furi_message_queue_free(event_queue);

    return return_code;
}
