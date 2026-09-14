#include "hitags.h"

#include <furi.h>
#include <furi_hal_rfid.h>
#include <lib/bit_lib/bit_lib.h>

#define TAG "HitagS"

// =================================================================================================
// Protocol constants. Datasheet = NXP HITAG S product specification rev 3.1; frames cross-checked
// against Proxmark3 armsrc/hitagS.c and include/protocols.h.
// =================================================================================================

// Reader->tag BPLM (datasheet 7.4.1), T0 = one 125 kHz cycle = 8 us. Every bit cell opens with an
// 8 T0 gap and holds the field on for the rest: 20 T0 in total for a '0', 28 T0 for a '1'. This is
// on/off keying of the field, not Manchester - Manchester is only the tag->reader direction.
// Same T_LOW / T_0 / T_1 as tools/hitagmicro.c - one BPLM family, and neither set is
// re-derivable without a Proxmark, so keep the two in step.
#define HITAGS_GAP_US     64
#define HITAGS_BIT0_ON_US 96
#define HITAGS_BIT1_ON_US 160

#define HITAGS_CMD_UID_REQ_ADV 0xC8 // 11001, 5 bits: UID REQUEST advanced (Table 17)
#define HITAGS_CMD_SELECT      0x00 // 00000, 5 bits
#define HITAGS_CMD_WRITE_PAGE  0x80 // 1000, 4 bits

// An AUT=1 tag rejects a plain SELECT + WRITE. It is unlocked by writing the password to the
// magic page 64 - an ordinary WRITE PAGE frame - as Proxmark3 hts_select_tag() does for HTSF_82xx.
// This is the family default shared by every ID82xx clone, not a per-tag secret.
#define HITAGS_82XX_AUTH_PAGE 0x40
static const uint8_t hitags_82xx_password[LFRFID_HITAGS_PAGE_SIZE] = {0xBB, 0xDD, 0x33, 0x99};

// Pages the factory TTF config streams as the EM4100 frame. An ID8268 and its siblings ship with
// CON1 set to TTF "Page 4, Page 5", Manchester, 2 kBit, so writing the 64-bit EM4100 frame into
// these two pages is all it takes to change the emitted id.
#define HITAGS_EM_PAGE0 4
#define HITAGS_EM_PAGE1 5

#define HITAGS_UID_BITS (LFRFID_HITAGS_UID_SIZE * 8)
#define HITAGS_SOF_BITS 3 // advanced mode answers behind a '111' SOF (Table 18)

// =================================================================================================
// Read path. Every number below was measured on real ID82xx tags.
// =================================================================================================

// Tag->reader anticollision at 2 kBit/s (Table 1/18): a 512 us cell built from four 128 us half
// periods, '0' = one falling edge per cell, '1' = two. The Flipper's envelope detector low-passes
// a '1' into nothing, so a reply arrives as one edge per '0' with gaps that are whole multiples of
// the cell, and gap/cell - 1 is the number of '1's in between.
#define HITAGS_CELL_US         512
// Genuine gaps sit within ~12 us of an exact multiple, so this is five times the worst measured.
// It must stay well under half a cell: a tag idling in TTF is Manchester on a 256 us half period,
// so its gaps include 768 us = 1.5 cells, and widening this rounds those into acceptance. At 150 it
// let an artifact 112 us off a whole cell pass for a data cell and append a phantom '0' to the UID.
#define HITAGS_CELL_TOL_US     64
#define HITAGS_CELL_MAX        10 // longest gap between '0' cells, in cells, so at most 9 '1's
#define HITAGS_MAX_TRAILING    2 // a reply stops, so only a stray edge or two may follow it
#define HITAGS_MIN_ZEROS       6 // edges a UID frame must carry to be worth aligning
#define HITAGS_MAX_GUESS_BITS  6 // lost leading bits we are willing to enumerate against the tag
#define HITAGS_REANCHOR_EVERY  8 // candidates between Init-state refreshes
#define HITAGS_AC_SEQ_K        16 // UID bits the cross-check replays; the tag returns the rest
#define HITAGS_VERIFY_MIN_BITS 8 // span a cross-check reply must cover to mean anything
#define HITAGS_VERIFY_MIN_EDGE 2 // and edges inside it - span alone counts inferred '1' cells
#define HITAGS_TWRESP_US       1650 // 204..212 T0 turnaround; used only to shape self-test vectors

// Field-on listening windows measured from the command's EOF. SOF + 32 bits at 512 us is ~17.9 ms
// plus twresp; the AC SEQUENCE reply is half that. Both stay inside twsc (0.72..40 ms), the window
// in which the tag still accepts the next command.
#define HITAGS_WIN_UID_MS 26
#define HITAGS_WIN_AC_MS  14

// An ID82xx leaves the factory in Transponder Talks First mode - that is how it emits EM4100 - and
// a TTF tag only accepts a UID REQUEST whose first bit falls in the mode switch window, 280..520 T0
// after power-up (9.7, Table 16). Miss it and the tag streams its pages forever, deaf to commands.
// Measured on an F8268: a field-off of 5 ms or less leaves the tag powered and it ignores the
// command, 8 ms resets it every time. 10 ms for margin.
#define HITAGS_POWERDOWN_MS 10
#define HITAGS_TSWITCH_US   2800 // 350 T0: 0.56 ms inside the window, 1.36 ms before its end

// Both measured from the field's return: the receiver is deaf until somewhere between 7 and 11 ms,
// while the tag's answer starts at ~5.5 ms and runs ~18 ms, so the first reply always has its SOF
// inside the blind window. It does not have to be the one we read - the tag stays in Init state
// afterwards, where a further UID REQUEST is answered again (11.2), and a warm command costs only
// its own 64 us gaps, which the receiver shrugs off in 2-4 ms.
#define HITAGS_RECOVER_MS 25

#define HITAGS_SETTLE_MS       450 // envelope detector settle, same as the lfrfid reader
#define HITAGS_SETTLE_SLICE_MS 50 // sliced so aborting stays responsive
#define HITAGS_ATTEMPTS        4 // full read attempts per call

// A reply is at most SOF + 32 cells; the rest is headroom for command gaps and detector ringing.
#define HITAGS_MAX_EDGES 192

// Every frame this module decodes or lays out fits in one buffer of this size.
#define HITAGS_FRAME_BYTES 8

/** The bit pattern the receiver actually saw, relative to its own first edge. */
typedef struct {
    uint8_t bits[HITAGS_FRAME_BYTES]; /**< bit 0 is the first observed '0' cell */
    size_t span; /**< cells from the first observed edge through the last */
    size_t edges; /**< edges actually folded in - span counts inferred '1' cells too */
    size_t frame_bits; /**< frame this was decoded against; alignments are relative to it */
    bool truncated; /**< diagnostic: the run ended before the frame did */
} HitagSInterior;

typedef struct {
    uint32_t now; /**< running timestamp of the last rising edge, us */
    uint32_t edge[HITAGS_MAX_EDGES]; /**< falling-edge timestamps, us since the window start */
    volatile uint16_t count;
    volatile bool rebased; /**< the next rising edge defines t = 0 */
    volatile bool overflowed; /**< edges were dropped; the window is not the whole story */
} HitagSCapture;

/** Three outcomes, and conflating them is a bug: a reply that cannot be judged is not the tag
 * refusing the prefix. Only HitagSVerifyRefused is evidence against a candidate. */
typedef enum {
    HitagSVerifyProven,
    HitagSVerifyInconclusive, /**< nothing decodable came back, or too little of it */
    HitagSVerifyRefused, /**< a good reply, but not the one this prefix demands */
} HitagSVerify;

// Only one RF session can own the LF antenna at a time, and the capture ISR has no context of its
// own worth threading through. Allocated for the duration of a read rather than kept in bss: the
// edge buffer is 768 bytes and every other user of this firmware pays for it otherwise.
static HitagSCapture* s_capture;
static HitagSAbortCallback s_abort;
static void* s_abort_context;

static bool hitags_aborted(void) {
    return s_abort != NULL && s_abort(s_abort_context);
}

// =================================================================================================
// Frames. bit_lib is MSB-first (bit 0 is the MSB of byte 0), which is what Proxmark3's concatbits()
// and the datasheet's bit numbering both use, so frames pack identically.
// =================================================================================================

// Appends the TOP `nbits` of `value` - callers rely on that (0xC8 -> 11001), which is why
// bit_lib_set_bits (bottom-justified) is not the right primitive here.
static void hitags_put_bits(uint8_t* buf, size_t* pos, uint8_t value, uint8_t nbits) {
    furi_check(nbits <= 8);
    bit_lib_copy_bits(buf, *pos, nbits, &value, 0);
    *pos += nbits;
}

/** CRC-8/Hitag: poly 0x1D, init 0xFF, MSB-first, no reflection. Same as PM3 CRC8Hitag1Bits(); it
 * takes a bit length because the frames are not byte-aligned. */
static uint8_t hitags_crc8(const uint8_t* buf, size_t bitsize) {
    uint8_t crc = 0xFF;
    for(size_t i = 0; i < bitsize; i++) {
        uint8_t msb = (crc >> 7) & 1;
        crc <<= 1;
        if(msb ^ (bit_lib_get_bit(buf, i) ? 1 : 0)) crc ^= 0x1D;
    }
    return crc;
}

static size_t hitags_build_uid_request(uint8_t* tx) {
    size_t pos = 0;
    hitags_put_bits(tx, &pos, HITAGS_CMD_UID_REQ_ADV, 5);
    return pos;
}

/** AC SEQUENCE (11.3): a 5-bit collision position k, the first k UID bits, CRC-8. Only a
 * transponder whose UID really starts that way answers, with the remaining 32 - k bits - which
 * turns anticollision into a read-back check when k is one we already believe in. */
static size_t hitags_build_ac_sequence(uint8_t* tx, const uint8_t* uid, size_t k) {
    furi_check(k > 0 && k < HITAGS_UID_BITS);
    size_t pos = 0;
    hitags_put_bits(tx, &pos, (uint8_t)(k << 3), 5); // k4..k0, MSB-first
    bit_lib_copy_bits(tx, pos, k, uid, 0);
    pos += k;
    hitags_put_bits(tx, &pos, hitags_crc8(tx, pos), 8);
    return pos;
}

static size_t hitags_build_select(uint8_t* tx, const uint8_t* uid) {
    size_t pos = 0;
    hitags_put_bits(tx, &pos, HITAGS_CMD_SELECT, 5);
    bit_lib_copy_bits(tx, pos, HITAGS_UID_BITS, uid, 0);
    pos += HITAGS_UID_BITS;
    hitags_put_bits(tx, &pos, hitags_crc8(tx, pos), 8);
    return pos;
}

static size_t hitags_build_write_page(uint8_t* tx, uint8_t page) {
    size_t pos = 0;
    hitags_put_bits(tx, &pos, HITAGS_CMD_WRITE_PAGE, 4);
    hitags_put_bits(tx, &pos, page, 8);
    hitags_put_bits(tx, &pos, hitags_crc8(tx, pos), 8);
    return pos;
}

static size_t hitags_build_write_data(uint8_t* tx, const uint8_t* data) {
    size_t pos = 0;
    bit_lib_copy_bits(tx, pos, LFRFID_HITAGS_PAGE_SIZE * 8, data, 0);
    pos += LFRFID_HITAGS_PAGE_SIZE * 8;
    hitags_put_bits(tx, &pos, hitags_crc8(tx, pos), 8);
    return pos;
}

// =================================================================================================
// Field and modulation.
// =================================================================================================

// Stopping the PWM leaves the tank ringing down at its own Q, so a gap may only dent the field
// where the datasheet wants a modulation index of 0.95..1. Damping the antenna through the gap was
// tried and measured worse at every gap length (41 edges -> 11), so the gap is left undamped.
static void hitags_gap(void) {
    furi_hal_rfid_tim_read_pause();
    furi_delay_us(HITAGS_GAP_US);
    furi_hal_rfid_tim_read_continue();
}

static void hitags_send_bit(bool one) {
    hitags_gap();
    furi_delay_us(one ? HITAGS_BIT1_ON_US : HITAGS_BIT0_ON_US);
}

// Sends `nbits` MSB-first then the EOF gap, after which the field stays on far longer than TEOF.
// Callers hold the critical section: a bit cell stretched by a preemption is a malformed command.
static void hitags_send_frame(const uint8_t* tx, size_t nbits) {
    for(size_t i = 0; i < nbits; i++) {
        hitags_send_bit(bit_lib_get_bit(tx, i));
    }
    hitags_gap();
}

// =================================================================================================
// Capture. The HAL reports one pair per modulation cycle: on the falling edge `duration` is the
// high time since the last rising edge, on the rising edge it is the whole period since the
// previous rising edge. Accumulating the periods gives every falling edge an absolute timestamp,
// which is all the anticollision decoder needs.
// =================================================================================================

// `level == true` is a FALLING edge carrying the high time; `level == false` is a RISING edge
// carrying the whole period, and the HAL zeroes the capture counter there - which is why only that
// edge can anchor t = 0.
static void hitags_capture_callback(bool level, uint32_t duration, void* context) {
    UNUSED(context);

    // The capture timer is only reset by a rising edge, so the first period after a rebase was
    // measured from an edge before it and says nothing about this window.
    if(s_capture->rebased) {
        if(level) return;
        s_capture->now = 0;
        s_capture->rebased = false;
        return;
    }

    if(level) {
        uint16_t count = s_capture->count;
        if(count < HITAGS_MAX_EDGES) {
            s_capture->edge[count] = s_capture->now + duration;
            // `edge` is not volatile and `count` is, and C orders volatile accesses only against
            // each other - so without this the payload store may sink past the publish and the
            // worker can read a slot it believes is already written.
            __DMB();
            s_capture->count = count + 1;
        } else {
            s_capture->overflowed = true;
        }
    } else {
        s_capture->now += duration;
    }
}

// Callers must hold FURI_CRITICAL, or run before the capture is armed: these stores have to land
// together, and the ISR is the other writer.
static void hitags_capture_rebase(void) {
    s_capture->count = 0;
    s_capture->now = 0;
    s_capture->overflowed = false;
    __DMB();
    s_capture->rebased = true;
}

/** Send a frame to a tag that is already powered and listening (inside twsc), then rebase the
 * capture so response timestamps are relative to the EOF. */
static void hitags_tx_warm(const uint8_t* tx, size_t nbits) {
    FURI_CRITICAL_ENTER();
    hitags_send_frame(tx, nbits);
    hitags_capture_rebase();
    FURI_CRITICAL_EXIT();
}

/** Reset the tag by dropping the field, then send `tx` in the mode switch window. Field-on, the
 * tswitch wait and the frame are one uninterrupted unit: the window is 1.9 ms wide and we aim
 * 0.56 ms inside its leading edge, so a preemption costs the whole interrogation. This masks
 * interrupts for ~4 ms and loses system ticks - do not "fix" it by widening the window. */
static void hitags_tx_cold(const uint8_t* tx, size_t nbits) {
    furi_hal_rfid_tim_read_pause();
    furi_delay_ms(HITAGS_POWERDOWN_MS);

    FURI_CRITICAL_ENTER();
    furi_hal_rfid_tim_read_continue();
    furi_delay_us(HITAGS_TSWITCH_US);
    hitags_send_frame(tx, nbits);
    hitags_capture_rebase();
    FURI_CRITICAL_EXIT();
}

// =================================================================================================
// Anticollision decoding.
// =================================================================================================

/** Validate the whole capture window as one anticollision frame and expand it into bits.
 *
 * The run starts at the first edge and ends at the first gap that is not a whole number of cells,
 * is longer than HITAGS_CELL_MAX, or would overrun the frame. What separates a real reply from a
 * tag idling in TTF is what is left over afterwards: a reply stops, so it trails nothing, while TTF
 * never stops and its 768 us Manchester gaps are 1.5 cells, which ends the run early and leaves a
 * pile of edges behind. The run must start at the *first* edge for that to hold - searching for the
 * longest clean run inside the window would step straight over those 768 us gaps.
 *
 * The result is RELATIVE: bit 0 is the first cell that carried an edge. Where that sits inside the
 * frame is deliberately not derived from absolute time - t = 0 is whichever rising edge the
 * detector produces once the field returns, and it drifts by whole cells with the receiver's
 * recovery. The caller enumerates the alignments instead and lets the tag pick.
 */
static bool hitags_decode_frame(size_t payload_bits, size_t min_edges, HitagSInterior* out) {
    const size_t total = HITAGS_SOF_BITS + payload_bits;
    const uint16_t count = s_capture->count;

    furi_check(total <= sizeof(out->bits) * 8);

    // Fully assigned before the first failure return, so a rejected window leaves a consistent
    // interior rather than the previous decode's span against this one's frame_bits.
    *out = (HitagSInterior){.frame_bits = total};
    memset(out->bits, 0xFF, sizeof(out->bits)); // an unobserved cell is a '1'

    // Nothing modulated at all. Bailing here rather than below keeps `edges` honest: the loop
    // counter starts at 1 because the first edge needs no gap to interpret it.
    if(count == 0) return false;

    // An overflowed window is NOT rejected, deliberately. It looks like it should be - the run's
    // leftovers are what tell a reply from a tag streaming TTF, and an overflow drops some of
    // them - but the trailing bound is a ceiling, so dropped edges can only ever make a window
    // look cleaner than it was, never dirtier. Rejecting on it throws away real replies whenever
    // the detector is chattering, which is exactly when reads are already scarce.

    bit_lib_set_bit(out->bits, 0, false); // the first edge is, by definition, a '0' cell
    size_t position = 0;

    uint16_t i = 1;
    for(; i < count; i++) {
        uint32_t delta = s_capture->edge[i] - s_capture->edge[i - 1];
        uint32_t cells = (delta + HITAGS_CELL_US / 2) / HITAGS_CELL_US;
        uint32_t error = (delta > cells * HITAGS_CELL_US) ? delta - cells * HITAGS_CELL_US :
                                                            cells * HITAGS_CELL_US - delta;

        // All three mean the frame ended before this edge: a gap that is not a whole number of
        // cells is the tag having gone quiet and left an artifact behind, and a gap that is too
        // long or would overrun the frame cannot belong to it either. Stop rather than folding it
        // in - that is what appended a phantom '0' to a UID. Rejecting the whole window instead is
        // wrong: the 19-cell AC SEQUENCE frame is short enough that an ordinary trailing artifact
        // overruns it, and good replies got thrown away.
        if(cells < 1 || error > HITAGS_CELL_TOL_US) break;
        if(cells > HITAGS_CELL_MAX) break;
        if(position + cells + HITAGS_SOF_BITS >= total) break;

        position += cells;
        bit_lib_set_bit(out->bits, position, false);
    }

    // The floor is on edges actually folded into the frame, not on what the window happened to
    // contain - trailing artifacts must not count towards it.
    out->edges = i;
    if(out->edges < min_edges) return false;

    // A reply stops, so it leaves nothing behind. This is what rejects a tag streaming TTF: its
    // 768 us gaps end the run early and the pile of edges after that overruns the bound.
    if((size_t)(count - i) > HITAGS_MAX_TRAILING) return false;

    out->span = position + 1;
    out->truncated = (count - i) > 0;
    return true;
}

/** Highest alignment that still fits the observed pattern inside the frame it was decoded against.
 * Taking the frame length from the interior rather than as a parameter is what keeps this from
 * underflowing and sending hitags_place off the end of the caller's buffer. */
static size_t hitags_lead_max(const HitagSInterior* interior) {
    furi_check(interior->span <= interior->frame_bits);
    return interior->frame_bits - interior->span;
}

/** Lay the observed pattern into a frame at `lead`, leaving every unobserved cell as '1'. */
static void hitags_place(const HitagSInterior* interior, size_t lead, uint8_t* frame) {
    memset(frame, 0xFF, HITAGS_FRAME_BYTES);
    bit_lib_copy_bits(frame, lead, interior->span, interior->bits, 0);
}

// =================================================================================================
// Interrogation.
// =================================================================================================

/** Dump the capture window's gaps. This is the diagnostic that cracked every decode bug so far -
 * brute-forcing the alignments offline against a known UID - but it must not be slow: it runs
 * before the cross-check, which has to land inside the tag's twsc with the read window already
 * spent, so a slow log line here looks like the tag refusing a candidate. */
static void hitags_log_capture(const char* what) {
#ifndef LOGS_RELEASE_BUILD
    if(furi_log_get_level() < FuriLogLevelDebug) return;

    uint16_t count = s_capture->count;
    FuriString* line = furi_string_alloc_printf(
        "%s: %u edges%s", what, count, s_capture->overflowed ? " (BUFFER FULL)" : "");
    for(uint16_t i = 0; i < count && i < 40; i++) {
        uint32_t previous = (i == 0) ? 0 : s_capture->edge[i - 1];
        furi_string_cat_printf(line, " %lu", (unsigned long)(s_capture->edge[i] - previous));
    }
    if(count > 40) furi_string_cat_printf(line, " ... (%u more)", count - 40);
    FURI_LOG_D(TAG, "%s", furi_string_get_cstr(line));
    furi_string_free(line);
#else
    UNUSED(what);
#endif
}

/** Switch the tag out of TTF and read its UID reply. Two commands, for a reason measured on
 * hardware: the reset that opens the mode switch window leaves the receiver deaf past the start of
 * the first reply, so that one is always lost. The tag stays in Init state afterwards, and a warm
 * command costs only its own bit gaps. The reset is not optional either - without it the tag stays
 * in TTF and simply streams. */
static bool hitags_read_uid_frame(HitagSInterior* out) {
    uint8_t tx[1] = {0};
    size_t nbits = hitags_build_uid_request(tx);

    hitags_tx_cold(tx, nbits); // switches the tag out of TTF; its reply is unreachable
    furi_delay_ms(HITAGS_RECOVER_MS);
    hitags_tx_warm(tx, nbits); // ask again into a receiver that has had time to recover
    furi_delay_ms(HITAGS_WIN_UID_MS);
    hitags_log_capture("reply");

    if(!hitags_decode_frame(HITAGS_UID_BITS, HITAGS_MIN_ZEROS, out)) return false;

    FURI_LOG_D(
        TAG,
        "%u cells / %u edges observed, %u alignments to try%s",
        (unsigned)out->span,
        (unsigned)out->edges,
        (unsigned)(hitags_lead_max(out) - HITAGS_SOF_BITS + 1),
        out->truncated ? " (run ended early)" : "");
    return true;
}

/** How many cells of a candidate's tail past `k` would actually produce an edge. A '1' leaves none,
 * so a tail of all '1's cannot be told from the tag staying silent. */
static size_t hitags_tail_zeros(const uint8_t* uid, size_t k) {
    size_t zeros = 0;
    for(size_t i = k; i < HITAGS_UID_BITS; i++) {
        if(!bit_lib_get_bit(uid, i)) zeros++;
    }
    return zeros;
}

/** Does `reply` carry the bits that must follow the first `k` bits of `uid`?
 *
 * Separated from the RF above it because this is the module's whole trust anchor and the only part
 * of it a test can reach. Permissive by construction - unobserved cells are '1', and any alignment
 * counts - so what bounds it is the span and edge floors the caller applies first. */
static bool hitags_reply_matches(const HitagSInterior* reply, const uint8_t* uid, size_t k) {
    // What the tag must answer with: the SOF, then the bits that follow the replayed prefix.
    uint8_t expected[HITAGS_FRAME_BYTES];
    memset(expected, 0xFF, sizeof(expected));
    bit_lib_copy_bits(expected, HITAGS_SOF_BITS, HITAGS_UID_BITS - k, uid, k);

    const size_t lead_max = hitags_lead_max(reply);
    for(size_t lead = HITAGS_SOF_BITS; lead <= lead_max; lead++) {
        bool match = true;
        for(size_t i = 0; i < reply->span && match; i++) {
            match = bit_lib_get_bit(reply->bits, i) == bit_lib_get_bit(expected, lead + i);
        }
        if(match) return true;
    }
    return false;
}

/** Make the tag prove a candidate's first `k` bits. An answered AC SEQUENCE is proof of the prefix
 * structurally, in a way no amount of comparing captures against each other can be. The answer's
 * content is checked too, but that part is weaker than it looks: it comes from the same tag through
 * the same lossy receiver, so a '0' missed here and in the UID read is self-consistent and matches.
 * Raising `k` is what actually moves bits from "matched a lossy capture" to "the tag selected it". */
static HitagSVerify hitags_verify_prefix(const uint8_t* uid, size_t k) {
    uint8_t tx[8] = {0};
    size_t nbits = hitags_build_ac_sequence(tx, uid, k);

    hitags_tx_warm(tx, nbits);
    furi_delay_ms(HITAGS_WIN_AC_MS);

    const size_t tail_bits = HITAGS_UID_BITS - k;
    HitagSInterior reply = {0};
    if(!hitags_decode_frame(tail_bits, HITAGS_VERIFY_MIN_EDGE, &reply)) {
        return HitagSVerifyInconclusive;
    }
    if(reply.span < HITAGS_VERIFY_MIN_BITS) return HitagSVerifyInconclusive;

    return hitags_reply_matches(&reply, uid, k) ? HitagSVerifyProven : HitagSVerifyRefused;
}

/** For the sweep, anything short of proof is a non-answer: a candidate that cannot be judged has no
 * business winning. */
static bool hitags_verify_uid(const uint8_t* uid) {
    return hitags_verify_prefix(uid, HITAGS_AC_SEQ_K) == HitagSVerifyProven;
}

/** Put the tag back into Init state. It answers a UID REQUEST whenever it is powered, but it does
 * not stay in Init indefinitely and the sweep below can run for seconds - observed: a 127-candidate
 * sweep whose correct entry sat ~1.3 s in got no reply at all until the state was refreshed. */
static void hitags_reanchor(void) {
    uint8_t tx[1] = {0};
    size_t nbits = hitags_build_uid_request(tx);
    hitags_tx_warm(tx, nbits);
    furi_delay_ms(HITAGS_WIN_UID_MS); // its answer is not needed, only the state change
}

/** Walk `k` upwards over the winner, converting its tail from "matched one lossy capture" into "the
 * tag selected it" - the sweep already proved bits 0..HITAGS_AC_SEQ_K that way. A `k` whose
 * expected answer is all '1's is skipped: that answer carries no edges, so it cannot be told from
 * the tag staying silent. Returns the highest k proven, 0 if the tag refused one of them.
 *
 * Only k=22 is walked. A reply's span can never exceed the tail it answers with, so a k past 24
 * leaves fewer than HITAGS_VERIFY_MIN_BITS cells to judge and comes back Inconclusive every time -
 * k=26 and k=30 were in this list and cost ~50 ms of dead RF per read without ever deciding
 * anything. Going further needs that floor made relative to the tail length, not more entries. */
static size_t hitags_prove_prefix(const uint8_t* uid) {
    static const size_t k_walk[] = {22};
    size_t proven = HITAGS_AC_SEQ_K;

    for(size_t i = 0; i < COUNT_OF(k_walk); i++) {
        if(hitags_aborted()) break;
        if(hitags_tail_zeros(uid, k_walk[i]) < HITAGS_VERIFY_MIN_EDGE) continue;

        hitags_reanchor();
        const HitagSVerify result = hitags_verify_prefix(uid, k_walk[i]);
        if(result == HitagSVerifyRefused) {
            FURI_LOG_W(
                TAG, "k=%u refused after k=%u proven", (unsigned)k_walk[i], (unsigned)proven);
            return 0; // the candidate is wrong past the bits already proven
        }
        // Inconclusive only means this k could not be judged - its tail's '0' cells were too few or
        // too clustered to come back as a usable reply. Keep what is already proven.
        if(result == HitagSVerifyProven) proven = k_walk[i];
    }
    return proven;
}

/** Resolve where the observed pattern sits in the frame, and the bits ahead of it, by putting every
 * possibility to the tag.
 *
 * Both unknowns sit inside the prefix the AC SEQUENCE replays, so the reply's *content* is the same
 * for every candidate - what separates them is which one the tag answers at all. That makes "it
 * answered" load-bearing, which is exactly the signal that once produced a confident wrong UID, so
 * every candidate is tried rather than stopping at the first hit and exactly one must answer.
 */
static bool hitags_resolve_candidate(const HitagSInterior* interior, uint8_t* winner) {
    // Clamped here rather than broken out of below, because this bound is what decides whether the
    // sweep costs one second or ten - each alignment past the first doubles the candidates.
    const size_t lead_max =
        MIN(hitags_lead_max(interior), (size_t)(HITAGS_SOF_BITS + HITAGS_MAX_GUESS_BITS));
    uint32_t hits = 0;
    uint32_t tried = 0;

    for(size_t lead = HITAGS_SOF_BITS; lead <= lead_max; lead++) {
        // Cells ahead of the pattern are UID bits nobody saw; enumerate them.
        const size_t guessed = lead - HITAGS_SOF_BITS;

        uint8_t frame[HITAGS_FRAME_BYTES];
        hitags_place(interior, lead, frame);

        // Only the leading `guessed` bits vary across the combinations, so lift the rest out.
        uint8_t base[LFRFID_HITAGS_UID_SIZE];
        bit_lib_copy_bits(base, 0, HITAGS_UID_BITS, frame, HITAGS_SOF_BITS);

        for(uint32_t combination = 0; combination < (1u << guessed); combination++) {
            if(hitags_aborted()) return false;

            uint8_t candidate[LFRFID_HITAGS_UID_SIZE];
            memcpy(candidate, base, sizeof(candidate));
            for(size_t bit = 0; bit < guessed; bit++) {
                bit_lib_set_bit(candidate, bit, (combination >> (guessed - 1 - bit)) & 1);
            }

            // Refresh Init state periodically or the tail of a long sweep talks to a tag that has
            // stopped listening, and the right candidate goes unanswered.
            if((tried % HITAGS_REANCHOR_EVERY) == 0) hitags_reanchor();

            tried++;
            if(!hitags_verify_uid(candidate)) continue;
            hits++;
            memcpy(winner, candidate, LFRFID_HITAGS_UID_SIZE);
        }
    }

    if(hits != 1) {
        FURI_LOG_W(
            TAG,
            "%lu of %lu candidates answered - not selecting",
            (unsigned long)hits,
            (unsigned long)tried);
        return false;
    }

    // Negative control: the same UID with one prefix bit flipped must NOT be answered. Silence
    // scores as a pass, but silence is also what a tag that has stopped listening produces - so
    // re-anchor first, and afterwards make the winner answer again. Without that second half the
    // control passes vacuously exactly when it matters most, at the end of a long sweep.
    hitags_reanchor();
    uint8_t control[LFRFID_HITAGS_UID_SIZE];
    memcpy(control, winner, LFRFID_HITAGS_UID_SIZE);
    bit_lib_set_bit(control, HITAGS_AC_SEQ_K - 1, !bit_lib_get_bit(control, HITAGS_AC_SEQ_K - 1));
    if(hitags_verify_uid(control)) {
        FURI_LOG_W(TAG, "negative control answered - field is chattering, discarding");
        return false;
    }

    hitags_reanchor();
    if(!hitags_verify_uid(winner)) {
        FURI_LOG_W(TAG, "winner went quiet - the control proved nothing, discarding");
        return false;
    }

    // Now push the proof as far along the UID as the tag will carry it.
    return hitags_prove_prefix(winner) != 0;
}

// =================================================================================================
// Write. Open-loop: the tag's answers are on the 4 kBit Manchester link, which this receiver
// low-passes into a flat line, so nothing here is read back. Inter-frame waits have to clear the
// whole answer (twresp + reply + twsc) blind, which is why they are generous while staying under
// twsc max (40 ms): the UID reply is ~20 ms, the SELECT config answer ~14 ms, short acks ~4 ms, and
// a page write adds tprog on top.
// =================================================================================================
// The three with room to spare are slept rather than spun (furi_delay_us busy-waits, and these
// run five times per page); furi_delay_ms overshoots by up to a tick, which still leaves 15 ms of
// margin on the longest of them. The write-data wait keeps spinning - at 33 ms its margin to twsc
// max is only 7 ms, and losing the session there loses the page.
#define HITAGS_CHARGE_US          3000 // also lands the first frame in the mode switch window
#define HITAGS_WAIT_UIDRESP_MS    24
#define HITAGS_WAIT_SELECT_MS     20
#define HITAGS_WAIT_WRITECMD_MS   12
#define HITAGS_WAIT_WRITEDATA_US  32000
// Both are field-off minimums with the tag unpowered, so nothing here is timing-critical and they
// are slept rather than spun - furi_delay_us busy-waits, and these two are most of a write.
#define HITAGS_WRITE_POWERDOWN_MS 20
#define HITAGS_WRITE_RESET_MS     100

static void hitags_tx_blind(const uint8_t* tx, size_t nbits, uint32_t wait_ms) {
    FURI_CRITICAL_ENTER();
    hitags_send_frame(tx, nbits);
    FURI_CRITICAL_EXIT();
    furi_delay_ms(wait_ms);
}

/** One WRITE PAGE transaction: the command frame, then the 32-bit data frame. `data_wait` must
 * clear the answer to the data frame - the real page write adds tprog, the auth page does not. */
static void hitags_tx_write_page(uint8_t page, const uint8_t* data, bool programs) {
    uint8_t tx[HITAGS_FRAME_BYTES] = {0};
    size_t bits = hitags_build_write_page(tx, page);
    hitags_tx_blind(tx, bits, HITAGS_WAIT_WRITECMD_MS);

    memset(tx, 0, sizeof(tx));
    bits = hitags_build_write_data(tx, data);
    if(!programs) {
        hitags_tx_blind(tx, bits, HITAGS_WAIT_WRITECMD_MS);
        return;
    }
    // A real page write adds tprog before the tag listens again, which leaves too little margin
    // to twsc max to round the wait up to whole ticks.
    FURI_CRITICAL_ENTER();
    hitags_send_frame(tx, bits);
    FURI_CRITICAL_EXIT();
    furi_delay_us(HITAGS_WAIT_WRITEDATA_US);
}

/** Power-cycle the tag, select it and write one page. Each page gets its own session because the
 * tag drops the selection after a write, and a blind WRITE without the select does nothing. */
static void hitags_write_one_page(uint8_t page, const uint8_t* pagedata, const uint8_t* uid) {
    uint8_t tx[HITAGS_FRAME_BYTES] = {0};
    size_t bits = hitags_build_uid_request(tx);

    // The field is already off on entry - the UID read and the previous page's session both leave
    // it that way - so this only has to hold it off long enough for the tag to lose its supply. Do
    // NOT call tim_read_stop() here: it asserts that the timer bus is currently running.
    furi_delay_ms(HITAGS_WRITE_RESET_MS);

    // Field-on, the charge and the first frame are one uninterrupted unit, for the same reason
    // hitags_tx_cold is: the charge doubles as the mode switch wait, so a preemption inside it
    // puts the UID REQUEST outside the tag's 2.24..4.16 ms window, where a tag still in TTF
    // ignores it - and the page then silently does not get written.
    // No pin_pull_release here, unlike the blind writers: tim_read_start leaves the pull pin low,
    // which is the reader's undamped RX bias, and damping was measured worse on this link.
    FURI_CRITICAL_ENTER();
    furi_hal_rfid_tim_read_start(125000, 0.5);
    furi_delay_us(HITAGS_CHARGE_US);
    hitags_send_frame(tx, bits);
    FURI_CRITICAL_EXIT();
    furi_delay_ms(HITAGS_WAIT_UIDRESP_MS);

    memset(tx, 0, sizeof(tx));
    bits = hitags_build_select(tx, uid);
    hitags_tx_blind(tx, bits, HITAGS_WAIT_SELECT_MS);

    hitags_tx_write_page(HITAGS_82XX_AUTH_PAGE, hitags_82xx_password, false);
    hitags_tx_write_page(page, pagedata, true);

    furi_hal_rfid_tim_read_stop();
    furi_hal_rfid_pins_reset();
    furi_delay_ms(HITAGS_WRITE_POWERDOWN_MS);
}

// =================================================================================================
// Public API.
// =================================================================================================

bool hitags_read_uid(uint8_t* uid, HitagSAbortCallback abort, void* context) {
    furi_check(uid);
    // One LF session owns the antenna at a time, and the ISR reaches the capture through a static.
    // A second caller would clobber the pointer and leave the first freeing a dangling one.
    furi_check(s_capture == NULL);

    s_capture = malloc(sizeof(HitagSCapture));
    s_abort = abort;
    s_abort_context = context;
    hitags_capture_rebase(); // before the ISR is armed, so no critical section is needed

    // Plain reader configuration: 125 kHz at 50% duty with the antenna undamped, exactly what the
    // lfrfid reader uses, so the comparator sees the envelope it is tuned for. Retuning it was
    // tried and measured worse at every threshold and hysteresis setting.
    furi_hal_rfid_tim_read_start(125000, 0.5);
    furi_hal_rfid_tim_read_capture_start(hitags_capture_callback, NULL);
    for(uint32_t elapsed = 0; elapsed < HITAGS_SETTLE_MS; elapsed += HITAGS_SETTLE_SLICE_MS) {
        if(hitags_aborted()) break;
        furi_delay_ms(HITAGS_SETTLE_SLICE_MS);
    }

    // Resolved into a local, so a discarded sweep cannot leave half a UID in the caller's buffer.
    uint8_t winner[LFRFID_HITAGS_UID_SIZE] = {0};
    bool found = false;
    bool saw_reply = false;
    for(uint8_t attempt = 0; attempt < HITAGS_ATTEMPTS && !found; attempt++) {
        if(hitags_aborted()) break;

        HitagSInterior interior;
        if(!hitags_read_uid_frame(&interior)) {
            // The receiver's own recovery from the command's EOF leaves a couple of edges behind
            // with no tag at all, so the floor is what separates "present" from "dead field".
            if(s_capture->count >= HITAGS_MIN_ZEROS) saw_reply = true;
            continue;
        }
        saw_reply = true;
        found = hitags_resolve_candidate(&interior, winner);
    }

    const bool aborted = hitags_aborted(); // while s_abort is still set, for the log below

    furi_hal_rfid_tim_read_capture_stop();
    furi_hal_rfid_tim_read_stop();
    furi_hal_rfid_pins_reset();

    free(s_capture);
    s_capture = NULL;
    s_abort = NULL;
    s_abort_context = NULL;

    if(found) {
        memcpy(uid, winner, LFRFID_HITAGS_UID_SIZE);
        FURI_LOG_I(TAG, "UID %02X%02X%02X%02X confirmed", uid[0], uid[1], uid[2], uid[3]);
    } else if(!aborted) {
        // The negative path is the one a user reports, and D is compiled out of release builds, so
        // say something at W or the whole interrogation leaves no trace at all. Which of the two
        // it is decides what they should try: move the tag, or check it is the right kind.
        FURI_LOG_W(
            TAG,
            "no UID confirmed in %u attempts (%s)",
            HITAGS_ATTEMPTS,
            saw_reply ? "a tag is modulating but did not answer" : "nothing in the field");
    }
    return found;
}

void hitags_write(const LFRFIDHitagS* data, const uint8_t* uid) {
    furi_check(data);
    furi_check(uid);

    // At I, not D: this overwrites two pages that hold application data on a genuine Hitag S, and
    // release builds compile D out - so without this the one destructive act leaves no trace.
    FURI_LOG_I(TAG, "writing EM pages of %02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);

    // Pages 4 and 5 are the two halves of one EM4100 frame, so they are written as a unit: stopping
    // between them would leave a half-updated, corrupt id on the tag.
    hitags_write_one_page(HITAGS_EM_PAGE0, data->page4, uid);
    hitags_write_one_page(HITAGS_EM_PAGE1, data->page5, uid);
}

// Each vector names itself, so a failure says which one rather than only that something did. A
// crash would take the whole unit-test run with it, and the harness reads output that never comes.
#define HITAGS_SELFTEST_CHECK(what, condition) \
    do {                                       \
        if(!(condition)) return (what);        \
    } while(0)

// One falling edge per '0' cell, which is all this detector yields for a reply.
static void hitags_selftest_capture(const uint8_t* pattern, size_t nbits) {
    hitags_capture_rebase();
    for(size_t i = 0; i < nbits; i++) {
        if(bit_lib_get_bit(pattern, i)) continue;
        s_capture->edge[s_capture->count++] =
            HITAGS_TWRESP_US + i * HITAGS_CELL_US + HITAGS_CELL_US / 2;
    }
}

static const char* hitags_selftest_run(void) {
    // Datasheet 13.3: cmd 0x00 (5 bits) + 2C 68 0D B4 -> CRC 0x9E. Also the only vector that runs
    // the CRC over a length that is not a whole number of bytes.
    uint8_t buf[8] = {0};
    size_t pos = 0;
    hitags_put_bits(buf, &pos, 0x00, 5);
    hitags_put_bits(buf, &pos, 0x2C, 8);
    hitags_put_bits(buf, &pos, 0x68, 8);
    hitags_put_bits(buf, &pos, 0x0D, 8);
    hitags_put_bits(buf, &pos, 0xB4, 8);
    HITAGS_SELFTEST_CHECK("crc8", hitags_crc8(buf, pos) == 0x9E);

    uint8_t tx[8] = {0};
    HITAGS_SELFTEST_CHECK(
        "uid request", hitags_build_uid_request(tx) == 5 && tx[0] == HITAGS_CMD_UID_REQ_ADV);

    // Proxmark3 SELECT trace: UID 95 D3 24 08 -> CRC 0x3E, 45-bit frame 04 AE 99 20 41 F0.
    const uint8_t uid[LFRFID_HITAGS_UID_SIZE] = {0x95, 0xD3, 0x24, 0x08};
    memset(tx, 0, sizeof(tx));
    HITAGS_SELFTEST_CHECK("select length", hitags_build_select(tx, uid) == 45);
    HITAGS_SELFTEST_CHECK(
        "select frame",
        tx[0] == 0x04 && tx[1] == 0xAE && tx[2] == 0x99 && tx[3] == 0x20 && tx[4] == 0x41 &&
            tx[5] == 0xF0);

    // The two frames that actually mutate a tag, and the only place a 4-bit command appears: as a
    // 5-bit one this frame is 21 bits, which the tag ignores in silence on an open-loop write.
    memset(tx, 0, sizeof(tx));
    HITAGS_SELFTEST_CHECK(
        "write page auth",
        hitags_build_write_page(tx, HITAGS_82XX_AUTH_PAGE) == 20 && tx[0] == 0x84 &&
            tx[1] == 0x09 && tx[2] == 0x50);
    memset(tx, 0, sizeof(tx));
    HITAGS_SELFTEST_CHECK(
        "write page data",
        hitags_build_write_page(tx, HITAGS_EM_PAGE0) == 20 && tx[0] == 0x80 && tx[1] == 0x4F &&
            tx[2] == 0x20);

    // Pins the 82xx password itself: a typo in it is as silent as a wrong CRC.
    memset(tx, 0, sizeof(tx));
    HITAGS_SELFTEST_CHECK(
        "write data",
        hitags_build_write_data(tx, hitags_82xx_password) == 40 && tx[0] == 0xBB &&
            tx[1] == 0xDD && tx[2] == 0x33 && tx[3] == 0x99 && tx[4] == 0xEC);

    // AC SEQUENCE packs k pre-shifted so the top 5 bits carry it - the one place two bit
    // conventions meet. A misencoded k just makes every read fail to confirm, on hardware only.
    memset(tx, 0, sizeof(tx));
    HITAGS_SELFTEST_CHECK(
        "ac sequence",
        hitags_build_ac_sequence(tx, uid, HITAGS_AC_SEQ_K) == 29 && tx[0] == 0x84 &&
            tx[1] == 0xAE && tx[2] == 0x98 && tx[3] == 0x98);

    // Synthesise the edge train a reply produces on this hardware - one edge per '0' cell, '1'
    // cells invisible - and require the UID back. Then feed it trains shaped like a tag idling in
    // TTF and require them rejected, because mistaking one for a reply is how a wrong UID gets
    // reported as confirmed.
    const uint8_t reply[] = {0xF2, 0xBA, 0x63, 0x29, 0x80}; // SOF '111' + UID 95D3194C
    hitags_selftest_capture(reply, HITAGS_SOF_BITS + HITAGS_UID_BITS);

    HitagSInterior interior;
    HITAGS_SELFTEST_CHECK(
        "decode", hitags_decode_frame(HITAGS_UID_BITS, HITAGS_MIN_ZEROS, &interior));
    // 95D3194C starts '1001...', so behind the '111' SOF the first '0' is cell 4 and the last 34.
    HITAGS_SELFTEST_CHECK("decode span", interior.span == 31);
    HITAGS_SELFTEST_CHECK("decode alignment", hitags_lead_max(&interior) == 4);
    HITAGS_SELFTEST_CHECK("decode edges", interior.edges == 17 && !interior.truncated);

    // Placed at that alignment the frame must come back bit-for-bit, and placed one cell earlier it
    // must not - otherwise the resolver would have nothing to tell the alignments apart by.
    uint8_t frame[HITAGS_FRAME_BYTES];
    hitags_place(&interior, 4, frame);
    for(size_t i = 0; i < HITAGS_UID_BITS; i++) {
        HITAGS_SELFTEST_CHECK(
            "place",
            bit_lib_get_bit(frame, HITAGS_SOF_BITS + i) ==
                bit_lib_get_bit(reply, HITAGS_SOF_BITS + i));
    }
    hitags_place(&interior, 3, frame);
    bool shifted_differs = false;
    for(size_t i = 0; i < HITAGS_UID_BITS; i++) {
        shifted_differs |= bit_lib_get_bit(frame, HITAGS_SOF_BITS + i) !=
                           bit_lib_get_bit(reply, HITAGS_SOF_BITS + i);
    }
    HITAGS_SELFTEST_CHECK("place shifted", shifted_differs);

    // An empty window is the commonest input of all - no tag in the field - and it must not decode
    // as anything, at either frame length.
    hitags_capture_rebase();
    HITAGS_SELFTEST_CHECK(
        "empty capture", !hitags_decode_frame(HITAGS_UID_BITS, HITAGS_MIN_ZEROS, &interior));
    HITAGS_SELFTEST_CHECK(
        "empty capture tail",
        !hitags_decode_frame(HITAGS_UID_BITS - HITAGS_AC_SEQ_K, HITAGS_VERIFY_MIN_EDGE, &interior));

    // A train that runs longer than a frame is a tag streaming TTF, not a reply.
    hitags_capture_rebase();
    for(size_t i = 0; i < 40; i++) {
        s_capture->edge[s_capture->count++] = HITAGS_TWRESP_US + i * HITAGS_CELL_US;
    }
    HITAGS_SELFTEST_CHECK(
        "ttf run length", !hitags_decode_frame(HITAGS_UID_BITS, HITAGS_MIN_ZEROS, &interior));

    // Manchester: a 768 us gap is 1.5 cells, so it ends the run. Sweeping where the gap falls
    // covers both rejection paths - at bad=4 too few edges were folded in, past that it is the pile
    // of edges left over - and a single position would only ever exercise one of them.
    for(size_t bad = 4; bad <= 8; bad++) {
        hitags_capture_rebase();
        uint32_t when = HITAGS_TWRESP_US;
        for(size_t i = 0; i < 12; i++) {
            s_capture->edge[s_capture->count++] = when;
            when += (i == bad) ? 768 : HITAGS_CELL_US;
        }
        HITAGS_SELFTEST_CHECK(
            "ttf half cell", !hitags_decode_frame(HITAGS_UID_BITS, HITAGS_MIN_ZEROS, &interior));
    }

    // The trust anchor: a cross-check reply must be accepted for the UID that owes it and refused
    // for one sharing the replayed prefix but not the tail. Without the negative half the matcher
    // could accept anything and every guard above it would still pass.
    const uint8_t uid_a[LFRFID_HITAGS_UID_SIZE] = {0x95, 0xD3, 0x19, 0x4C};
    const size_t tail_bits = HITAGS_UID_BITS - HITAGS_AC_SEQ_K;
    uint8_t tail[HITAGS_FRAME_BYTES];
    memset(tail, 0xFF, sizeof(tail)); // SOF '111', then the bits past the prefix
    bit_lib_copy_bits(tail, HITAGS_SOF_BITS, tail_bits, uid_a, HITAGS_AC_SEQ_K);

    hitags_selftest_capture(tail, HITAGS_SOF_BITS + tail_bits);
    HITAGS_SELFTEST_CHECK(
        "tail decode", hitags_decode_frame(tail_bits, HITAGS_VERIFY_MIN_EDGE, &interior));
    HITAGS_SELFTEST_CHECK("tail match", hitags_reply_matches(&interior, uid_a, HITAGS_AC_SEQ_K));
    // uid shares 95D3 with uid_a and differs past it, which is exactly what the tail must catch.
    HITAGS_SELFTEST_CHECK("tail mismatch", !hitags_reply_matches(&interior, uid, HITAGS_AC_SEQ_K));

    return NULL;
}

const char* hitags_selftest(void) {
    furi_check(s_capture == NULL); // a read in flight owns it; see hitags_read_uid
    s_capture = malloc(sizeof(HitagSCapture));

    const char* failure = hitags_selftest_run();

    free(s_capture);
    s_capture = NULL;
    return failure;
}
