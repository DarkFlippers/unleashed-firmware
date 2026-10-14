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

static void t5577_write_bit(bool value) {
    if(value) {
        furi_delay_us(T5577_TIMING_DATA_1 * 8);
    } else {
        furi_delay_us(T5577_TIMING_DATA_0 * 8);
    }
    t5577_write_gap(T5577_TIMING_WRITE_GAP);
}

static void t5577_write_opcode(uint8_t value) {
    t5577_write_bit((value >> 1) & 1);
    t5577_write_bit((value >> 0) & 1);
}

static void t5577_write_reset(void) {
    t5577_write_gap(T5577_TIMING_START_GAP);
    t5577_write_bit(1);
    t5577_write_bit(0);
}

static void t5577_write_block_pass(
    uint8_t page,
    uint8_t block,
    bool lock_bit,
    uint32_t data,
    bool with_pass,
    uint32_t password) {
    furi_delay_us(T5577_TIMING_WAIT_TIME * 8);

    // start gap
    t5577_write_gap(T5577_TIMING_START_GAP);

    // opcode for page
    t5577_write_opcode((page == 1) ? T5577_OPCODE_PAGE_1 : T5577_OPCODE_PAGE_0);

    // password
    if(with_pass) {
        for(uint8_t i = 0; i < 32; i++) {
            t5577_write_bit((password >> (31 - i)) & 1);
        }
    }

    // lock bit
    t5577_write_bit(lock_bit);

    // data
    for(uint8_t i = 0; i < 32; i++) {
        t5577_write_bit((data >> (31 - i)) & 1);
    }

    // block address
    t5577_write_bit((block >> 2) & 1);
    t5577_write_bit((block >> 1) & 1);
    t5577_write_bit((block >> 0) & 1);

    furi_delay_us(T5577_TIMING_PROGRAM * 8);

    furi_delay_us(T5577_TIMING_WAIT_TIME * 8);
    t5577_write_reset();
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

// Read packet: opcode, optional password, lock bit, block address. The same frame as a write
// minus the 32 data bits. Bit-banged with busy waits, so it owns the critical section; the
// capture that follows needs interrupts back on, which is safe because the tag keeps repeating
// the block for as long as the field is up - listening late costs a few bits, nothing more.
static void t5577_send_read_command(const T5577ReadRequest* request) {
    FURI_CRITICAL_ENTER();

    furi_delay_us(T5577_TIMING_WAIT_TIME * 8);

    t5577_write_gap(T5577_TIMING_START_GAP);

    t5577_write_opcode((request->page == 1) ? T5577_OPCODE_PAGE_1 : T5577_OPCODE_PAGE_0);

    if(request->with_pass) {
        for(uint8_t i = 0; i < 32; i++) {
            t5577_write_bit((request->password >> (31 - i)) & 1);
        }
    }

    t5577_write_bit(false); // lock

    t5577_write_bit((request->block >> 2) & 1);
    t5577_write_bit((request->block >> 1) & 1);
    t5577_write_bit((request->block >> 0) & 1);

    FURI_CRITICAL_EXIT();
}

// A tag still streaming one block would answer the next reader with that word instead of its
// data, so every read ends by putting it back into regular read mode.
static void t5577_reset_to_read_mode(void) {
    FURI_CRITICAL_ENTER();
    t5577_write_reset();
    FURI_CRITICAL_EXIT();
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

    t5577_send_read_command(request);

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
    t5577_reset_to_read_mode();

    t5577_stop();

    return reader.found;
}

static void t5577_write_block_simple(uint8_t block, bool lock_bit, uint32_t data) {
    t5577_write_block_pass(0, block, lock_bit, data, false, 0);
}

void t5577_write(LFRFIDT5577* data) {
    t5577_start();
    FURI_CRITICAL_ENTER();
    for(size_t i = 0; i < data->blocks_to_write; i++) {
        t5577_write_block_simple(i, false, data->block[i]);
    }
    t5577_write_reset();
    FURI_CRITICAL_EXIT();
    t5577_stop();
}

void t5577_write_with_pass(LFRFIDT5577* data, uint32_t password) {
    t5577_start();
    FURI_CRITICAL_ENTER();
    for(size_t i = 0; i < data->blocks_to_write; i++) {
        t5577_write_block_pass(0, i, false, data->block[i], true, password);
    }
    t5577_write_reset();
    FURI_CRITICAL_EXIT();
    t5577_stop();
}

void t5577_write_with_mask(LFRFIDT5577* data, uint8_t page, bool with_pass, uint32_t password) {
    t5577_start();
    FURI_CRITICAL_ENTER();

    uint8_t mask = data->mask;

    size_t pages_total = (page == 0) ? T5577_BLOCKS_IN_PAGE_0 : T5577_BLOCKS_IN_PAGE_1;

    for(size_t i = 0; i < pages_total; i++) {
        bool need_to_write = mask & 1;
        mask >>= 1;
        if(!need_to_write) continue;
        t5577_write_block_pass(page, i, false, data->block[i], with_pass, password);
    }
    t5577_write_reset();
    FURI_CRITICAL_EXIT();
    t5577_stop();
}
