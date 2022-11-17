#include "dtmf_dolphin_data.h"

typedef struct {
    const uint8_t row;
    const uint8_t col;
    const uint8_t span;
} DTMFDolphinTonePos;

typedef struct {
    const char* name;
    const float frequency_1;
    const float frequency_2;
    const DTMFDolphinTonePos pos;
    const uint16_t pulses; // for Redbox
    const uint16_t pulse_ms; // for Redbox
    const uint16_t gap_duration; // for Redbox
} DTMFDolphinTones;

typedef struct {
    const char* name;
    DTMFDolphinToneSection block;
    uint8_t tone_count;
    DTMFDolphinTones tones[DTMF_DOLPHIN_MAX_TONE_COUNT];
} DTMFDolphinSceneData;

DTMFDolphinSceneData DTMFDolphinSceneDataDialer = {
    .name = "Dialer",
    .block = DTMF_DOLPHIN_TONE_BLOCK_DIALER,
    .tone_count = 16,
    .tones = {
        {"1", 697.0, 1209.0, {0, 0, 1}, 0, 0, 0},
        {"2", 697.0, 1336.0, {0, 1, 1}, 0, 0, 0},
        {"3", 697.0, 1477.0, {0, 2, 1}, 0, 0, 0},
        {"A", 697.0, 1633.0, {0, 3, 1}, 0, 0, 0},
        {"4", 770.0, 1209.0, {1, 0, 1}, 0, 0, 0},
        {"5", 770.0, 1336.0, {1, 1, 1}, 0, 0, 0},
        {"6", 770.0, 1477.0, {1, 2, 1}, 0, 0, 0},
        {"B", 770.0, 1633.0, {1, 3, 1}, 0, 0, 0},
        {"7", 852.0, 1209.0, {2, 0, 1}, 0, 0, 0},
        {"8", 852.0, 1336.0, {2, 1, 1}, 0, 0, 0},
        {"9", 852.0, 1477.0, {2, 2, 1}, 0, 0, 0},
        {"C", 852.0, 1633.0, {2, 3, 1}, 0, 0, 0},
        {"*", 941.0, 1209.0, {3, 0, 1}, 0, 0, 0},
        {"0", 941.0, 1336.0, {3, 1, 1}, 0, 0, 0},
        {"#", 941.0, 1477.0, {3, 2, 1}, 0, 0, 0},
        {"D", 941.0, 1633.0, {3, 3, 1}, 0, 0, 0},
    }};

DTMFDolphinSceneData DTMFDolphinSceneDataBluebox = {
    .name = "Bluebox",
    .block = DTMF_DOLPHIN_TONE_BLOCK_BLUEBOX,
    .tone_count = 13,
    .tones = {
        {"1", 700.0, 900.0, {0, 0, 1}, 0, 0, 0},
        {"2", 700.0, 1100.0, {0, 1, 1}, 0, 0, 0},
        {"3", 900.0, 1100.0, {0, 2, 1}, 0, 0, 0},
        {"4", 700.0, 1300.0, {1, 0, 1}, 0, 0, 0},
        {"5", 900.0, 1300.0, {1, 1, 1}, 0, 0, 0},
        {"6", 1100.0, 1300.0, {1, 2, 1}, 0, 0, 0},
        {"7", 700.0, 1500.0, {2, 0, 1}, 0, 0, 0},
        {"8", 900.0, 1500.0, {2, 1, 1}, 0, 0, 0},
        {"9", 1100.0, 1500.0, {2, 2, 1}, 0, 0, 0},
        {"0", 1300.0, 1500.0, {3, 1, 1}, 0, 0, 0},
        {"KP", 1100.0, 1700.0, {0, 3, 2}, 0, 0, 0},
        {"ST", 1500.0, 1700.0, {1, 3, 2}, 0, 0, 0},
        {"2600", 2600.0, 0.0, {3, 2, 3}, 0, 0, 0},
    }};

DTMFDolphinSceneData DTMFDolphinSceneDataRedboxUS = {
    .name = "Redbox (US)",
    .block = DTMF_DOLPHIN_TONE_BLOCK_REDBOX_US,
    .tone_count = 4,
    .tones = {
        {"Nickel", 1700.0, 2200.0, {0, 0, 5}, 1, 66, 0},
        {"Dime", 1700.0, 2200.0, {1, 0, 5}, 2, 66, 66},
        {"Quarter", 1700.0, 2200.0, {2, 0, 5}, 5, 33, 33},
        {"Dollar", 1700.0, 2200.0, {3, 0, 5}, 1, 650, 0},
    }};

DTMFDolphinSceneData DTMFDolphinSceneDataRedboxCA = {
    .name = "Redbox (CA)",
    .block = DTMF_DOLPHIN_TONE_BLOCK_REDBOX_CA,
    .tone_count = 3,
    .tones = {
        {"Nickel", 2200.0, 0.0, {0, 0, 5}, 1, 66, 0},
        {"Dime", 2200.0, 0.0, {1, 0, 5}, 2, 66, 66},
        {"Quarter", 2200.0, 0.0, {2, 0, 5}, 5, 33, 33},
    }};

DTMFDolphinSceneData DTMFDolphinSceneDataRedboxUK = {
    .name = "Redbox (UK)",
    .block = DTMF_DOLPHIN_TONE_BLOCK_REDBOX_UK,
    .tone_count = 2,
    .tones = {
        {"10p", 1000.0, 0.0, {0, 0, 5}, 1, 200, 0},
        {"50p", 1000.0, 0.0, {1, 0, 5}, 1, 350, 0},
    }};

DTMFDolphinSceneData DTMFDolphinSceneDataMisc = {
    .name = "Misc",
    .block = DTMF_DOLPHIN_TONE_BLOCK_MISC,
    .tone_count = 3,
    .tones = {
        {"CCITT 11", 700.0, 1700.0, {0, 0, 5}, 0, 0, 0},
        {"CCITT 12", 900.0, 1700.0, {1, 0, 5}, 0, 0, 0},
        {"CCITT KP2", 1300.0, 1700.0, {2, 0, 5}, 0, 0, 0},
    }};

DTMFDolphinToneSection current_section;
DTMFDolphinSceneData* current_scene_data;

void dtmf_dolphin_data_set_current_section(DTMFDolphinToneSection section) {
    current_section = section;

    switch(current_section) {
    case DTMF_DOLPHIN_TONE_BLOCK_BLUEBOX:
        current_scene_data = &DTMFDolphinSceneDataBluebox;
        break;
    case DTMF_DOLPHIN_TONE_BLOCK_REDBOX_US:
        current_scene_data = &DTMFDolphinSceneDataRedboxUS;
        break;
    case DTMF_DOLPHIN_TONE_BLOCK_REDBOX_CA:
        current_scene_data = &DTMFDolphinSceneDataRedboxCA;
        break;
    case DTMF_DOLPHIN_TONE_BLOCK_REDBOX_UK:
        current_scene_data = &DTMFDolphinSceneDataRedboxUK;
        break;
    case DTMF_DOLPHIN_TONE_BLOCK_MISC:
        current_scene_data = &DTMFDolphinSceneDataMisc;
        break;
    default: // DTMF_DOLPHIN_TONE_BLOCK_DIALER:
        current_scene_data = &DTMFDolphinSceneDataDialer;
        break;
    }
}

DTMFDolphinToneSection dtmf_dolphin_data_get_current_section() {
    return current_section;
}

DTMFDolphinSceneData* dtmf_dolphin_data_get_current_scene_data() {
    return current_scene_data;
}

bool dtmf_dolphin_data_get_tone_frequencies(float* freq1, float* freq2, uint8_t row, uint8_t col) {
    for(size_t i = 0; i < current_scene_data->tone_count; i++) {
        DTMFDolphinTones tones = current_scene_data->tones[i];
        if(tones.pos.row == row && tones.pos.col == col) {
            freq1[0] = tones.frequency_1;
            freq2[0] = tones.frequency_2;
            return true;
        }
    }
    return false;
}

bool dtmf_dolphin_data_get_filter_data(
    uint16_t* pulses,
    uint16_t* pulse_ms,
    uint16_t* gap_ms,
    uint8_t row,
    uint8_t col) {
    for(size_t i = 0; i < current_scene_data->tone_count; i++) {
        DTMFDolphinTones tones = current_scene_data->tones[i];
        if(tones.pos.row == row && tones.pos.col == col) {
            pulses[0] = tones.pulses;
            pulse_ms[0] = tones.pulse_ms;
            gap_ms[0] = tones.gap_duration;
            return true;
        }
    }
    return false;
}

const char* dtmf_dolphin_data_get_tone_name(uint8_t row, uint8_t col) {
    for(size_t i = 0; i < current_scene_data->tone_count; i++) {
        DTMFDolphinTones tones = current_scene_data->tones[i];
        if(tones.pos.row == row && tones.pos.col == col) {
            return tones.name;
        }
    }
    return NULL;
}

const char* dtmf_dolphin_data_get_current_section_name() {
    if(current_scene_data) {
        return current_scene_data->name;
    }
    return NULL;
}

void dtmf_dolphin_tone_get_max_pos(uint8_t* max_rows, uint8_t* max_cols, uint8_t* max_span) {
    max_rows[0] = 0;
    max_cols[0] = 0;
    max_span[0] = 0;
    uint8_t tmp_rowspan[5] = {0, 0, 0, 0, 0};
    for(size_t i = 0; i < current_scene_data->tone_count; i++) {
        DTMFDolphinTones tones = current_scene_data->tones[i];
        if(tones.pos.row > max_rows[0]) {
            max_rows[0] = tones.pos.row;
        }
        if(tones.pos.col > max_cols[0]) {
            max_cols[0] = tones.pos.col;
        }
        tmp_rowspan[tones.pos.row] += tones.pos.span;
        if(tmp_rowspan[tones.pos.row] > max_span[0]) max_span[0] = tmp_rowspan[tones.pos.row];
    }
    max_rows[0]++;
    max_cols[0]++;
}

uint8_t dtmf_dolphin_get_tone_span(uint8_t row, uint8_t col) {
    for(size_t i = 0; i < current_scene_data->tone_count; i++) {
        DTMFDolphinTones tones = current_scene_data->tones[i];
        if(tones.pos.row == row && tones.pos.col == col) {
            return tones.pos.span;
        }
    }
    return 0;
}
