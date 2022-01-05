#include "cyfral_decoder.h"
#include <furi.h>

void CyfralDecoder::reset_state() {
    state = State::WAIT_START_NIBBLE;
    bit_state = BitState::WAIT_FRONT_LOW;

    period_time = 0;
    bit_index = 0;
    ready = false;
    index = 0;

    key_data = 0;
    readed_nibble = 0;
    data_valid = true;
}

bool CyfralDecoder::nibble_valid(uint8_t data) {
    uint8_t data_value = data & 0x0F;

    switch(data_value) {
    case 0b1110:
    case 0b1101:
    case 0b1011:
    case 0b0111:
        return true;
        break;
    default:
        return false;
    }
}

CyfralDecoder::CyfralDecoder() {
    reset_state();
    max_period = 0;
}

void CyfralDecoder::process_front(bool polarity, uint32_t time) {
    bool readed;
    bool value;

    if(max_period == 0) {
        max_period = 230 * (SystemCoreClock / 1000000.0f);
    }

    if(ready) return;

    switch(state) {
    case State::WAIT_START_NIBBLE:
        // wait for start word
        if(process_bit(polarity, time, &readed, &value)) {
            if(readed) {
                readed_nibble = ((readed_nibble << 1) | value) & 0x0F;
                if(readed_nibble == 0b0001) {
                    readed_nibble = 0;
                    state = State::READ_NIBBLE;
                }
            }
        } else {
            reset_state();
        }

        break;
    case State::READ_NIBBLE:
        // read nibbles
        if(process_bit(polarity, time, &readed, &value)) {
            if(readed) {
                readed_nibble = (readed_nibble << 1) | value;

                bit_index++;

                //convert every nibble to 2-bit index
                if(bit_index == 4) {
                    switch(readed_nibble) {
                    case 0b1110:
                        key_data = (key_data << 2) | 0b11;
                        break;
                    case 0b1101:
                        key_data = (key_data << 2) | 0b10;
                        break;
                    case 0b1011:
                        key_data = (key_data << 2) | 0b01;
                        break;
                    case 0b0111:
                        key_data = (key_data << 2) | 0b00;
                        break;
                    default:
                        data_valid = false;
                        break;
                    }

                    readed_nibble = 0;
                    bit_index = 0;
                    index++;
                }

                // succefully read 8 nibbles
                if(index == 8) {
                    state = State::READ_STOP_NIBBLE;
                }
            }
        } else {
            reset_state();
        }
        break;
    case State::READ_STOP_NIBBLE:
        // read stop nibble
        if(process_bit(polarity, time, &readed, &value)) {
            if(readed) {
                readed_nibble = ((readed_nibble << 1) | value) & 0x0F;
                bit_index++;

                switch(bit_index) {
                case 0:
                case 1:
                case 2:
                case 3:
                    break;
                case 4:
                    if(readed_nibble == 0b0001) {
                        // validate data
                        if(data_valid) {
                            ready = true;
                        } else {
                            reset_state();
                        }
                    } else {
                        reset_state();
                    }
                    break;
                default:
                    reset_state();
                    break;
                }
            }
        } else {
            reset_state();
        }
        break;
    }
}

bool CyfralDecoder::process_bit(bool polarity, uint32_t time, bool* readed, bool* readed_value) {
    bool result = true;
    *readed = false;

    // bit start from low
    switch(bit_state) {
    case BitState::WAIT_FRONT_LOW:
        if(polarity == true) {
            period_time += time;

            *readed = true;
            if(period_time <= max_period) {
                if((period_time / 2) > time) {
                    *readed_value = false;
                } else {
                    *readed_value = true;
                }
            } else {
                result = false;
            }

            bit_state = BitState::WAIT_FRONT_HIGH;
        } else {
            result = false;
        }
        break;
    case BitState::WAIT_FRONT_HIGH:
        if(polarity == false) {
            period_time = time;
            bit_state = BitState::WAIT_FRONT_LOW;
        } else {
            result = false;
        }
        break;
    }

    return result;
}

bool CyfralDecoder::read(uint8_t* _data, uint8_t data_size) {
    furi_check(data_size <= 2);
    bool result = false;

    if(ready) {
        memcpy(_data, &key_data, data_size);
        reset_state();
        result = true;
    }

    return result;
}
