#include "message.h"

/** Returns the data within the MidiEvent as a NoteOffEvent struct */
NoteOffEvent AsNoteOff(MidiEvent* event) {
    NoteOffEvent m;
    m.channel = event->channel;
    m.note = event->data[0];
    m.velocity = event->data[1];
    return m;
}

/** Returns the data within the MidiEvent as a NoteOnEvent struct */
NoteOnEvent AsNoteOn(MidiEvent* event) {
    NoteOnEvent m;
    m.channel = event->channel;
    m.note = event->data[0];
    m.velocity = event->data[1];
    return m;
}

/** Returns the data within the MidiEvent as a PolyphonicKeyPressureEvent struct */
PolyphonicKeyPressureEvent AsPolyphonicKeyPressure(MidiEvent* event) {
    PolyphonicKeyPressureEvent m;
    m.channel = event->channel;
    m.note = event->data[0];
    m.pressure = event->data[1];
    return m;
}

/** Returns the data within the MidiEvent as a ControlChangeEvent struct.*/
ControlChangeEvent AsControlChange(MidiEvent* event) {
    ControlChangeEvent m;
    m.channel = event->channel;
    m.control_number = event->data[0];
    m.value = event->data[1];
    return m;
}

/** Returns the data within the MidiEvent as a ProgramChangeEvent struct.*/
ProgramChangeEvent AsProgramChange(MidiEvent* event) {
    ProgramChangeEvent m;
    m.channel = event->channel;
    m.program = event->data[0];
    return m;
}

/** Returns the data within the MidiEvent as a ProgramChangeEvent struct.*/
ChannelPressureEvent AsChannelPressure(MidiEvent* event) {
    ChannelPressureEvent m;
    m.channel = event->channel;
    m.pressure = event->data[0];
    return m;
}

/** Returns the data within the MidiEvent as a PitchBendEvent struct.*/
PitchBendEvent AsPitchBend(MidiEvent* event) {
    PitchBendEvent m;
    m.channel = event->channel;
    m.value = ((uint16_t)(event->data[1]) << 7) + (event->data[0] - 8192);
    return m;
}

SystemExclusiveEvent AsSystemExclusive(MidiEvent* event) {
    SystemExclusiveEvent m;
    m.length = event->sysex_message_len;
    for(int i = 0; i < SYSEX_BUFFER_LEN; i++) {
        m.data[i] = 0;
        if(i < m.length) {
            m.data[i] = event->sysex_data[i];
        }
    }
    return m;
}

MTCQuarterFrameEvent AsMTCQuarterFrame(MidiEvent* event) {
    MTCQuarterFrameEvent m;
    m.message_type = (event->data[0] & 0x70) >> 4;
    m.value = (event->data[0]) & 0x0f;
    return m;
}

SongPositionPointerEvent AsSongPositionPointer(MidiEvent* event) {
    SongPositionPointerEvent m;
    m.position = ((uint16_t)(event->data[1]) << 7) | (event->data[0]);
    return m;
}

SongSelectEvent AsSongSelect(MidiEvent* event) {
    SongSelectEvent m;
    m.song = event->data[0];
    return m;
}

AllSoundOffEvent AsAllSoundOff(MidiEvent* event) {
    AllSoundOffEvent m;
    m.channel = event->channel;
    return m;
}

ResetAllControllersEvent AsResetAllControllers(MidiEvent* event) {
    ResetAllControllersEvent m;
    m.channel = event->channel;
    m.value = event->data[1];
    return m;
}

LocalControlEvent AsLocalControl(MidiEvent* event) {
    LocalControlEvent m;
    m.channel = event->channel;
    m.local_control_off = (event->data[1] == 0);
    m.local_control_on = (event->data[1] == 127);
    return m;
}

AllNotesOffEvent AsAllNotesOff(MidiEvent* event) {
    AllNotesOffEvent m;
    m.channel = event->channel;
    return m;
}

OmniModeOffEvent AsOmniModeOff(MidiEvent* event) {
    OmniModeOffEvent m;
    m.channel = event->channel;
    return m;
}

OmniModeOnEvent AsOmniModeOn(MidiEvent* event) {
    OmniModeOnEvent m;
    m.channel = event->channel;
    return m;
}

MonoModeOnEvent AsMonoModeOn(MidiEvent* event) {
    MonoModeOnEvent m;
    m.channel = event->channel;
    m.num_channels = event->data[1];
    return m;
}

PolyModeOnEvent AsPolyModeOn(MidiEvent* event) {
    PolyModeOnEvent m;
    m.channel = event->channel;
    return m;
}