#include <stdlib.h>
#include <stdbool.h>

#include <jetpack_joyride_icons.h>
#include <gui/gui.h>

#include "states.h"
#include "game_sprites.h"
#include "missile.h"
#include "barry.h"

void missile_tick(MISSILE* const missiles, BARRY* const barry, void (*death_handler)()) {
    // Move missiles towards the player
    for(int i = 0; i < MISSILES_MAX; i++) {
        if(missiles[i].visible && missile_colides(&missiles[i], barry)) {
            death_handler();
        }
        if(missiles[i].visible) {
            missiles[i].point.x -= 2; // move left by 2 units
            if(missiles[i].point.x < -MISSILE_WIDTH) { // if the missile is out of screen
                missiles[i].visible = false; // set missile as "inactive"
            }
        }
    }
}

void spawn_random_missile(MISSILE* const missiles) {
    // Check for an available slot for a new missile
    for(int i = 0; i < MISSILES_MAX; ++i) {
        if(!missiles[i].visible) {
            missiles[i].point.x = 2 * SCREEN_WIDTH;
            missiles[i].point.y = rand() % (SCREEN_HEIGHT - MISSILE_HEIGHT);
            missiles[i].visible = true;
            break;
        }
    }
}

void draw_missiles(const MISSILE* missiles, Canvas* const canvas, const GameSprites* sprites) {
    for(int i = 0; i < MISSILES_MAX; ++i) {
        if(missiles[i].visible) {
            canvas_set_color(canvas, ColorBlack);

            if(missiles[i].point.x > 128) {
                canvas_draw_icon_animation(
                    canvas, SCREEN_WIDTH - 7, missiles[i].point.y, sprites->alert);
            } else {
                canvas_draw_icon_animation(
                    canvas, missiles[i].point.x, missiles[i].point.y, sprites->missile);

                canvas_set_color(canvas, ColorWhite);
                canvas_draw_icon(
                    canvas, missiles[i].point.x, missiles[i].point.y, sprites->missile_infill);
            }
        }
    }
}

bool missile_colides(MISSILE* const missile, BARRY* const barry) {
    return !(
        barry->point.x >
            missile->point.x + MISSILE_WIDTH - 14 || // Barry is to the right of the missile
        barry->point.x + BARRY_WIDTH - 3 <
            missile->point.x || // Barry is to the left of the missile
        barry->point.y > missile->point.y + MISSILE_HEIGHT || // Barry is below the missile
        barry->point.y + BARRY_HEIGHT < missile->point.y); // Barry is above the missile
}

int get_rocket_spawn_distance(int player_distance) {
    // Define the start and end points for rocket spawn distance
    int start_distance = 256;
    int end_distance = 24;

    // Define the maximum player distance at which the spawn distance should be at its minimum
    int max_player_distance = 5000; // Adjust this value based on your game's difficulty curve

    if(player_distance >= max_player_distance) {
        return end_distance;
    }

    // Calculate the linear interpolation factor
    float t = (float)player_distance / max_player_distance;

    // Interpolate the rocket spawn distance
    return start_distance + t * (end_distance - start_distance);
}
