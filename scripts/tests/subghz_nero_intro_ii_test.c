/* Host-only regression tests. All payloads are synthetic, not remote captures.
 * Run from the repository root (no Flipper SDK required):
 * cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
 *   -Ilib/subghz/protocols scripts/tests/subghz_nero_intro_ii_test.c \
 *   lib/subghz/protocols/nero_intro_ii_code.c -o /tmp/nero_intro_ii_test
 * /tmp/nero_intro_ii_test
 */
#include "nero_intro_ii_code.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const uint8_t symbols[NERO_INTRO_II_SYMBOLS] = {
    1,
    2,
    3,
    5,
    8,
    13,
    21,
    34,
    55,
    65,
    66,
    67,
    69,
    72,
    77,
};

typedef struct {
    uint8_t chips[6000];
    size_t count;
} TestStream;

static void append(TestStream* stream, bool bit) {
    assert(stream->count < sizeof(stream->chips));
    stream->chips[stream->count++] = bit;
}

static void append_idle(TestStream* stream) {
    for(unsigned i = 0; i < 160; i++) {
        append(stream, false);
    }
}

static void append_frame(TestStream* stream, int slip, bool flip, bool bad_sync) {
    const uint64_t marker = UINT64_C(0x53E87126D6F634BB);
    for(unsigned i = 0; i < 64; i++) {
        bool bit = (marker >> (63 - i)) & 1;
        append(stream, bit ^ (bad_sync && i == 20));
    }
    /* The decoder skips the remaining synchronization chips. Deliberately
     * use synthetic padding rather than copy a captured waveform. */
    for(unsigned i = 64; i < 255; i++) {
        append(stream, i & 1);
    }
    for(unsigned symbol = 0; symbol < NERO_INTRO_II_SYMBOLS; symbol++) {
        uint8_t bits[63];
        for(unsigned i = 0; i < 6; i++) {
            bits[i] = (symbols[symbol] >> (5 - i)) & 1;
        }
        for(unsigned i = 6; i < 63; i++) {
            bits[i] = bits[i - 5] ^ bits[i - 6];
        }
        for(unsigned i = 0; i < 63; i++) {
            bool bit = bits[i] ^ (symbols[symbol] >> 6);
            if(symbol == 7 && i == 26) {
                if(slip < 0) continue;
                if(slip > 0) append(stream, !bit);
                bit ^= flip;
            }
            append(stream, bit);
        }
        append(stream, false);
        append(stream, true);
    }
}

static unsigned decode(const TestStream* stream, unsigned expected_edits) {
    NeroIntroIICode* decoder = calloc(1, sizeof(*decoder));
    assert(decoder);
    nero_intro_ii_code_reset(decoder);
    unsigned frames = 0;
    for(size_t start = 0; start < stream->count;) {
        size_t end = start + 1;
        bool level = stream->chips[start];
        while(end < stream->count && stream->chips[end] == level) {
            end++;
        }
        /* Model the measured slicer bias; long idle runs also merge with
         * the first zero chip of the following synchronization marker. */
        uint32_t duration = (end - start) * 125;
        duration = level ? duration - 15 : duration + 15;
        NeroIntroIIFrame frame;
        if(nero_intro_ii_code_feed(decoder, level, duration, &frame)) {
            assert(!frame.ambiguous);
            assert(frame.corrections == expected_edits);
            assert(memcmp(frame.symbols, symbols, sizeof(symbols)) == 0);
            frames++;
        }
        start = end;
    }
    free(decoder);
    return frames;
}

int main(void) {
    TestStream stream = {0};
    append_idle(&stream);
    append_frame(&stream, 0, false, false);
    append_idle(&stream);
    assert(decode(&stream, 0) == 1);

    /* Three frames in one stream, with the synchronization's leading zero
     * merged into the preceding idle each time. */
    append_frame(&stream, 0, false, false);
    append_idle(&stream);
    append_frame(&stream, 0, false, false);
    append_idle(&stream);
    assert(decode(&stream, 0) == 3);

    for(int slip = -1; slip <= 1; slip += 2) {
        memset(&stream, 0, sizeof(stream));
        append_idle(&stream);
        append_frame(&stream, slip, false, false);
        append_idle(&stream);
        assert(decode(&stream, 1) == 1);
    }

    memset(&stream, 0, sizeof(stream));
    append_frame(&stream, 0, true, false);
    append_idle(&stream);
    assert(decode(&stream, 1) == 1);

    memset(&stream, 0, sizeof(stream));
    append_frame(&stream, 0, false, true);
    append_idle(&stream);
    assert(decode(&stream, 0) == 0);

    memset(&stream, 0, sizeof(stream));
    append_frame(&stream, 0, false, false);
    stream.count = 800;
    append_idle(&stream);
    assert(decode(&stream, 0) == 0);

    memset(&stream, 0, sizeof(stream));
    for(unsigned i = 0; i < 4000; i++) {
        append(&stream, i & 1);
    }
    append_idle(&stream);
    assert(decode(&stream, 0) == 0);
    puts("Nero Intro II: 8 synthetic regression cases passed");
    return 0;
}
