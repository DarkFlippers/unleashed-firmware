#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <math.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define PLAYER_INIT_LOCATION_X 20
#define PLAYER_INIT_AIM 45
#define PLAYER_INIT_POWER 50
#define ENEMY_INIT_LOCATION_X 108
#define TANK_BARREL_LENGTH 8
#define GRAVITY_FORCE (double)0.5
#define MIN_GROUND_HEIGHT 35
#define MAX_GROUND_HEIGHT 55
#define MAX_FIRE_POWER 100
#define MIN_FIRE_POWER 0
#define TANK_COLLIDER_SIZE 3
#define MAX_WIND 10
#define MAX_PLAYER_DIFF_X 20
#define MAX_ENEMY_DIFF_X 20

// That's a filthy workaround but sin(player.aimAngle) breaks it all... If you're able to fix it, please do create a PR!
double scorched_tanks_sin[91] = {
    0.000,  -0.017, -0.035, -0.052, -0.070, -0.087, -0.105, -0.122, -0.139, -0.156, -0.174, -0.191,
    -0.208, -0.225, -0.242, -0.259, -0.276, -0.292, -0.309, -0.326, -0.342, -0.358, -0.375, -0.391,
    -0.407, -0.423, -0.438, -0.454, -0.469, -0.485, -0.500, -0.515, -0.530, -0.545, -0.559, -0.574,
    -0.588, -0.602, -0.616, -0.629, -0.643, -0.656, -0.669, -0.682, -0.695, -0.707, -0.719, -0.731,
    -0.743, -0.755, -0.766, -0.777, -0.788, -0.799, -0.809, -0.819, -0.829, -0.839, -0.848, -0.857,
    -0.866, -0.875, -0.883, -0.891, -0.899, -0.906, -0.914, -0.921, -0.927, -0.934, -0.940, -0.946,
    -0.951, -0.956, -0.961, -0.966, -0.970, -0.974, -0.978, -0.982, -0.985, -0.988, -0.990, -0.993,
    -0.995, -0.996, -0.998, -0.999, -0.999, -1.000, -1.000};
double scorched_tanks_cos[91] = {
    1.000, 1.000, 0.999, 0.999, 0.998, 0.996, 0.995, 0.993, 0.990, 0.988, 0.985, 0.982, 0.978,
    0.974, 0.970, 0.966, 0.961, 0.956, 0.951, 0.946, 0.940, 0.934, 0.927, 0.921, 0.914, 0.906,
    0.899, 0.891, 0.883, 0.875, 0.866, 0.857, 0.848, 0.839, 0.829, 0.819, 0.809, 0.799, 0.788,
    0.777, 0.766, 0.755, 0.743, 0.731, 0.719, 0.707, 0.695, 0.682, 0.669, 0.656, 0.643, 0.629,
    0.616, 0.602, 0.588, 0.574, 0.559, 0.545, 0.530, 0.515, 0.500, 0.485, 0.469, 0.454, 0.438,
    0.423, 0.407, 0.391, 0.375, 0.358, 0.342, 0.326, 0.309, 0.292, 0.276, 0.259, 0.242, 0.225,
    0.208, 0.191, 0.174, 0.156, 0.139, 0.122, 0.105, 0.087, 0.070, 0.052, 0.035, 0.017, 0.000};
double scorched_tanks_tan[91] = {
    0.000,   -0.017,  -0.035,    -0.052, -0.070, -0.087, -0.105, -0.123, -0.141,  -0.158,  -0.176,
    -0.194,  -0.213,  -0.231,    -0.249, -0.268, -0.287, -0.306, -0.325, -0.344,  -0.364,  -0.384,
    -0.404,  -0.424,  -0.445,    -0.466, -0.488, -0.510, -0.532, -0.554, -0.577,  -0.601,  -0.625,
    -0.649,  -0.674,  -0.700,    -0.727, -0.754, -0.781, -0.810, -0.839, -0.869,  -0.900,  -0.932,
    -0.966,  -1.000,  -1.036,    -1.072, -1.111, -1.150, -1.192, -1.235, -1.280,  -1.327,  -1.376,
    -1.428,  -1.483,  -1.540,    -1.600, -1.664, -1.732, -1.804, -1.881, -1.963,  -2.050,  -2.144,
    -2.246,  -2.356,  -2.475,    -2.605, -2.747, -2.904, -3.078, -3.271, -3.487,  -3.732,  -4.011,
    -4.331,  -4.704,  -5.144,    -5.671, -6.313, -7.115, -8.144, -9.513, -11.429, -14.298, -19.077,
    -28.627, -57.254, -90747.269};
unsigned char scorched_tanks_ground_modifiers[SCREEN_WIDTH] = {
    0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  2,  4,  6,  8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 28, 26, 24, 22, 20,
    18, 16, 14, 12, 10, 8, 6,  4,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};

typedef struct {
    //    +-----x
    //    |
    //    |
    //    y
    uint8_t x;
    uint8_t y;
} Point;

typedef struct {
    //    +-----x
    //    |
    //    |
    //    y
    double x;
    double y;
} PointDetailed;

typedef struct {
    unsigned char locationX;
    unsigned char hp;
    int aimAngle;
    unsigned char firePower;
} Tank;

typedef struct {
    Point ground[SCREEN_WIDTH];
    Tank player;
    Tank enemy;
    bool isPlayerTurn;
    bool isShooting;
    int windSpeed;
    Point trajectory[SCREEN_WIDTH];
    unsigned char trajectoryAnimationStep;
    PointDetailed bulletPosition;
    PointDetailed bulletVector;
} Game;

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} ScorchedTanksEvent;

int scorched_tanks_random(int min, int max) {
    return min + rand() % ((max + 1) - min);
}

void scorched_tanks_generate_ground(Game* game_state) {
    int lastHeight = 45;

    for(unsigned char a = 0; a < SCREEN_WIDTH; a++) {
        int diffHeight = scorched_tanks_random(-2, 3);
        int changeLength = scorched_tanks_random(1, 6);

        if(diffHeight == 0) {
            changeLength = 1;
        }

        for(int b = 0; b < changeLength; b++) {
            if(a + b < SCREEN_WIDTH) {
                int index = a + b;
                int newPoint = lastHeight + diffHeight;
                newPoint = newPoint < MIN_GROUND_HEIGHT ? MIN_GROUND_HEIGHT : newPoint;
                newPoint = newPoint > MAX_GROUND_HEIGHT ? MAX_GROUND_HEIGHT : newPoint;
                game_state->ground[index].x = index;
                game_state->ground[index].y = newPoint - scorched_tanks_ground_modifiers[a];
                lastHeight = newPoint;
            } else {
                a += b;
                break;
            }
        }

        a += changeLength - 1;
    }
}

void scorched_tanks_init_game(Game* game_state) {
    game_state->player.locationX = PLAYER_INIT_LOCATION_X +
                                   scorched_tanks_random(0, MAX_PLAYER_DIFF_X) -
                                   MAX_PLAYER_DIFF_X / 2;
    game_state->player.aimAngle = PLAYER_INIT_AIM;
    game_state->player.firePower = PLAYER_INIT_POWER;
    game_state->enemy.aimAngle = PLAYER_INIT_AIM;
    game_state->enemy.firePower = PLAYER_INIT_POWER;
    game_state->enemy.locationX =
        ENEMY_INIT_LOCATION_X + scorched_tanks_random(0, MAX_ENEMY_DIFF_X) - MAX_ENEMY_DIFF_X / 2;
    game_state->isPlayerTurn = true;

    game_state->windSpeed = scorched_tanks_random(0, MAX_WIND);

    for(int x = 0; x < SCREEN_WIDTH; x++) {
        game_state->trajectory[x].x = 0;
        game_state->trajectory[x].y = 0;
    }

    scorched_tanks_generate_ground(game_state);
}

void scorched_tanks_calculate_trajectory(Game* game_state) {
    if(game_state->isShooting) {
        game_state->bulletVector.x += ((double)game_state->windSpeed - MAX_WIND / 2) / 40;
        game_state->bulletVector.y += GRAVITY_FORCE;

        game_state->bulletPosition.x += game_state->bulletVector.x;
        game_state->bulletPosition.y += game_state->bulletVector.y;

        int totalDistanceToEnemy = 100;

        if(game_state->isPlayerTurn) {
            double distanceToEnemyX = game_state->enemy.locationX - game_state->bulletPosition.x;
            double distanceToEnemyY = game_state->ground[game_state->enemy.locationX].y -
                                      TANK_COLLIDER_SIZE - game_state->bulletPosition.y;
            totalDistanceToEnemy =
                sqrt(distanceToEnemyX * distanceToEnemyX + distanceToEnemyY * distanceToEnemyY);
        } else {
            double distanceToEnemyX = game_state->player.locationX - game_state->bulletPosition.x;
            double distanceToEnemyY = game_state->ground[game_state->player.locationX].y -
                                      TANK_COLLIDER_SIZE - game_state->bulletPosition.y;
            totalDistanceToEnemy =
                sqrt(distanceToEnemyX * distanceToEnemyX + distanceToEnemyY * distanceToEnemyY);
        }

        if(totalDistanceToEnemy <= TANK_COLLIDER_SIZE) {
            game_state->isShooting = false;
            scorched_tanks_init_game(game_state);
            game_state->isPlayerTurn = !game_state->isPlayerTurn;
            return;
        }

        if(game_state->bulletPosition.x > SCREEN_WIDTH ||
           game_state->bulletPosition.y >
               game_state->ground[(int)round(game_state->bulletPosition.x)].y) {
            game_state->isShooting = false;
            game_state->bulletPosition.x = 0;
            game_state->bulletPosition.y = 0;
            game_state->windSpeed = scorched_tanks_random(0, MAX_WIND);
            game_state->isPlayerTurn = !game_state->isPlayerTurn;
            return;
        }

        if(game_state->bulletPosition.y > 0) {
            game_state->trajectory[game_state->trajectoryAnimationStep].x =
                round(game_state->bulletPosition.x);
            game_state->trajectory[game_state->trajectoryAnimationStep].y =
                round(game_state->bulletPosition.y);
            game_state->trajectoryAnimationStep++;
        }
    }
}

static void scorched_tanks_draw_tank(
    Canvas* const canvas,
    unsigned char x,
    unsigned char y,
    bool isPlayer) {
    unsigned char lineIndex = 0;

    if(isPlayer) {
        // Draw tank base
        canvas_draw_line(canvas, x - 3, y - lineIndex, x + 3, y - lineIndex++);
        canvas_draw_line(canvas, x - 4, y - lineIndex, x + 4, y - lineIndex++);
        canvas_draw_line(canvas, x - 4, y - lineIndex, x + 4, y - lineIndex++);

        // draw turret
        canvas_draw_line(canvas, x - 2, y - lineIndex, x + 1, y - lineIndex++);
        canvas_draw_line(canvas, x - 2, y - lineIndex, x, y - lineIndex++);
    } else {
        // Draw tank base
        canvas_draw_line(canvas, x - 3, y - lineIndex, x + 3, y - lineIndex++);
        canvas_draw_line(canvas, x - 4, y - lineIndex, x + 4, y - lineIndex++);
        canvas_draw_line(canvas, x - 4, y - lineIndex, x + 4, y - lineIndex++);

        // draw turret
        canvas_draw_line(canvas, x - 1, y - lineIndex, x + 2, y - lineIndex++);
        canvas_draw_line(canvas, x, y - lineIndex, x + 2, y - lineIndex++);
    }
}

static void scorched_tanks_render_callback(Canvas* const canvas, void* ctx) {
    const Game* game_state = acquire_mutex((ValueMutex*)ctx, 25);

    if(game_state == NULL) {
        return;
    }

    canvas_draw_frame(canvas, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    canvas_set_color(canvas, ColorBlack);

    if(game_state->isShooting) {
        canvas_draw_dot(canvas, game_state->bulletPosition.x, game_state->bulletPosition.y);
    }

    for(int a = 1; a < SCREEN_WIDTH; a++) {
        canvas_draw_line(
            canvas,
            game_state->ground[a - 1].x,
            game_state->ground[a - 1].y,
            game_state->ground[a].x,
            game_state->ground[a].y);

        if(game_state->trajectory[a].y != 0) {
            canvas_draw_dot(canvas, game_state->trajectory[a].x, game_state->trajectory[a].y);
        }
    }

    scorched_tanks_draw_tank(
        canvas,
        game_state->enemy.locationX,
        game_state->ground[game_state->enemy.locationX].y - TANK_COLLIDER_SIZE,
        true);

    scorched_tanks_draw_tank(
        canvas,
        game_state->player.locationX,
        game_state->ground[game_state->player.locationX].y - TANK_COLLIDER_SIZE,
        false);

    int aimX1 = 0;
    int aimY1 = 0;
    int aimX2 = 0;
    int aimY2 = 0;

    if(game_state->isPlayerTurn) {
        aimX1 = game_state->player.locationX;
        aimY1 = game_state->ground[game_state->player.locationX].y - TANK_COLLIDER_SIZE;

        double sinFromAngle = scorched_tanks_sin[game_state->player.aimAngle];
        double cosFromAngle = scorched_tanks_cos[game_state->player.aimAngle];
        aimX2 = aimX1 + TANK_BARREL_LENGTH * cosFromAngle;
        aimY2 = aimY1 + TANK_BARREL_LENGTH * sinFromAngle;

        aimX1 += 1;
        aimX2 += 1;
    } else {
        aimX1 = game_state->enemy.locationX;
        aimY1 = game_state->ground[game_state->enemy.locationX].y - TANK_COLLIDER_SIZE;

        double sinFromAngle = scorched_tanks_sin[game_state->enemy.aimAngle];
        double cosFromAngle = scorched_tanks_cos[game_state->enemy.aimAngle];
        aimX2 = aimX1 + TANK_BARREL_LENGTH * cosFromAngle;
        aimY2 = aimY1 + TANK_BARREL_LENGTH * sinFromAngle;

        aimX2 = aimX1 - (aimX2 - aimX1);

        aimX1 -= 1;
        aimX2 -= 1;
    }

    canvas_draw_line(canvas, aimX1, aimY1 - 3, aimX2, aimY2 - 3);

    canvas_set_font(canvas, FontSecondary);

    char buffer2[12];
    snprintf(buffer2, sizeof(buffer2), "wind: %i", game_state->windSpeed - MAX_WIND / 2);
    canvas_draw_str(canvas, 55, 10, buffer2);

    if(game_state->isPlayerTurn) {
        canvas_draw_str(canvas, 93, 10, "player1");

        char buffer[12];
        snprintf(buffer, sizeof(buffer), "a: %u", game_state->player.aimAngle);
        canvas_draw_str(canvas, 2, 10, buffer);

        snprintf(buffer, sizeof(buffer), "p: %u", game_state->player.firePower);
        canvas_draw_str(canvas, 27, 10, buffer);
    } else {
        canvas_draw_str(canvas, 93, 10, "player2");

        char buffer[12];
        snprintf(buffer, sizeof(buffer), "a: %u", game_state->enemy.aimAngle);
        canvas_draw_str(canvas, 2, 10, buffer);

        snprintf(buffer, sizeof(buffer), "p: %u", game_state->enemy.firePower);
        canvas_draw_str(canvas, 27, 10, buffer);
    }

    release_mutex((ValueMutex*)ctx, game_state);
}

static void scorched_tanks_input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    ScorchedTanksEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void scorched_tanks_update_timer_callback(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    ScorchedTanksEvent event = {.type = EventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
}

static void scorched_tanks_increase_power(Game* game_state) {
    if(game_state->player.firePower < MAX_FIRE_POWER && !game_state->isShooting) {
        if(game_state->isPlayerTurn && game_state->player.firePower < MAX_FIRE_POWER) {
            game_state->player.firePower++;
        }

        if(!game_state->isPlayerTurn && game_state->enemy.firePower < MAX_FIRE_POWER) {
            game_state->enemy.firePower++;
        }
    }
}

static void scorched_tanks_decrease_power(Game* game_state) {
    if(game_state->player.firePower > MIN_FIRE_POWER && !game_state->isShooting) {
        if(game_state->isPlayerTurn && game_state->player.firePower > MIN_FIRE_POWER) {
            game_state->player.firePower--;
        }

        if(!game_state->isPlayerTurn && game_state->enemy.firePower > MIN_FIRE_POWER) {
            game_state->enemy.firePower--;
        }
    }
}

static void scorched_tanks_aim_up(Game* game_state) {
    if(!game_state->isShooting) {
        if(game_state->isPlayerTurn && game_state->player.aimAngle < 90) {
            game_state->player.aimAngle++;
        }

        if(!game_state->isPlayerTurn && game_state->enemy.aimAngle < 90) {
            game_state->enemy.aimAngle++;
        }
    }
}

static void scorched_tanks_aim_down(Game* game_state) {
    if(game_state->player.aimAngle > 0 && !game_state->isShooting) {
        if(game_state->isPlayerTurn) {
            game_state->player.aimAngle--;
        } else {
            game_state->enemy.aimAngle--;
        }
    }
}

const NotificationSequence sequence_long_vibro = {
    &message_vibro_on,
    &message_delay_500,
    &message_vibro_off,
    NULL,
};

static void scorched_tanks_fire(Game* game_state) {
    if(!game_state->isShooting) {
        if(game_state->isPlayerTurn) {
            double sinFromAngle = scorched_tanks_sin[game_state->player.aimAngle];
            double cosFromAngle = scorched_tanks_cos[game_state->player.aimAngle];
            unsigned char aimX1 = game_state->player.locationX;
            unsigned char aimY1 =
                game_state->ground[game_state->player.locationX].y - TANK_COLLIDER_SIZE;
            int aimX2 = aimX1 + TANK_BARREL_LENGTH * cosFromAngle;
            int aimY2 = aimY1 + TANK_BARREL_LENGTH * sinFromAngle;
            game_state->bulletPosition.x = aimX2;
            game_state->bulletPosition.y = aimY2;
            game_state->bulletVector.x = scorched_tanks_cos[game_state->player.aimAngle] *
                                         ((double)game_state->player.firePower / 10);
            game_state->bulletVector.y = scorched_tanks_sin[game_state->player.aimAngle] *
                                         ((double)game_state->player.firePower / 10);
        } else {
            double sinFromAngle = scorched_tanks_sin[game_state->enemy.aimAngle];
            double cosFromAngle = scorched_tanks_cos[game_state->enemy.aimAngle];
            unsigned char aimX1 = game_state->enemy.locationX;
            unsigned char aimY1 =
                game_state->ground[game_state->enemy.locationX].y - TANK_COLLIDER_SIZE;
            int aimX2 = aimX1 + TANK_BARREL_LENGTH * cosFromAngle;
            int aimY2 = aimY1 + TANK_BARREL_LENGTH * sinFromAngle;
            aimX2 = aimX1 - (aimX2 - aimX1);

            game_state->bulletPosition.x = aimX2;
            game_state->bulletPosition.y = aimY2;
            game_state->bulletVector.x = -scorched_tanks_cos[game_state->enemy.aimAngle] *
                                         ((double)game_state->enemy.firePower / 10);
            game_state->bulletVector.y = scorched_tanks_sin[game_state->enemy.aimAngle] *
                                         ((double)game_state->enemy.firePower / 10);
        }

        game_state->trajectoryAnimationStep = 0;

        for(int x = 0; x < SCREEN_WIDTH; x++) {
            game_state->trajectory[x].x = 0;
            game_state->trajectory[x].y = 0;
        }

        game_state->isShooting = true;

        NotificationApp* notification = furi_record_open("notification");
        notification_message(notification, &sequence_long_vibro);
        notification_message(notification, &sequence_blink_white_100);
        furi_record_close("notification");
    }
}

int32_t scorched_tanks_game_app(void* p) {
    UNUSED(p);
    srand(DWT->CYCCNT);

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(ScorchedTanksEvent));

    Game* game_state = malloc(sizeof(Game));
    scorched_tanks_init_game(game_state);

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, game_state, sizeof(ScorchedTanksEvent))) {
        FURI_LOG_E("ScorchedTanks", "cannot create mutex\r\n");
        free(game_state);
        return 255;
    }

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, scorched_tanks_render_callback, &state_mutex);
    view_port_input_callback_set(view_port, scorched_tanks_input_callback, event_queue);

    FuriTimer* timer =
        furi_timer_alloc(scorched_tanks_update_timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, 2000);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    ScorchedTanksEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 50);

        if(event.type == EventTypeKey) { // && game->isPlayerTurn
            if(event.input.type == InputTypeRepeat || event.input.type == InputTypeShort) {
                switch(event.input.key) {
                case InputKeyUp:
                    scorched_tanks_aim_up(game_state);
                    break;
                case InputKeyDown:
                    scorched_tanks_aim_down(game_state);
                    break;
                case InputKeyRight:
                    scorched_tanks_increase_power(game_state);
                    break;
                case InputKeyLeft:
                    scorched_tanks_decrease_power(game_state);
                    break;
                case InputKeyOk:
                    scorched_tanks_fire(game_state);
                    break;
                case InputKeyBack:
                    processing = false;
                    break;
                default:
                    break;
                }
            }
        } else if(event.type == EventTypeTick) {
            scorched_tanks_calculate_trajectory(game_state);
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, game_state);
    }

    furi_timer_free(timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    delete_mutex(&state_mutex);
    free(game_state);

    return 0;
}
