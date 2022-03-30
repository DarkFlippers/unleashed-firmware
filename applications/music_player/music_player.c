#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <input/input.h>

// TODO float note freq
typedef enum {
    // Delay
    N = 0,
    // Octave 4
    B4 = 494,
    // Octave 5
    C5 = 523,
    D5 = 587,
    E5 = 659,
    F_5 = 740,
    G5 = 784,
    A5 = 880,
    B5 = 988,
    // Octave 6
    C6 = 1046,
    D6 = 1175,
    E6 = 1319,
} MelodyEventNote;

typedef enum {
    L1 = 1,
    L2 = 2,
    L4 = 4,
    L8 = 8,
    L16 = 16,
    L32 = 32,
    L64 = 64,
    L128 = 128,
} MelodyEventLength;

typedef struct {
    MelodyEventNote note;
    MelodyEventLength length;
} MelodyEventRecord;

typedef struct {
    const MelodyEventRecord* record;
    int8_t loop_count;
} SongPattern;

const MelodyEventRecord melody_start[] = {
    {E6, L8}, {N, L8},   {E5, L8}, {B5, L8},  {N, L4},  {E5, L8},  {A5, L8},  {G5, L8}, {A5, L8},
    {E5, L8}, {B5, L8},  {N, L8},  {G5, L8},  {A5, L8}, {D6, L8},  {N, L4},   {D5, L8}, {B5, L8},
    {N, L4},  {D5, L8},  {A5, L8}, {G5, L8},  {A5, L8}, {D5, L8},  {F_5, L8}, {N, L8},  {G5, L8},
    {A5, L8}, {D6, L8},  {N, L4},  {F_5, L8}, {B5, L8}, {N, L4},   {F_5, L8}, {D6, L8}, {C6, L8},
    {B5, L8}, {F_5, L8}, {A5, L8}, {N, L8},   {G5, L8}, {F_5, L8}, {E5, L8},  {N, L8},  {C5, L8},
    {E5, L8}, {B5, L8},  {B4, L8}, {C5, L8},  {D5, L8}, {D6, L8},  {C6, L8},  {B5, L8}, {F_5, L8},
    {A5, L8}, {N, L8},   {G5, L8}, {A5, L8},  {E6, L8}};

const MelodyEventRecord melody_loop[] = {
    {N, L4},   {E5, L8}, {B5, L8},  {N, L4},  {E5, L8},  {A5, L8},  {G5, L8}, {A5, L8},  {E5, L8},
    {B5, L8},  {N, L8},  {G5, L8},  {A5, L8}, {D6, L8},  {N, L4},   {D5, L8}, {B5, L8},  {N, L4},
    {D5, L8},  {A5, L8}, {G5, L8},  {A5, L8}, {D5, L8},  {F_5, L8}, {N, L8},  {G5, L8},  {A5, L8},
    {D6, L8},  {N, L4},  {F_5, L8}, {B5, L8}, {N, L4},   {F_5, L8}, {D6, L8}, {C6, L8},  {B5, L8},
    {F_5, L8}, {A5, L8}, {N, L8},   {G5, L8}, {F_5, L8}, {E5, L8},  {N, L8},  {C5, L8},  {E5, L8},
    {B5, L8},  {B4, L8}, {C5, L8},  {D5, L8}, {D6, L8},  {C6, L8},  {B5, L8}, {F_5, L8}, {A5, L8},
    {N, L8},   {G5, L8}, {A5, L8},  {E6, L8}};

const MelodyEventRecord melody_chords_1bar[] = {
    {E6, L8},   {N, L8},    {B4, L128}, {E5, L128}, {B4, L128}, {E5, L128}, {B4, L128}, {E5, L128},
    {B4, L128}, {E5, L128}, {B4, L128}, {E5, L128}, {B4, L128}, {E5, L128}, {B4, L128}, {E5, L128},
    {B4, L128}, {E5, L128}, {B5, L8},   {N, L4},    {B4, L128}, {E5, L128}, {B4, L128}, {E5, L128},
    {B4, L128}, {E5, L128}, {B4, L128}, {E5, L128}, {B4, L128}, {E5, L128}, {B4, L128}, {E5, L128},
    {B4, L128}, {E5, L128}, {B4, L128}, {E5, L128}, {A5, L8}};

const SongPattern song[] = {{melody_start, 1}, {melody_loop, -1}};

typedef enum {
    EventTypeTick,
    EventTypeKey,
    EventTypeNote,
    // add your events type
} MusicDemoEventType;

typedef struct {
    union {
        InputEvent input;
        const MelodyEventRecord* note_record;
    } value;
    MusicDemoEventType type;
} MusicDemoEvent;

typedef struct {
    ValueMutex* state_mutex;
    osMessageQueueId_t event_queue;

} MusicDemoContext;

#define note_stack_size 4
typedef struct {
    // describe state here
    const MelodyEventRecord* note_record;
    const MelodyEventRecord* note_stack[note_stack_size];
    uint8_t volume_id;
    uint8_t volume_id_max;
} State;

const float volumes[] = {0, .25, .5, .75, 1};

bool is_white_note(const MelodyEventRecord* note_record, uint8_t id) {
    if(note_record == NULL) return false;

    switch(note_record->note) {
    case C5:
    case C6:
        if(id == 0) return true;
        break;
    case D5:
    case D6:
        if(id == 1) return true;
        break;
    case E5:
    case E6:
        if(id == 2) return true;
        break;
    case G5:
        if(id == 4) return true;
        break;
    case A5:
        if(id == 5) return true;
        break;
    case B4:
    case B5:
        if(id == 6) return true;
        break;
    default:
        break;
    }

    return false;
}

bool is_black_note(const MelodyEventRecord* note_record, uint8_t id) {
    if(note_record == NULL) return false;

    switch(note_record->note) {
    case F_5:
        if(id == 3) return true;
        break;
    default:
        break;
    }

    return false;
}

const char* get_note_name(const MelodyEventRecord* note_record) {
    if(note_record == NULL) return "";

    switch(note_record->note) {
    case N:
        return "---";
        break;
    case B4:
        return "B4-";
        break;
    case C5:
        return "C5-";
        break;
    case D5:
        return "D5-";
        break;
    case E5:
        return "E5-";
        break;
    case F_5:
        return "F#5";
        break;
    case G5:
        return "G5-";
        break;
    case A5:
        return "A5-";
        break;
    case B5:
        return "B5-";
        break;
    case C6:
        return "C6-";
        break;
    case D6:
        return "D6-";
        break;
    case E6:
        return "E6-";
        break;
    default:
        return "UNK";
        break;
    }
}
const char* get_note_len_name(const MelodyEventRecord* note_record) {
    if(note_record == NULL) return "";

    switch(note_record->length) {
    case L1:
        return "1-";
        break;
    case L2:
        return "2-";
        break;
    case L4:
        return "4-";
        break;
    case L8:
        return "8-";
        break;
    case L16:
        return "16";
        break;
    case L32:
        return "32";
        break;
    case L64:
        return "64";
        break;
    case L128:
        return "1+";
        break;
    default:
        return "--";
        break;
    }
}

static void render_callback(Canvas* canvas, void* ctx) {
    State* state = (State*)acquire_mutex((ValueMutex*)ctx, 25);

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 12, "MusicPlayer");

    uint8_t x_pos = 0;
    uint8_t y_pos = 24;
    const uint8_t white_w = 10;
    const uint8_t white_h = 40;

    const int8_t black_x = 6;
    const int8_t black_y = -5;
    const uint8_t black_w = 8;
    const uint8_t black_h = 32;

    // white keys
    for(size_t i = 0; i < 7; i++) {
        if(is_white_note(state->note_record, i)) {
            canvas_draw_box(canvas, x_pos + white_w * i, y_pos, white_w + 1, white_h);
        } else {
            canvas_draw_frame(canvas, x_pos + white_w * i, y_pos, white_w + 1, white_h);
        }
    }

    // black keys
    for(size_t i = 0; i < 7; i++) {
        if(i != 2 && i != 6) {
            canvas_set_color(canvas, ColorWhite);
            canvas_draw_box(
                canvas, x_pos + white_w * i + black_x, y_pos + black_y, black_w + 1, black_h);
            canvas_set_color(canvas, ColorBlack);
            if(is_black_note(state->note_record, i)) {
                canvas_draw_box(
                    canvas, x_pos + white_w * i + black_x, y_pos + black_y, black_w + 1, black_h);
            } else {
                canvas_draw_frame(
                    canvas, x_pos + white_w * i + black_x, y_pos + black_y, black_w + 1, black_h);
            }
        }
    }

    // volume view_port
    x_pos = 124;
    y_pos = 0;
    const uint8_t volume_h = (64 / (state->volume_id_max - 1)) * state->volume_id;
    canvas_draw_frame(canvas, x_pos, y_pos, 4, 64);
    canvas_draw_box(canvas, x_pos, y_pos + (64 - volume_h), 4, volume_h);

    // note stack view_port
    x_pos = 73;
    y_pos = 0;
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_frame(canvas, x_pos, y_pos, 49, 64);
    canvas_draw_line(canvas, x_pos + 28, 0, x_pos + 28, 64);

    for(uint8_t i = 0; i < note_stack_size; i++) {
        if(i == 0) {
            canvas_draw_box(canvas, x_pos, y_pos + 48, 49, 16);
            canvas_set_color(canvas, ColorWhite);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }
        canvas_draw_str(canvas, x_pos + 4, 64 - 16 * i - 3, get_note_name(state->note_stack[i]));
        canvas_draw_str(
            canvas, x_pos + 31, 64 - 16 * i - 3, get_note_len_name(state->note_stack[i]));
        canvas_draw_line(canvas, x_pos, 64 - 16 * i, x_pos + 48, 64 - 16 * i);
    }

    release_mutex((ValueMutex*)ctx, state);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    osMessageQueueId_t event_queue = ctx;

    MusicDemoEvent event;
    event.type = EventTypeKey;
    event.value.input = *input_event;
    osMessageQueuePut(event_queue, &event, 0, 0);
}

void process_note(
    const MelodyEventRecord* note_record,
    float bar_length_ms,
    MusicDemoContext* context) {
    MusicDemoEvent event;
    // send note event
    event.type = EventTypeNote;
    event.value.note_record = note_record;
    osMessageQueuePut(context->event_queue, &event, 0, 0);

    // read volume
    State* state = (State*)acquire_mutex(context->state_mutex, 25);
    float volume = volumes[state->volume_id];
    release_mutex(context->state_mutex, state);

    // play note
    float note_delay = bar_length_ms / (float)note_record->length;
    if(note_record->note != N) {
        furi_hal_speaker_start(note_record->note, volume);
    }
    furi_hal_delay_ms(note_delay);
    furi_hal_speaker_stop();
}

void music_player_thread(void* p) {
    MusicDemoContext* context = (MusicDemoContext*)p;

    const float bpm = 130.0f;
    // 4/4
    const float bar_length_ms = (60.0f * 1000.0f / bpm) * 4;
    const uint16_t melody_start_events_count = sizeof(melody_start) / sizeof(melody_start[0]);
    const uint16_t melody_loop_events_count = sizeof(melody_loop) / sizeof(melody_loop[0]);

    for(size_t i = 0; i < melody_start_events_count; i++) {
        process_note(&melody_start[i], bar_length_ms, context);
    }

    while(1) {
        for(size_t i = 0; i < melody_loop_events_count; i++) {
            process_note(&melody_loop[i], bar_length_ms, context);
        }
    }
}

int32_t music_player_app(void* p) {
    osMessageQueueId_t event_queue = osMessageQueueNew(8, sizeof(MusicDemoEvent), NULL);

    State _state;
    _state.note_record = NULL;
    for(size_t i = 0; i < note_stack_size; i++) {
        _state.note_stack[i] = NULL;
    }
    _state.volume_id = 1;
    _state.volume_id_max = sizeof(volumes) / sizeof(volumes[0]);

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, &_state, sizeof(State))) {
        printf("cannot create mutex\r\n");
        return 255;
    }

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // start player thread
    // TODO change to fuirac_start
    osThreadAttr_t player_attr = {.name = "music_player_thread", .stack_size = 512};
    MusicDemoContext context = {.state_mutex = &state_mutex, .event_queue = event_queue};
    osThreadId_t player = osThreadNew(music_player_thread, &context, &player_attr);

    if(player == NULL) {
        printf("cannot create player thread\r\n");
        return 255;
    }

    MusicDemoEvent event;
    while(1) {
        osStatus_t event_status = osMessageQueueGet(event_queue, &event, NULL, 100);

        State* state = (State*)acquire_mutex_block(&state_mutex);

        if(event_status == osOK) {
            if(event.type == EventTypeKey) {
                // press events
                if(event.value.input.type == InputTypeShort &&
                   event.value.input.key == InputKeyBack) {
                    release_mutex(&state_mutex, state);
                    break;
                }

                if(event.value.input.type == InputTypePress &&
                   event.value.input.key == InputKeyUp) {
                    if(state->volume_id < state->volume_id_max - 1) state->volume_id++;
                }

                if(event.value.input.type == InputTypePress &&
                   event.value.input.key == InputKeyDown) {
                    if(state->volume_id > 0) state->volume_id--;
                }

                if(event.value.input.type == InputTypePress &&
                   event.value.input.key == InputKeyLeft) {
                }

                if(event.value.input.type == InputTypePress &&
                   event.value.input.key == InputKeyRight) {
                }

                if(event.value.input.key == InputKeyOk) {
                }

            } else if(event.type == EventTypeNote) {
                state->note_record = event.value.note_record;

                for(size_t i = note_stack_size - 1; i > 0; i--) {
                    state->note_stack[i] = state->note_stack[i - 1];
                }
                state->note_stack[0] = state->note_record;
            }
        } else {
            // event timeout
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, state);
    }

    osThreadTerminate(player);
    furi_hal_speaker_stop();
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    osMessageQueueDelete(event_queue);
    delete_mutex(&state_mutex);

    return 0;
}
