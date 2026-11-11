#include "ra_sweep.h"

/* Long enough for a few whole frames of any protocol in the table to reach the decoders, and
 * short enough that a full cycle finishes while a thumb is still on the button. */
#define RA_SWEEP_HOLD_MS        1000
/* How long a signal gets on the base modulation before the other ones are tried */
#define RA_SWEEP_ARM_MS         2000
/* One last try once the cycle is back where it started */
#define RA_SWEEP_SETTLE_MS      1000
/* Long enough to read, short enough that the next cycle is already under way */
#define RA_SWEEP_ERROR_MS       1000
/* A remote keys the carrier off between frames, so the signal is held over the gaps */
#define RA_SWEEP_SIGNAL_HOLD_MS 500
/* How long a decode parks the hop, so the reading on screen belongs to the header above it */
#define RA_SWEEP_HOP_HOLD_MS    2000
/* One tick per frequency: a hop is only looking for a carrier, not decoding one */
#define RA_SWEEP_HOP_MS         RA_SWEEP_TICK_MS

void ra_sweep_init(RaSweep* instance, const RaSweepConfig* config) {
    instance->state = RaSweepStateIdle;
    instance->enabled = config->sweep;
    instance->hop = config->hop && (config->hop_count > 1);
    instance->trigger = config->trigger;

    instance->preset_count = config->preset_count ? config->preset_count : 1;
    instance->preset_config = config->preset_index;
    instance->preset_base = config->preset_index;
    instance->preset_now = config->preset_index;
    instance->tried = 0;

    instance->hop_count = config->hop_count;
    instance->hop_now = 0;

    instance->t_mark = 0;
    instance->t_signal = 0;
    instance->t_decode = 0;
    instance->have_signal_mark = false;
    instance->have_decode_mark = false;
    instance->signal = false;

    uint32_t tps = config->ticks_per_second;
    if(!tps) tps = 1000;
    instance->hold = RA_SWEEP_HOLD_MS * tps / 1000;
    instance->arm = RA_SWEEP_ARM_MS * tps / 1000;
    instance->settle = RA_SWEEP_SETTLE_MS * tps / 1000;
    instance->error_show = RA_SWEEP_ERROR_MS * tps / 1000;
    instance->signal_hold = RA_SWEEP_SIGNAL_HOLD_MS * tps / 1000;
    instance->hop_hold = RA_SWEEP_HOP_HOLD_MS * tps / 1000;
    instance->hop_dwell = RA_SWEEP_HOP_MS * tps / 1000;
}

size_t ra_sweep_get_preset(const RaSweep* instance) {
    return instance->preset_now;
}

size_t ra_sweep_get_hop_index(const RaSweep* instance) {
    return instance->hop_now;
}

bool ra_sweep_is_signal(const RaSweep* instance) {
    return instance->signal;
}

bool ra_sweep_is_sweeping(const RaSweep* instance) {
    return (instance->state == RaSweepStateSweeping) || (instance->state == RaSweepStateSettle);
}

const char* ra_sweep_get_status(const RaSweep* instance) {
    return (instance->state == RaSweepStateError) ? "Unknown remote" : NULL;
}

/** True while the cycle is allowed to move off the base modulation. */
static bool ra_sweep_can_sweep(const RaSweep* instance) {
    return instance->enabled && (instance->preset_count > 1);
}

/** True once a decode is far enough behind that hopping away from it is fair. */
static bool ra_sweep_hop_free(const RaSweep* instance, uint32_t now) {
    if(!instance->have_decode_mark) return true;
    return (now - instance->t_decode) >= instance->hop_hold;
}

/** Start the cycle at the first modulation after the base one. */
static void ra_sweep_begin(RaSweep* instance, uint32_t now) {
    instance->state = RaSweepStateSweeping;
    instance->t_mark = now;
    instance->tried = 1;
    instance->preset_now = (instance->preset_base + 1) % instance->preset_count;
}

void ra_sweep_decoded(RaSweep* instance, uint32_t now) {
    /* Where it decoded is where to stay, which is the whole point of having swept. The base
     * moves with it, so a later cycle starts from what is known to work.
     *
     * Dropping back to Idle is also what keeps a working remote from being swept away from:
     * the arming window restarts on every frame, so it only runs out once nothing has
     * decoded for a couple of seconds, and the cycle then starts again from this modulation.
     * There is deliberately no flag holding it here - one that outlived a band whose noise
     * sits above the trigger would park the sweep for good. */
    instance->preset_base = instance->preset_now;
    instance->tried = 0;
    instance->state = RaSweepStateIdle;
    instance->t_mark = now;
    instance->t_decode = now;
    instance->have_decode_mark = true;
}

bool ra_sweep_reset(RaSweep* instance, uint32_t now) {
    const bool retune = (instance->preset_now != instance->preset_config) ||
                        (instance->hop_now != 0);
    instance->preset_base = instance->preset_config;
    instance->preset_now = instance->preset_config;
    instance->hop_now = 0;
    instance->tried = 0;
    instance->state = RaSweepStateIdle;
    instance->t_mark = now;
    instance->have_signal_mark = false;
    instance->have_decode_mark = false;
    instance->signal = false;
    return retune;
}

RaSweepAction ra_sweep_tick(RaSweep* instance, float rssi, uint32_t now) {
    const bool carrier = rssi >= instance->trigger;
    if(carrier) {
        instance->t_signal = now;
        instance->have_signal_mark = true;
    }
    instance->signal = instance->have_signal_mark &&
                       ((now - instance->t_signal) < instance->signal_hold);

    const uint32_t held = now - instance->t_mark;

    switch(instance->state) {
    case RaSweepStateIdle:
        if(instance->signal) {
            instance->state = RaSweepStateArmed;
            instance->t_mark = now;
            break;
        }

        // Nothing on this frequency, so try the next one in the list
        if(instance->hop && ra_sweep_hop_free(instance, now) && (held >= instance->hop_dwell)) {
            instance->t_mark = now;
            instance->hop_now = (instance->hop_now + 1) % instance->hop_count;
            return RaSweepActionRetune;
        }
        break;

    case RaSweepStateArmed:
        /* Dropping back to Idle on silence is what keeps a passing burst from starting a
         * sweep that would then be spent on an empty band. */
        if(!instance->signal) {
            instance->state = RaSweepStateIdle;
            instance->t_mark = now;
        } else if(ra_sweep_can_sweep(instance) && (held >= instance->arm)) {
            ra_sweep_begin(instance, now);
            return RaSweepActionRetune;
        }
        break;

    case RaSweepStateSweeping:
        /* The sweep is not abandoned on a gap in the signal: retuning costs a receiver
         * restart, so a thumb lifted for a moment would otherwise throw the cycle away. */
        if(held < instance->hold) break;
        instance->t_mark = now;
        instance->tried++;
        if(instance->tried >= instance->preset_count) {
            // Round again to the base modulation, then one more second to decode
            instance->state = RaSweepStateSettle;
            instance->preset_now = instance->preset_base;
            instance->tried = 0;
            return RaSweepActionRetune;
        }
        instance->preset_now = (instance->preset_now + 1) % instance->preset_count;
        return RaSweepActionRetune;

    case RaSweepStateSettle:
        if(held >= instance->settle) {
            instance->state = RaSweepStateError;
            instance->t_mark = now;
        }
        break;

    case RaSweepStateError:
        if(held < instance->error_show) break;
        instance->t_mark = now;
        /* Said what it had to say. A remote still being held gets another cycle rather than
         * the screen sitting on the answer; a quiet band goes back to listening. */
        if(!instance->signal) {
            instance->state = RaSweepStateIdle;
            break;
        }
        ra_sweep_begin(instance, now);
        return RaSweepActionRetune;
    }

    return RaSweepActionNone;
}
