/**
 * @file ra_sweep.h
 * @brief What the Read screen does with the air: hunt for a carrier, then work out what it is.
 *
 * Three jobs, one state machine, because all three turn on the same question - is something
 * transmitting right now:
 *
 * - while the air is quiet it walks the hopper frequency list, so a remote on a band other
 *   than the chosen one still shows up;
 * - once a carrier appears it parks, and if nothing has decoded it after a couple of seconds
 *   it walks the modulations, since a remote that will not decode is often only on the wrong
 *   one, and says so rather than leaving a blank screen;
 * - the first decode moves the cycle's base onto the modulation that produced it, and holds
 *   the frequency briefly, so a remote that keeps decoding there is never swept away from.
 *
 * Pure state machine: it is told the time and the RSSI and says when to retune. It touches no
 * radio and no GUI, so the scene stays in charge of both.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define RA_SWEEP_TICK_MS 100

typedef enum {
    RaSweepStateIdle, /**< nothing on the air: where frequency hopping happens */
    RaSweepStateArmed, /**< a carrier is up on the current settings, still hoping */
    RaSweepStateSweeping, /**< working through the other modulations */
    RaSweepStateSettle, /**< back on the base modulation for one last try */
    RaSweepStateError, /**< the cycle found nothing; shown briefly, then it goes round again */
} RaSweepState;

typedef enum {
    RaSweepActionNone,
    /** Tune to ra_sweep_get_preset() and ra_sweep_get_hop_index() before the next tick. */
    RaSweepActionRetune,
} RaSweepAction;

typedef struct {
    bool sweep; /**< walk the modulations when a signal will not decode */
    bool hop; /**< walk the frequencies while the air is quiet */
    float trigger; /**< dBm above which something is transmitting */
    size_t preset_index; /**< the modulation the user chose; the cycle starts from it */
    size_t preset_count;
    size_t hop_count; /**< frequencies in the hopper list; under 2 leaves hopping off */
    uint32_t ticks_per_second; /**< the kernel tick rate, used to scale the periods once */
} RaSweepConfig;

typedef struct {
    RaSweepState state;
    bool enabled;
    bool hop;
    float trigger;

    /** What the user chose, and what the cycle currently returns to. They differ once a
     * decode has moved the base onto the modulation that produced it. */
    size_t preset_config;
    size_t preset_base;
    size_t preset_count;
    size_t preset_now;
    size_t tried;

    size_t hop_count;
    size_t hop_now;

    /* Kernel ticks, not milliseconds: every comparison here is a difference, which stays
     * right across the 32 bit wrap only if the clock is never rescaled. */
    uint32_t t_mark;
    uint32_t t_signal;
    uint32_t t_decode; /**< when a frame last decoded, which parks the hop for a moment */
    bool have_signal_mark;
    bool have_decode_mark;
    uint32_t hold;
    uint32_t arm;
    uint32_t settle;
    uint32_t error_show;
    uint32_t signal_hold;
    uint32_t hop_hold;
    uint32_t hop_dwell;
    bool signal;
} RaSweep;

void ra_sweep_init(RaSweep* instance, const RaSweepConfig* config);

/** Call every RA_SWEEP_TICK_MS with the current RSSI in dBm and the kernel tick count. */
RaSweepAction ra_sweep_tick(RaSweep* instance, float rssi, uint32_t now);

/**
 * Call when a protocol decoded a frame. The cycle's base moves onto the modulation that
 * produced it and the arming window starts again, so frames that keep decoding there never
 * start another sweep, and one that stops decoding does.
 */
void ra_sweep_decoded(RaSweep* instance, uint32_t now);

/** Call on the Reset button. Returns true when the caller must retune. */
bool ra_sweep_reset(RaSweep* instance, uint32_t now);

size_t ra_sweep_get_preset(const RaSweep* instance);

/** Index into the hopper frequency list. Only meaningful while hopping is on. */
size_t ra_sweep_get_hop_index(const RaSweep* instance);

bool ra_sweep_is_signal(const RaSweep* instance);

/** True while the modulation on air is not the one to stay on, which the header marks. */
bool ra_sweep_is_sweeping(const RaSweep* instance);

/** What the Read screen should say, or NULL while there is nothing to add. */
const char* ra_sweep_get_status(const RaSweep* instance);
