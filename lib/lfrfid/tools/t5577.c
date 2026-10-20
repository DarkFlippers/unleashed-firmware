#include "t5577.h"
#include <furi.h>
#include <furi_hal_cortex.h>
#include <furi_hal_rfid.h>
#include <stddef.h>
#include <toolbox/manchester_decoder.h>
#include <bit_lib/bit_lib.h>

#define TAG "T5577"

// Every timing here is in field clocks: one is 8 us at 125 kHz.
#define T5577_TIMING_WAIT_TIME 400
#define T5577_TIMING_PROGRAM   700

// A long leading reference stretches one symbol by this much (Proxmark3 T55_LLR_REF).
#define T5577_LLR_REFERENCE 136

// Longest frame is a 1-of-4 write with a password: 2 lead-in + 2 opcode + 2 password padding
// + 32 password + 1 lock + 32 data + 3 address = 74. Rounded up, with slack.
#define T5577_FRAME_BITS_MAX 80

// How long the envelope detector takes to recover from the gaps a command frame cuts into the
// carrier. The same 450 ms the read worker waits from a cold field
// (LFRFID_WORKER_READ_STABILIZE_TIME_MS), and measured here rather than inherited.
//
// The edge count recovers long before the signal does, so counting edges says this could be
// 150 ms. Decoding says otherwise: at RF/32 nothing usable comes back below 250 ms, and at
// 250 ms the comparator offset is still 100 us against the 54 us it settles to - enough that
// only 26 consecutive bits decode, where 450 ms gives 219. RF/64 is more forgiving, which is
// exactly why the shorter rate sets the number.
#define T5577_READ_SETTLE_MS 450

/**************************************************************************************************/
/********************************************** FIELD *********************************************/
/**************************************************************************************************/

// Nesting depth, so a sequence of frames raises the carrier once instead of dropping it between
// them - which would reset the tag. Not thread safe, and does not need to be: a transaction owns
// the reader for its duration. The count is this module's own bookkeeping, so a caller driving
// furi_hal_rfid directly desynchronises it; t5577_field_reset() is the way back.
static uint8_t t5577_field_depth = 0;

void t5577_field_start(void) {
    furi_check(t5577_field_depth < UINT8_MAX);

    if(t5577_field_depth++ > 0) return;

    furi_hal_rfid_tim_read_start(125000, 0.5);

    // Do not ground the antenna: a gap has to be able to collapse the field.
    furi_hal_rfid_pin_pull_release();
}

void t5577_field_stop(void) {
    // Not fatal: the count is this module's bookkeeping, and a third party can desynchronise
    // it through the exported HAL. Crashing the device over its arithmetic is not warranted.
    if(t5577_field_depth == 0) {
        FURI_LOG_W(TAG, "field stopped more times than started");
        return;
    }

    if(--t5577_field_depth > 0) return;

    furi_hal_rfid_tim_read_stop();
    furi_hal_rfid_pins_reset();
}

/**************************************************************************************************/
/********************************************** SEND **********************************************/
/**************************************************************************************************/

void t5577_field_reset(void) {
    t5577_field_depth = 0;
    furi_hal_rfid_tim_read_stop();
    furi_hal_rfid_pins_reset();
}

static void t5577_write_gap(uint32_t gap_time) {
    furi_hal_rfid_tim_read_pause();
    furi_delay_us(gap_time * 8);
    furi_hal_rfid_tim_read_continue();
}

static const T5577Timing t5577_timing[T5577DownlinkModeMax] = {
    // What every write this driver has sent used.
    [T5577DownlinkFixedBitLength] =
        {.start_gap = 30, .write_gap = 18, .write_0 = 24, .write_1 = 56},
    // Proxmark3 gives its long-leading-reference row the same lengths as its fixed row, so these
    // match deliberately rather than by omission.
    [T5577DownlinkLongLeadingReference] =
        {.start_gap = 30, .write_gap = 18, .write_0 = 24, .write_1 = 56},
    [T5577DownlinkLeadingZero] = {.start_gap = 30, .write_gap = 18, .write_0 = 18, .write_1 = 40},
    [T5577Downlink1Of4] =
        {.start_gap = 30,
         .write_gap = 18,
         .write_0 = 18,
         .write_1 = 34,
         .write_2 = 50,
         .write_3 = 66},
};

const T5577Timing* t5577_timing_default(T5577DownlinkMode mode) {
    furi_check(mode < T5577DownlinkModeMax);
    return &t5577_timing[mode];
}

// Packed, as hitags.c and hitagmicro.c build their frames: a byte per bit cost 81 bytes of
// stack inside the critical section where ten will do.
typedef struct {
    uint8_t bit[(T5577_FRAME_BITS_MAX + 7) / 8];
    uint8_t count;
} T5577Bitstream;

static void t5577_bits_add(T5577Bitstream* bs, uint32_t value, uint8_t count) {
    furi_check((bs->count + count) <= T5577_FRAME_BITS_MAX);

    uint8_t be[4];
    bit_lib_num_to_bytes_be((uint32_t)(value << (32 - count)), sizeof(be), be);
    bit_lib_copy_bits(bs->bit, bs->count, count, be, 0);
    bs->count += count;
}

static void t5577_send_symbol(uint16_t length, uint16_t write_gap) {
    furi_delay_us(length * 8);
    t5577_write_gap(write_gap);
}

static void t5577_build_frame(const T5577Frame* frame, T5577Bitstream* bs) {
    const bool wide = (frame->downlink == T5577Downlink1Of4);

    // Both of these modes are told apart from a fixed-length frame by what precedes the opcode.
    if(frame->downlink == T5577DownlinkLeadingZero) {
        t5577_bits_add(bs, 0, 1);
    } else if(wide) {
        t5577_bits_add(bs, 0, 2); // one 1-of-4 symbol of zero
    }

    if(frame->command == T5577CommandReset) {
        t5577_bits_add(bs, 0, 2);
        return;
    }

    // First opcode bit says "not test mode", second selects the page.
    t5577_bits_add(bs, 1, 1);
    t5577_bits_add(bs, frame->page ? 1 : 0, 1);

    if(frame->with_pass) {
        // Those same two modes need the opcode padded out before a password follows it.
        if(frame->downlink == T5577DownlinkLeadingZero || wide) {
            t5577_bits_add(bs, 0, 2);
        }
        t5577_bits_add(bs, frame->password, 32);
    }

    // A regular read is the opcode alone: it asks the tag to go back to broadcasting its data
    // blocks, so there is nothing to address.
    if(frame->command == T5577CommandRegularRead) return;

    t5577_bits_add(bs, frame->lock ? 1 : 0, 1);

    if(frame->command == T5577CommandWrite) {
        t5577_bits_add(bs, frame->data, 32);
    }

    t5577_bits_add(bs, frame->block, 3);
}

// True for the frames that carry a block address, and so the ones a bad block number can misdirect.
static bool t5577_command_is_addressed(T5577Command command) {
    return (command == T5577CommandRead) || (command == T5577CommandWrite);
}

void t5577_send(const T5577Frame* frame) {
    furi_check(frame);
    furi_check(frame->downlink < T5577DownlinkModeMax);
    furi_check(frame->page <= 1);
    // The address is three bits wide, so an out of range block would not fail - it would quietly
    // address a different one, and on a write that is a corrupted tag.
    furi_check(
        !t5577_command_is_addressed(frame->command) ||
        frame->block < t5577_page_blocks(frame->page));
    furi_check(t5577_field_depth > 0);

    const T5577Timing* timing = (frame->timing != NULL) ? frame->timing :
                                                          t5577_timing_default(frame->downlink);

    const bool wide = (frame->downlink == T5577Downlink1Of4);

    // A zero length is not a short symbol, it is no symbol: the frame goes out malformed and
    // reports nothing. Worth checking on the one struct meant to be filled in by hand.
    furi_check(timing->start_gap && timing->write_gap && timing->write_0 && timing->write_1);
    furi_check(!wide || (timing->write_2 && timing->write_3));

    T5577Bitstream bs = {0};
    t5577_build_frame(frame, &bs);

    // Two bits per symbol only divides a frame that holds an even number of them. Proxmark3
    // drops the odd bit; every frame built above is even, so an odd one is a bug, not a tag.
    furi_check(!wide || (bs.count % 2) == 0);

    FURI_CRITICAL_ENTER();

    // Let the tag charge up before cutting the field, or it answers nothing.
    furi_delay_us(T5577_TIMING_WAIT_TIME * 8);

    t5577_write_gap(timing->start_gap);

    // Indexed by the symbol itself, which is what makes 1-of-4 a change of alphabet rather than
    // a change of encoder. Built once per frame, not once per symbol.
    const uint16_t length[] = {
        timing->write_0,
        timing->write_1,
        timing->write_2,
        timing->write_3,
    };
    const uint8_t step = wide ? 2 : 1;

    if(frame->downlink == T5577DownlinkLongLeadingReference) {
        t5577_send_symbol(timing->write_0 + T5577_LLR_REFERENCE, timing->write_gap);
    }

    for(uint8_t i = 0; i < bs.count; i += step) {
        const uint8_t symbol = wide ? bit_lib_get_bits(bs.bit, i, 2) : bit_lib_get_bit(bs.bit, i);
        t5577_send_symbol(length[symbol], timing->write_gap);
    }

    // The tag burns the word into its EEPROM here, and needs the field all the way through.
    if(frame->command == T5577CommandWrite) {
        furi_delay_us(T5577_TIMING_PROGRAM * 8);
    }

    FURI_CRITICAL_EXIT();
}

// Putting the tag back into regular read mode. Note this is not the chip's reset opcode: a tag
// that has been asked for one block keeps repeating it, and what stops that is being told to
// broadcast its data blocks again.
static void t5577_back_to_read_mode(T5577DownlinkMode downlink, const T5577Timing* timing) {
    t5577_send(&(T5577Frame){
        .command = T5577CommandRegularRead,
        .downlink = downlink,
        .timing = timing,
    });
}

static void t5577_write_block_pass(
    uint8_t page,
    uint8_t block,
    bool lock_bit,
    uint32_t data,
    bool with_pass,
    uint32_t password) {
    t5577_send(&(T5577Frame){
        .command = T5577CommandWrite,
        .page = page,
        .block = block,
        .lock = lock_bit,
        .data = data,
        .with_pass = with_pass,
        .password = password,
    });

    t5577_back_to_read_mode(T5577DownlinkFixedBitLength, NULL);
}

/**************************************************************************************************/
/********************************************** READ **********************************************/
/**************************************************************************************************/

// Pairs to collect before decoding. One per envelope period, and a pair is worth about one
// decoded bit, so this covers the 64 bits every alignment of a 32 bit answer needs several times
// over: 199 pairs measured in a 60 ms window at RF/32 decoded to 219 bits.
#define T5577_READ_PAIRS_MAX 256

// Consecutive decoded bits that count as "a tag is talking", for telling a wrong answer from
// no answer at all. Consecutive and decoded, not a tally of plausible runs: a raw count
// reaches any threshold on comparator chatter alone, and a quarter of a word is little
// enough to be a coincidence, so this is a full repetition.
#define T5577_READ_ANSWER_BITS 32

// Long enough for the buffer to fill at the slowest rate: 256 pairs at RF/64 is ~131 ms.
#define T5577_READ_TIMEOUT_MS 150

// Give up earlier when nothing has arrived at all. A password sweep pays this on every wrong
// password, where waiting out the whole window costs tens of seconds.
#define T5577_READ_SILENCE_MS 30

// Tail of the settle delay to spin out rather than sleep through. Long enough to absorb the
// scheduler handing the thread back late, short enough not to starve anything that matters.
#define T5577_READ_SETTLE_SPIN_MS 3

// Below this a run is noise, not envelope. Same floor the read worker uses.
#define T5577_READ_MIN_TIME_US 16

// Runs are classified against the bit period with the same +-20% the EM4100 decoder uses (100 us
// on a 512 us bit), which is what makes those timings proven rather than guessed.
#define T5577_READ_JITTER_DIV 5

// The envelope as the comparator reported it: one high run and the low run that followed it,
// plus the bits decoded out of them.
//
// The interrupt only stores. Everything that has to see the signal as a whole - the comparator
// offset above all - happens afterwards, where it can look at every pair at once.
//
// The decoded bits live here rather than in their own buffer because a verify needs both at
// once and this is already the one allocation a read makes.
typedef struct {
    uint16_t high[T5577_READ_PAIRS_MAX];
    uint16_t low[T5577_READ_PAIRS_MAX];
    uint32_t time[T5577_READ_PAIRS_MAX];
    uint32_t elapsed;
    uint32_t pulse;
    bool pulse_valid;
    volatile uint16_t count;
    uint8_t bits[LFRFID_T5577_READ_BITS_BYTES]; // decoded, not captured
} T5577Capture;

static void t5577_read_capture(bool level, uint32_t duration, void* context) {
    T5577Capture* capture = context;

    // A high run arrives with the level set; the whole rise-to-rise period arrives with it clear,
    // so the low run is what is left of the period. This is the pairing the read worker does with
    // its varint pairs before feeding its decoders.
    if(level) {
        capture->pulse = duration;
        capture->pulse_valid = true;
        return;
    }

    // Timestamp the rising edge this period started at, before anything below can return early.
    // The capture counter is zeroed when the window opens and every period is reported against
    // the one before it, so the running sum is an absolute time - but only as long as the sum is
    // kept for the pairs that get thrown away too.
    const uint32_t rise = capture->elapsed;
    capture->elapsed += duration;

    if(capture->count >= T5577_READ_PAIRS_MAX) return;

    // A period no longer than its own high run means the pairing is torn, and the stored run must
    // not be subtracted from the next period either.
    if(!capture->pulse_valid || duration <= capture->pulse) {
        capture->pulse_valid = false;
        return;
    }

    const uint32_t high = capture->pulse;
    const uint32_t low = duration - capture->pulse;
    capture->pulse_valid = false;

    if(high > UINT16_MAX || low > UINT16_MAX) return;

    capture->high[capture->count] = (uint16_t)high;
    capture->low[capture->count] = (uint16_t)low;
    capture->time[capture->count] = rise;
    capture->count++;
}

// A pair worth averaging and decoding. Runs longer than two bit periods are the detector settling
// or a glitch, and letting them into the average would move it further than the offset being
// measured.
static bool t5577_pair_plausible(uint16_t high, uint16_t low, uint32_t bit_us) {
    const uint32_t limit = bit_us * 2;
    return (high > T5577_READ_MIN_TIME_US) && (low > T5577_READ_MIN_TIME_US) &&
           ((uint32_t)high < limit) && ((uint32_t)low < limit);
}

// What the comparator adds to every high run and takes off every low one.
//
// It reports a high run longer than it is, by a margin set by the detector and the coupling rather
// than by the bit rate: measured 52 us against a 128 us half-bit at RF/32, 68 us against 256 us at
// RF/64, and 141 us on a capture armed before the detector had settled. A Manchester answer spends
// equal time high and low, so whatever is left after averaging both is the comparator and not the
// tag.
//
// Subtracting it is what makes the short rates decode at all. At RF/32 a 52 us offset puts the
// two runs at 180 and 76 us, and the windows either side of a 128 us half-bit accept neither:
// uncorrected, that capture recovered 4 consecutive bits, and corrected it recovered 219.
static int32_t t5577_capture_bias(const T5577Capture* capture, uint32_t bit_us) {
    int32_t high_sum = 0;
    int32_t low_sum = 0;
    uint16_t used = 0;

    for(uint16_t i = 0; i < capture->count; i++) {
        if(!t5577_pair_plausible(capture->high[i], capture->low[i], bit_us)) continue;
        high_sum += capture->high[i];
        low_sum += capture->low[i];
        used++;
    }

    if(used == 0) return 0;
    return (high_sum - low_sum) / (2 * (int32_t)used);
}

// Walk the capture and decode it, leaving the longest unbroken run of bits at offset 0 of `out`
// and returning its length. Breaks matter: a 32 bit window that straddles one is bits from two
// different moments, so only the longest run is kept rather than everything that decoded.
static size_t t5577_capture_bits(
    const T5577Capture* capture,
    uint32_t bit_us,
    uint8_t* out,
    size_t max_bits,
    T5577ReadMark* mark) {
    const int32_t bias = t5577_capture_bias(capture, bit_us);
    const uint32_t jitter = bit_us / T5577_READ_JITTER_DIV;
    const int32_t short_min = (int32_t)(bit_us / 2 - jitter);
    const int32_t short_max = (int32_t)(bit_us / 2 + jitter);
    const int32_t long_min = (int32_t)(bit_us - jitter);
    const int32_t long_max = (int32_t)(bit_us + jitter);

    ManchesterState state = ManchesterStateMid1;
    size_t best_start = 0;
    size_t best_len = 0;
    size_t run_start = 0;
    size_t len = 0;
    uint32_t run_first_us = 0;
    uint32_t bit_us_last = 0;
    uint32_t next_us = 0;
    T5577ReadMark best_mark = {0};

    // One past the end closes the last run, so a run is finished in one place rather than two.
    for(uint16_t i = 0; i <= capture->count; i++) {
        bool broken = (i == capture->count) ||
                      !t5577_pair_plausible(capture->high[i], capture->low[i], bit_us);

        if(!broken) {
            const int32_t corrected[] = {
                (int32_t)capture->high[i] - bias,
                (int32_t)capture->low[i] + bias,
            };

            // When each half ended. A rising edge is reported where it happened and a falling one
            // late by the comparator offset, which is the same asymmetry the run lengths above
            // are corrected for.
            const uint32_t edge_us[] = {
                (uint32_t)((int32_t)(capture->time[i] + capture->high[i]) - bias),
                capture->time[i] + capture->high[i] + capture->low[i],
            };

            for(uint8_t half = 0; half < 2 && !broken; half++) {
                const bool from_high_run = (half == 0);
                const int32_t value = corrected[half];
                ManchesterEvent event;

                // Same convention as the protocol decoders: a high run counts as a Low event, so
                // the sense reads inverted here.
                if(value > short_min && value < short_max) {
                    event = from_high_run ? ManchesterEventShortLow : ManchesterEventShortHigh;
                } else if(value > long_min && value < long_max) {
                    event = from_high_run ? ManchesterEventLongLow : ManchesterEventLongHigh;
                } else {
                    broken = true;
                    break;
                }

                bool data;
                if(!manchester_advance(state, event, &state, &data)) continue;

                // Every bit of an answer arrives one bit period after the one before it, so a bit
                // that does not is the decoder having gained or lost one. Ending the run there is
                // what makes the run's own clock trustworthy: a dropped bit leaves everything
                // after it still agreeing with itself while sitting a bit away from where the
                // time says it is, and no amount of checking the bits against each other sees
                // that. It showed up as a word read two bits rotated, once in four reads.
                if(len > run_start) {
                    const int32_t drift = (int32_t)edge_us[half] - (int32_t)next_us;
                    if(drift > (int32_t)(bit_us / 4) || drift < -(int32_t)(bit_us / 4)) {
                        broken = true;
                        break;
                    }
                }

                if(len < max_bits) {
                    if(len == run_start) run_first_us = edge_us[half];
                    bit_lib_set_bit(out, len, data);
                    bit_us_last = edge_us[half];
                    next_us = edge_us[half] + bit_us;
                    len++;
                }
            }
        }

        if(!broken) continue;

        // End of a run: keep it if it beats the best so far, then start counting again.
        if(len - run_start > best_len) {
            best_start = run_start;
            best_len = len - run_start;
            best_mark.first_us = run_first_us;
            best_mark.last_us = bit_us_last;
        }
        run_start = len;
    }

    if(best_start > 0 && best_len > 0) {
        // Shift the winner down to offset 0 so a caller can index from there.
        for(size_t i = 0; i < best_len; i++) {
            bit_lib_set_bit(out, i, bit_lib_get_bit(out, best_start + i));
        }
    }

    if(mark) *mark = best_mark;

    return best_len;
}

T5577ReadClock t5577_read_clock_from_config(uint32_t block0) {
    // Only ASK/Manchester, and only at a rate the decoder is proven at. Direct, biphase and
    // diphase are ASK too and are excluded with it: none of them puts a transition mid-bit for
    // the decoder to lock onto. PSK and FSK are a different modulation entirely.
    const uint32_t modulation = t5577_config_modulation(block0);
    if(modulation != LFRFID_T5577_MODULATION_MANCHESTER) {
        FURI_LOG_W(TAG, "Modulation %08lX is not ASK/Manchester", modulation);
        return T5577ReadClockUnsupported;
    }

    const uint32_t bitrate = t5577_config_bitrate(block0);
    switch(bitrate) {
    case LFRFID_T5577_BITRATE_RF_16:
        return T5577ReadClock16;
    case LFRFID_T5577_BITRATE_RF_32:
        return T5577ReadClock32;
    case LFRFID_T5577_BITRATE_RF_64:
        return T5577ReadClock64;
    default:
        FURI_LOG_W(TAG, "Bit rate %08lX is not RF/16, 32 or 64", bitrate);
        return T5577ReadClockUnsupported;
    }
}

// The clocks the decoder is proven at, which is what both a reject and an assert below mean
// by "supported": adding a rate means editing this and nothing else.
static bool t5577_clock_supported(T5577ReadClock clock) {
    return clock == T5577ReadClock16 || clock == T5577ReadClock32 || clock == T5577ReadClock64;
}

// Address the block and collect whatever the tag says back. Shared by the verifier and the
// answer probe, which differ only in what they do with the capture afterwards.
static bool t5577_block_capture(const T5577ReadRequest* request, T5577Capture* capture) {
    if(request->clock == T5577ReadClockUnsupported) return false;
    furi_check(t5577_clock_supported(request->clock));

    t5577_field_start();

    t5577_send(&(T5577Frame){
        .command = T5577CommandRead,
        .downlink = request->downlink,
        .timing = request->timing,
        .page = request->page,
        .block = request->block,
        .with_pass = request->with_pass,
        .password = request->password,
    });

    // The frame left the pull pin where a write wants it; the read path drives it the other way
    // (furi_hal_rfid_pins_read), and this is the read path now.
    furi_hal_rfid_pin_pull_pulldown();

    // Field stays up: this is the detector recovering from the frame gaps, and arming the capture
    // before it has is why a tag that was answering perfectly read as silence. Measured at RF/32:
    // nothing usable below 250 ms, and at 250 ms the offset is still twice what it settles to.
    //
    // To the microsecond rather than furi_delay_ms, because the capture counter starts when this
    // ends: every time in T5577ReadMark is measured from here, and a tick of slop is two bits of
    // phase at RF/64. Most of it is still slept through so the rest of the system keeps running,
    // and only the tail is spun out.
    const uint32_t settle_ms = (request->settle_ms != 0) ? request->settle_ms :
                                                           T5577_READ_SETTLE_MS;
    const FuriHalCortexTimer settle = furi_hal_cortex_timer_get(settle_ms * 1000);
    if(settle_ms > T5577_READ_SETTLE_SPIN_MS) {
        furi_delay_ms(settle_ms - T5577_READ_SETTLE_SPIN_MS);
    }
    furi_hal_cortex_timer_wait(settle);

    furi_hal_rfid_tim_read_capture_start(t5577_read_capture, capture);

    const uint32_t start_tick = furi_get_tick();
    const uint32_t silence = furi_ms_to_ticks(T5577_READ_SILENCE_MS);
    const uint32_t timeout = furi_ms_to_ticks(T5577_READ_TIMEOUT_MS);

    while(capture->count < T5577_READ_PAIRS_MAX) {
        const uint32_t elapsed = furi_get_tick() - start_tick;
        if(elapsed >= timeout) break;
        if(elapsed >= silence && capture->count == 0) break;
        furi_delay_ms(1);
    }

    furi_hal_rfid_tim_read_capture_stop();

    furi_hal_rfid_pin_pull_release();
    t5577_back_to_read_mode(request->downlink, request->timing);

    t5577_field_stop();

    return true;
}

// Covers every value, so adding one is a -Wswitch warning rather than a silent "no answer".
static const char* t5577_block_result_name(T5577BlockResult result) {
    switch(result) {
    case T5577BlockHeld:
        return "held";
    case T5577BlockMismatch:
        return "mismatch";
    case T5577BlockNoAnswer:
        return "no answer";
    case T5577BlockCannotVerify:
        return "cannot verify";
    }
    return "?";
}

size_t t5577_block_read_bits(
    const T5577ReadRequest* request,
    uint8_t* bits,
    size_t max_bits,
    T5577ReadMark* mark) {
    furi_check(request);
    furi_check(bits);
    furi_check(request->page <= 1);
    furi_check(request->block < t5577_page_blocks(request->page));

    if(max_bits > LFRFID_T5577_READ_BITS_MAX) max_bits = LFRFID_T5577_READ_BITS_MAX;

    T5577Capture* capture = malloc(sizeof(T5577Capture));

    size_t decoded = 0;
    if(t5577_block_capture(request, capture)) {
        const uint32_t bit_us = t5577_bit_us(request->clock);
        decoded = t5577_capture_bits(capture, bit_us, bits, max_bits, mark);

        FURI_LOG_I(
            TAG,
            "p%u b%u rf/%u pass:%08lX -> %u bits from %u pairs",
            request->page,
            request->block,
            request->clock,
            request->with_pass ? request->password : 0,
            (unsigned)decoded,
            capture->count);
    }

    free(capture);

    return decoded;
}

// The checks a word has to pass to be a configuration block read at this modulation and rate,
// after Proxmark3 t55xx_block0_plausible. The last two are the whole mechanism: a word that says
// it is ASK/Manchester RF/64 and decoded cleanly as ASK/Manchester RF/64 is almost certainly the
// real configuration, and nothing in a data block has reason to agree.
// A word and the word one block later, which have to agree: the answer repeats every 32 bits,
// so a window that decoded wrong anywhere stops matching itself.
static bool t5577_word_repeats(const uint8_t* bits, size_t offset, uint32_t* word) {
    const uint32_t first = bit_lib_get_bits_32(bits, offset, LFRFID_T5577_WORD_BITS);
    if(first !=
       bit_lib_get_bits_32(bits, offset + LFRFID_T5577_WORD_BITS, LFRFID_T5577_WORD_BITS)) {
        return false;
    }

    *word = first;
    return true;
}

static bool t5577_block0_plausible(uint32_t block0, uint32_t bitrate) {
    if(((block0 >> 24) & 0x0F) != 0) return false; // reserved nibble, zero on every chip

    // Master key: 0x6 and 0x9 are the ATA5577 revisions, 0x0 the T55x7 that has no key there.
    const uint8_t master = (block0 >> 28) & 0x0F;
    if(master != 0x0 && master != 0x6 && master != 0x9) return false;

    if(t5577_config_modulation(block0) != LFRFID_T5577_MODULATION_MANCHESTER) return false;
    if(t5577_config_bitrate(block0) != bitrate) return false;

    return true;
}

// Where in a word a time falls. Alignment is carried as a time rather than as a bit index
// because the two are not equivalent: quantising each read to its own bit grid puts a boundary
// right where the measurement jitters, and a wobble of a few tens of microseconds then lands a
// whole half-bit out. Subtracting two times leaves the jitter as jitter.
static uint32_t t5577_word_us(uint32_t us, uint32_t bit_us) {
    return us % (LFRFID_T5577_WORD_BITS * bit_us);
}

static bool t5577_detect_access(T5577Config* config, bool with_pass, uint32_t password) {
    furi_check(config);

    // Slowest first: it is both the common case and the rate with the most margin, so a tag that
    // would decode at more than one is read at the one least likely to be a coincidence.
    static const struct {
        T5577ReadClock clock;
        uint32_t bitrate;
    } rates[] = {
        {T5577ReadClock64, LFRFID_T5577_BITRATE_RF_64},
        {T5577ReadClock32, LFRFID_T5577_BITRATE_RF_32},
        {T5577ReadClock16, LFRFID_T5577_BITRATE_RF_16},
    };

    uint8_t bits[LFRFID_T5577_READ_BITS_BYTES];
    bool found = false;

    for(size_t rate = 0; rate < COUNT_OF(rates) && !found; rate++) {
        const T5577ReadRequest request = {
            .clock = rates[rate].clock,
            .with_pass = with_pass,
            .password = password,
        };

        T5577ReadMark mark = {0};
        const size_t count =
            t5577_block_read_bits(&request, bits, LFRFID_T5577_READ_BITS_MAX, &mark);
        const uint32_t bit_us = t5577_bit_us(rates[rate].clock);

        // Both polarities count, because nothing in the stream says which way round it is.
        for(size_t offset = 0; !found && offset + 2 * LFRFID_T5577_WORD_BITS <= count; offset++) {
            uint32_t window = 0;
            if(!t5577_word_repeats(bits, offset, &window)) continue;

            for(uint8_t inverted = 0; inverted < 2 && !found; inverted++) {
                const uint32_t candidate = inverted ? ~window : window;
                if(!t5577_block0_plausible(candidate, rates[rate].bitrate)) continue;

                found = true;
                *config = (T5577Config){
                    .block0 = candidate,
                    .clock = rates[rate].clock,
                    .word_us = t5577_word_us(mark.first_us + offset * bit_us, bit_us),
                    .inverted = (inverted != 0),
                    .password = password,
                    .with_pass = with_pass,
                };
            }
        }

        FURI_LOG_I(
            TAG,
            "detect rf/%u: %u bits, first %lu us -> %s",
            rates[rate].clock,
            (unsigned)count,
            mark.first_us,
            found ? "configuration" : "nothing");
    }

    return found;
}

// Take the alignment again from a second read, now that the field has been up long enough for
// the detector to have stopped moving. The first read of a session is the one with the envelope
// still drifting, and it is the read the alignment would otherwise be measured on.
//
// Finding the same configuration a second time is also the only check there is that the
// alignment is reusable at all, so a failure here is left to the caller as a failure to detect.
static bool t5577_detect_realign(T5577Config* config) {
    const uint32_t bit_us = t5577_bit_us(config->clock);
    const T5577ReadRequest request = {
        .clock = config->clock,
        .with_pass = config->with_pass,
        .password = config->password,
    };

    uint8_t bits[LFRFID_T5577_READ_BITS_BYTES];
    T5577ReadMark mark = {0};
    const size_t count = t5577_block_read_bits(&request, bits, LFRFID_T5577_READ_BITS_MAX, &mark);

    bool aligned = false;
    for(size_t offset = 0; !aligned && offset + 2 * LFRFID_T5577_WORD_BITS <= count; offset++) {
        uint32_t word = 0;
        if(!t5577_word_repeats(bits, offset, &word)) continue;
        if(config->inverted) word = ~word;
        if(word != config->block0) continue;

        config->word_us = t5577_word_us(mark.first_us + offset * bit_us, bit_us);
        aligned = true;
    }

    FURI_LOG_I(TAG, "realign: %s, word at %lu us", aligned ? "ok" : "failed", config->word_us);

    return aligned;
}

// A window that opened a bit early returns the word rotated right by that many bits, because
// the answer repeats with no marker: the bits before a word are the end of the one before it.
static uint32_t t5577_rotate_right(uint32_t word, uint8_t bits) {
    if(bits == 0) return word; // shifting by the word width is undefined, so say it outright
    return (word >> bits) | (word << (LFRFID_T5577_WORD_BITS - bits));
}

// How far `word` is rotated from `expected`, if it is a rotation of it at all.
static bool t5577_rotation_of(uint32_t expected, uint32_t word, uint8_t* rotated) {
    for(uint8_t bits = 1; bits < LFRFID_T5577_WORD_BITS; bits++) {
        if(t5577_rotate_right(expected, bits) != word) continue;
        *rotated = bits;
        return true;
    }

    return false;
}

// Reads of block 0 before giving up on the alignment: one to find the error, one to prove the
// correction, and one spare. A tag still disagreeing after two corrections is not answering
// steadily, and the last read is judged rather than corrected - a correction with no read left
// to confirm it is a guess, and would be thrown away with the config anyway.
#define T5577_CONFIRM_TRIES 3

// Confirm the alignment the way a caller will use it, which is not the way it was found.
//
// t5577_detect_realign() locates block 0 by content - it knows the word it is looking for.
// Every read after it instead trusts word_us, and the two disagree on real tags about a third
// of the time: word_us comes out one bit early, and then every block reads one bit rotated.
// Nothing in a data block shows that, since every rotation of a repeating word is also in the
// stream - block 0 is the only block whose value is known independently, so it is the only
// place the error is visible.
//
// So read it by address, and if what comes back is a rotation of it, move the alignment by that
// many bits and read again. What is left after that is a bit grid that may itself move, which
// this does not address: it corrects only the time the grid is measured against.
//
// It cannot turn a bad alignment into a good dump. A correction is only ever accepted by a
// further read of block 0 matching exactly, so a wrong move fails the detect rather than
// rotating everything after it.
static bool t5577_detect_confirm(T5577Config* config) {
    const uint32_t bit_us = t5577_bit_us(config->clock);

    for(uint8_t attempt = 0; attempt < T5577_CONFIRM_TRIES; attempt++) {
        uint32_t word = 0;
        if(!t5577_block_value(config, 0, 0, &word)) return false;
        if(word == config->block0) return true;

        // Not a rotation of block 0 at all: this is not an alignment problem, and moving the
        // alignment on a guess would be worse than saying the tag was not read. A second tag in
        // the field reads like this, which is worth telling apart from a tag that went away.
        uint8_t rotated = 0;
        if(!t5577_rotation_of(config->block0, word, &rotated)) {
            FURI_LOG_W(TAG, "block 0 read as %08lX, which is not block 0", word);
            return false;
        }

        if(attempt + 1 == T5577_CONFIRM_TRIES) break;

        config->word_us = t5577_word_us(config->word_us + rotated * bit_us, bit_us);
        FURI_LOG_I(
            TAG, "alignment was %u bits early, word now at %lu us", rotated, config->word_us);
    }

    FURI_LOG_W(TAG, "block 0 does not read back where the alignment says it is");

    return false;
}

// A password the tag never asked for, dropped as soon as the tag's own answer says so.
//
// This runs before the confirm, not after the whole detect: every passworded read is another
// password frame, and a password frame to a chip that is not protected is a write of block 0.
// The configuration is in hand the moment the access read returns, so there is no reason to
// send even one more.
static void t5577_detect_drop_unwanted_pass(T5577Config* config) {
    if(!config->with_pass) return;
    if(t5577_config_needs_password(config->block0)) return;

    FURI_LOG_I(TAG, "tag asks for no password, dropping the one it was read with");
    config->with_pass = false;
    config->password = 0;
}

// t5577_detect_access() fills the config in as soon as it has a candidate, and any stage after
// it can still fail - a tag lifted mid-detect is exactly that, and so is a block 0 that will not
// read back where the alignment says it is. Clearing the config is what keeps "it returned false"
// and "the config holds nothing" the same answer, since callers hold on to the config and ask it
// later.
static bool t5577_detect_both(T5577Config* config, bool with_pass, uint32_t password) {
    if(t5577_detect_access(config, with_pass, password) && t5577_detect_realign(config)) {
        t5577_detect_drop_unwanted_pass(config);
        if(t5577_detect_confirm(config)) return true;
    }

    *config = (T5577Config){0};
    return false;
}

bool t5577_detect(T5577Config* config) {
    return t5577_detect_both(config, false, 0);
}

bool t5577_detect_with_pass(T5577Config* config, uint32_t password) {
    return t5577_detect_both(config, true, password);
}

// Reads of one block before giving up on it. A read that locked onto the boundary transitions
// instead of the mid-bit ones decodes against a grid half a bit out, and nothing inside that read
// says so - but the clock it is measured against does, and reading again is cheap next to
// handing back a word that is wrong.
#define T5577_VALUE_TRIES 3

bool t5577_block_value(const T5577Config* config, uint8_t page, uint8_t block, uint32_t* value) {
    furi_check(config);
    furi_check(value);

    const uint32_t bit_us = t5577_bit_us(config->clock);
    const T5577ReadRequest request = {
        .clock = config->clock,
        .page = page,
        .block = block,
        .with_pass = config->with_pass,
        .password = config->password,
    };

    uint8_t bits[LFRFID_T5577_READ_BITS_BYTES];
    bool found = false;

    for(uint8_t attempt = 0; attempt < T5577_VALUE_TRIES && !found; attempt++) {
        T5577ReadMark mark = {0};
        const size_t count =
            t5577_block_read_bits(&request, bits, LFRFID_T5577_READ_BITS_MAX, &mark);

        // How far this read's first bit is from the start of a word, by the clock both reads
        // share. Rounding the gap to whole bits is what leaves the slip: the answer's own
        // transitions land a whole number of bits apart, so a gap that is not a multiple of one
        // means this read locked onto the bit boundaries rather than the mid-bits, half a bit out
        // from where the configuration was found.
        const uint32_t period = LFRFID_T5577_WORD_BITS * bit_us;
        const uint32_t gap =
            (config->word_us + period - t5577_word_us(mark.first_us, bit_us)) % period;
        const uint32_t bits_ahead = (gap + bit_us / 2) / bit_us;
        const int32_t slip = (int32_t)gap - (int32_t)(bits_ahead * bit_us);
        const size_t offset = bits_ahead % LFRFID_T5577_WORD_BITS;
        const bool half_locked = (slip > (int32_t)(bit_us / 4)) || (slip < -(int32_t)(bit_us / 4));

        if((offset + 2 * LFRFID_T5577_WORD_BITS) > count) continue;

        uint32_t word = 0;
        if(!t5577_word_repeats(bits, offset, &word)) continue;

        if(half_locked) {
            // Locked onto the boundaries. Mixed data gives the decoder nothing to lock onto
            // there - between two unequal bits the envelope does not change at the boundary at
            // all, so the run breaks - and a word whose every bit is the same is the one case
            // that survives: a square wave reads as all zeroes or all ones depending on which
            // half of it you call the bit. Such a word has no alignment left to get wrong, so
            // the one thing to undo is which half this read picked.
            //
            // This is the whole of why an erased block is readable. Nothing else distinguishes
            // 00000000 from FFFFFFFF, because the tag sends the same waveform for both.
            if(word != 0 && word != UINT32_MAX) continue;
            word = ~word;
        }

        *value = config->inverted ? ~word : word;
        found = true;
    }

    FURI_LOG_I(
        TAG,
        "p%u b%u rf/%u -> %s %08lX",
        page,
        block,
        config->clock,
        found ? "value" : "no value",
        found ? *value : 0);

    return found;
}

T5577BlockResult t5577_block_holds(const T5577ReadRequest* request, uint32_t expected) {
    furi_check(request);
    furi_check(request->page <= 1);
    furi_check(request->block < t5577_page_blocks(request->page));
    // Two ways to have nothing to say, neither of them the tag's fault: a word that is its
    // own complement cannot be told from its inverse, because polarity is unknowable here, and a
    // clock this decoder cannot follow was never going to produce bits. Both are verdicts, not
    // crashes - `expected` is data a caller may well have got from a tag, and verifying that a
    // wiped data block reads back as zero is a reasonable thing to ask for.
    if(expected == 0 || expected == UINT32_MAX) {
        FURI_LOG_W(TAG, "%08lX cannot be told from its own inverse", expected);
        return T5577BlockCannotVerify;
    }

    if(!t5577_clock_supported(request->clock)) {
        FURI_LOG_I(
            TAG, "p%u b%u not read: no clock for that config", request->page, request->block);
        return T5577BlockCannotVerify;
    }

    T5577Capture* capture = malloc(sizeof(T5577Capture));

    if(!t5577_block_capture(request, capture)) {
        free(capture);
        return T5577BlockCannotVerify;
    }

    const uint32_t bit_us = t5577_bit_us(request->clock);
    const size_t bits =
        t5577_capture_bits(capture, bit_us, capture->bits, LFRFID_T5577_READ_BITS_MAX, NULL);

    // The answer repeats every 32 bits, so sliding one bit at a time covers every alignment, and
    // a wrong one can only delay a match rather than invent one. Which polarity the detector
    // settled on says nothing about what the tag holds and there is no frame marker to resolve
    // it, so both readings count.
    bool found = false;
    for(size_t offset = 0; !found && offset + LFRFID_T5577_WORD_BITS <= bits; offset++) {
        const uint32_t window = bit_lib_get_bits_32(capture->bits, offset, LFRFID_T5577_WORD_BITS);
        found = (window == expected) || (~window == expected);
    }

    T5577BlockResult result = T5577BlockNoAnswer;
    if(found) {
        result = T5577BlockHeld;
    } else if(bits >= T5577_READ_ANSWER_BITS) {
        result = T5577BlockMismatch;
    }

    // The only diagnostic there is behind a verdict the user sees as one word.
    FURI_LOG_I(
        TAG,
        "p%u b%u rf/%u pass:%08lX expect:%08lX -> %s, %u pairs, %u bits",
        request->page,
        request->block,
        request->clock,
        request->with_pass ? request->password : 0,
        expected,
        t5577_block_result_name(result),
        capture->count,
        (unsigned)bits);

    free(capture);

    return result;
}

/**************************************************************************************************/
/********************************************** WRITE *********************************************/
/**************************************************************************************************/

// Every write entry point is this loop: the masked blocks of one page, each in its own frame,
// under one field. Interrupts are held per frame rather than across the sequence - eight blocks
// is hundreds of milliseconds, and every frame begins by charging the tag anyway, so a pause
// between them costs nothing.
static void t5577_write_blocks(
    LFRFIDT5577* data,
    uint8_t page,
    uint8_t mask,
    bool with_pass,
    uint32_t password) {
    const size_t blocks = t5577_page_blocks(page);

    t5577_field_start();

    for(size_t i = 0; i < blocks; i++) {
        if(!(mask & (1u << i))) continue;
        t5577_write_block_pass(page, i, false, data->block[i], with_pass, password);
    }

    t5577_back_to_read_mode(T5577DownlinkFixedBitLength, NULL);
    t5577_field_stop();
}

// blocks_to_write counts from block 0, so the mask is that many low bits.
static uint8_t t5577_mask_from_count(uint32_t blocks_to_write) {
    if(blocks_to_write >= LFRFID_T5577_BLOCKS_IN_PAGE_0) return 0xFF;
    return (uint8_t)((1u << blocks_to_write) - 1);
}

void t5577_write(LFRFIDT5577* data) {
    t5577_write_blocks(data, 0, t5577_mask_from_count(data->blocks_to_write), false, 0);
}

void t5577_write_with_pass(LFRFIDT5577* data, uint32_t password) {
    t5577_write_blocks(data, 0, t5577_mask_from_count(data->blocks_to_write), true, password);
}

void t5577_write_with_mask(LFRFIDT5577* data, uint8_t page, bool with_pass, uint32_t password) {
    t5577_write_blocks(data, page, data->mask, with_pass, password);
}
