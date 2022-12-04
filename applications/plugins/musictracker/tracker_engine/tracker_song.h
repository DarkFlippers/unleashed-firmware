#pragma once
#include <stdint.h>

/**
 * @brief Row
 * 
 * AH       AL
 * FEDCBA98 76543210
 * nnnnnnee eedddddd
 * -------- --------
 * nnnnnn            = [0] do nothing, [1..60] note number, [61] note off, [62..63] not used
 *       ee ee       = [0..F] effect 
 *            111222 = [0..63] or [0..7, 0..7] effect data 
 */
typedef uint16_t Row;

#define ROW_NOTE_MASK 0x3F
#define ROW_EFFECT_MASK 0x0F
#define ROW_EFFECT_DATA_MASK 0x3F

typedef enum {
    // 0xy, x - first semitones offset, y - second semitones offset. 0 - no offset .. 7 - +7 semitones...
    // Play the arpeggio chord with three notes. The first note is the base note, the second and third are offset by x and y.
    // Each note plays one tick.
    EffectArpeggio = 0x00,

    // 1xx, xx - effect speed, 0 - no effect, 1 - slowest, 0x3F - fastest.
    // Slide the note pitch up by xx Hz every tick.
    EffectSlideUp = 0x01,

    // 2xx, xx - effect speed, 0 - no effect, 1 - slowest, 0x3F - fastest.
    // Slide the note pitch down by xx Hz every tick.
    EffectSlideDown = 0x02,

    // 3xx, xx - effect speed, 0 - no effect, 1 - slowest, 0x3F - fastest.
    // Slide the already playing note pitch towards another one by xx Hz every tick.
    // The note value is saved until the note is playing, so you don't have to repeat the note value to continue sliding.
    EffectSlideToNote = 0x03,

    // 4xy, x - vibrato speed (0..7), y - vibrato depth (0..7).
    // Vibrato effect. The pitch of the note increases by x Hz each tick to a positive vibrato depth, then decreases to a negative depth.
    // Value 1 of depth means 1/7 of a semitone (about 14.28 ct), so value 7 means full semitone.
    // Note will play without vibrato on the first tick at the beginning of the effect.
    // Vibrato speed and depth are saved until the note is playing, and will be updated only if they are not zero, so you doesn't have to repeat them every tick.
    EffectVibrato = 0x04,

    // Effect05 = 0x05,
    // Effect06 = 0x06,
    // Effect07 = 0x07,
    // Effect08 = 0x08,
    // Effect09 = 0x09,
    // Effect0A = 0x0A,

    // Bxx, xx - pattern number
    // Jump to the order xx in the pattern order table at first tick of current row.
    // So if you want to jump to the pattern after note 4, you should put this effect on the 5th note.
    EffectJumpToOrder = 0x0B,

    // Cxx, xx - pwm value
    // Set the PWM value to xx for current row.
    EffectPWM = 0x0C,

    // Bxx, xx - row number
    // Jump to the row xx in next pattern at first tick of current row.
    // So if you want to jump to the pattern after note 4, you should put this effect on the 5th note.
    EffectBreakPattern = 0x0D,

    // Effect0E = 0x0E,

    // Fxx, xx - song speed, 0 - 1 tick per note, 1 - 2 ticks per note, 0x3F - 64 ticks per note.
    // Set the speed of the song in terms of ticks per note.
    // Will be applied at the first tick of current row.
    EffectSetSpeed = 0x0F,
} Effect;

#define EFFECT_DATA_2(x, y) ((x) | ((y) << 3))
#define EFFECT_DATA_GET_X(data) ((data)&0x07)
#define EFFECT_DATA_GET_Y(data) (((data) >> 3) & 0x07)
#define EFFECT_DATA_NONE 0
#define EFFECT_DATA_1_MAX 0x3F
#define EFFECT_DATA_2_MAX 0x07

#define FREQUENCY_UNSET -1.0f

#define PWM_MIN 0.01f
#define PWM_MAX 0.5f
#define PWM_DEFAULT PWM_MAX

#define PATTERN_SIZE 64

#define ROW_MAKE(note, effect, data) \
    ((Row)(((note)&0x3F) | (((effect)&0xF) << 6) | (((data)&0x3F) << 10)))

typedef struct {
    Row rows[PATTERN_SIZE];
} Channel;

typedef struct {
    Channel* channels;
} Pattern;

typedef struct {
    uint8_t channels_count;
    uint8_t patterns_count;
    Pattern* patterns;
    uint8_t order_list_size;
    uint8_t* order_list;
    uint16_t ticks_per_second;
} Song;