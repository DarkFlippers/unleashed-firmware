#include "usb_message.h"

CodeIndex code_index_from_data(uint8_t data) {
    return (CodeIndex)(data & 0b00001111);
}

uint8_t cable_id_from_data(uint8_t data) {
    return (data >> 4) & 0b00001111;
}

uint8_t usb_message_data_size(CodeIndex code_index) {
    uint8_t data_size = 0;
    switch(code_index) {
    case CodeIndexCommon1Byte:
    /* case CodeIndexSysExEnd1Byte: */
    case CodeIndexSingleByte:
        data_size = 1;
        break;
    case CodeIndexSysEx2Byte:
    case CodeIndexSysExEnd2Byte:
    case CodeIndexProgramChange:
    case CodeIndexChannelPressure:
        data_size = 2;
        break;
    case CodeIndexSysEx3Byte:
    case CodeIndexSysExStart:
    case CodeIndexSysExEnd3Byte:
    case CodeIndexNoteOff:
    case CodeIndexNoteOn:
    case CodeIndexPolyKeyPress:
    case CodeIndexControlChange:
    case CodeIndexPitchBendChange:
        data_size = 3;
        break;
    default:
        break;
    }

    return data_size;
}