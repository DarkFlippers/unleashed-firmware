#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <gui/view.h>

#define TAG "Arkanoid"

unsigned int COLUMNS = 13; //Columns of bricks
unsigned int ROWS = 4; //Rows of bricks
int dx = -1; //Initial movement of ball
int dy = -1; //Initial movement of ball
int xb; //Balls starting possition
int yb; //Balls starting possition
bool released; //If the ball has been released by the player
bool paused = false; //If the game has been paused
int xPaddle; //X position of paddle
bool isHit[4][13]; //Array of if bricks are hit or not
bool bounced = false; //Used to fix double bounce glitch
int lives = 3; //Amount of lives
int level = 1; //Current level
unsigned int score = 0; //Score for the game
unsigned int brickCount; //Amount of bricks hit
int pad, pad2, pad3; //Button press buffer used to stop pause repeating
int oldpad, oldpad2, oldpad3;
char text[16]; //General string buffer
bool start = false; //If in menu or in game
bool initialDraw = false; //If the inital draw has happened
char initials[3]; //Initials used in high score

//Ball Bounds used in collision detection
int leftBall;
int rightBall;
int topBall;
int bottomBall;

//Brick Bounds used in collision detection
int leftBrick;
int rightBrick;
int topBrick;
int bottomBrick;

int tick;

#define FLIPPER_LCD_WIDTH 128
#define FLIPPER_LCD_HEIGHT 64

typedef enum { EventTypeTick, EventTypeKey } EventType;

typedef enum { DirectionUp, DirectionRight, DirectionDown, DirectionLeft } Direction;

typedef enum { GameStatePlaying, GameStateGameOver } GameState;

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    GameState game_state;
} ArkanoidState;

typedef struct {
    EventType type;
    InputEvent input;
} GameEvent;

// generate number in range [min,max)
int rand_range(int min, int max) {
    int number = min + rand() % (max - min);
    return number;
}

void intro(Canvas* canvas) {
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 46, 0, "Arkanoid");

    //arduboy.tunes.tone(987, 160);
    //delay(160);
    //arduboy.tunes.tone(1318, 400);
    //delay(2000);
}

void move_ball(Canvas* canvas) {
    tick++;
    if(released) {
        //Move ball
        if(abs(dx) == 2) {
            xb += dx / 2;
            // 2x speed is really 1.5 speed
            if(tick % 2 == 0) xb += dx / 2;
        } else {
            xb += dx;
        }
        yb = yb + dy;

        //Set bounds
        leftBall = xb;
        rightBall = xb + 2;
        topBall = yb;
        bottomBall = yb + 2;

        //Bounce off top edge
        if(yb <= 0) {
            yb = 2;
            dy = -dy;
            // arduboy.tunes.tone(523, 250);
        }

        //Lose a life if bottom edge hit
        if(yb >= FLIPPER_LCD_HEIGHT) {
            canvas_draw_frame(canvas, xPaddle, FLIPPER_LCD_HEIGHT - 1, 11, 1);
            xPaddle = 54;
            yb = 60;
            released = false;
            lives--;

            sprintf(text, "LIVES:%u", lives);
            canvas_draw_str(canvas, 0, 90, text);

            // arduboy.tunes.tone(175, 250);
            if(rand_range(0, 2) == 0) {
                dx = 1;
            } else {
                dx = -1;
            }
        }

        //Bounce off left side
        if(xb <= 0) {
            xb = 2;
            dx = -dx;
            // arduboy.tunes.tone(523, 250);
        }

        //Bounce off right side
        if(xb >= FLIPPER_LCD_WIDTH - 2) {
            xb = FLIPPER_LCD_WIDTH - 4;
            dx = -dx;
            // arduboy.tunes.tone(523, 250);
        }

        //Bounce off paddle
        if(xb + 1 >= xPaddle && xb <= xPaddle + 12 && yb + 2 >= FLIPPER_LCD_HEIGHT - 1 &&
           yb <= FLIPPER_LCD_HEIGHT) {
            dy = -dy;
            dx = ((xb - (xPaddle + 6)) / 3); //Applies spin on the ball
            // prevent straight bounce
            if(dx == 0) {
                dx = (rand_range(0, 2) == 1) ? 1 : -1;
            }
            // arduboy.tunes.tone(200, 250);
        }

        //Bounce off Bricks
        for(unsigned int row = 0; row < ROWS; row++) {
            for(unsigned int column = 0; column < COLUMNS; column++) {
                if(!isHit[row][column]) {
                    //Sets Brick bounds
                    leftBrick = 10 * column;
                    rightBrick = 10 * column + 10;
                    topBrick = 6 * row + 1;
                    bottomBrick = 6 * row + 7;

                    //If A collison has occured
                    if(topBall <= bottomBrick && bottomBall >= topBrick &&
                       leftBall <= rightBrick && rightBall >= leftBrick) {
                        // Draw score
                        score += (level * 10);
                        sprintf(text, "SCORE:%u", score);
                        canvas_draw_str(canvas, 80, 90, text);

                        brickCount++;
                        isHit[row][column] = true;
                        canvas_draw_frame(canvas, 10 * column, 2 + 6 * row, 8, 4);

                        //Vertical collision
                        if(bottomBall > bottomBrick || topBall < topBrick) {
                            //Only bounce once each ball move
                            if(!bounced) {
                                dy = -dy;
                                yb += dy;
                                bounced = true;
                                // arduboy.tunes.tone(261, 250);
                            }
                        }

                        //Hoizontal collision
                        if(leftBall < leftBrick || rightBall > rightBrick) {
                            //Only bounce once brick each ball move
                            if(!bounced) {
                                dx = -dx;
                                xb += dx;
                                bounced = true;
                                // arduboy.tunes.tone(261, 250);
                            }
                        }
                    }
                }
            }
        }

        //Reset Bounce
        bounced = false;
    } else {
        //Ball follows paddle
        xb = xPaddle + 5;
    }
}

void draw_lives(Canvas* canvas) {
    sprintf(text, "LIVES:%u", lives);
    canvas_draw_str(canvas, 0, 90, text);
}

void draw_ball(Canvas* canvas) {
    canvas_draw_dot(canvas, xb, yb);
    canvas_draw_dot(canvas, xb + 1, yb);
    canvas_draw_dot(canvas, xb, yb + 1);
    canvas_draw_dot(canvas, xb + 1, yb + 1);

    move_ball(canvas);
}

void draw_paddle(Canvas* canvas) {
    canvas_draw_frame(canvas, xPaddle, FLIPPER_LCD_HEIGHT - 1, 11, 1);
}

void reset_level(Canvas* canvas) {
    //Undraw paddle
    canvas_draw_frame(canvas, xPaddle, FLIPPER_LCD_HEIGHT - 1, 11, 1);

    //Undraw ball
    canvas_draw_dot(canvas, xb, yb);
    canvas_draw_dot(canvas, xb + 1, yb);
    canvas_draw_dot(canvas, xb, yb + 1);
    canvas_draw_dot(canvas, xb + 1, yb + 1);

    //Alter various variables to reset the game
    xPaddle = 54;
    yb = 60;
    brickCount = 0;
    released = false;

    // Reset all brick hit states
    for(unsigned int row = 0; row < ROWS; row++) {
        for(unsigned int column = 0; column < COLUMNS; column++) {
            isHit[row][column] = false;
        }
    }
}

static void arkanoid_state_init(ArkanoidState* const arkanoid_state) {
    // Set the initial game state
    arkanoid_state->game_state = GameStatePlaying;

    // Reset initial state
    initialDraw = false;
}

static void arkanoid_draw_callback(Canvas* const canvas, void* ctx) {
    const ArkanoidState* arkanoid_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(arkanoid_state == NULL) {
        return;
    }

    //Initial level draw
    if(!initialDraw) {
        initialDraw = true;

        // Set default font for text
        canvas_set_font(canvas, FontPrimary);

        //Draws the new level
        reset_level(canvas);
    }

    //Draws new bricks and resets their values
    for(unsigned int row = 0; row < ROWS; row++) {
        for(unsigned int column = 0; column < COLUMNS; column++) {
            if(!isHit[row][column]) {
                canvas_draw_frame(canvas, 10 * column, 2 + 6 * row, 8, 4);
            }
        }
    }

    if(lives > 0) {
        draw_paddle(canvas);

        draw_ball(canvas);

        if(brickCount == ROWS * COLUMNS) {
            level++;
            reset_level(canvas);
        }
    } else {
        reset_level(canvas);
        initialDraw = false;
        start = false;
        lives = 3;
        score = 0;
    }

    release_mutex((ValueMutex*)ctx, arkanoid_state);
}

static void arkanoid_input_callback(InputEvent* input_event, osMessageQueueId_t event_queue) {
    furi_assert(event_queue);

    GameEvent event = {.type = EventTypeKey, .input = *input_event};
    osMessageQueuePut(event_queue, &event, 0, osWaitForever);
}

static void arkanoid_update_timer_callback(osMessageQueueId_t event_queue) {
    furi_assert(event_queue);

    GameEvent event = {.type = EventTypeTick};
    osMessageQueuePut(event_queue, &event, 0, 0);
}

int32_t arkanoid_game_app(void* p) {
    UNUSED(p);
    // Set random seed from interrupts
    srand(DWT->CYCCNT);

    osMessageQueueId_t event_queue = osMessageQueueNew(8, sizeof(GameEvent), NULL);

    ArkanoidState* arkanoid_state = malloc(sizeof(ArkanoidState));
    arkanoid_state_init(arkanoid_state);

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, arkanoid_state, sizeof(ArkanoidState))) {
        FURI_LOG_E(TAG, "Cannot create mutex\r\n");
        free(arkanoid_state);
        return 255;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, arkanoid_draw_callback, &state_mutex);
    view_port_input_callback_set(view_port, arkanoid_input_callback, event_queue);

    osTimerId_t timer =
        osTimerNew(arkanoid_update_timer_callback, osTimerPeriodic, event_queue, NULL);
    osTimerStart(timer, osKernelGetTickFreq() / 22);

    // Open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    GameEvent event;
    for(bool processing = true; processing;) {
        osStatus_t event_status = osMessageQueueGet(event_queue, &event, NULL, 100);
        ArkanoidState* arkanoid_state = (ArkanoidState*)acquire_mutex_block(&state_mutex);

        if(event_status == osOK) {
            // Key events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress || event.input.type == InputTypeLong ||
                   event.input.type == InputTypeRepeat) {
                    switch(event.input.key) {
                    case InputKeyBack:
                        processing = false;
                        break;
                    case InputKeyRight:
                        if(xPaddle < FLIPPER_LCD_WIDTH - 12) {
                            xPaddle += 8;
                        }
                        break;
                    case InputKeyLeft:
                        if(xPaddle > 0) {
                            xPaddle -= 8;
                        }
                        break;
                    case InputKeyUp:
                        break;
                    case InputKeyDown:
                        break;
                    case InputKeyOk:
                        //Release ball if FIRE pressed
                        released = true;

                        //Apply random direction to ball on release
                        if(rand_range(0, 2) == 0) {
                            dx = 1;
                        } else {
                            dx = -1;
                        }

                        //Makes sure the ball heads upwards
                        dy = -1;
                        break;
                    }
                }
            }
        } else {
            // Event timeout
            FURI_LOG_D(TAG, "osMessageQueue: Event timeout");
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, arkanoid_state);
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    osMessageQueueDelete(event_queue);

    return 0;
}