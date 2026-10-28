/**
 * @file t5577_sniff.h
 * @brief What another reader is writing to a tag, read off its own downlink.
 *
 * A 125 kHz reader sends commands by cutting gaps into its carrier, and the field-on time
 * between two gaps is the bit. Nothing in the firmware demodulates that and the envelope
 * comparator cannot see it at all - with no field of our own it self-oscillates near the
 * threshold - so this takes the digital route: TIM2's external trigger is wired to the carrier,
 * so the carrier RESETS the timer rather than clocking it. While the field runs, an edge every
 * 8 us keeps the timer from reaching its reload. It fires only when the carrier stops, which is
 * exactly a gap, with no threshold to fool.
 *
 * The detector came from applications_user/lfgap and was validated in applications_user/varta
 * against a reader whose command was already known. What differs here is everything above it.
 * That reader repeats one frame forever, so agreement across frames is available and the length
 * is fixed; a cloner setting a
 * password sends each command ONCE, and the lengths vary. So nothing is decoded in the handler:
 * it records raw gap widths and the periods between them, and the framing and decoding run
 * afterwards over the whole capture. A one-shot burst cannot be captured twice, and a framing
 * rule applied in the handler throws away the only copy. The handler does count runs, so a
 * listening screen has something true to show while it waits, but it keeps no decision - the
 * decode re-frames the whole ring from scratch.
 *
 * @warning Listening only. Nothing here energises the coil or sends a frame, so no password can
 * reach a tag by this path - which is what makes pointing it at an unknown reader safe.
 */
#pragma once

#include <furi.h>

/** Gaps kept. The ring holds the LAST this many, so noise arriving before the burst is
 * evicted by it rather than filling the buffer ahead of it.
 *
 * A twelve-block passworded restore is twelve 71-gap frames plus its markers. Measured larger
 * than that: a blue cloner's write run came to 1860 gaps, counting what it does either side of
 * the writes, and at 1024 the ring wrapped on every attempt.
 */
#define T5577_SNIFF_ENTRIES 2048

/** Symbols kept per frame, and the value `T5577SniffFrame.symbols` saturates at.
 *
 * The longest frame any supported mode sends is a leading-zero passworded write at 73; the slack
 * above it is for a sender or a mode this does not model, so that an unreadable burst still
 * reports a length worth looking at rather than one trimmed to fit. Past this the boundary is
 * not where we think it is and there is nothing to gain by storing more.
 */
#define T5577_SNIFF_SYMBOLS_MAX 96

/** Frames the decode reports. A sequence longer than this is cut, and says so.
 *
 * Sized against the same worst case as the ring and not independently of it: a twelve-block
 * passworded restore is twelve writes, each followed by the read-mode command that stops the tag
 * repeating the block, plus the one that closes the sequence - 25 frames. At 24 the table
 * truncated the exact capture the ring was built to hold. Raised again with the ring, because a
 * real cloner idles either side of its write run and those bursts are frames in the table too -
 * a blue cloner filled 32 slots and dropped one with twelve real frames among them.
 */
#define T5577_SNIFF_FRAMES_MAX 48

/** Where one symbol class ends and the other begins when a burst will not say, in
 * microseconds.
 *
 * Measured, not chosen. This firmware's fixed-bit-length write sends 24 field clocks for a zero
 * and 56 for a one, which with the 18-clock write gap is 336 and 592 us; a real Varta reader
 * measured 349 and 636. Both sit well clear of 480 on either side.
 *
 * It is only the FALLBACK. A burst whose own two classes are far enough apart is split between
 * them instead - see T5577_SNIFF_CLASSES_MIN_US - because this constant describes one sender and
 * a sniffer meets senders it has never measured. The one in the firmware next door already
 * breaks it: leading-zero mode sends 18 and 40 field clocks, so its classes sit at 288 and
 * 464 us and BOTH are below 480. Against a fixed split every symbol of that mode reads zero.
 */
#define T5577_SNIFF_SPLIT_US 480

/** How far apart the capture's two symbol classes must sit before they are used in place of the
 * fixed split.
 *
 * Narrower than any real pair - 256 us apart in fixed mode, 176 in leading-zero, 287 on the
 * Varta reader - and far wider than the jitter within one class, so a capture that is all
 * zeroes or all ones falls back rather than inventing a boundary through the middle of one
 * class.
 */
#define T5577_SNIFF_CLASSES_MIN_US 120

/** How the stall-width histogram bins a gap, in microseconds, and how many bins it has.
 *
 * Every stall is binned, in band or not, which is the whole point: the band is a guess about a
 * sender this has never met, and a sender whose gaps fall outside it is otherwise invisible
 * except as a number in `stalls`. 64 us is fine enough to separate a write gap from a start gap
 * - 144 against 240 in this firmware - and 24 bins reach 1536 us, past any downlink gap.
 */
#define T5577_SNIFF_WIDTH_BUCKET_US 64
#define T5577_SNIFF_WIDTH_BUCKETS   24

/** Symbols a burst needs before it is evidence that anything was said.
 *
 * Six is the shortest ADDRESSED command - opcode, lock and block - so below it a burst carries
 * no address and cannot be a reader interrogating a tag. The two-symbol commands are real and
 * are decoded, but they are not enough on their own: a reset or a read-mode command arrives in
 * the middle of a sequence, and open air produces two-symbol bursts by itself. Measured on a
 * desk over thirty seconds: 826 in-band gaps, one burst of two symbols, nothing longer.
 */
#define T5577_SNIFF_COMMAND_MIN 6

/** Symbols a burst needs before it is counted as a frame.
 *
 * Two is the shortest real command - a reset, or the regular read that follows every write - and
 * those matter here precisely because they bracket the writes. Below that a burst is one stray
 * stall with quiet either side, which a carrier with a little ripple produces constantly.
 */
#define T5577_SNIFF_FRAME_MIN 2

/** The longest pause that can still be a write's programming time.
 *
 * The read-mode command that follows every write also follows every READ in this firmware
 * (`t5577_block_read_bits()`), so the command alone proves nothing - what separates them is how
 * long the carrier stayed up first. A write resumes 8.8 ms later: 5.6 ms of programming, then
 * the 3.2 ms charge-up at the head of the next frame. A read waits for the tag to answer, which
 * is 450 ms of settling plus up to 150 of capture. Twenty milliseconds sits between them with
 * room on both sides.
 */
#define T5577_SNIFF_MARKER_MAX_US 20000

/** What a frame turned out to be, by its length and what follows it. */
typedef enum {
    /** A length no downlink mode produces, so none of the fields mean anything. */
    T5577SniffFrameUnknown,
    /** Two zero bits: the chip's reset opcode. */
    T5577SniffFrameReset,
    /** The opcode alone - go back to broadcasting data blocks. Sent after every write, which is
     * what makes it worth decoding: it brackets the command before it.
     */
    T5577SniffFrameRegularRead,
    /** Opcode, lock, block. Carries no password and is always safe for the tag. */
    T5577SniffFramePlainRead,
    /** 38 bits, which a plain write and a passworded read both are, with different layouts. The
     * chip cannot tell them apart either - that is the hazard this whole app is careful about -
     * so BOTH readings are filled in and neither is claimed.
     */
    T5577SniffFrameAmbiguous,
    /** A write with no password: `data` is what went into `block`. Reached either by length, in
     * a downlink mode where it is unambiguous, or by what follows an ambiguous frame.
     */
    T5577SniffFramePlainWrite,
    /** A read that sent a password: `password` is one the sender already holds. */
    T5577SniffFramePasswordedRead,
    /** A write that sent a password: `password` is the old one, `data` the new contents. The
     * most valuable frame a cloner can emit, and unambiguous by length.
     */
    T5577SniffFramePasswordedWrite,
} T5577SniffFrameKind;

/** How an ambiguous frame was resolved, so a screen can say why rather than just what. */
typedef enum {
    T5577SniffWhyNone, // unresolved, or nothing needed resolving. First, so that it is what a
        // zeroed frame carries and no code has to remember to set it
    T5577SniffWhyLength, // the length admits only one reading
    T5577SniffWhyMarker, // a read-mode command followed it, after a wait only a write explains
    T5577SniffWhyPause, // the carrier was cut again before a programming time had passed, so
        // nothing can have been burned into EEPROM and the frame was not a write
} T5577SniffWhy;

typedef struct {
    T5577SniffFrameKind kind;
    T5577SniffWhy why;

    uint8_t symbols; // symbols measured, before any mode's preamble is stripped. Saturates
        // at T5577_SNIFF_SYMBOLS_MAX, so that value means "at least that many"
    bool leading_zero; // decoded as leading-zero mode rather than fixed bit length
    bool page; // the opcode's second bit
    bool lock;
    uint8_t block;

    // Filled per kind. For T5577SniffFrameAmbiguous BOTH are filled, from the same 38 bits read
    // two different ways, and `block` is then the plain-write reading's - the passworded-read
    // layout puts the address in the same three bits, so the two agree on it.
    uint32_t data;
    uint32_t password;

    /** The lock bit of the OTHER reading of an ambiguous frame.
     *
     * A plain write puts its lock bit two symbols past the opcode and a passworded read puts it
     * three from the end, so the two readings disagree about it where they agree about the
     * block. `lock` holds the plain-write reading's; this holds the other, and a resolution that
     * lands on the read swaps them. Without it a frame resolved to a passworded read reported
     * the top bit of its own password as a lock bit.
     */
    bool lock_alt;

    /** Carrier-on time between this frame's last gap and the next frame's first, in
     * microseconds. A write needs its programming time here and a read does not, so this is
     * evidence about the frame BEFORE it. Zero when nothing followed - which is the capture
     * ending, not the sender stopping.
     */
    uint32_t pause_us;
    /** The pause ran past what a gap period can hold, so pause_us is a floor and not a
     * measurement. A reader waiting for a tag to answer leaves hundreds of milliseconds, and
     * reporting the 65 ms ceiling as a figure is what let the marker rule mistake one for a
     * write's programming time.
     */
    bool pause_capped;
} T5577SniffFrame;

/** Counters cheap enough to read on a UI tick, for a screen that has to show something while
 * it waits and has to know when the burst has ended.
 */
typedef struct {
    uint32_t stalls; // every completed stall, in band or not: ripple has to be visible
    uint32_t gaps; // stalls inside the width band a downlink gap falls in
    uint32_t bursts; // runs of T5577_SNIFF_FRAME_MIN symbols or more, so far
    bool overflowed; // more gaps arrived than the ring holds, so the oldest were dropped
} T5577SniffProgress;

/** How wide the carrier stalls were, binned at T5577_SNIFF_WIDTH_BUCKET_US.
 *
 * Counted before the band test, so this is the one place a reader whose gaps this cannot use is
 * visible as something other than silence. The last bin holds everything above its own edge.
 */
typedef struct {
    uint16_t bucket[T5577_SNIFF_WIDTH_BUCKETS];
} T5577SniffWidths;

/** The capture, framed and decoded. Large enough that it belongs in the app struct rather than
 * on a 4 KiB stack.
 */
typedef struct {
    T5577SniffProgress progress;

    uint32_t short_us; // mean of each symbol class against the FIXED split, for checking the
    uint32_t long_us; // fallback still separates a sender this had to measure for itself

    /** The threshold the capture's symbols were classed against.
     *
     * The capture's own two class means where they separated, and T5577_SNIFF_SPLIT_US where
     * they did not. One sender is talking in one mode, so this is a property of the capture
     * rather than of a frame - and measuring it over hundreds of symbols instead of one burst's
     * two extremes is what keeps a single missed gap from taking a frame down with it.
     */
    uint16_t split_us;

    T5577SniffWidths widths;

    uint8_t frames; // entries filled in frame[]
    uint8_t dropped; // frames found past the end of frame[]
    T5577SniffFrame frame[T5577_SNIFF_FRAMES_MAX];
} T5577SniffResult;

typedef struct T5577Sniff T5577Sniff;

T5577Sniff* t5577_sniff_alloc(void);

void t5577_sniff_free(T5577Sniff* sniff);

/** Start listening. Claims TIM2, so no read, write or emulation may run while this does. */
void t5577_sniff_start(T5577Sniff* sniff);

void t5577_sniff_stop(T5577Sniff* sniff);

/** Counters only. Safe while listening and cheap enough for a UI tick. */
void t5577_sniff_poll(const T5577Sniff* sniff, T5577SniffProgress* progress);

/** Frame the capture and decode it.
 *
 * Call after t5577_sniff_stop(): it walks the whole ring, and a gap arriving mid-walk would be
 * framed into a sequence that was already half read.
 */
void t5577_sniff_decode(const T5577Sniff* sniff, T5577SniffResult* result);

/** Whether anything in this capture is long enough to have been a reader talking.
 *
 * The question a verdict screen must ask before it says what it heard - counting frames instead
 * reports a reader from a desk's worth of carrier ripple.
 */
static inline bool t5577_sniff_commanded(const T5577SniffResult* result) {
    for(uint8_t i = 0; i < result->frames; i++) {
        if(result->frame[i].symbols >= T5577_SNIFF_COMMAND_MIN) return true;
    }

    return false;
}

// The kinds are not ordered by anything, and nothing may compare them. `>= PlainRead` happening
// to mean "carries a block address" today is the respelling these helpers exist to prevent -
// which is also why what a kind implies is asked here rather than stored beside it.

/** Whether `block` means anything, which the short commands carry no room for.
 *
 * `lock` goes with it for every kind but T5577SniffFrameAmbiguous, whose two readings put their
 * lock bits in different places - see `lock_alt`.
 */
static inline bool t5577_sniff_frame_addressed(const T5577SniffFrame* frame) {
    return frame->kind == T5577SniffFramePlainRead || frame->kind == T5577SniffFrameAmbiguous ||
           frame->kind == T5577SniffFramePlainWrite ||
           frame->kind == T5577SniffFramePasswordedRead ||
           frame->kind == T5577SniffFramePasswordedWrite;
}

/** Whether `data` is what the sender put in the block, which only the writes carry. */
static inline bool t5577_sniff_frame_has_data(const T5577SniffFrame* frame) {
    return frame->kind == T5577SniffFrameAmbiguous || frame->kind == T5577SniffFramePlainWrite ||
           frame->kind == T5577SniffFramePasswordedWrite;
}

/** Whether this frame's password field is a password its sender holds.
 *
 * For an ambiguous frame the answer is "only on one of its two readings", which is a thing to
 * show with both words on screen and not a thing to report as a password.
 */
static inline bool t5577_sniff_frame_tells_password(const T5577SniffFrame* frame) {
    return frame->kind == T5577SniffFramePasswordedRead ||
           frame->kind == T5577SniffFramePasswordedWrite;
}
