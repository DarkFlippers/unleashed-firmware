#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"

typedef enum {
    NoteOff, /**< & */
    NoteOn, /**< & */
    PolyphonicKeyPressure, /**< & */
    ControlChange, /**< & */
    ProgramChange, /**< & */
    ChannelPressure, /**< & */
    PitchBend, /**< & */
    SystemCommon, /**< & */
    SystemRealTime, /**< & */
    ChannelMode, /**< & */
    MessageLast, /**< & */
} MidiMessageType;

typedef enum {
    SystemExclusive, /**< & */
    MTCQuarterFrame, /**< & */
    SongPositionPointer, /**< & */
    SongSelect, /**< & */
    SCUndefined0, /**< & */
    SCUndefined1, /**< & */
    TuneRequest, /**< & */
    SysExEnd, /**< & */
    SystemCommonLast, /**< & */
} SystemCommonType;

typedef enum {
    TimingClock, /**< & */
    SRTUndefined0, /**< & */
    Start, /**< & */
    Continue, /**< & */
    Stop, /**< & */
    SRTUndefined1, /**< & */
    ActiveSensing, /**< & */
    Reset, /**< & */
    SystemRealTimeLast, /**< & */
} SystemRealTimeType;

typedef enum {
    AllSoundOff, /**< & */
    ResetAllControllers, /**< & */
    LocalControl, /**< & */
    AllNotesOff, /**< & */
    OmniModeOff, /**< & */
    OmniModeOn, /**< & */
    MonoModeOn, /**< & */
    PolyModeOn, /**< & */
    ChannelModeLast, /**< & */
} ChannelModeType;

/** Struct containing note, and velocity data for a given channel.
Can be made from MidiEvent
*/
typedef struct {
    int channel; /**< & */
    uint8_t note; /**< & */
    uint8_t velocity; /**< & */
} NoteOffEvent;

/** Struct containing note, and velocity data for a given channel.
Can be made from MidiEvent
*/
typedef struct {
    int channel; /**< & */
    uint8_t note; /**< & */
    uint8_t velocity; /**< & */
} NoteOnEvent;

/** Struct containing note, and pressure data for a given channel.
Can be made from MidiEvent
*/
typedef struct {
    int channel;
    uint8_t note;
    uint8_t pressure;
} PolyphonicKeyPressureEvent;

/** Struct containing control number, and value for a given channel.
Can be made from MidiEvent
*/
typedef struct {
    int channel; /**< & */
    uint8_t control_number; /**< & */
    uint8_t value; /**< & */
} ControlChangeEvent;

/** Struct containing new program number, for a given channel.
Can be made from MidiEvent
*/
typedef struct {
    int channel; /**< & */
    uint8_t program; /**< & */
} ProgramChangeEvent;

/** Struct containing pressure (aftertouch), for a given channel.
Can be made from MidiEvent
*/
typedef struct {
    int channel; /**< & */
    uint8_t pressure; /**< & */
} ChannelPressureEvent;

/** Struct containing pitch bend value for a given channel.
Can be made from MidiEvent
*/
typedef struct {
    int channel; /**< & */
    int16_t value; /**< & */
} PitchBendEvent;

/** Struct containing sysex data.
Can be made from MidiEvent
*/
typedef struct {
    int length;
    uint8_t data[SYSEX_BUFFER_LEN]; /**< & */
} SystemExclusiveEvent;

/** Struct containing QuarterFrame data.
Can be made from MidiEvent
*/
typedef struct {
    uint8_t message_type; /**< & */
    uint8_t value; /**< & */
} MTCQuarterFrameEvent;

/** Struct containing song position data.
Can be made from MidiEvent
*/
typedef struct {
    uint16_t position; /**< & */
} SongPositionPointerEvent;

/** Struct containing song select data.
Can be made from MidiEvent
*/
typedef struct {
    uint8_t song; /**< & */
} SongSelectEvent;

/** Struct containing sound off data.
Can be made from MidiEvent
*/
typedef struct {
    int channel; /**< & */
} AllSoundOffEvent;

/** Struct containing ResetAllControllersEvent data.
Can be made from MidiEvent
*/
typedef struct {
    int channel; /**< & */
    uint8_t value; /**< & */
} ResetAllControllersEvent;

/** Struct containing LocalControlEvent data.
Can be made from MidiEvent
*/
typedef struct {
    int channel; /**< & */
    bool local_control_off; /**< & */
    bool local_control_on; /**< & */
} LocalControlEvent;

/** Struct containing AllNotesOffEvent data.
Can be made from MidiEvent
*/
typedef struct {
    int channel; /**< & */
} AllNotesOffEvent;

/** Struct containing OmniModeOffEvent data. 
 * Can be made from MidiEvent
*/
typedef struct {
    int channel; /**< & */
} OmniModeOffEvent;

/** Struct containing OmniModeOnEvent data.
Can be made from MidiEvent
*/
typedef struct {
    int channel; /**< & */
} OmniModeOnEvent;

/** Struct containing MonoModeOnEvent data.
Can be made from MidiEvent
*/
typedef struct {
    int channel; /**< & */
    uint8_t num_channels; /**< & */
} MonoModeOnEvent;

/** Struct containing PolyModeOnEvent data.
Can be made from MidiEvent
*/
typedef struct {
    int channel; /**< & */
} PolyModeOnEvent;

/** Simple MidiEvent with message type, channel, and data[2] members.
*/
typedef struct {
    MidiMessageType type;
    int channel;
    uint8_t data[2];
    uint8_t sysex_data[SYSEX_BUFFER_LEN];
    uint8_t sysex_message_len;
    SystemCommonType sc_type;
    SystemRealTimeType srt_type;
    ChannelModeType cm_type;
} MidiEvent;

/** Returns the data within the MidiEvent as a NoteOffEvent struct */
NoteOffEvent AsNoteOff(MidiEvent* event);

/** Returns the data within the MidiEvent as a NoteOnEvent struct */
NoteOnEvent AsNoteOn(MidiEvent* event);

/** Returns the data within the MidiEvent as a PolyphonicKeyPressureEvent struct */
PolyphonicKeyPressureEvent AsPolyphonicKeyPressure(MidiEvent* event);

/** Returns the data within the MidiEvent as a ControlChangeEvent struct.*/
ControlChangeEvent AsControlChange(MidiEvent* event);

/** Returns the data within the MidiEvent as a ProgramChangeEvent struct.*/
ProgramChangeEvent AsProgramChange(MidiEvent* event);

/** Returns the data within the MidiEvent as a ProgramChangeEvent struct.*/
ChannelPressureEvent AsChannelPressure(MidiEvent* event);

/** Returns the data within the MidiEvent as a PitchBendEvent struct.*/
PitchBendEvent AsPitchBend(MidiEvent* event);

SystemExclusiveEvent AsSystemExclusive(MidiEvent* event);
MTCQuarterFrameEvent AsMTCQuarterFrame(MidiEvent* event);
SongPositionPointerEvent AsSongPositionPointer(MidiEvent* event);
SongSelectEvent AsSongSelect(MidiEvent* event);
AllSoundOffEvent AsAllSoundOff(MidiEvent* event);
ResetAllControllersEvent AsResetAllControllers(MidiEvent* event);
LocalControlEvent AsLocalControl(MidiEvent* event);
AllNotesOffEvent AsAllNotesOff(MidiEvent* event);
OmniModeOffEvent AsOmniModeOff(MidiEvent* event);
OmniModeOnEvent AsOmniModeOn(MidiEvent* event);
MonoModeOnEvent AsMonoModeOn(MidiEvent* event);
PolyModeOnEvent AsPolyModeOn(MidiEvent* event);