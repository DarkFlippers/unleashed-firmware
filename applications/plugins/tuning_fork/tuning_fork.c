#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <m-string.h>
#include <string.h>
#include <stdlib.h>

#include <gui/gui.h>
#include <gui/elements.h>
#include <gui/canvas.h>

#include <notification/notification.h>
#include <notification/notification_messages.h>

#include "notes.h"
#include "tunings.h"

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

enum Page { Tunings, Notes };

typedef struct {
    bool playing;
    enum Page page;
    int current_tuning_note_index;
    int current_tuning_index;
    float volume;
    TUNING tuning;
} TuningForkState;

static TUNING current_tuning(TuningForkState* tuningForkState) {
    return tuningForkState->tuning;
}

static NOTE current_tuning_note(TuningForkState* tuningForkState) {
    return current_tuning(tuningForkState).notes[tuningForkState->current_tuning_note_index];
}

static float current_tuning_note_freq(TuningForkState* tuningForkState) {
    return current_tuning_note(tuningForkState).frequency;
}

static void current_tuning_note_label(TuningForkState* tuningForkState, char* outNoteLabel) {
    for(int i = 0; i < 20; ++i) {
        outNoteLabel[i] = current_tuning_note(tuningForkState).label[i];
    }
}

static void current_tuning_label(TuningForkState* tuningForkState, char* outTuningLabel) {
    for(int i = 0; i < 20; ++i) {
        outTuningLabel[i] = current_tuning(tuningForkState).label[i];
    }
}

static void updateTuning(TuningForkState* tuning_fork_state) {
    tuning_fork_state->tuning = TuningList[tuning_fork_state->current_tuning_index];
    tuning_fork_state->current_tuning_note_index = 0;
}

static void next_tuning(TuningForkState* tuning_fork_state) {
    if(tuning_fork_state->current_tuning_index == TUNINGS_COUNT - 1) {
        tuning_fork_state->current_tuning_index = 0;
    } else {
        tuning_fork_state->current_tuning_index += 1;
    }
    updateTuning(tuning_fork_state);
}

static void prev_tuning(TuningForkState* tuning_fork_state) {
    if(tuning_fork_state->current_tuning_index - 1 < 0) {
        tuning_fork_state->current_tuning_index = TUNINGS_COUNT - 1;
    } else {
        tuning_fork_state->current_tuning_index -= 1;
    }
    updateTuning(tuning_fork_state);
}

static void next_note(TuningForkState* tuning_fork_state) {
    if(tuning_fork_state->current_tuning_note_index ==
       current_tuning(tuning_fork_state).notes_length - 1) {
        tuning_fork_state->current_tuning_note_index = 0;
    } else {
        tuning_fork_state->current_tuning_note_index += 1;
    }
}

static void prev_note(TuningForkState* tuning_fork_state) {
    if(tuning_fork_state->current_tuning_note_index == 0) {
        tuning_fork_state->current_tuning_note_index =
            current_tuning(tuning_fork_state).notes_length - 1;
    } else {
        tuning_fork_state->current_tuning_note_index -= 1;
    }
}

static void increase_volume(TuningForkState* tuning_fork_state) {
    if(tuning_fork_state->volume < 1.0f) {
        tuning_fork_state->volume += 0.1f;
    }
}

static void decrease_volume(TuningForkState* tuning_fork_state) {
    if(tuning_fork_state->volume > 0.0f) {
        tuning_fork_state->volume -= 0.1f;
    }
}

static void play(TuningForkState* tuning_fork_state) {
    if(furi_hal_speaker_is_mine() || furi_hal_speaker_acquire(30)) {
        furi_hal_speaker_start(
            current_tuning_note_freq(tuning_fork_state), tuning_fork_state->volume);
    }
}

static void stop() {
    if(furi_hal_speaker_is_mine()) {
        furi_hal_speaker_stop();
        furi_hal_speaker_release();
    }
}

static void replay(TuningForkState* tuning_fork_state) {
    stop();
    play(tuning_fork_state);
}

static void render_callback(Canvas* const canvas, void* ctx) {
    TuningForkState* tuning_fork_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(tuning_fork_state == NULL) {
        return;
    }

    string_t tempStr;
    string_init(tempStr);

    canvas_draw_frame(canvas, 0, 0, 128, 64);

    canvas_set_font(canvas, FontPrimary);

    if(tuning_fork_state->page == Tunings) {
        char tuningLabel[20];
        current_tuning_label(tuning_fork_state, tuningLabel);
        string_printf(tempStr, "< %s >", tuningLabel);
        canvas_draw_str_aligned(
            canvas, 64, 28, AlignCenter, AlignCenter, string_get_cstr(tempStr));
        string_reset(tempStr);
    } else {
        char tuningLabel[20];
        current_tuning_label(tuning_fork_state, tuningLabel);
        string_printf(tempStr, "%s", tuningLabel);
        canvas_draw_str_aligned(canvas, 64, 8, AlignCenter, AlignCenter, string_get_cstr(tempStr));
        string_reset(tempStr);

        char tuningNoteLabel[20];
        current_tuning_note_label(tuning_fork_state, tuningNoteLabel);
        string_printf(tempStr, "< %s >", tuningNoteLabel);
        canvas_draw_str_aligned(
            canvas, 64, 24, AlignCenter, AlignCenter, string_get_cstr(tempStr));
        string_reset(tempStr);
    }

    canvas_set_font(canvas, FontSecondary);
    elements_button_left(canvas, "Prev");
    elements_button_right(canvas, "Next");

    if(tuning_fork_state->page == Notes) {
        if(tuning_fork_state->playing) {
            elements_button_center(canvas, "Stop ");
        } else {
            elements_button_center(canvas, "Play");
        }
    } else {
        elements_button_center(canvas, "Select");
    }
    if(tuning_fork_state->page == Notes) {
        elements_progress_bar(canvas, 8, 36, 112, tuning_fork_state->volume);
    }

    string_clear(tempStr);
    release_mutex((ValueMutex*)ctx, tuning_fork_state);
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void tuning_fork_state_init(TuningForkState* const tuning_fork_state) {
    tuning_fork_state->playing = false;
    tuning_fork_state->page = Tunings;
    tuning_fork_state->volume = 1.0f;
    tuning_fork_state->tuning = GuitarStandard6;
    tuning_fork_state->current_tuning_index = 2;
    tuning_fork_state->current_tuning_note_index = 0;
}

int32_t tuning_fork_app() {
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));

    TuningForkState* tuning_fork_state = malloc(sizeof(TuningForkState));
    tuning_fork_state_init(tuning_fork_state);

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, tuning_fork_state, sizeof(TuningForkState))) {
        FURI_LOG_E("TuningFork", "cannot create mutex\r\n");
        free(tuning_fork_state);
        return 255;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    PluginEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);

        TuningForkState* tuning_fork_state = (TuningForkState*)acquire_mutex_block(&state_mutex);

        if(event_status == FuriStatusOk) {
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypeShort) {
                    // push events
                    switch(event.input.key) {
                    case InputKeyUp:
                        if(tuning_fork_state->page == Notes) {
                            increase_volume(tuning_fork_state);
                            if(tuning_fork_state->playing) {
                                replay(tuning_fork_state);
                            }
                        }
                        break;
                    case InputKeyDown:
                        if(tuning_fork_state->page == Notes) {
                            decrease_volume(tuning_fork_state);
                            if(tuning_fork_state->playing) {
                                replay(tuning_fork_state);
                            }
                        }
                        break;
                    case InputKeyRight:
                        if(tuning_fork_state->page == Tunings) {
                            next_tuning(tuning_fork_state);
                        } else {
                            next_note(tuning_fork_state);
                            if(tuning_fork_state->playing) {
                                replay(tuning_fork_state);
                            }
                        }
                        break;
                    case InputKeyLeft:
                        if(tuning_fork_state->page == Tunings) {
                            prev_tuning(tuning_fork_state);
                        } else {
                            prev_note(tuning_fork_state);
                            if(tuning_fork_state->playing) {
                                replay(tuning_fork_state);
                            }
                        }
                        break;
                    case InputKeyOk:
                        if(tuning_fork_state->page == Tunings) {
                            tuning_fork_state->page = Notes;
                        } else {
                            tuning_fork_state->playing = !tuning_fork_state->playing;
                            if(tuning_fork_state->playing) {
                                play(tuning_fork_state);
                            } else {
                                stop();
                            }
                        }
                        break;
                    case InputKeyBack:
                        if(tuning_fork_state->page == Tunings) {
                            processing = false;
                        } else {
                            tuning_fork_state->playing = false;
                            tuning_fork_state->current_tuning_note_index = 0;
                            stop();
                            tuning_fork_state->page = Tunings;
                        }
                        break;
                    default:
                        break;
                    }
                } else if(event.input.type == InputTypeLong) {
                    // hold events
                    switch(event.input.key) {
                    case InputKeyUp:
                        break;
                    case InputKeyDown:
                        break;
                    case InputKeyRight:
                        if(tuning_fork_state->page == Tunings) {
                            next_tuning(tuning_fork_state);
                        } else {
                            next_note(tuning_fork_state);
                            if(tuning_fork_state->playing) {
                                replay(tuning_fork_state);
                            }
                        }

                        break;
                    case InputKeyLeft:
                        if(tuning_fork_state->page == Tunings) {
                            prev_tuning(tuning_fork_state);
                        } else {
                            prev_note(tuning_fork_state);
                            if(tuning_fork_state->playing) {
                                replay(tuning_fork_state);
                            }
                        }

                        break;
                    case InputKeyOk:
                        break;
                    case InputKeyBack:
                        if(tuning_fork_state->page == Tunings) {
                            processing = false;
                        } else {
                            tuning_fork_state->playing = false;
                            stop();
                            tuning_fork_state->page = Tunings;
                            tuning_fork_state->current_tuning_note_index = 0;
                        }
                        break;
                    default:
                        break;
                    }
                } else if(event.input.type == InputTypeRepeat) {
                    // repeat events
                    switch(event.input.key) {
                    case InputKeyUp:
                        break;
                    case InputKeyDown:
                        break;
                    case InputKeyRight:
                        if(tuning_fork_state->page == Tunings) {
                            next_tuning(tuning_fork_state);
                        } else {
                            next_note(tuning_fork_state);
                            if(tuning_fork_state->playing) {
                                replay(tuning_fork_state);
                            }
                        }

                        break;
                    case InputKeyLeft:
                        if(tuning_fork_state->page == Tunings) {
                            prev_tuning(tuning_fork_state);
                        } else {
                            prev_note(tuning_fork_state);
                            if(tuning_fork_state->playing) {
                                replay(tuning_fork_state);
                            }
                        }

                        break;
                    case InputKeyOk:
                        break;
                    case InputKeyBack:
                        if(tuning_fork_state->page == Tunings) {
                            processing = false;
                        } else {
                            tuning_fork_state->playing = false;
                            stop();
                            tuning_fork_state->page = Tunings;
                            tuning_fork_state->current_tuning_note_index = 0;
                        }
                        break;
                    default:
                        break;
                    }
                }
            }
        } else {
            FURI_LOG_D("TuningFork", "FuriMessageQueue: event timeout");
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, tuning_fork_state);
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    delete_mutex(&state_mutex);
    furi_record_close(RECORD_NOTIFICATION);
    free(tuning_fork_state);

    return 0;
}
