#include <furi.h>
#include <furi_hal.h>
#include "usb/usb_midi_driver.h"
#include "midi/parser.h"
#include "midi/usb_message.h"
#include <math.h>

float note_to_frequency(int note) {
    float a = 440;
    return (a / 32) * powf(2, ((note - 9) / 12.0));
}

typedef enum {
    MidiThreadEventStop = (1 << 0),
    MidiThreadEventRx = (1 << 1),
    MidiThreadEventAll = MidiThreadEventStop | MidiThreadEventRx,
} MidiThreadEvent;

static void midi_rx_callback(void* context) {
    furi_assert(context);
    FuriThreadId thread_id = (FuriThreadId)context;
    furi_thread_flags_set(thread_id, MidiThreadEventRx);
}

int32_t usb_midi_app(void* p) {
    UNUSED(p);

    FuriHalUsbInterface* usb_config_prev;
    usb_config_prev = furi_hal_usb_get_config();
    midi_usb_set_context(furi_thread_get_id(furi_thread_get_current()));
    midi_usb_set_rx_callback(midi_rx_callback);
    furi_hal_usb_set_config(&midi_usb_interface, NULL);

    MidiParser* parser = midi_parser_alloc();
    uint32_t events;
    uint8_t current_note = 255;

    while(1) {
        events = furi_thread_flags_wait(MidiThreadEventAll, FuriFlagWaitAny, FuriWaitForever);

        if(!(events & FuriFlagError)) {
            if(events & MidiThreadEventRx) {
                uint8_t buffer[64];
                size_t size = midi_usb_rx(buffer, sizeof(buffer));
                // loopback
                // midi_usb_tx(buffer, size);
                size_t start = 0;
                while(start < size) {
                    CodeIndex code_index = code_index_from_data(buffer[start]);
                    uint8_t data_size = usb_message_data_size(code_index);
                    if(data_size == 0) break;

                    start += 1;
                    for(size_t j = 0; j < data_size; j++) {
                        if(midi_parser_parse(parser, buffer[start + j])) {
                            MidiEvent* event = midi_parser_get_message(parser);
                            if(event->type == NoteOn) {
                                NoteOnEvent note_on = AsNoteOn(event);
                                current_note = note_on.note;
                                if(furi_hal_speaker_is_mine() || furi_hal_speaker_acquire(30)) {
                                    furi_hal_speaker_start(
                                        note_to_frequency(note_on.note),
                                        note_on.velocity / 127.0f);
                                }
                            } else if(event->type == NoteOff) {
                                NoteOffEvent note_off = AsNoteOff(event);
                                if(note_off.note == current_note && furi_hal_speaker_is_mine()) {
                                    furi_hal_speaker_stop();
                                    furi_hal_speaker_release();
                                }
                            }
                        }
                    }
                    start += data_size;
                }
            }
        }
    }

    midi_parser_free(parser);
    furi_hal_usb_set_config(usb_config_prev, NULL);

    return 0;
}