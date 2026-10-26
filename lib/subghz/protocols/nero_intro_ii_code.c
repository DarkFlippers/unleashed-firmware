#include "nero_intro_ii_code.h"

#include <limits.h>
#include <stddef.h>
#include <string.h>

#define INTRO_CHIP_US         125
#define INTRO_SYNC_BITS       255
#define INTRO_SYNC_PREFIX     UINT64_C(0x53E87126D6F634BB)
#define INTRO_MAX_CORRECTIONS 12
#define INTRO_PATH_CENTER     (NERO_INTRO_II_PATHS / 2)

/* The 63-chip alphabet is a degree-6 m-sequence and its complement.
 * Canonical symbol bits 5..0 are its initial state, bit 6 is inversion.
 * This convention deliberately makes no claim about NERO's payload mapping. */
static const uint64_t intro_words[128] = {
    UINT64_C(0x0000000000000000), UINT64_C(0x0218a7a392dd9abf), UINT64_C(0x04314f4725bb357e),
    UINT64_C(0x0629e8e4b766afc1), UINT64_C(0x08629e8e4b766afc), UINT64_C(0x0a7a392dd9abf043),
    UINT64_C(0x0c53d1c96ecd5f82), UINT64_C(0x0e4b766afc10c53d), UINT64_C(0x10c53d1c96ecd5f8),
    UINT64_C(0x12dd9abf04314f47), UINT64_C(0x14f4725bb357e086), UINT64_C(0x16ecd5f8218a7a39),
    UINT64_C(0x18a7a392dd9abf04), UINT64_C(0x1abf04314f4725bb), UINT64_C(0x1c96ecd5f8218a7a),
    UINT64_C(0x1e8e4b766afc10c5), UINT64_C(0x218a7a392dd9abf0), UINT64_C(0x2392dd9abf04314f),
    UINT64_C(0x25bb357e08629e8e), UINT64_C(0x27a392dd9abf0431), UINT64_C(0x29e8e4b766afc10c),
    UINT64_C(0x2bf04314f4725bb3), UINT64_C(0x2dd9abf04314f472), UINT64_C(0x2fc10c53d1c96ecd),
    UINT64_C(0x314f4725bb357e08), UINT64_C(0x3357e08629e8e4b7), UINT64_C(0x357e08629e8e4b76),
    UINT64_C(0x3766afc10c53d1c9), UINT64_C(0x392dd9abf04314f4), UINT64_C(0x3b357e08629e8e4b),
    UINT64_C(0x3d1c96ecd5f8218a), UINT64_C(0x3f04314f4725bb35), UINT64_C(0x410c53d1c96ecd5f),
    UINT64_C(0x4314f4725bb357e0), UINT64_C(0x453d1c96ecd5f821), UINT64_C(0x4725bb357e08629e),
    UINT64_C(0x496ecd5f8218a7a3), UINT64_C(0x4b766afc10c53d1c), UINT64_C(0x4d5f8218a7a392dd),
    UINT64_C(0x4f4725bb357e0862), UINT64_C(0x51c96ecd5f8218a7), UINT64_C(0x53d1c96ecd5f8218),
    UINT64_C(0x55f8218a7a392dd9), UINT64_C(0x57e08629e8e4b766), UINT64_C(0x59abf04314f4725b),
    UINT64_C(0x5bb357e08629e8e4), UINT64_C(0x5d9abf04314f4725), UINT64_C(0x5f8218a7a392dd9a),
    UINT64_C(0x608629e8e4b766af), UINT64_C(0x629e8e4b766afc10), UINT64_C(0x64b766afc10c53d1),
    UINT64_C(0x66afc10c53d1c96e), UINT64_C(0x68e4b766afc10c53), UINT64_C(0x6afc10c53d1c96ec),
    UINT64_C(0x6cd5f8218a7a392d), UINT64_C(0x6ecd5f8218a7a392), UINT64_C(0x704314f4725bb357),
    UINT64_C(0x725bb357e08629e8), UINT64_C(0x74725bb357e08629), UINT64_C(0x766afc10c53d1c96),
    UINT64_C(0x78218a7a392dd9ab), UINT64_C(0x7a392dd9abf04314), UINT64_C(0x7c10c53d1c96ecd5),
    UINT64_C(0x7e08629e8e4b766a), UINT64_C(0x7fffffffffffffff), UINT64_C(0x7de7585c6d226540),
    UINT64_C(0x7bceb0b8da44ca81), UINT64_C(0x79d6171b4899503e), UINT64_C(0x779d6171b4899503),
    UINT64_C(0x7585c6d226540fbc), UINT64_C(0x73ac2e369132a07d), UINT64_C(0x71b4899503ef3ac2),
    UINT64_C(0x6f3ac2e369132a07), UINT64_C(0x6d226540fbceb0b8), UINT64_C(0x6b0b8da44ca81f79),
    UINT64_C(0x69132a07de7585c6), UINT64_C(0x67585c6d226540fb), UINT64_C(0x6540fbceb0b8da44),
    UINT64_C(0x6369132a07de7585), UINT64_C(0x6171b4899503ef3a), UINT64_C(0x5e7585c6d226540f),
    UINT64_C(0x5c6d226540fbceb0), UINT64_C(0x5a44ca81f79d6171), UINT64_C(0x585c6d226540fbce),
    UINT64_C(0x56171b4899503ef3), UINT64_C(0x540fbceb0b8da44c), UINT64_C(0x5226540fbceb0b8d),
    UINT64_C(0x503ef3ac2e369132), UINT64_C(0x4eb0b8da44ca81f7), UINT64_C(0x4ca81f79d6171b48),
    UINT64_C(0x4a81f79d6171b489), UINT64_C(0x4899503ef3ac2e36), UINT64_C(0x46d226540fbceb0b),
    UINT64_C(0x44ca81f79d6171b4), UINT64_C(0x42e369132a07de75), UINT64_C(0x40fbceb0b8da44ca),
    UINT64_C(0x3ef3ac2e369132a0), UINT64_C(0x3ceb0b8da44ca81f), UINT64_C(0x3ac2e369132a07de),
    UINT64_C(0x38da44ca81f79d61), UINT64_C(0x369132a07de7585c), UINT64_C(0x34899503ef3ac2e3),
    UINT64_C(0x32a07de7585c6d22), UINT64_C(0x30b8da44ca81f79d), UINT64_C(0x2e369132a07de758),
    UINT64_C(0x2c2e369132a07de7), UINT64_C(0x2a07de7585c6d226), UINT64_C(0x281f79d6171b4899),
    UINT64_C(0x26540fbceb0b8da4), UINT64_C(0x244ca81f79d6171b), UINT64_C(0x226540fbceb0b8da),
    UINT64_C(0x207de7585c6d2265), UINT64_C(0x1f79d6171b489950), UINT64_C(0x1d6171b4899503ef),
    UINT64_C(0x1b4899503ef3ac2e), UINT64_C(0x19503ef3ac2e3691), UINT64_C(0x171b4899503ef3ac),
    UINT64_C(0x1503ef3ac2e36913), UINT64_C(0x132a07de7585c6d2), UINT64_C(0x1132a07de7585c6d),
    UINT64_C(0x0fbceb0b8da44ca8), UINT64_C(0x0da44ca81f79d617), UINT64_C(0x0b8da44ca81f79d6),
    UINT64_C(0x099503ef3ac2e369), UINT64_C(0x07de7585c6d22654), UINT64_C(0x05c6d226540fbceb),
    UINT64_C(0x03ef3ac2e369132a), UINT64_C(0x01f79d6171b48995),
};

static uint64_t intro_word(uint8_t symbol) {
    return intro_words[symbol];
}

static uint64_t intro_read(const uint8_t* bits, unsigned count) {
    uint64_t word = 0;
    for(unsigned i = 0; i < count; i++)
        word = (word << 1) | bits[i];
    return word;
}

/* Remove a bit addressed from the MSB. */
static uint64_t intro_remove(uint64_t word, unsigned width, unsigned index) {
    unsigned shift = width - index - 1;
    uint64_t low = word & ((UINT64_C(1) << shift) - 1);
    uint64_t high = shift == 63 ? 0 : word >> (shift + 1);
    return (high << shift) | low;
}

static uint64_t intro_insert(uint64_t word, unsigned width, unsigned index, bool bit) {
    unsigned shift = width - index;
    uint64_t low = word & ((UINT64_C(1) << shift) - 1);
    uint64_t high = word >> shift;
    return (high << (shift + 1)) | ((uint64_t)bit << shift) | low;
}

static void intro_candidate(
    uint8_t candidate,
    unsigned distance,
    uint8_t* symbol,
    unsigned* best,
    bool* ambiguous) {
    if(distance < *best) {
        *best = distance;
        *symbol = candidate;
        *ambiguous = false;
    } else if(distance == *best && candidate != *symbol) {
        *ambiguous = true;
    }
}

/* A 63-chip word's first six bits leave only two exact alphabet candidates. */
static void intro_exact(uint64_t word, uint8_t* symbol, unsigned* best, bool* ambiguous) {
    uint8_t prefix = word >> 57;
    if(intro_word(prefix) == word) intro_candidate(prefix, 1, symbol, best, ambiguous);
    uint8_t inverted = 0x40 | (prefix ^ 0x3f);
    if(intro_word(inverted) == word) intro_candidate(inverted, 1, symbol, best, ambiguous);
}

static bool intro_match(
    const uint8_t* bits,
    unsigned width,
    uint8_t* symbol,
    unsigned* errors,
    bool* ambiguous) {
    uint64_t received = intro_read(bits, width);
    unsigned best = UINT_MAX;
    *ambiguous = false;
    *symbol = 0;
    if(width == 63) {
        for(unsigned value = 0; value < 128; value++) {
            unsigned distance = __builtin_popcountll(received ^ intro_word(value));
            intro_candidate(value, distance, symbol, &best, ambiguous);
        }
        if(best > 2) return false;
    } else if(width == 64) {
        for(unsigned i = 0; i < 64; i++) {
            intro_exact(intro_remove(received, 64, i), symbol, &best, ambiguous);
        }
        if(best != 1) return false;
    } else {
        for(unsigned i = 0; i <= 62; i++) {
            intro_exact(intro_insert(received, 62, i, false), symbol, &best, ambiguous);
            intro_exact(intro_insert(received, 62, i, true), symbol, &best, ambiguous);
        }
        if(best != 1) return false;
    }
    *errors = best;
    return true;
}

static bool intro_finish(NeroIntroIICode* instance, NeroIntroIIFrame* frame) {
    if(instance->count < 1210) return false;
    memset(instance->paths, 0xff, sizeof(instance->paths));
    for(int delta = -2; delta <= 2; delta++) {
        NeroIntroIIPath* path = &instance->paths[0][INTRO_PATH_CENTER + delta];
        path->cost = delta < 0 ? -delta : delta;
        path->ambiguous = false;
        memset(path->symbols, 0, sizeof(path->symbols));
    }
    for(unsigned n = 0; n < NERO_INTRO_II_SYMBOLS; n++) {
        NeroIntroIIPath* previous = instance->paths[n & 1];
        NeroIntroIIPath* next = instance->paths[(n + 1) & 1];
        memset(next, 0xff, sizeof(instance->paths[0]));
        for(unsigned path_index = 0; path_index < NERO_INTRO_II_PATHS; path_index++) {
            if(previous[path_index].cost > INTRO_MAX_CORRECTIONS) continue;
            int delta = (int)path_index - INTRO_PATH_CENTER;
            int position = INTRO_SYNC_BITS + n * 65 + delta;
            const unsigned widths[] = {63, 62, 64};
            for(unsigned w = 0; w < 3; w++) {
                unsigned width = widths[w];
                int next_index = (int)path_index + (int)width - 63;
                if(next_index < 0 || next_index >= NERO_INTRO_II_PATHS) continue;
                if(position < 0 || position + width + 2 > instance->count) continue;
                uint8_t value;
                unsigned errors;
                bool ambiguous;
                if(!intro_match(&instance->chips[position], width, &value, &errors, &ambiguous))
                    continue;
                unsigned cost = previous[path_index].cost + errors +
                                instance->chips[position + width] +
                                !instance->chips[position + width + 1];
                if(cost > INTRO_MAX_CORRECTIONS) continue;
                NeroIntroIIPath* target = &next[next_index];
                if(cost < target->cost) {
                    *target = previous[path_index];
                    target->cost = cost;
                    target->ambiguous |= ambiguous;
                    target->symbols[n] = value;
                } else if(cost == target->cost) {
                    if(memcmp(target->symbols, previous[path_index].symbols, n) ||
                       target->symbols[n] != value || ambiguous)
                        target->ambiguous = true;
                }
            }
        }
    }
    NeroIntroIIPath* paths = instance->paths[NERO_INTRO_II_SYMBOLS & 1];
    NeroIntroIIPath* best = NULL;
    for(unsigned i = 0; i < NERO_INTRO_II_PATHS; i++) {
        if(paths[i].cost > INTRO_MAX_CORRECTIONS) continue;
        if(!best || paths[i].cost < best->cost) {
            best = &paths[i];
        } else if(
            paths[i].cost == best->cost &&
            memcmp(paths[i].symbols, best->symbols, NERO_INTRO_II_SYMBOLS)) {
            best->ambiguous = true;
        }
    }
    if(!best || best->ambiguous) return false;
    memcpy(frame->symbols, best->symbols, sizeof(frame->symbols));
    frame->corrections = best->cost;
    frame->ambiguous = false;
    return true;
}

void nero_intro_ii_code_reset(NeroIntroIICode* instance) {
    instance->sync = 0;
    instance->sync_count = 0;
    instance->count = 0;
}

bool nero_intro_ii_code_feed(
    NeroIntroIICode* instance,
    bool level,
    uint32_t duration,
    NeroIntroIIFrame* frame) {
    if(duration > 8500) {
        /* The last separator chip can merge with an idle run of the same level. */
        if(instance->count && instance->count + 2 <= NERO_INTRO_II_MAX_CHIPS) {
            instance->chips[instance->count++] = level;
            instance->chips[instance->count++] = level;
        }
        bool found = intro_finish(instance, frame);
        nero_intro_ii_code_reset(instance);
        /* A long idle can include the first sync chip of the same polarity.
         * Keep its constant tail in the shift register so that a sync starting
         * on the last idle chip is not lost (notably on repeated frames). */
        instance->sync = level ? UINT64_MAX : 0;
        instance->sync_count = 64;
        return found;
    }
    int adjusted = (int)duration + (level ? 15 : -15);
    unsigned count = adjusted > 0 ? (adjusted + INTRO_CHIP_US / 2) / INTRO_CHIP_US : 1;
    if(!count) count = 1;
    for(unsigned i = 0; i < count; i++) {
        if(instance->count) {
            instance->chips[instance->count++] = level;
            if(instance->count == NERO_INTRO_II_MAX_CHIPS) {
                bool found = intro_finish(instance, frame);
                nero_intro_ii_code_reset(instance);
                if(found) return true;
            }
        } else {
            instance->sync = (instance->sync << 1) | level;
            if(instance->sync_count < 64) instance->sync_count++;
            if(instance->sync_count == 64 && instance->sync == INTRO_SYNC_PREFIX) {
                for(unsigned bit = 0; bit < 64; bit++)
                    instance->chips[bit] = (INTRO_SYNC_PREFIX >> (63 - bit)) & 1;
                instance->count = 64;
            }
        }
    }
    return false;
}
