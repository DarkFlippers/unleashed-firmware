#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include "sound.h"
#include "display.h"
#include "assets.h"
#include "constants.h"
#include "entities.h"
#include "types.h"
#include "level.h"
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <dolphin/dolphin.h>

#define SOUND

// Useful macros
#define swap(a, b)          \
    do {                    \
        typeof(a) temp = a; \
        a = b;              \
        b = temp;           \
    } while(0)
#define sign(a, b) (double)(a > b ? 1 : (b > a ? -1 : 0))
#define pgm_read_byte(addr) (*(const unsigned char*)(addr))

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef struct {
    FuriMutex* mutex;
    Player player;
    Entity entity[MAX_ENTITIES];
    StaticEntity static_entity[MAX_STATIC_ENTITIES];
    uint8_t num_entities;
    uint8_t num_static_entities;

    uint8_t scene;
    uint8_t gun_pos;
    double jogging;
    double view_height;
    bool init;

    bool up;
    bool down;
    bool left;
    bool right;
    bool fired;
    bool gun_fired;

    double rot_speed;
    double old_dir_x;
    double old_plane_x;
    NotificationApp* notify;
#ifdef SOUND
    MusicPlayer* music_instance;
    bool intro_sound;
#endif
} PluginState;

static const NotificationSequence sequence_short_sound = {
    &message_note_c5,
    &message_delay_50,
    &message_sound_off,
    NULL,
};
static const NotificationSequence sequence_long_sound = {
    &message_note_c3,
    &message_delay_100,
    &message_sound_off,
    NULL,
};

Coords translateIntoView(Coords* pos, PluginState* const plugin_state);
void updateHud(Canvas* const canvas, PluginState* const plugin_state);
// general

bool invert_screen = false;
uint8_t flash_screen = 0;

// game
// player and entities

uint8_t getBlockAt(const uint8_t level[], uint8_t x, uint8_t y) {
    if(x >= LEVEL_WIDTH || y >= LEVEL_HEIGHT) {
        return E_FLOOR;
    }

    // y is read in inverse order
    return pgm_read_byte(level + (((LEVEL_HEIGHT - 1 - y) * LEVEL_WIDTH + x) / 2)) >>
               (!(x % 2) * 4) // displace part of wanted bits
           & 0b1111; // mask wanted bits
}

// Finds the player in the map
void initializeLevel(const uint8_t level[], PluginState* const plugin_state) {
    for(uint8_t y = LEVEL_HEIGHT - 1; y > 0; y--) {
        for(uint8_t x = 0; x < LEVEL_WIDTH; x++) {
            uint8_t block = getBlockAt(level, x, y);

            if(block == E_PLAYER) {
                plugin_state->player = create_player(x, y);
                return;
            }

            // todo create other static entities
        }
    }
}

bool isSpawned(UID uid, PluginState* const plugin_state) {
    for(uint8_t i = 0; i < plugin_state->num_entities; i++) {
        if(plugin_state->entity[i].uid == uid) return true;
    }

    return false;
}

bool isStatic(UID uid, PluginState* const plugin_state) {
    for(uint8_t i = 0; i < plugin_state->num_static_entities; i++) {
        if(plugin_state->static_entity[i].uid == uid) return true;
    }

    return false;
}

void spawnEntity(uint8_t type, uint8_t x, uint8_t y, PluginState* const plugin_state) {
    // Limit the number of spawned entities
    if(plugin_state->num_entities >= MAX_ENTITIES) {
        return;
    }

    // todo: read static entity status

    switch(type) {
    case E_ENEMY:
        plugin_state->entity[plugin_state->num_entities] = create_enemy(x, y);
        plugin_state->num_entities++;
        break;

    case E_KEY:
        plugin_state->entity[plugin_state->num_entities] = create_key(x, y);
        plugin_state->num_entities++;
        break;

    case E_MEDIKIT:
        plugin_state->entity[plugin_state->num_entities] = create_medikit(x, y);
        plugin_state->num_entities++;
        break;
    }
}

void spawnFireball(double x, double y, PluginState* const plugin_state) {
    // Limit the number of spawned entities
    if(plugin_state->num_entities >= MAX_ENTITIES) {
        return;
    }

    UID uid = create_uid(E_FIREBALL, x, y);
    // Remove if already exists, don't throw anything. Not the best, but shouldn't happen too often
    if(isSpawned(uid, plugin_state)) return;

    // Calculate direction. 32 angles
    int16_t dir =
        FIREBALL_ANGLES + atan2(y - plugin_state->player.pos.y, x - plugin_state->player.pos.x) /
                              (double)PI * FIREBALL_ANGLES;
    if(dir < 0) dir += FIREBALL_ANGLES * 2;
    plugin_state->entity[plugin_state->num_entities] = create_fireball(x, y, dir);
    plugin_state->num_entities++;
}

void removeEntity(UID uid, PluginState* const plugin_state) {
    uint8_t i = 0;
    bool found = false;

    while(i < plugin_state->num_entities) {
        if(!found && plugin_state->entity[i].uid == uid) {
            // todo: doze it
            found = true;
            plugin_state->num_entities--;
        }

        // displace entities
        if(found) {
            plugin_state->entity[i] = plugin_state->entity[i + 1];
        }

        i++;
    }
}

void removeStaticEntity(UID uid, PluginState* const plugin_state) {
    uint8_t i = 0;
    bool found = false;

    while(i < plugin_state->num_static_entities) {
        if(!found && plugin_state->static_entity[i].uid == uid) {
            found = true;
            plugin_state->num_static_entities--;
        }

        // displace entities
        if(found) {
            plugin_state->static_entity[i] = plugin_state->static_entity[i + 1];
        }

        i++;
    }
}

UID detectCollision(
    const uint8_t level[],
    Coords* pos,
    double relative_x,
    double relative_y,
    bool only_walls,
    PluginState* const plugin_state) {
    // Wall collision
    uint8_t round_x = (int)(pos->x + relative_x);
    uint8_t round_y = (int)(pos->y + relative_y);
    uint8_t block = getBlockAt(level, round_x, round_y);

    if(block == E_WALL) {
        // playSound(hit_wall_snd, HIT_WALL_SND_LEN);
        return create_uid(block, round_x, round_y);
    }

    if(only_walls) {
        return UID_null;
    }

    // Entity collision
    for(uint8_t i = 0; i < plugin_state->num_entities; i++) {
        // Don't collide with itself
        if(&(plugin_state->entity[i].pos) == pos) {
            continue;
        }

        uint8_t type = uid_get_type(plugin_state->entity[i].uid);

        // Only ALIVE enemy collision
        if(type != E_ENEMY || plugin_state->entity[i].state == S_DEAD ||
           plugin_state->entity[i].state == S_HIDDEN) {
            continue;
        }

        Coords new_coords = {
            plugin_state->entity[i].pos.x - relative_x,
            plugin_state->entity[i].pos.y - relative_y};
        uint8_t distance = coords_distance(pos, &new_coords);

        // Check distance and if it's getting closer
        if(distance < ENEMY_COLLIDER_DIST && distance < plugin_state->entity[i].distance) {
            return plugin_state->entity[i].uid;
        }
    }

    return UID_null;
}

// Shoot
void fire(PluginState* const plugin_state) {
    //playSound(shoot_snd, SHOOT_SND_LEN);

    for(uint8_t i = 0; i < plugin_state->num_entities; i++) {
        // Shoot only ALIVE enemies
        if(uid_get_type(plugin_state->entity[i].uid) != E_ENEMY ||
           plugin_state->entity[i].state == S_DEAD || plugin_state->entity[i].state == S_HIDDEN) {
            continue;
        }

        Coords transform = translateIntoView(&(plugin_state->entity[i].pos), plugin_state);
        if(fabs(transform.x) < 20 && transform.y > 0) {
            uint8_t damage = (double)fmin(
                GUN_MAX_DAMAGE,
                GUN_MAX_DAMAGE / (fabs(transform.x) * plugin_state->entity[i].distance) / 5);
            if(damage > 0) {
                plugin_state->entity[i].health = fmax(0, plugin_state->entity[i].health - damage);
                plugin_state->entity[i].state = S_HIT;
                plugin_state->entity[i].timer = 4;
            }
        }
    }
}

UID updatePosition(
    const uint8_t level[],
    Coords* pos,
    double relative_x,
    double relative_y,
    bool only_walls,
    PluginState* const plugin_state) {
    UID collide_x = detectCollision(level, pos, relative_x, 0, only_walls, plugin_state);
    UID collide_y = detectCollision(level, pos, 0, relative_y, only_walls, plugin_state);

    if(!collide_x) pos->x += relative_x;
    if(!collide_y) pos->y += relative_y;

    return collide_x || collide_y || UID_null;
}

void updateEntities(const uint8_t level[], Canvas* const canvas, PluginState* const plugin_state) {
    uint8_t i = 0;
    while(i < plugin_state->num_entities) {
        // update distance
        plugin_state->entity[i].distance =
            coords_distance(&(plugin_state->player.pos), &(plugin_state->entity[i].pos));

        // Run the timer. Works with actual frames.
        // Todo: use delta here. But needs double type and more memory
        if(plugin_state->entity[i].timer > 0) plugin_state->entity[i].timer--;

        // too far away. put it in doze mode
        if(plugin_state->entity[i].distance > MAX_ENTITY_DISTANCE) {
            removeEntity(plugin_state->entity[i].uid, plugin_state);
            // don't increase 'i', since current one has been removed
            continue;
        }

        // bypass render if hidden
        if(plugin_state->entity[i].state == S_HIDDEN) {
            i++;
            continue;
        }

        uint8_t type = uid_get_type(plugin_state->entity[i].uid);

        switch(type) {
        case E_ENEMY: {
            // Enemy "IA"
            if(plugin_state->entity[i].health == 0) {
                if(plugin_state->entity[i].state != S_DEAD) {
                    plugin_state->entity[i].state = S_DEAD;
                    plugin_state->entity[i].timer = 6;
                }
            } else if(plugin_state->entity[i].state == S_HIT) {
                if(plugin_state->entity[i].timer == 0) {
                    // Back to alert state
                    plugin_state->entity[i].state = S_ALERT;
                    plugin_state->entity[i].timer = 40; // delay next fireball thrown
                }
            } else if(plugin_state->entity[i].state == S_FIRING) {
                if(plugin_state->entity[i].timer == 0) {
                    // Back to alert state
                    plugin_state->entity[i].state = S_ALERT;
                    plugin_state->entity[i].timer = 40; // delay next fireball throwm
                }
            } else {
                // ALERT STATE
                if(plugin_state->entity[i].distance > ENEMY_MELEE_DIST &&
                   plugin_state->entity[i].distance < MAX_ENEMY_VIEW) {
                    if(plugin_state->entity[i].state != S_ALERT) {
                        plugin_state->entity[i].state = S_ALERT;
                        plugin_state->entity[i].timer = 20; // used to throw fireballs
                    } else {
                        if(plugin_state->entity[i].timer == 0) {
                            // Throw a fireball
                            spawnFireball(
                                plugin_state->entity[i].pos.x,
                                plugin_state->entity[i].pos.y,
                                plugin_state);
                            plugin_state->entity[i].state = S_FIRING;
                            plugin_state->entity[i].timer = 6;
                        } else {
                            // move towards to the player.
                            updatePosition(
                                level,
                                &(plugin_state->entity[i].pos),
                                sign(plugin_state->player.pos.x, plugin_state->entity[i].pos.x) *
                                    (double)ENEMY_SPEED * 1, // NOT SURE (delta)
                                sign(plugin_state->player.pos.y, plugin_state->entity[i].pos.y) *
                                    (double)ENEMY_SPEED * 1, // NOT SURE (delta)
                                true,
                                plugin_state);
                        }
                    }
                } else if(plugin_state->entity[i].distance <= ENEMY_MELEE_DIST) {
                    if(plugin_state->entity[i].state != S_MELEE) {
                        // Preparing the melee attack
                        plugin_state->entity[i].state = S_MELEE;
                        plugin_state->entity[i].timer = 10;
                    } else if(plugin_state->entity[i].timer == 0) {
                        // Melee attack
                        plugin_state->player.health =
                            fmax(0, plugin_state->player.health - ENEMY_MELEE_DAMAGE);
                        plugin_state->entity[i].timer = 14;
                        flash_screen = 1;
                        updateHud(canvas, plugin_state);
                    }
                } else {
                    // stand
                    plugin_state->entity[i].state = S_STAND;
                }
            }
            break;
        }

        case E_FIREBALL: {
            if(plugin_state->entity[i].distance < FIREBALL_COLLIDER_DIST) {
                // Hit the player and disappear
                plugin_state->player.health =
                    fmax(0, plugin_state->player.health - ENEMY_FIREBALL_DAMAGE);
                flash_screen = 1;
                updateHud(canvas, plugin_state);
                removeEntity(plugin_state->entity[i].uid, plugin_state);
                continue; // continue in the loop
            } else {
                // Move. Only collide with walls.
                // Note: using health to store the angle of the movement
                UID collided = updatePosition(
                    level,
                    &(plugin_state->entity[i].pos),
                    cos((double)plugin_state->entity[i].health / FIREBALL_ANGLES * (double)PI) *
                        (double)FIREBALL_SPEED,
                    sin((double)plugin_state->entity[i].health / FIREBALL_ANGLES * (double)PI) *
                        (double)FIREBALL_SPEED,
                    true,
                    plugin_state);

                if(collided) {
                    removeEntity(plugin_state->entity[i].uid, plugin_state);
                    continue; // continue in the entity check loop
                }
            }
            break;
        }

        case E_MEDIKIT: {
            if(plugin_state->entity[i].distance < ITEM_COLLIDER_DIST) {
                // pickup
                notification_message(plugin_state->notify, &sequence_long_sound);
                //playSound(medkit_snd, MEDKIT_SND_LEN);
                plugin_state->entity[i].state = S_HIDDEN;
                plugin_state->player.health = fmin(100, plugin_state->player.health + 50);
                updateHud(canvas, plugin_state);
                flash_screen = 1;
            }
            break;
        }

        case E_KEY: {
            if(plugin_state->entity[i].distance < ITEM_COLLIDER_DIST) {
                // pickup
                notification_message(plugin_state->notify, &sequence_long_sound);
                //playSound(get_key_snd, GET_KEY_SND_LEN);
                plugin_state->entity[i].state = S_HIDDEN;
                plugin_state->player.keys++;
                updateHud(canvas, plugin_state);
                flash_screen = 1;
            }
            break;
        }
        }

        i++;
    }
}

// The map raycaster. Based on https://lodev.org/cgtutor/raycasting.html
void renderMap(
    const uint8_t level[],
    double view_height,
    Canvas* const canvas,
    PluginState* const plugin_state) {
    UID last_uid = 0; // NOT SURE ?

    for(uint8_t x = 0; x < SCREEN_WIDTH; x += RES_DIVIDER) {
        double camera_x = 2 * (double)x / SCREEN_WIDTH - 1;
        double ray_x = plugin_state->player.dir.x + plugin_state->player.plane.x * camera_x;
        double ray_y = plugin_state->player.dir.y + plugin_state->player.plane.y * camera_x;
        uint8_t map_x = (uint8_t)plugin_state->player.pos.x;
        uint8_t map_y = (uint8_t)plugin_state->player.pos.y;
        Coords map_coords = {plugin_state->player.pos.x, plugin_state->player.pos.y};
        double delta_x = fabs(1 / ray_x);
        double delta_y = fabs(1 / ray_y);

        int8_t step_x;
        int8_t step_y;
        double side_x;
        double side_y;

        if(ray_x < 0) {
            step_x = -1;
            side_x = (plugin_state->player.pos.x - map_x) * delta_x;
        } else {
            step_x = 1;
            side_x = (map_x + (double)1.0 - plugin_state->player.pos.x) * delta_x;
        }

        if(ray_y < 0) {
            step_y = -1;
            side_y = (plugin_state->player.pos.y - map_y) * delta_y;
        } else {
            step_y = 1;
            side_y = (map_y + (double)1.0 - plugin_state->player.pos.y) * delta_y;
        }

        // Wall detection
        uint8_t depth = 0;
        bool hit = 0;
        bool side;
        while(!hit && depth < MAX_RENDER_DEPTH) {
            if(side_x < side_y) {
                side_x += delta_x;
                map_x += step_x;
                side = 0;
            } else {
                side_y += delta_y;
                map_y += step_y;
                side = 1;
            }

            uint8_t block = getBlockAt(level, map_x, map_y);

            if(block == E_WALL) {
                hit = 1;
            } else {
                // Spawning entities here, as soon they are visible for the
                // player. Not the best place, but would be a very performance
                // cost scan for them in another loop
                if(block == E_ENEMY || (block & 0b00001000) /* all collectable items */) {
                    // Check that it's close to the player
                    if(coords_distance(&(plugin_state->player.pos), &map_coords) <
                       MAX_ENTITY_DISTANCE) {
                        UID uid = create_uid(block, map_x, map_y);
                        if(last_uid != uid && !isSpawned(uid, plugin_state)) {
                            spawnEntity(block, map_x, map_y, plugin_state);
                            last_uid = uid;
                        }
                    }
                }
            }

            depth++;
        }

        if(hit) {
            double distance;

            if(side == 0) {
                distance =
                    fmax(1, (map_x - plugin_state->player.pos.x + (1 - step_x) / 2) / ray_x);
            } else {
                distance =
                    fmax(1, (map_y - plugin_state->player.pos.y + (1 - step_y) / 2) / ray_y);
            }

            // store zbuffer value for the column
            zbuffer[x / Z_RES_DIVIDER] = fmin(distance * DISTANCE_MULTIPLIER, 255);

            // rendered line height
            uint8_t line_height = RENDER_HEIGHT / distance;

            drawVLine(
                x,
                view_height / distance - line_height / 2 + RENDER_HEIGHT / 2,
                view_height / distance + line_height / 2 + RENDER_HEIGHT / 2,
                GRADIENT_COUNT - (int)distance / MAX_RENDER_DEPTH * GRADIENT_COUNT - side * 2,
                canvas);
        }
    }
}

// Sort entities from far to close
uint8_t sortEntities(PluginState* const plugin_state) {
    uint8_t gap = plugin_state->num_entities;
    bool swapped = false;
    while(gap > 1 || swapped) {
        //shrink factor 1.3
        gap = (gap * 10) / 13;
        if(gap == 9 || gap == 10) gap = 11;
        if(gap < 1) gap = 1;
        swapped = false;
        for(uint8_t i = 0; i < plugin_state->num_entities - gap; i++) {
            uint8_t j = i + gap;
            if(plugin_state->entity[i].distance < plugin_state->entity[j].distance) {
                swap(plugin_state->entity[i], plugin_state->entity[j]);
                swapped = true;
            }
        }
    }
    return swapped;
}

Coords translateIntoView(Coords* pos, PluginState* const plugin_state) {
    //translate sprite position to relative to camera
    double sprite_x = pos->x - plugin_state->player.pos.x;
    double sprite_y = pos->y - plugin_state->player.pos.y;

    //required for correct matrix multiplication
    double inv_det =
        ((double)1.0 /
         ((double)plugin_state->player.plane.x * (double)plugin_state->player.dir.y -
          (double)plugin_state->player.dir.x * (double)plugin_state->player.plane.y));
    double transform_x =
        inv_det * (plugin_state->player.dir.y * sprite_x - plugin_state->player.dir.x * sprite_y);
    double transform_y = inv_det * (-plugin_state->player.plane.y * sprite_x +
                                    plugin_state->player.plane.x * sprite_y); // Z in screen
    Coords res = {transform_x, transform_y};
    return res;
}

void renderEntities(double view_height, Canvas* const canvas, PluginState* const plugin_state) {
    sortEntities(plugin_state);

    for(uint8_t i = 0; i < plugin_state->num_entities; i++) {
        if(plugin_state->entity[i].state == S_HIDDEN) continue;

        Coords transform = translateIntoView(&(plugin_state->entity[i].pos), plugin_state);

        // don´t render if behind the player or too far away
        if(transform.y <= (double)0.1 || transform.y > MAX_SPRITE_DEPTH) {
            continue;
        }

        int16_t sprite_screen_x = HALF_WIDTH * ((double)1.0 + transform.x / transform.y);
        int8_t sprite_screen_y = RENDER_HEIGHT / 2 + view_height / transform.y;
        uint8_t type = uid_get_type(plugin_state->entity[i].uid);

        // don´t try to render if outside of screen
        // doing this pre-shortcut due int16 -> int8 conversion makes out-of-screen
        // values fit into the screen space
        if(sprite_screen_x < -HALF_WIDTH || sprite_screen_x > SCREEN_WIDTH + HALF_WIDTH) {
            continue;
        }

        switch(type) {
        case E_ENEMY: {
            uint8_t sprite;
            if(plugin_state->entity[i].state == S_ALERT) {
                // walking
                sprite = ((int)furi_get_tick() / 500) % 2;
            } else if(plugin_state->entity[i].state == S_FIRING) {
                // fireball
                sprite = 2;
            } else if(plugin_state->entity[i].state == S_HIT) {
                // hit
                sprite = 3;
            } else if(plugin_state->entity[i].state == S_MELEE) {
                // melee atack
                sprite = plugin_state->entity[i].timer > 10 ? 2 : 1;
            } else if(plugin_state->entity[i].state == S_DEAD) {
                // dying
                sprite = plugin_state->entity[i].timer > 0 ? 3 : 4;
            } else {
                // stand
                sprite = 0;
            }

            drawSprite(
                sprite_screen_x - BMP_IMP_WIDTH * (double).5 / transform.y,
                sprite_screen_y - 8 / transform.y,
                imp_inv,
                imp_mask_inv,
                BMP_IMP_WIDTH,
                BMP_IMP_HEIGHT,
                sprite,
                transform.y,
                canvas);
            break;
        }

        case E_FIREBALL: {
            drawSprite(
                sprite_screen_x - BMP_FIREBALL_WIDTH / 2 / transform.y,
                sprite_screen_y - BMP_FIREBALL_HEIGHT / 2 / transform.y,
                fireball,
                fireball_mask,
                BMP_FIREBALL_WIDTH,
                BMP_FIREBALL_HEIGHT,
                0,
                transform.y,
                canvas);
            break;
        }

        case E_MEDIKIT: {
            drawSprite(
                sprite_screen_x - BMP_ITEMS_WIDTH / 2 / transform.y,
                sprite_screen_y + 5 / transform.y,
                item,
                item_mask,
                BMP_ITEMS_WIDTH,
                BMP_ITEMS_HEIGHT,
                0,
                transform.y,
                canvas);
            break;
        }

        case E_KEY: {
            drawSprite(
                sprite_screen_x - BMP_ITEMS_WIDTH / 2 / transform.y,
                sprite_screen_y + 5 / transform.y,
                item,
                item_mask,
                BMP_ITEMS_WIDTH,
                BMP_ITEMS_HEIGHT,
                1,
                transform.y,
                canvas);
            break;
        }
        }
    }
}

void renderGun(uint8_t gun_pos, double amount_jogging, Canvas* const canvas) {
    // jogging
    char x = 48 + sin((double)furi_get_tick() * (double)JOGGING_SPEED) * 10 * amount_jogging;
    char y = RENDER_HEIGHT - gun_pos +
             fabs(cos((double)furi_get_tick() * (double)JOGGING_SPEED)) * 8 * amount_jogging;

    if(gun_pos > GUN_SHOT_POS - 2) {
        // Gun fire
        drawBitmap(x + 6, y - 11, &I_fire_inv, BMP_FIRE_WIDTH, BMP_FIRE_HEIGHT, 1, canvas);
    }

    // Don't draw over the hud!
    uint8_t clip_height = fmax(0, fmin(y + BMP_GUN_HEIGHT, RENDER_HEIGHT) - y);

    // Draw the gun (black mask + actual sprite).
    drawBitmap(x, y, &I_gun_mask_inv, BMP_GUN_WIDTH, clip_height, 0, canvas);
    drawBitmap(x, y, &I_gun_inv, BMP_GUN_WIDTH, clip_height, 1, canvas);
    //drawGun(x,y,gun_mask, BMP_GUN_WIDTH, clip_height, 0, canvas);
    //drawGun(x,y,gun, BMP_GUN_WIDTH, clip_height, 1, canvas);
}

// Only needed first time
void renderHud(Canvas* const canvas, PluginState* plugin_state) {
    drawTextSpace(2, 58, "{}", 0, canvas); // Health symbol
    drawTextSpace(40, 58, "[]", 0, canvas); // Keys symbol
    updateHud(canvas, plugin_state);
}

// Render values for the HUD
void updateHud(Canvas* const canvas, PluginState* plugin_state) {
    clearRect(12, 58, 15, 6, canvas);
    clearRect(50, 58, 15, 6, canvas);
    drawText(12, 58, plugin_state->player.health, canvas);
    drawText(50, 58, plugin_state->player.keys, canvas);
}

// Debug stats
void renderStats(Canvas* const canvas, PluginState* plugin_state) {
    clearRect(58, 58, 70, 6, canvas);
    drawText(114, 58, (int)getActualFps(), canvas);
    drawText(82, 58, plugin_state->num_entities, canvas);
    // drawText(94, 58, freeMemory());
}

// Intro screen
void loopIntro(Canvas* const canvas) {
    canvas_draw_icon(canvas, 0, 0, &I_logo_inv);
    //drawTextSpace(SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT * .8, "PRESS FIRE", 1, canvas);
}

static void render_callback(Canvas* const canvas, void* ctx) {
    furi_assert(ctx);
    PluginState* plugin_state = ctx;
    furi_mutex_acquire(plugin_state->mutex, FuriWaitForever);

    canvas_set_font(canvas, FontPrimary);

    switch(plugin_state->scene) {
    case INTRO: {
        loopIntro(canvas);
        break;
    }
    case GAME_PLAY: {
        updateEntities(sto_level_1, canvas, plugin_state);

        renderGun(plugin_state->gun_pos, plugin_state->jogging, canvas);
        renderMap(sto_level_1, plugin_state->view_height, canvas, plugin_state);

        renderEntities(plugin_state->view_height, canvas, plugin_state);

        renderHud(canvas, plugin_state);
        updateHud(canvas, plugin_state);
        renderStats(canvas, plugin_state);
        break;
    }
    }
    furi_mutex_release(plugin_state->mutex);
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, 0);
}

static void doom_state_init(PluginState* const plugin_state) {
    plugin_state->notify = furi_record_open(RECORD_NOTIFICATION);
    plugin_state->num_entities = 0;
    plugin_state->num_static_entities = 0;

    plugin_state->scene = INTRO;
    plugin_state->gun_pos = 0;
    plugin_state->view_height = 0;
    plugin_state->init = true;

    plugin_state->up = false;
    plugin_state->down = false;
    plugin_state->left = false;
    plugin_state->right = false;
    plugin_state->fired = false;
    plugin_state->gun_fired = false;
#ifdef SOUND

    plugin_state->music_instance = malloc(sizeof(MusicPlayer));
    plugin_state->music_instance->model = malloc(sizeof(MusicPlayerModel));
    memset(
        plugin_state->music_instance->model->duration_history,
        0xff,
        MUSIC_PLAYER_SEMITONE_HISTORY_SIZE);
    memset(
        plugin_state->music_instance->model->semitone_history,
        0xff,
        MUSIC_PLAYER_SEMITONE_HISTORY_SIZE);
    plugin_state->music_instance->model->volume = 2;

    plugin_state->music_instance->model_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    //plugin_state->music_instance->view_port = view_port_alloc();

    plugin_state->music_instance->worker = music_player_worker_alloc();
    //music_player_worker_set_volume(plugin_state->music_instance->worker, 0.75);
    music_player_worker_set_volume(
        plugin_state->music_instance->worker,
        MUSIC_PLAYER_VOLUMES[plugin_state->music_instance->model->volume]);
    plugin_state->intro_sound = true;
    //init_sound(plugin_state->music_instance);
#endif
}

static void doom_game_update_timer_callback(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
}

static void doom_game_tick(PluginState* const plugin_state) {
    if(plugin_state->scene == GAME_PLAY) {
        //fps();
        //player is alive
        if(plugin_state->player.health > 0) {
            if(plugin_state->up) {
                plugin_state->player.velocity +=
                    ((double)MOV_SPEED - plugin_state->player.velocity) * (double).4;
                plugin_state->jogging = fabs(plugin_state->player.velocity) * MOV_SPEED_INV;
                //plugin_state->up = false;
            } else if(plugin_state->down) {
                plugin_state->player.velocity +=
                    (-(double)MOV_SPEED - plugin_state->player.velocity) * (double).4;
                plugin_state->jogging = fabs(plugin_state->player.velocity) * MOV_SPEED_INV;
                //plugin_state->down = false;
            } else {
                plugin_state->player.velocity *= (double).5;
                plugin_state->jogging = fabs(plugin_state->player.velocity) * MOV_SPEED_INV;
            }

            if(plugin_state->right) {
                plugin_state->rot_speed = (double)ROT_SPEED * delta;
                plugin_state->old_dir_x = plugin_state->player.dir.x;
                plugin_state->player.dir.x =
                    plugin_state->player.dir.x * cos(-(plugin_state->rot_speed)) -
                    plugin_state->player.dir.y * sin(-(plugin_state->rot_speed));
                plugin_state->player.dir.y =
                    plugin_state->old_dir_x * sin(-(plugin_state->rot_speed)) +
                    plugin_state->player.dir.y * cos(-(plugin_state->rot_speed));
                plugin_state->old_plane_x = plugin_state->player.plane.x;
                plugin_state->player.plane.x =
                    plugin_state->player.plane.x * cos(-(plugin_state->rot_speed)) -
                    plugin_state->player.plane.y * sin(-(plugin_state->rot_speed));
                plugin_state->player.plane.y =
                    plugin_state->old_plane_x * sin(-(plugin_state->rot_speed)) +
                    plugin_state->player.plane.y * cos(-(plugin_state->rot_speed));

                //plugin_state->right = false;
            } else if(plugin_state->left) {
                plugin_state->rot_speed = (double)ROT_SPEED * delta;
                plugin_state->old_dir_x = plugin_state->player.dir.x;
                plugin_state->player.dir.x =
                    plugin_state->player.dir.x * cos(plugin_state->rot_speed) -
                    plugin_state->player.dir.y * sin(plugin_state->rot_speed);
                plugin_state->player.dir.y =
                    plugin_state->old_dir_x * sin(plugin_state->rot_speed) +
                    plugin_state->player.dir.y * cos(plugin_state->rot_speed);
                plugin_state->old_plane_x = plugin_state->player.plane.x;
                plugin_state->player.plane.x =
                    plugin_state->player.plane.x * cos(plugin_state->rot_speed) -
                    plugin_state->player.plane.y * sin(plugin_state->rot_speed);
                plugin_state->player.plane.y =
                    plugin_state->old_plane_x * sin(plugin_state->rot_speed) +
                    plugin_state->player.plane.y * cos(plugin_state->rot_speed);
                //plugin_state->left = false;
            }
            plugin_state->view_height =
                fabs(sin((double)furi_get_tick() * (double)JOGGING_SPEED)) * 6 *
                plugin_state->jogging;

            if(plugin_state->gun_pos > GUN_TARGET_POS) {
                // Right after fire
                plugin_state->gun_pos -= 1;
            } else if(plugin_state->gun_pos < GUN_TARGET_POS) {
                plugin_state->gun_pos += 2;
            } else if(!plugin_state->gun_fired && plugin_state->fired) {
                //furi_hal_speaker_start(20480 / 10, 0.45f);
                /*#ifdef SOUND
        music_player_worker_start(plugin_state->music_instance->worker);
#endif*/
                plugin_state->gun_pos = GUN_SHOT_POS;
                plugin_state->gun_fired = true;
                plugin_state->fired = false;
                fire(plugin_state);

            } else if(plugin_state->gun_fired && !plugin_state->fired) {
                //furi_hal_speaker_stop();
                plugin_state->gun_fired = false;

                notification_message(plugin_state->notify, &sequence_short_sound);

                /*#ifdef SOUND
        music_player_worker_stop(plugin_state->music_instance->worker);
#endif*/
            }
        } else {
            // Player is dead
            if(plugin_state->view_height > -10) plugin_state->view_height--;
            if(plugin_state->gun_pos > 1) plugin_state->gun_pos -= 2;
        }

        if(fabs(plugin_state->player.velocity) > (double)0.003) {
            updatePosition(
                sto_level_1,
                &(plugin_state->player.pos),
                plugin_state->player.dir.x * plugin_state->player.velocity * delta,
                plugin_state->player.dir.y * plugin_state->player.velocity * delta,
                false,
                plugin_state);
        } else {
            plugin_state->player.velocity = 0;
        }
    }
}

int32_t doom_app() {
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));
    PluginState* plugin_state = malloc(sizeof(PluginState));
    doom_state_init(plugin_state);
    plugin_state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    if(!plugin_state->mutex) {
        FURI_LOG_E("Doom_game", "cannot create mutex\r\n");
        furi_record_close(RECORD_NOTIFICATION);
        furi_message_queue_free(event_queue);
        free(plugin_state);
        return 255;
    }
    FuriTimer* timer =
        furi_timer_alloc(doom_game_update_timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_kernel_get_tick_frequency() / 12);
    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, plugin_state);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    //////////////////////////////////
    plugin_state->init = false;

    PluginEvent event;
#ifdef SOUND
    music_player_worker_load_rtttl_from_string(plugin_state->music_instance->worker, dsintro);
    music_player_worker_start(plugin_state->music_instance->worker);
#endif
    // Call dolphin deed on game start
    dolphin_deed(DolphinDeedPluginGameStart);

    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        furi_mutex_acquire(plugin_state->mutex, FuriWaitForever);
#ifdef SOUND
        furi_check(
            furi_mutex_acquire(plugin_state->music_instance->model_mutex, FuriWaitForever) ==
            FuriStatusOk);
#endif
        if(event_status == FuriStatusOk) {
            // press events
            if(event.type == EventTypeKey) {
                if(event.input.key == InputKeyBack) {
                    processing = false;
#ifdef SOUND
                    if(plugin_state->intro_sound) {
                        furi_mutex_release(plugin_state->music_instance->model_mutex);
                        music_player_worker_stop(plugin_state->music_instance->worker);
                    }
#endif
                }

                if(event.input.type == InputTypePress) {
                    if(plugin_state->scene == INTRO && event.input.key == InputKeyOk) {
                        plugin_state->scene = GAME_PLAY;
                        initializeLevel(sto_level_1, plugin_state);
#ifdef SOUND
                        furi_mutex_release(plugin_state->music_instance->model_mutex);
                        music_player_worker_stop(plugin_state->music_instance->worker);
                        plugin_state->intro_sound = false;
#endif
                        goto skipintro;
                    }

                    //While playing game
                    if(plugin_state->scene == GAME_PLAY) {
                        // If the player is alive
                        if(plugin_state->player.health > 0) {
                            //Player speed
                            if(event.input.key == InputKeyUp) {
                                plugin_state->up = true;
                            } else if(event.input.key == InputKeyDown) {
                                plugin_state->down = true;
                            }
                            // Player rotation
                            if(event.input.key == InputKeyRight) {
                                plugin_state->right = true;
                            } else if(event.input.key == InputKeyLeft) {
                                plugin_state->left = true;
                            }
                            if(event.input.key == InputKeyOk) {
                                /*#ifdef SOUND
                        music_player_worker_load_rtttl_from_string(plugin_state->music_instance->worker, dspistol);
#endif*/
                                if(plugin_state->fired) {
                                    plugin_state->fired = false;
                                } else {
                                    plugin_state->fired = true;
                                }
                            }
                        } else {
                            // Player is dead
                            if(event.input.key == InputKeyOk) plugin_state->scene = INTRO;
                        }
                    }
                }
                if(event.input.type == InputTypeRelease) {
                    if(plugin_state->player.health > 0) {
                        //Player speed
                        if(event.input.key == InputKeyUp) {
                            plugin_state->up = false;
                        } else if(event.input.key == InputKeyDown) {
                            plugin_state->down = false;
                        }
                        // Player rotation
                        if(event.input.key == InputKeyRight) {
                            plugin_state->right = false;
                        } else if(event.input.key == InputKeyLeft) {
                            plugin_state->left = false;
                        }
                    }
                }
            }

        skipintro:
            if(event.type == EventTypeTick) {
                doom_game_tick(plugin_state);
            }
        }
#ifdef SOUND
        furi_mutex_release(plugin_state->music_instance->model_mutex);
#endif
        view_port_update(view_port);
        furi_mutex_release(plugin_state->mutex);
    }
#ifdef SOUND
    music_player_worker_free(plugin_state->music_instance->worker);
    furi_mutex_free(plugin_state->music_instance->model_mutex);
    free(plugin_state->music_instance->model);
    free(plugin_state->music_instance);
#endif
    furi_record_close(RECORD_NOTIFICATION);
    furi_timer_free(timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_mutex_free(plugin_state->mutex);
    furi_message_queue_free(event_queue);
    free(plugin_state);
    return 0;
}
