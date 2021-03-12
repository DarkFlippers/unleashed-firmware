#include "metakom-decoder.h"
#include <furi.h>

bool MetakomDecoder::read(uint8_t* _data, uint8_t data_size) {
    bool result = false;

    if(ready) {
        memcpy(_data, &key_data, 4);
        reset_state();
        result = true;
    }

    return result;
}

void MetakomDecoder::process_front(bool polarity, uint32_t time) {
    if(max_period == 0) {
        max_period = 230 * (SystemCoreClock / 1000000.0f);
    }

    if(ready) return;

    uint32_t high_time = 0;
    uint32_t low_time = 0;

    switch(state) {
    case State::WAIT_PERIOD_SYNC:
        if(process_bit(polarity, time, &high_time, &low_time)) {
            period_sample_data[period_sample_index] = high_time + low_time;
            period_sample_index++;

            if(period_sample_index == period_sample_count) {
                for(uint8_t i = 0; i < period_sample_count; i++) {
                    period_time += period_sample_data[i];
                };
                period_time /= period_sample_count;

                state = State::WAIT_START_BIT;
            }
        }

        break;
    case State::WAIT_START_BIT:
        if(process_bit(polarity, time, &high_time, &low_time)) {
            tmp_counter++;
            if(high_time > period_time) {
                tmp_counter = 0;
                state = State::WAIT_START_WORD;
            }

            if(tmp_counter > 40) {
                reset_state();
            }
        }

        break;
    case State::WAIT_START_WORD:
        if(process_bit(polarity, time, &high_time, &low_time)) {
            if(low_time < (period_time / 2)) {
                tmp_data = (tmp_data << 1) | 0b0;
            } else {
                tmp_data = (tmp_data << 1) | 0b1;
            }
            tmp_counter++;

            if(tmp_counter == 3) {
                if(tmp_data == 0b010) {
                    tmp_counter = 0;
                    tmp_data = 0;
                    state = State::READ_WORD;
                } else {
                    reset_state();
                }
            }
        }
        break;
    case State::READ_WORD:
        if(process_bit(polarity, time, &high_time, &low_time)) {
            if(low_time < (period_time / 2)) {
                tmp_data = (tmp_data << 1) | 0b0;
            } else {
                tmp_data = (tmp_data << 1) | 0b1;
            }
            tmp_counter++;

            if(tmp_counter == 8) {
                if(parity_check(tmp_data)) {
                    key_data = (key_data << 8) | tmp_data;
                    key_data_index++;
                    tmp_data = 0;
                    tmp_counter = 0;

                    if(key_data_index == 4) {
                        // check for stop bit
                        if(high_time > period_time) {
                            state = State::READ_STOP_WORD;
                        } else {
                            reset_state();
                        }
                    }
                } else {
                    reset_state();
                }
            }
        }
        break;
    case State::READ_STOP_WORD:
        if(process_bit(polarity, time, &high_time, &low_time)) {
            if(low_time < (period_time / 2)) {
                tmp_data = (tmp_data << 1) | 0b0;
            } else {
                tmp_data = (tmp_data << 1) | 0b1;
            }
            tmp_counter++;

            if(tmp_counter == 3) {
                if(tmp_data == 0b010) {
                    ready = true;
                } else {
                    reset_state();
                }
            }
        }
        break;
    }
}

MetakomDecoder::MetakomDecoder() {
    reset_state();
}

void MetakomDecoder::reset_state() {
    ready = false;
    period_sample_index = 0;
    period_time = 0;

    tmp_counter = 0;
    tmp_data = 0;

    for(uint8_t i = 0; i < period_sample_count; i++) {
        period_sample_data[i] = 0;
    };

    state = State::WAIT_PERIOD_SYNC;
    bit_state = BitState::WAIT_FRONT_LOW;

    key_data = 0;
    key_data_index = 0;
}

bool MetakomDecoder::parity_check(uint8_t data) {
    uint8_t ones_count = 0;
    bool result;

    for(uint8_t i = 0; i < 8; i++) {
        if((data >> i) & 0b00000001) {
            ones_count++;
        }
    }

    result = (ones_count % 2 == 0);

    return result;
}

bool MetakomDecoder::process_bit(
    bool polarity,
    uint32_t time,
    uint32_t* high_time,
    uint32_t* low_time) {
    bool result = false;

    switch(bit_state) {
    case BitState::WAIT_FRONT_LOW:
        if(polarity == false) {
            *low_time = low_time_storage;
            *high_time = time;
            result = true;
            bit_state = BitState::WAIT_FRONT_HIGH;
        }
        break;
    case BitState::WAIT_FRONT_HIGH:
        if(polarity == true) {
            low_time_storage = time;
            bit_state = BitState::WAIT_FRONT_LOW;
        }
        break;
    }

    return result;
}