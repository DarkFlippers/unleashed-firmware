

#ifndef PARTICLE_H
#define PARTICLE_H

#include "point.h"
#include "scientist.h"
#include "barry.h"

#define PARTICLES_MAX 50
#define PARTICLE_VELOCITY 2

typedef struct {
    POINT point;
} PARTICLE;

void particle_tick(PARTICLE* const particles, SCIENTIST* const scientists);
void spawn_random_particles(PARTICLE* const particles, BARRY* const barry);
void draw_particles(const PARTICLE* particles, Canvas* const canvas);

#endif // PARTICLE_H