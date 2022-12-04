#pragma once
#include <stdint.h>

typedef enum {
    CodeIndexMisc = 0x0, /**< Reserved,  MIDI Size: 1, 2, 3 */
    CodeIndexCableEvent = 0x1, /**< Reserved, MIDI Size: 1, 2, 3 */
    CodeIndexSysEx2Byte = 0x2, /**< MIDI Size: 2 */
    CodeIndexSysEx3Byte = 0x3, /**< MIDI Size: 3 */
    CodeIndexSysExStart = 0x4, /**< MIDI Size: 3 */
    CodeIndexCommon1Byte = 0x5, /**< MIDI Size: 1 */
    CodeIndexSysExEnd1Byte = 0x5, /**< MIDI Size: 1 */
    CodeIndexSysExEnd2Byte = 0x6, /**< MIDI Size: 2 */
    CodeIndexSysExEnd3Byte = 0x7, /**< MIDI Size: 3 */
    CodeIndexNoteOff = 0x8, /**< MIDI Size: 3 */
    CodeIndexNoteOn = 0x9, /**< MIDI Size: 3 */
    CodeIndexPolyKeyPress = 0xA, /**< MIDI Size: 3 */
    CodeIndexControlChange = 0xB, /**< MIDI Size: 3 */
    CodeIndexProgramChange = 0xC, /**< MIDI Size: 2 */
    CodeIndexChannelPressure = 0xD, /**< MIDI Size: 2 */
    CodeIndexPitchBendChange = 0xE, /**< MIDI Size: 3 */
    CodeIndexSingleByte = 0xF, /**< MIDI Size: 1 */
} CodeIndex;

CodeIndex code_index_from_data(uint8_t data);

uint8_t cable_id_from_data(uint8_t data);

uint8_t usb_message_data_size(CodeIndex code_index);