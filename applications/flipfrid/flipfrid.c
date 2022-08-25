#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>

#include <lib/lfrfid/lfrfid_worker.h>
#include "raw_em4100.h"
#include <lfrfid/protocols/lfrfid_protocols.h>

#include "flipfrid.h"

#define NUMBER_OF_ATTACKS 3
#define TIME_BETWEEN_CARDS \
    5 // Emulate 2 cards per second : (5 * (configTICK_RATE_HZ_RAW/10)) == (5*(1000/10)) == (5*100) == (500)ms
#define TAG "FLIPFRID"

uint8_t id_list[16][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Null bytes
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, // Only FF
    {0x11, 0x11, 0x11, 0x11, 0x11}, // Only 11
    {0x22, 0x22, 0x22, 0x22, 0x22}, // Only 22
    {0x33, 0x33, 0x33, 0x33, 0x33}, // Only 33
    {0x44, 0x44, 0x44, 0x44, 0x44}, // Only 44
    {0x55, 0x55, 0x55, 0x55, 0x55}, // Only 55
    {0x66, 0x66, 0x66, 0x66, 0x66}, // Only 66
    {0x77, 0x77, 0x77, 0x77, 0x77}, // Only 77
    {0x88, 0x88, 0x88, 0x88, 0x88}, // Only 88
    {0x99, 0x99, 0x99, 0x99, 0x99}, // Only 99
    {0x12, 0x34, 0x56, 0x78, 0x9A}, // Incremental UID
    {0x04, 0xd0, 0x9b, 0x0d, 0x6a}, // From arha
    {0x34, 0x00, 0x29, 0x3d, 0x9e}, // From arha
    {0x04, 0xdf, 0x00, 0x00, 0x01}, // From arha
    {0xCA, 0xCA, 0xCA, 0xCA, 0xCA}, // From arha
};

typedef enum {
    DefaultKeys,
    BruteForceCustomerId,
    BadCrc,
} AttackType;

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType evt_type;
    InputKey key;
    InputType input_type;
} FlipFridEvent;

// STRUCTS
typedef struct {
    bool emitting;
    AttackType current_attack_type;
    uint8_t* current_uid;
    uint8_t meta_data;
    NotificationApp* notify;
} FlipFridState;

static void flipfrid_draw_callback(Canvas* const canvas, void* ctx) {
    const FlipFridState* flipfrid_state = (FlipFridState*)acquire_mutex((ValueMutex*)ctx, 100);

    if(flipfrid_state == NULL) {
        return;
    }

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    // Frame
    canvas_draw_frame(canvas, 0, 0, 128, 64);

    // Title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 8, AlignCenter, AlignTop, "Flip/Frid");

    // Badge Type
    char uid[16];
    char badge_type[21];
    switch(flipfrid_state->current_attack_type) {
    case DefaultKeys:
        strcpy(badge_type, "   Default Values  >");
        break;
    case BruteForceCustomerId:
        strcpy(badge_type, "<  BF Customer ID  >");
        break;
    case BadCrc:
        strcpy(badge_type, "<      Bad CRC      ");
        break;
    default:
        break;
    }

    if(flipfrid_state->current_attack_type == BruteForceCustomerId) {
        snprintf(uid, sizeof(uid), "    ID : %2X    ", flipfrid_state->current_uid[0]);
    } else if(flipfrid_state->current_attack_type == BadCrc) {
        snprintf(uid, sizeof(uid), "Sending packets");
    } else {
        snprintf(
            uid,
            sizeof(uid),
            "%2X:%2X:%2X:%2X:%2X",
            flipfrid_state->current_uid[0],
            flipfrid_state->current_uid[1],
            flipfrid_state->current_uid[2],
            flipfrid_state->current_uid[3],
            flipfrid_state->current_uid[4]);
    }

    // Badge infos
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 28, AlignCenter, AlignCenter, badge_type);

    if(flipfrid_state->emitting) {
        canvas_draw_str_aligned(canvas, 64, 42, AlignCenter, AlignCenter, uid);
    } else {
        canvas_draw_str_aligned(
            canvas, 64, 42, AlignCenter, AlignCenter, "Press OK to start/stop");
    }

    release_mutex((ValueMutex*)ctx, flipfrid_state);
}

void flipfrid_input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    FlipFridEvent event = {
        .evt_type = EventTypeKey, .key = input_event->key, .input_type = input_event->type};
    furi_message_queue_put(event_queue, &event, 25);
}

static void flipfrid_timer_callback(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    FlipFridEvent event = {
        .evt_type = EventTypeTick, .key = InputKeyUp, .input_type = InputTypeRelease};
    furi_message_queue_put(event_queue, &event, 25);
}

// ENTRYPOINT
int32_t flipfrid_start(void* p) {
    UNUSED(p);

    // Input
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(FlipFridEvent));
    FlipFridState* flipfrid_state = (FlipFridState*)malloc(sizeof(FlipFridState));
    ValueMutex flipfrid_state_mutex;

    // Mutex
    if(!init_mutex(&flipfrid_state_mutex, flipfrid_state, sizeof(FlipFridState))) {
        furi_message_queue_free(event_queue);
        free(flipfrid_state);
    }

    // Configure view port
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, flipfrid_draw_callback, &flipfrid_state_mutex);
    view_port_input_callback_set(view_port, flipfrid_input_callback, event_queue);

    // Configure timer
    FuriTimer* timer =
        furi_timer_alloc(flipfrid_timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_kernel_get_tick_frequency() / 10); // configTICK_RATE_HZ_RAW 1000

    // Register view port in GUI
    Gui* gui = (Gui*)furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Init values
    FlipFridEvent event;
    flipfrid_state->emitting = false;
    flipfrid_state->current_uid = id_list[0];
    flipfrid_state->current_attack_type = DefaultKeys;
    flipfrid_state->meta_data = 0;
    flipfrid_state->notify = furi_record_open(RECORD_NOTIFICATION);

    // RFID Configuration
    size_t data_size = 5; // Default EM4100 data size is 5 (1 customer id + 4 uid)
    LFRFIDWorker* worker;
    const ProtocolBase* lfrfid_protocol[] = {&protocol_raw_em4100, &protocol_raw_wrong_crc_em4100};
    ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocol, 2);
    worker = lfrfid_worker_alloc(dict);
    uint8_t selectedProtocol = CLEAN;

    // Application state
    int menu_selected_item_index = 0; // Menu current item index
    uint8_t counter = 0; // Used to count the step and wqaiting time between each test
    uint8_t attack_state = 0; // Used to store the current attack state
    uint8_t candidate[5]; // uid candidate
    bool running = true; // Used to stop the application

    // Main loop
    while(running) {
        // Get next event
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 25);
        if(event_status == FuriStatusOk) {
            if(event.evt_type == EventTypeKey) {
                if(event.input_type == InputTypeShort) {
                    counter = 0;
                    switch(event.key) {
                    case InputKeyUp:
                    case InputKeyDown:
                        // OSEF
                        break;
                    case InputKeyRight:
                        // Next badge type
                        flipfrid_state->emitting = false;
                        attack_state = 0;
                        notification_message(flipfrid_state->notify, &sequence_blink_stop);
                        if(menu_selected_item_index < (NUMBER_OF_ATTACKS - 1)) {
                            menu_selected_item_index++;
                            flipfrid_state->current_attack_type =
                                (AttackType)menu_selected_item_index;
                        }
                        break;
                    case InputKeyLeft:
                        // Previous badge type
                        flipfrid_state->emitting = false;
                        attack_state = 0;
                        notification_message(flipfrid_state->notify, &sequence_blink_stop);
                        if(menu_selected_item_index > 0) {
                            menu_selected_item_index--;
                            flipfrid_state->current_attack_type =
                                (AttackType)menu_selected_item_index;
                        }
                        break;
                    case InputKeyOk:
                        if(flipfrid_state->emitting) {
                            flipfrid_state->emitting = false;
                            attack_state = 0;
                            // TODO FIX BLINK
                            notification_message(flipfrid_state->notify, &sequence_blink_stop);
                        } else {
                            flipfrid_state->emitting = true;
                            attack_state = 0;
                            // TODO FIX BLINK
                            notification_message(
                                flipfrid_state->notify, &sequence_blink_start_blue);
                        }
                        break;
                    case InputKeyBack:
                        notification_message(flipfrid_state->notify, &sequence_blink_stop);
                        flipfrid_state->emitting = false;
                        running = false;
                        break;
                    }
                }
            } else if(event.evt_type == EventTypeTick) {
                // Emulate card
                if(flipfrid_state->emitting) {
                    if(1 == counter) {
                        protocol_dict_set_data(
                            dict, (LFRFIDProtocol)0, flipfrid_state->current_uid, data_size);
                        worker = lfrfid_worker_alloc(dict);
                        lfrfid_worker_start_thread(worker);
                        lfrfid_worker_emulate_start(worker, (LFRFIDProtocol)selectedProtocol);
                    } else if(0 == counter) {
                        lfrfid_worker_stop(worker);
                        lfrfid_worker_stop_thread(worker);
                        // set next value
                        switch(flipfrid_state->current_attack_type) {
                        case DefaultKeys: {
                            selectedProtocol = CLEAN;
                            data_size = 5;
                            flipfrid_state->current_uid = id_list[attack_state];
                            attack_state = attack_state + 1;
                            if(attack_state >= sizeof(id_list) / sizeof(id_list[0])) {
                                attack_state = 0;
                            }
                            break;
                        }
                        case BruteForceCustomerId: {
                            data_size = 5;
                            selectedProtocol = CLEAN;
                            candidate[0] = attack_state;
                            flipfrid_state->current_uid = candidate;
                            attack_state = attack_state + 1;
                            if((attack_state + 1) == 256) {
                                attack_state = 0;
                            }
                            break;
                        }
                        case BadCrc: {
                            selectedProtocol = BAD_CRC;
                            data_size = 5;
                            candidate[0] = 0xFF;
                            candidate[1] = 0xDE;
                            candidate[2] = 0xAD;
                            candidate[3] = 0xBE;
                            candidate[4] = 0xEF;
                            flipfrid_state->current_uid = candidate;
                            break;
                        }
                        }
                    }
                    if(counter > TIME_BETWEEN_CARDS) {
                        counter = 0;
                    } else {
                        counter++;
                    }
                }
                view_port_update(view_port);
            }
        }
    }

    // Cleanup
    notification_message(flipfrid_state->notify, &sequence_blink_stop);
    lfrfid_worker_stop(worker);
    lfrfid_worker_stop_thread(worker);
    lfrfid_worker_free(worker);
    protocol_dict_free(dict);
    furi_timer_stop(timer);
    furi_timer_free(timer);
    free(flipfrid_state);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_record_close(RECORD_GUI);
    return 0;
}