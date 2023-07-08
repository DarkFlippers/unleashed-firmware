#include <stdlib.h>

#include "particle.h"
#include "scientist.h"
#include "barry.h"

void particle_tick(PARTICLE* const particles, SCIENTIST* const scientists) {
    // Move particles
    for(int i = 0; i < PARTICLES_MAX; i++) {
        if(particles[i].point.y > 0) {
            particles[i].point.y += PARTICLE_VELOCITY;

            // Check collision with scientists
            for(int j = 0; j < SCIENTISTS_MAX; j++) {
                if(scientists[j].state == ScientistStateAlive && scientists[j].point.x > 0) {
                    // Check whether the particle lies within the scientist's bounding box
                    if(!(particles[i].point.x > scientists[j].point.x + SCIENTIST_WIDTH ||
                         particles[i].point.x < scientists[j].point.x ||
                         particles[i].point.y > scientists[j].point.y + SCIENTIST_HEIGHT ||
                         particles[i].point.y < scientists[j].point.y)) {
                        scientists[j].state = ScientistStateDead;
                        // (*points) += 2; // Increase the score by 2
                    }
                }
            }

            if(particles[i].point.x < 0 || particles[i].point.x > SCREEN_WIDTH ||
               particles[i].point.y < 0 || particles[i].point.y > SCREEN_HEIGHT) {
                particles[i].point.y = 0;
            }
        }
    }
}

void spawn_random_particles(PARTICLE* const particles, BARRY* const barry) {
    for(int i = 0; i < PARTICLES_MAX; i++) {
        if(particles[i].point.y <= 0) {
            particles[i].point.x = barry->point.x + (rand() % 4);
            particles[i].point.y = barry->point.y + 14;
            break;
        }
    }
}

void draw_particles(const PARTICLE* particles, Canvas* const canvas) {
    canvas_set_color(canvas, ColorBlack);
    for(int i = 0; i < PARTICLES_MAX; i++) {
        if(particles[i].point.y > 0) {
            canvas_draw_line(
                canvas,
                particles[i].point.x,
                particles[i].point.y,
                particles[i].point.x,
                particles[i].point.y + 3);
        }
    }
}
