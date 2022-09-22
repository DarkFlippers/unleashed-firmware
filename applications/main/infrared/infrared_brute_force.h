#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct InfraredBruteForce InfraredBruteForce;

InfraredBruteForce* infrared_brute_force_alloc();
void infrared_brute_force_free(InfraredBruteForce* brute_force);
void infrared_brute_force_set_db_filename(InfraredBruteForce* brute_force, const char* db_filename);
bool infrared_brute_force_calculate_messages(InfraredBruteForce* brute_force);
bool infrared_brute_force_start(
    InfraredBruteForce* brute_force,
    uint32_t index,
    uint32_t* record_count);
bool infrared_brute_force_is_started(InfraredBruteForce* brute_force);
void infrared_brute_force_stop(InfraredBruteForce* brute_force);
bool infrared_brute_force_send_next(InfraredBruteForce* brute_force);
void infrared_brute_force_add_record(
    InfraredBruteForce* brute_force,
    uint32_t index,
    const char* name);
void infrared_brute_force_reset(InfraredBruteForce* brute_force);
