#include "entities.h"

//extern "C"
/*Player create_player(double x, double y){
	return {create_coords((double) x + (double)0.5, (double) y + (double)0.5), create_coords(1, 0), create_coords(0, -0.66), 0, 100, 0};
}*/

Player create_player(double x, double y) {
    Player p;
    p.pos = create_coords((double)x + (double)0.5, (double)y + (double)0.5);
    p.dir = create_coords(1, 0);
    p.plane = create_coords(0, -0.66);
    p.velocity = 0;
    p.health = 100;
    p.keys = 0;
    return p; //{create_coords((double) x + (double)0.5, (double) y + (double)0.5), create_coords(1, 0), create_coords(0, -0.66), 0, 100, 0};
}

//extern "C"
Entity
    create_entity(uint8_t type, uint8_t x, uint8_t y, uint8_t initialState, uint8_t initialHealth) {
    UID uid = create_uid(type, x, y);
    Coords pos = create_coords((double)x + (double).5, (double)y + (double).5);
    Entity new_entity; // = { uid, pos, initialState, initialHealth, 0, 0 };
    new_entity.uid = uid;
    new_entity.pos = pos;
    new_entity.state = initialState;
    new_entity.health = initialHealth;
    new_entity.distance = 0;
    new_entity.timer = 0;
    return new_entity;
}

//extern "C"
StaticEntity crate_static_entity(UID uid, uint8_t x, uint8_t y, bool active) {
    StaticEntity ent;
    ent.uid = uid;
    ent.x = x;
    ent.y = y;
    ent.active = active;
    return ent;
}