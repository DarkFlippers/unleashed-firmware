#include "texkom_signal.h"

#include <digital_signal/digital_sequence.h>

#include <nfc/protocols/texkom/texkom_i.h>

#include <furi.h>

// This module describes a waveform and has no business knowing what a variant is, so it keeps its
// own copies of the frame size and the coding set. Pin them to the protocol layer's so the two
// cannot drift apart unnoticed.
_Static_assert(
    TEXKOM_SIGNAL_FRAME_SIZE == TEXKOM_FRAME_SIZE,
    "Texkom frame size disagrees with the protocol layer");
_Static_assert(
    (int)TexkomSignalCodingTk13 == (int)TexkomCodingTk13 &&
        (int)TexkomSignalCodingTk17 == (int)TexkomCodingTk17,
    "Texkom coding values disagree with the protocol layer");

#define BITS_IN_BYTE (8U)

/**
 * Everything is timed in units of one 212 kHz carrier subdivision (Fc / 64, about 4.72 us), which
 * is the granularity the tag itself works in. DigitalSignal counts in 10 ps ticks, so one second
 * is 1e11 of them - hence 64.0e11 over the carrier.
 */
#define TEXKOM_SIGNAL_FC    (13.56e6)
#define TEXKOM_SIGNAL_FC_64 (64.0e11 / TEXKOM_SIGNAL_FC)

/** Impulse width. One unit is all the tag gives, and all a reader looks for. */
#define TEXKOM_SIGNAL_IMPULSE (1U)

/**
 * Low time after an impulse, counted the way PM3's HfEncodeTkm does - so the impulse-to-impulse
 * interval a reader measures is one unit longer than the number here.
 */
#define TEXKOM_SIGNAL_TK13_LONG  (17U)
#define TEXKOM_SIGNAL_TK13_SHORT (7U)

/**
 * TK17 low times, one pair per two-bit value, indexed by that value.
 *
 * The four pairs share one total but their splits sit off the band midpoints. PM3 warns in
 * capitals not to normalise them; the reason is undocumented. Values verbatim from hfops.c.
 */
static const uint8_t texkom_signal_tk17_lows[4][2] = {
    {25, 5}, // 00
    {12, 18}, // 01
    {17, 13}, // 10
    {7, 23}, // 11
};

/**
 * Low period the frame ends on.
 *
 * Only has to be long enough to be a defined level; the gap a reader uses to tell one repetition
 * from the next is the pause between transmissions, which the caller owns.
 */
#define TEXKOM_SIGNAL_TAIL (10U)

/** Periods in a symbol: impulse, gap, impulse, gap. The tails are shorter and fit too. */
#define TEXKOM_SIGNAL_SYMBOL_PERIODS (4U)

/** Longest sequence: 64 TK13 bits plus the tail. */
#define TEXKOM_SIGNAL_BUFFER_SIZE (TEXKOM_SIGNAL_FRAME_SIZE * BITS_IN_BYTE + 1)

/** The four TK17 symbols are contiguous and in two-bit value order, so they can be indexed. */
typedef enum {
    TexkomSignalIndexTk13Zero,
    TexkomSignalIndexTk13One,
    TexkomSignalIndexTk13Tail,
    TexkomSignalIndexTk17Sym00,
    TexkomSignalIndexTk17Sym01,
    TexkomSignalIndexTk17Sym10,
    TexkomSignalIndexTk17Sym11,
    TexkomSignalIndexTk17Tail,

    TexkomSignalIndexNum,
} TexkomSignalIndex;

struct TexkomSignal {
    DigitalSequence* tx_sequence;
    DigitalSignal* signals[TexkomSignalIndexNum];
};

/** Two impulses, each followed by the given low time. */
static void
    texkom_signal_add_symbol(DigitalSignal* signal, uint32_t first_low, uint32_t second_low) {
    digital_signal_add_period_with_level(
        signal, TEXKOM_SIGNAL_FC_64 * TEXKOM_SIGNAL_IMPULSE, true);
    digital_signal_add_period_with_level(signal, TEXKOM_SIGNAL_FC_64 * first_low, false);
    digital_signal_add_period_with_level(
        signal, TEXKOM_SIGNAL_FC_64 * TEXKOM_SIGNAL_IMPULSE, true);
    digital_signal_add_period_with_level(signal, TEXKOM_SIGNAL_FC_64 * second_low, false);
}

/** A low period to park the line on; see texkom_signal_fill() for why every tail needs one. */
static void texkom_signal_add_park(DigitalSignal* signal) {
    digital_signal_add_period_with_level(signal, TEXKOM_SIGNAL_FC_64 * TEXKOM_SIGNAL_TAIL, false);
}

static void texkom_signal_fill(TexkomSignal* instance) {
    DigitalSignal** signals = instance->signals;

    for(size_t i = 0; i < TexkomSignalIndexNum; i++) {
        signals[i] = digital_signal_alloc(TEXKOM_SIGNAL_SYMBOL_PERIODS);
        digital_sequence_register_signal(instance->tx_sequence, i, signals[i]);
    }

    texkom_signal_add_symbol(
        signals[TexkomSignalIndexTk13Zero], TEXKOM_SIGNAL_TK13_SHORT, TEXKOM_SIGNAL_TK13_LONG);
    texkom_signal_add_symbol(
        signals[TexkomSignalIndexTk13One], TEXKOM_SIGNAL_TK13_LONG, TEXKOM_SIGNAL_TK13_SHORT);

    for(size_t value = 0; value < COUNT_OF(texkom_signal_tk17_lows); value++) {
        texkom_signal_add_symbol(
            signals[TexkomSignalIndexTk17Sym00 + value],
            texkom_signal_tk17_lows[value][0],
            texkom_signal_tk17_lows[value][1]);
    }

    // The sequence library holds the last period's level indefinitely once a sequence ends, so
    // every frame has to finish on silence. Finishing high would leave the modulator on and swamp
    // the field until the next transmission.
    //
    // A TK13 symbol already ends low, so its tail is only the park. TK17 closes with one more
    // impulse first, so that its final gap has an end to it.
    texkom_signal_add_park(signals[TexkomSignalIndexTk13Tail]);
    digital_signal_add_period_with_level(
        signals[TexkomSignalIndexTk17Tail], TEXKOM_SIGNAL_FC_64 * TEXKOM_SIGNAL_IMPULSE, true);
    texkom_signal_add_park(signals[TexkomSignalIndexTk17Tail]);
}

TexkomSignal* texkom_signal_alloc(const GpioPin* pin) {
    furi_check(pin);

    TexkomSignal* instance = malloc(sizeof(TexkomSignal));

    instance->tx_sequence = digital_sequence_alloc(TEXKOM_SIGNAL_BUFFER_SIZE, pin);
    texkom_signal_fill(instance);

    return instance;
}

void texkom_signal_free(TexkomSignal* instance) {
    furi_check(instance);

    digital_sequence_free(instance->tx_sequence);

    for(size_t i = 0; i < TexkomSignalIndexNum; i++) {
        digital_signal_free(instance->signals[i]);
    }

    free(instance);
}

/** Most significant bit first, one symbol per bit. */
static void texkom_signal_encode_tk13(TexkomSignal* instance, const uint8_t* frame) {
    for(size_t i = 0; i < TEXKOM_SIGNAL_FRAME_SIZE; i++) {
        for(size_t bit = 0; bit < BITS_IN_BYTE; bit++) {
            const bool one = (frame[i] << bit) & 0x80;
            digital_sequence_add_signal(
                instance->tx_sequence, one ? TexkomSignalIndexTk13One : TexkomSignalIndexTk13Zero);
        }
    }

    digital_sequence_add_signal(instance->tx_sequence, TexkomSignalIndexTk13Tail);
}

/** Least significant bit pair first, one symbol per pair. */
static void texkom_signal_encode_tk17(TexkomSignal* instance, const uint8_t* frame) {
    for(size_t i = 0; i < TEXKOM_SIGNAL_FRAME_SIZE; i++) {
        for(size_t bit = 0; bit < BITS_IN_BYTE; bit += 2) {
            digital_sequence_add_signal(
                instance->tx_sequence, TexkomSignalIndexTk17Sym00 + ((frame[i] >> bit) & 0x03));
        }
    }

    digital_sequence_add_signal(instance->tx_sequence, TexkomSignalIndexTk17Tail);
}

void texkom_signal_tx(TexkomSignal* instance, TexkomSignalCoding coding, const uint8_t* frame) {
    furi_check(instance);
    furi_check(coding < TexkomSignalCodingNum);
    furi_check(frame);

    digital_sequence_clear(instance->tx_sequence);

    if(coding == TexkomSignalCodingTk17) {
        texkom_signal_encode_tk17(instance, frame);
    } else {
        texkom_signal_encode_tk13(instance, frame);
    }

    digital_sequence_transmit(instance->tx_sequence);
}
