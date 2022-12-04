#include "tracker.h"
#include <stdbool.h>
#include "speaker_hal.h"

// SongState song_state = {
//     .tick = 0,
//     .tick_limit = 2,
//     .row = 0,
// };

typedef struct {
    uint8_t speed;
    uint8_t depth;
    int8_t direction;
    int8_t value;
} IntegerOscillator;

typedef struct {
    float frequency;
    float frequency_target;
    float pwm;
    bool play;
    IntegerOscillator vibrato;
} ChannelState;

typedef struct {
    ChannelState* channels;
    uint8_t tick;
    uint8_t tick_limit;

    uint8_t pattern_index;
    uint8_t row_index;
    uint8_t order_list_index;
} SongState;

typedef struct {
    uint8_t note;
    uint8_t effect;
    uint8_t data;
} UnpackedRow;

struct Tracker {
    const Song* song;
    bool playing;
    TrackerMessageCallback callback;
    void* context;
    SongState song_state;
};

static void channels_state_init(ChannelState* channel) {
    channel->frequency = 0;
    channel->frequency_target = FREQUENCY_UNSET;
    channel->pwm = PWM_DEFAULT;
    channel->play = false;
    channel->vibrato.speed = 0;
    channel->vibrato.depth = 0;
    channel->vibrato.direction = 0;
    channel->vibrato.value = 0;
}

static void tracker_song_state_init(Tracker* tracker) {
    tracker->song_state.tick = 0;
    tracker->song_state.tick_limit = 2;
    tracker->song_state.row_index = 0;
    tracker->song_state.order_list_index = 0;
    tracker->song_state.pattern_index = tracker->song->order_list[0];

    if(tracker->song_state.channels != NULL) {
        free(tracker->song_state.channels);
    }

    tracker->song_state.channels = malloc(sizeof(ChannelState) * tracker->song->channels_count);
    for(uint8_t i = 0; i < tracker->song->channels_count; i++) {
        channels_state_init(&tracker->song_state.channels[i]);
    }
}

static void tracker_song_state_clear(Tracker* tracker) {
    if(tracker->song_state.channels != NULL) {
        free(tracker->song_state.channels);
        tracker->song_state.channels = NULL;
    }
}

static uint8_t record_get_note(Row note) {
    return note & ROW_NOTE_MASK;
}

static uint8_t record_get_effect(Row note) {
    return (note >> 6) & ROW_EFFECT_MASK;
}

static uint8_t record_get_effect_data(Row note) {
    return (note >> 10) & ROW_EFFECT_DATA_MASK;
}

#define NOTES_PER_OCT 12
const float notes_oct[NOTES_PER_OCT] = {
    130.813f,
    138.591f,
    146.832f,
    155.563f,
    164.814f,
    174.614f,
    184.997f,
    195.998f,
    207.652f,
    220.00f,
    233.082f,
    246.942f,
};

static float note_to_freq(uint8_t note) {
    if(note == NOTE_NONE) return 0.0f;
    note = note - NOTE_C2;
    uint8_t octave = note / NOTES_PER_OCT;
    uint8_t note_in_oct = note % NOTES_PER_OCT;
    return notes_oct[note_in_oct] * (1 << octave);
}

static float frequency_offset_semitones(float frequency, uint8_t semitones) {
    return frequency * (1.0f + ((1.0f / 12.0f) * semitones));
}

static float frequency_get_seventh_of_a_semitone(float frequency) {
    return frequency * ((1.0f / 12.0f) / 7.0f);
}

static UnpackedRow get_current_row(const Song* song, SongState* song_state, uint8_t channel) {
    const Pattern* pattern = &song->patterns[song_state->pattern_index];
    const Row row = pattern->channels[channel].rows[song_state->row_index];
    return (UnpackedRow){
        .note = record_get_note(row),
        .effect = record_get_effect(row),
        .data = record_get_effect_data(row),
    };
}

static int16_t advance_order_and_get_next_pattern_index(const Song* song, SongState* song_state) {
    song_state->order_list_index++;
    if(song_state->order_list_index >= song->order_list_size) {
        return -1;
    } else {
        return song->order_list[song_state->order_list_index];
    }
}

typedef struct {
    int16_t pattern;
    int16_t row;
    bool change_pattern;
    bool change_row;
} Location;

static void tracker_send_position_message(Tracker* tracker) {
    if(tracker->callback != NULL) {
        tracker->callback(
            (TrackerMessage){
                .type = TrackerPositionChanged,
                .data =
                    {
                        .position =
                            {
                                .order_list_index = tracker->song_state.order_list_index,
                                .row = tracker->song_state.row_index,
                            },
                    },
            },
            tracker->context);
    }
}

static void tracker_send_end_message(Tracker* tracker) {
    if(tracker->callback != NULL) {
        tracker->callback((TrackerMessage){.type = TrackerEndOfSong}, tracker->context);
    }
}

static void advance_to_pattern(Tracker* tracker, Location advance) {
    if(advance.change_pattern) {
        if(advance.pattern < 0 || advance.pattern >= tracker->song->patterns_count) {
            tracker->playing = false;
            tracker_send_end_message(tracker);
        } else {
            tracker->song_state.pattern_index = advance.pattern;
            tracker->song_state.row_index = 0;
        }
    }

    if(advance.change_row) {
        if(advance.row < 0) advance.row = 0;
        if(advance.row >= PATTERN_SIZE) advance.row = PATTERN_SIZE - 1;
        tracker->song_state.row_index = advance.row;
    }

    tracker_send_position_message(tracker);
}

static void tracker_interrupt_body(Tracker* tracker) {
    if(!tracker->playing) {
        tracker_speaker_stop();
        return;
    }

    const uint8_t channel_index = 0;
    SongState* song_state = &tracker->song_state;
    ChannelState* channel_state = &song_state->channels[channel_index];
    const Song* song = tracker->song;
    UnpackedRow row = get_current_row(song, song_state, channel_index);

    // load frequency from note at tick 0
    if(song_state->tick == 0) {
        bool invalidate_row = false;
        // handle "on first tick" effects
        if(row.effect == EffectBreakPattern) {
            int16_t next_row_index = row.data;
            int16_t next_pattern_index =
                advance_order_and_get_next_pattern_index(song, song_state);
            advance_to_pattern(
                tracker,
                (Location){
                    .pattern = next_pattern_index,
                    .row = next_row_index,
                    .change_pattern = true,
                    .change_row = true,
                });

            invalidate_row = true;
        }

        if(row.effect == EffectJumpToOrder) {
            song_state->order_list_index = row.data;
            int16_t next_pattern_index = song->order_list[song_state->order_list_index];

            advance_to_pattern(
                tracker,
                (Location){
                    .pattern = next_pattern_index,
                    .change_pattern = true,
                });

            invalidate_row = true;
        }

        // tracker state can be affected by effects
        if(!tracker->playing) {
            tracker_speaker_stop();
            return;
        }

        if(invalidate_row) {
            row = get_current_row(song, song_state, channel_index);

            if(row.effect == EffectSetSpeed) {
                song_state->tick_limit = row.data;
            }
        }

        // handle note effects
        if(row.note == NOTE_OFF) {
            channel_state->play = false;
        } else if((row.note > NOTE_NONE) && (row.note < NOTE_OFF)) {
            channel_state->play = true;

            // reset vibrato
            channel_state->vibrato.speed = 0;
            channel_state->vibrato.depth = 0;
            channel_state->vibrato.value = 0;
            channel_state->vibrato.direction = 0;

            // reset pwm
            channel_state->pwm = PWM_DEFAULT;

            if(row.effect == EffectSlideToNote) {
                channel_state->frequency_target = note_to_freq(row.note);
            } else {
                channel_state->frequency = note_to_freq(row.note);
                channel_state->frequency_target = FREQUENCY_UNSET;
            }
        }
    }

    if(channel_state->play) {
        float frequency, pwm;

        if((row.effect == EffectSlideUp || row.effect == EffectSlideDown) &&
           row.data != EFFECT_DATA_NONE) {
            // apply slide effect
            channel_state->frequency += (row.effect == EffectSlideUp ? 1 : -1) * row.data;
        } else if(row.effect == EffectSlideToNote) {
            // apply slide to note effect, if target frequency is set
            if(channel_state->frequency_target > 0) {
                if(channel_state->frequency_target > channel_state->frequency) {
                    channel_state->frequency += row.data;
                    if(channel_state->frequency > channel_state->frequency_target) {
                        channel_state->frequency = channel_state->frequency_target;
                        channel_state->frequency_target = FREQUENCY_UNSET;
                    }
                } else if(channel_state->frequency_target < channel_state->frequency) {
                    channel_state->frequency -= row.data;
                    if(channel_state->frequency < channel_state->frequency_target) {
                        channel_state->frequency = channel_state->frequency_target;
                        channel_state->frequency_target = FREQUENCY_UNSET;
                    }
                }
            }
        }

        frequency = channel_state->frequency;
        pwm = channel_state->pwm;

        // apply arpeggio effect
        if(row.effect == EffectArpeggio) {
            if(row.data != EFFECT_DATA_NONE) {
                if((song_state->tick % 3) == 1) {
                    uint8_t note_offset = EFFECT_DATA_GET_X(row.data);
                    frequency = frequency_offset_semitones(frequency, note_offset);
                } else if((song_state->tick % 3) == 2) {
                    uint8_t note_offset = EFFECT_DATA_GET_Y(row.data);
                    frequency = frequency_offset_semitones(frequency, note_offset);
                }
            }
        } else if(row.effect == EffectVibrato) {
            // apply vibrato effect, data = speed, depth
            uint8_t vibrato_speed = EFFECT_DATA_GET_X(row.data);
            uint8_t vibrato_depth = EFFECT_DATA_GET_Y(row.data);

            // update vibrato parameters if speed or depth is non-zero
            if(vibrato_speed != 0) channel_state->vibrato.speed = vibrato_speed;
            if(vibrato_depth != 0) channel_state->vibrato.depth = vibrato_depth;

            // update vibrato value
            channel_state->vibrato.value +=
                channel_state->vibrato.direction * channel_state->vibrato.speed;

            // change direction if value is at the limit
            if(channel_state->vibrato.value > channel_state->vibrato.depth) {
                channel_state->vibrato.direction = -1;
            } else if(channel_state->vibrato.value < -channel_state->vibrato.depth) {
                channel_state->vibrato.direction = 1;
            } else if(channel_state->vibrato.direction == 0) {
                // set initial direction, if it is not set
                channel_state->vibrato.direction = 1;
            }

            frequency +=
                (frequency_get_seventh_of_a_semitone(frequency) * channel_state->vibrato.value);
        } else if(row.effect == EffectPWM) {
            pwm = (pwm - PWM_MIN) / EFFECT_DATA_1_MAX * row.data + PWM_MIN;
        }

        tracker_speaker_play(frequency, pwm);
    } else {
        tracker_speaker_stop();
    }

    song_state->tick++;
    if(song_state->tick >= song_state->tick_limit) {
        song_state->tick = 0;

        // next note
        song_state->row_index = (song_state->row_index + 1);

        if(song_state->row_index >= PATTERN_SIZE) {
            int16_t next_pattern_index =
                advance_order_and_get_next_pattern_index(song, song_state);
            advance_to_pattern(
                tracker,
                (Location){
                    .pattern = next_pattern_index,
                    .change_pattern = true,
                });
        } else {
            tracker_send_position_message(tracker);
        }
    }
}

static void tracker_interrupt_cb(void* context) {
    Tracker* tracker = (Tracker*)context;
    tracker_debug_set(true);
    tracker_interrupt_body(tracker);
    tracker_debug_set(false);
}

/*********************************************************************
 * Tracker Interface
 *********************************************************************/

Tracker* tracker_alloc() {
    Tracker* tracker = malloc(sizeof(Tracker));
    return tracker;
}

void tracker_free(Tracker* tracker) {
    tracker_song_state_clear(tracker);
    free(tracker);
}

void tracker_set_message_callback(Tracker* tracker, TrackerMessageCallback callback, void* context) {
    furi_check(tracker->playing == false);
    tracker->callback = callback;
    tracker->context = context;
}

void tracker_set_song(Tracker* tracker, const Song* song) {
    furi_check(tracker->playing == false);
    tracker->song = song;
    tracker_song_state_init(tracker);
}

void tracker_set_order_index(Tracker* tracker, uint8_t order_index) {
    furi_check(tracker->playing == false);
    furi_check(order_index < tracker->song->order_list_size);
    tracker->song_state.order_list_index = order_index;
    tracker->song_state.pattern_index = tracker->song->order_list[order_index];
}

void tracker_set_row(Tracker* tracker, uint8_t row) {
    furi_check(tracker->playing == false);
    furi_check(row < PATTERN_SIZE);
    tracker->song_state.row_index = row;
}

void tracker_start(Tracker* tracker) {
    furi_check(tracker->song != NULL);

    tracker->playing = true;
    tracker_send_position_message(tracker);
    tracker_debug_init();
    tracker_speaker_init();
    tracker_interrupt_init(tracker->song->ticks_per_second, tracker_interrupt_cb, tracker);
}

void tracker_stop(Tracker* tracker) {
    tracker_interrupt_deinit();
    tracker_speaker_deinit();
    tracker_debug_deinit();

    tracker->playing = false;
}