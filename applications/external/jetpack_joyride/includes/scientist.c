#include "scientist.h"
#include "game_sprites.h"

#include <jetpack_joyride_icons.h>
#include <gui/gui.h>

void scientist_tick(SCIENTIST* const scientists) {
    for(int i = 0; i < SCIENTISTS_MAX; i++) {
        if(scientists[i].visible) {
            if(scientists[i].point.x < 64) scientists[i].velocity_x = 0.5f;

            scientists[i].point.x -= scientists[i].state == ScientistStateAlive ?
                                         1 - scientists[i].velocity_x :
                                         1; // move based on velocity_x
            int width = (scientists[i].state == ScientistStateAlive) ? SCIENTIST_WIDTH :
                                                                       SCIENTIST_HEIGHT;
            if(scientists[i].point.x <= -width) { // if the scientist is out of screen
                scientists[i].visible = false;
            }
        }
    }
}

void spawn_random_scientist(SCIENTIST* const scientists) {
    float velocities[] = {-0.5f, 0.0f, 0.5f, -1.0f};
    // Check for an available slot for a new scientist
    for(int i = 0; i < SCIENTISTS_MAX; ++i) {
        if(!scientists[i].visible &&
           (rand() % 1000) < 10) { // Spawn rate is less frequent than coins
            scientists[i].state = ScientistStateAlive;
            scientists[i].point.x = 127;
            scientists[i].point.y = 49;
            scientists[i].velocity_x = velocities[rand() % 4];
            scientists[i].visible = true;
            break;
        }
    }
}

void draw_scientists(const SCIENTIST* scientists, Canvas* const canvas, const GameSprites* sprites) {
    for(int i = 0; i < SCIENTISTS_MAX; ++i) {
        if(scientists[i].visible) {
            canvas_set_color(canvas, ColorBlack);
            if(scientists[i].state == ScientistStateAlive) {
                canvas_draw_icon(
                    canvas,
                    (int)scientists[i].point.x,
                    scientists[i].point.y,
                    scientists[i].velocity_x >= 0 ? sprites->scientist_right :
                                                    sprites->scientist_left);

                canvas_set_color(canvas, ColorWhite);
                canvas_draw_icon(
                    canvas,
                    (int)scientists[i].point.x,
                    scientists[i].point.y,
                    scientists[i].velocity_x >= 0 ? sprites->scientist_right_infill :
                                                    sprites->scientist_left_infill);

            } else {
                canvas_set_color(canvas, ColorBlack);
                canvas_draw_icon(
                    canvas,
                    (int)scientists[i].point.x,
                    scientists[i].point.y + 5,
                    &I_dead_scientist);

                canvas_set_color(canvas, ColorWhite);
                canvas_draw_icon(
                    canvas,
                    (int)scientists[i].point.x,
                    scientists[i].point.y + 5,
                    &I_dead_scientist_infill);
            }
        }
    }
}