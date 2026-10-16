#include "t5577.h"
#include <furi.h>
#include <furi_hal_rfid.h>
#include <stddef.h>
#include <toolbox/manchester_decoder.h>

#define T5577_TIMING_WAIT_TIME 400
#define T5577_TIMING_START_GAP 30
#define T5577_TIMING_WRITE_GAP 18
#define T5577_TIMING_DATA_0    24
#define T5577_TIMING_DATA_1    56
#define T5577_TIMING_PROGRAM   700

#define T5577_OPCODE_PAGE_0 0b10
#define T5577_OPCODE_PAGE_1 0b11
#define T5577_OPCODE_RESET  0b00

static void t5577_start(void) {
    furi_hal_rfid_tim_read_start(125000, 0.5);

    // do not ground the antenna
    furi_hal_rfid_pin_pull_release();
}

static void t5577_stop(void) {
    furi_hal_rfid_tim_read_stop();
    furi_hal_rfid_pins_reset();
}

static void t5577_write_gap(uint32_t gap_time) {
    furi_hal_rfid_tim_read_pause();
    furi_delay_us(gap_time * 8);
    furi_hal_rfid_tim_read_continue();
}

// Opcode second bit selects the page; the first says "not test mode".
#define T5577_OPCODE_BITS 2

// A long leading reference is one symbol stretched by this many field clocks.
#define T5577_LLR_REFERENCE 136

// Longest frame: 1-of-4 lead-in, opcode, the two fixed bits a password needs in that mode,
// password, lock, data, address.
#define T5577_FRAME_BITS_MAX 80

static const T5577Timing t5577_timing[T5577DownlinkModeCount] = {
    // What every write this driver has sent used.
    [T5577DownlinkFixedBitLength] =
        {.start_gap = 30, .write_gap = 18, .write_0 = 24, .write_1 = 56},
    // Same gaps, symbol lengths from Proxmark3's table. Untested here: nothing in the firmware
    // sends these yet.
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
    furi_check(mode < T5577DownlinkModeCount);
    return &t5577_timing[mode];
}

typedef struct {
    uint8_t bit[T5577_FRAME_BITS_MAX];
    uint8_t count;
} T5577Bitstream;

static void t5577_bits_add(T5577Bitstream* bs, uint32_t value, uint8_t count) {
    furi_check((bs->count + count) <= T5577_FRAME_BITS_MAX);
    while(count-- > 0) {
        bs->bit[bs->count++] = (value >> count) & 1;
    }
}

// Symbol lengths are indexed by the symbol itself, which is what makes 1-of-4 a change of
// alphabet rather than a change of encoder.
static void t5577_send_symbol(uint8_t symbol, const T5577Timing* timing, uint16_t extra) {
    const uint16_t length[] = {
        timing->write_0,
        timing->write_1,
        timing->write_2,
        timing->write_3,
    };

    furi_delay_us((length[symbol] + extra) * 8);
    t5577_write_gap(timing->write_gap);
}

static void t5577_build_frame(const T5577Frame* frame, T5577Bitstream* bs) {
    const bool wide = (frame->downlink == T5577Downlink1Of4);

    // Lead-in. Both of these modes are told apart from a fixed-length frame by what precedes the
    // opcode, and 1-of-4 needs its reference bit plus a zero to fill the symbol.
    if(frame->downlink == T5577DownlinkLeadingZero) {
        t5577_bits_add(bs, 0, 1);
    } else if(wide) {
        t5577_bits_add(bs, 0, 2);
    }

    if(frame->command == T5577CommandReset) {
        t5577_bits_add(bs, 0, T5577_OPCODE_BITS);
        return;
    }

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

void t5577_send(const T5577Frame* frame) {
    furi_check(frame);
    furi_check(frame->downlink < T5577DownlinkModeCount);
    furi_check(frame->page <= 1);

    const T5577Timing* timing = (frame->timing != NULL) ? frame->timing :
                                                          t5577_timing_default(frame->downlink);

    T5577Bitstream bs = {0};
    t5577_build_frame(frame, &bs);

    const bool wide = (frame->downlink == T5577Downlink1Of4);
    // Two bits per symbol only divides a frame that holds an even number of them. Proxmark3
    // drops the odd bit; every frame built above is even, so an odd one is a bug, not a tag.
    furi_check(!wide || (bs.count % 2) == 0);

    FURI_CRITICAL_ENTER();

    // Let the tag charge up before cutting the field, or it answers nothing.
    furi_delay_us(T5577_TIMING_WAIT_TIME * 8);

    t5577_write_gap(timing->start_gap);

    if(frame->downlink == T5577DownlinkLongLeadingReference) {
        t5577_send_symbol(0, timing, T5577_LLR_REFERENCE);
    }

    for(uint8_t i = 0; i < bs.count; i += (wide ? 2 : 1)) {
        t5577_send_symbol(wide ? ((bs.bit[i] << 1) | bs.bit[i + 1]) : bs.bit[i], timing, 0);
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
static void t5577_back_to_read_mode(void) {
    t5577_send(&(T5577Frame){.command = T5577CommandRegularRead});
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

    t5577_back_to_read_mode();
}

/**************************************************************************************************/
/*********************************************** READ *********************************************/
/**************************************************************************************************/

// A read answer has no frame marker: the tag just repeats the addressed word. 64 decoded bits
// therefore cover every alignment of a 32 bit value, and a wrong alignment can only delay the
// match, never invent one - a false hit would need a 32 bit coincidence.
#define T5577_READ_MIN_BITS 32

// Listen window. At RF/64 a bit takes 512 us, so 64 bits are ~33 ms; the rest is headroom for
// the tag to start answering and for a glitch to cost a few re-syncs.
#define T5577_READ_TIMEOUT_MS 200

// Envelope runs are classified against the bit period with the same +-20% the EM4100 decoder
// uses (100 us on a 512 us bit), which is what makes those timings proven rather than guessed.
#define T5577_READ_JITTER_DIV 5

typedef struct {
    ManchesterState state;
    uint32_t short_min;
    uint32_t short_max;
    uint32_t long_min;
    uint32_t long_max;
    uint64_t shift_reg;
    uint8_t bits;
    uint32_t expected;
    volatile bool found;
} T5577Reader;

static void t5577_reader_init(T5577Reader* reader, T5577ReadClock clock, uint32_t expected) {
    // One field clock is 8 us at 125 kHz, so a bit is clock*8 us and a Manchester half-bit is
    // half of that.
    const uint32_t bit_us = (uint32_t)clock * 8;
    const uint32_t jitter = bit_us / T5577_READ_JITTER_DIV;

    *reader = (T5577Reader){
        .state = ManchesterStateMid1,
        .short_min = (bit_us / 2) - jitter,
        .short_max = (bit_us / 2) + jitter,
        .long_min = bit_us - jitter,
        .long_max = bit_us + jitter,
        .expected = expected,
    };
}

// Runs in the capture interrupt, so it stays branch-light: classify the run, advance one
// Manchester step, compare.
static void t5577_read_capture(bool level, uint32_t duration, void* context) {
    T5577Reader* reader = context;
    if(reader->found) return;

    ManchesterEvent event = ManchesterEventReset;

    // Same convention as the protocol decoders: the capture reports a pulse width with the level
    // set and the whole period with it clear, so the level reads inverted here.
    if(duration > reader->short_min && duration < reader->short_max) {
        event = level ? ManchesterEventShortLow : ManchesterEventShortHigh;
    } else if(duration > reader->long_min && duration < reader->long_max) {
        event = level ? ManchesterEventLongLow : ManchesterEventLongHigh;
    }

    bool data;
    if(!manchester_advance(reader->state, event, &reader->state, &data)) return;

    reader->shift_reg = (reader->shift_reg << 1) | data;

    if(reader->bits < T5577_READ_MIN_BITS) {
        reader->bits++;
        return;
    }

    // manchester_advance is symmetric under swapping the high and low events, so an inverted
    // envelope yields exactly inverted bits. Which polarity the detector settled on says nothing
    // about what the tag holds, so both count - Proxmark3 instead carries an `inverted` flag it
    // learned during `t55xx detect`, which is a step this verifier does not have.
    const uint32_t window = (uint32_t)reader->shift_reg;
    if(window == reader->expected || (uint32_t)~window == reader->expected) {
        reader->found = true;
    }
}

T5577ReadClock t5577_read_clock_from_config(uint32_t block0) {
    // Everything but ASK answers in a modulation the Manchester decoder cannot follow. Direct
    // (unencoded) ASK is excluded too: without a mid-bit transition there is nothing to lock on.
    if((block0 & LFRFID_T5577_MODULATION_MASK) != LFRFID_T5577_MODULATION_MANCHESTER) {
        return T5577ReadClockUnsupported;
    }

    switch(block0 & LFRFID_T5577_BITRATE_MASK) {
    case LFRFID_T5577_BITRATE_RF_16:
        return T5577ReadClock16;
    case LFRFID_T5577_BITRATE_RF_32:
        return T5577ReadClock32;
    case LFRFID_T5577_BITRATE_RF_64:
        return T5577ReadClock64;
    default:
        return T5577ReadClockUnsupported;
    }
}

bool t5577_block_holds(const T5577ReadRequest* request, uint32_t expected) {
    furi_check(request);
    furi_check(request->page <= 1);
    furi_check(
        request->block < ((request->page == 1) ? T5577_BLOCKS_IN_PAGE_1 : T5577_BLOCKS_IN_PAGE_0));

    if(request->clock == T5577ReadClockUnsupported) return false;

    T5577Reader reader;
    t5577_reader_init(&reader, request->clock, expected);

    t5577_start();

    t5577_send(&(T5577Frame){
        .command = T5577CommandRead,
        .page = request->page,
        .block = request->block,
        .with_pass = request->with_pass,
        .password = request->password,
    });

    // t5577_start() left the pull pin where a write wants it; the read path drives it the other
    // way (furi_hal_rfid_pins_read), and this is the read path now.
    furi_hal_rfid_pin_pull_pulldown();

    furi_hal_rfid_tim_read_capture_start(t5577_read_capture, &reader);

    const uint32_t start_tick = furi_get_tick();
    const uint32_t timeout = furi_ms_to_ticks(T5577_READ_TIMEOUT_MS);
    while(!reader.found && (furi_get_tick() - start_tick) < timeout) {
        furi_delay_ms(1);
    }

    furi_hal_rfid_tim_read_capture_stop();

    furi_hal_rfid_pin_pull_release();
    t5577_back_to_read_mode();

    t5577_stop();

    return reader.found;
}

static void t5577_write_block_simple(uint8_t block, bool lock_bit, uint32_t data) {
    t5577_write_block_pass(0, block, lock_bit, data, false, 0);
}

void t5577_write(LFRFIDT5577* data) {
    t5577_start();
    // Each frame holds interrupts off for itself rather than the whole sequence doing it once:
    // eight blocks is about 100 ms, which is far too long to leave the rest of the system
    // waiting, and every frame begins by charging the tag anyway so a pause between them is free.
    for(size_t i = 0; i < data->blocks_to_write; i++) {
        t5577_write_block_simple(i, false, data->block[i]);
    }
    t5577_back_to_read_mode();
    t5577_stop();
}

void t5577_write_with_pass(LFRFIDT5577* data, uint32_t password) {
    t5577_start();
    for(size_t i = 0; i < data->blocks_to_write; i++) {
        t5577_write_block_pass(0, i, false, data->block[i], true, password);
    }
    t5577_back_to_read_mode();
    t5577_stop();
}

void t5577_write_with_mask(LFRFIDT5577* data, uint8_t page, bool with_pass, uint32_t password) {
    t5577_start();

    uint8_t mask = data->mask;

    size_t pages_total = (page == 0) ? T5577_BLOCKS_IN_PAGE_0 : T5577_BLOCKS_IN_PAGE_1;

    for(size_t i = 0; i < pages_total; i++) {
        bool need_to_write = mask & 1;
        mask >>= 1;
        if(!need_to_write) continue;
        t5577_write_block_pass(page, i, false, data->block[i], with_pass, password);
    }
    t5577_back_to_read_mode();
    t5577_stop();
}
