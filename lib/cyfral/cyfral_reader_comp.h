#pragma once
#include <furi.h>
#include "callback-connector.h"
#include <atomic>

enum class CyfralReaderCompError : uint8_t {
    NO_ERROR = 0,
    UNABLE_TO_DETECT = 1,
    RAW_DATA_SIZE_ERROR = 2,
    UNKNOWN_NIBBLE_VALUE = 3,
    NO_START_NIBBLE = 4,
    NOT_ENOUGH_DATA = 5,
};

extern COMP_HandleTypeDef hcomp1;

typedef struct {
    bool value;
    uint32_t dwt_value;
} CompEvent;

class CyfralReaderComp {
private:
    bool capture_data(bool* data, uint16_t capture_size);
    bool parse_data(bool* raw_data, uint16_t capture_size, uint8_t* data, uint8_t count);
    uint32_t search_array_in_array(
        const bool* haystack,
        const uint32_t haystack_size,
        const bool* needle,
        const uint32_t needle_size);

    // key is 9 nibbles
    static const uint16_t bits_in_nibble = 4;
    static const uint16_t key_length = 9;
    static const uint32_t capture_size = key_length * bits_in_nibble * 2;
    CyfralReaderCompError error;
    const GpioPin* pin_record;

    std::atomic<bool> ready_to_process;
    void comparator_trigger_callback(void* hcomp, void* comp_ctx);
    osMessageQueueId_t comp_event_queue;

public:
    CyfralReaderComp(const GpioPin* emulate_pin);
    ~CyfralReaderComp();
    void start(void);
    void stop(void);
    bool read(uint8_t* data, uint8_t count);
};

bool CyfralReaderComp::capture_data(bool* data, uint16_t capture_size) {
    uint32_t prev_timing = 0;
    uint16_t data_index = 0;
    CompEvent event_0, event_1;
    osStatus_t status;

    // read first event to get initial timing
    status = osMessageQueueGet(comp_event_queue, &event_0, NULL, 0);

    if(status != osOK) {
        return false;
    }

    prev_timing = event_0.dwt_value;

    // read second event until we get 0
    while(1) {
        status = osMessageQueueGet(comp_event_queue, &event_0, NULL, 0);
        if(status != osOK) {
            return false;
        }
        prev_timing = event_0.dwt_value;
        if(event_0.value == 0) break;
    }

    while(1) {
        // if event "zero" correct
        if(status == osOK && event_0.value == 0) {
            // get timing
            event_0.dwt_value -= prev_timing;
            prev_timing += event_0.dwt_value;

            // read next event
            status = osMessageQueueGet(comp_event_queue, &event_1, NULL, 0);

            // if event "one" correct
            if(status == osOK && event_1.value == 1) {
                // get timing
                event_1.dwt_value -= prev_timing;
                prev_timing += event_1.dwt_value;

                // calculate percentage of event "one" to full timing
                uint32_t full_timing = event_0.dwt_value + event_1.dwt_value;
                uint32_t percentage_1 = 1000000 / full_timing * event_1.dwt_value;

                // write captured data
                data[data_index] = percentage_1 > 500000 ? 0 : 1;
                data_index++;
                if(data_index >= capture_size) return true;

                status = osMessageQueueGet(comp_event_queue, &event_0, NULL, 0);
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    osMessageQueueReset(comp_event_queue);
}

uint32_t CyfralReaderComp::search_array_in_array(
    const bool* haystack,
    const uint32_t haystack_size,
    const bool* needle,
    const uint32_t needle_size) {
    uint32_t haystack_index = 0, needle_index = 0;

    while(haystack_index < haystack_size && needle_index < needle_size) {
        if(haystack[haystack_index] == needle[needle_index]) {
            haystack_index++;
            needle_index++;
            if(needle_index == needle_size) {
                return (haystack_index - needle_size);
            };
        } else {
            haystack_index = haystack_index - needle_index + 1;
            needle_index = 0;
        }
    }

    return haystack_index;
}

void CyfralReaderComp::comparator_trigger_callback(void* hcomp, void* comp_ctx) {
    CyfralReaderComp* _this = static_cast<CyfralReaderComp*>(comp_ctx);
    COMP_HandleTypeDef* _hcomp = static_cast<COMP_HandleTypeDef*>(hcomp);

    // check that hw is comparator 1
    if(_hcomp != &hcomp1) return;

    // if queue if not full
    if(_this->ready_to_process == false) {
        // send event to queue
        CompEvent event;
        // TOOD F4 and F5 differ
        event.value = (HAL_COMP_GetOutputLevel(_hcomp) == COMP_OUTPUT_LEVEL_LOW);
        event.dwt_value = DWT->CYCCNT;
        osStatus_t status = osMessageQueuePut(_this->comp_event_queue, &event, 0, 0);

        // queue is full, so we need to process data
        if(status != osOK) {
            _this->ready_to_process = true;
        };
    }
}

bool CyfralReaderComp::parse_data(
    bool* raw_data,
    uint16_t capture_size,
    uint8_t* data,
    uint8_t count) {
    const bool start_nibble[bits_in_nibble] = {1, 1, 1, 0};
    uint32_t start_position =
        search_array_in_array(raw_data, capture_size, start_nibble, bits_in_nibble);
    uint32_t end_position = 0;

    memset(data, 0, count);

    if(start_position < capture_size) {
        start_position = start_position + bits_in_nibble;
        end_position = start_position + count * 2 * bits_in_nibble;

        if(end_position >= capture_size) {
            error = CyfralReaderCompError::RAW_DATA_SIZE_ERROR;
            return false;
        }

        bool first_nibble = true;
        uint8_t data_position = 0;
        uint8_t nibble_value = 0;

        while(data_position < count) {
            nibble_value = !raw_data[start_position] << 3 | !raw_data[start_position + 1] << 2 |
                           !raw_data[start_position + 2] << 1 | !raw_data[start_position + 3];

            switch(nibble_value) {
            case(0x7):
            case(0xB):
            case(0xD):
            case(0xE):
                break;
            default:
                error = CyfralReaderCompError::UNKNOWN_NIBBLE_VALUE;
                return false;
                break;
            }

            if(first_nibble) {
                data[data_position] |= nibble_value << 4;
            } else {
                data[data_position] |= nibble_value;
            }

            first_nibble = !first_nibble;

            if(first_nibble) {
                data_position++;
            }

            start_position = start_position + bits_in_nibble;
        }

        error = CyfralReaderCompError::NO_ERROR;
        return true;
    }

    error = CyfralReaderCompError::NO_START_NIBBLE;
    return false;
}

CyfralReaderComp::CyfralReaderComp(const GpioPin* gpio_pin) {
    pin_record = gpio_pin;
}

CyfralReaderComp::~CyfralReaderComp() {
}

void CyfralReaderComp::start(void) {
    // pulldown lf-rfid pins to prevent interference
    // TODO open record
    GpioPin rfid_pull_pin = {.port = RFID_PULL_GPIO_Port, .pin = RFID_PULL_Pin};
    hal_gpio_init((GpioPin*)&rfid_pull_pin, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedLow);
    hal_gpio_write((GpioPin*)&rfid_pull_pin, false);

    // TODO open record
    GpioPin rfid_out_pin = {.port = RFID_OUT_GPIO_Port, .pin = RFID_OUT_Pin};
    hal_gpio_init((GpioPin*)&rfid_out_pin, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedLow);
    hal_gpio_write((GpioPin*)&rfid_out_pin, false);

    // connect comparator callback
    void* comp_ctx = this;
    comp_event_queue = osMessageQueueNew(capture_size * 2 + 2, sizeof(CompEvent), NULL);
    ready_to_process = false;

    auto cmp_cb = cbc::obtain_connector(this, &CyfralReaderComp::comparator_trigger_callback);
    api_interrupt_add(cmp_cb, InterruptTypeComparatorTrigger, comp_ctx);

    // start comaparator
    HAL_COMP_Start(&hcomp1);
}

void CyfralReaderComp::stop(void) {
    // stop comaparator
    HAL_COMP_Stop(&hcomp1);

    // disconnect comparator callback
    auto cmp_cb = cbc::obtain_connector(this, &CyfralReaderComp::comparator_trigger_callback);
    api_interrupt_remove(cmp_cb, InterruptTypeComparatorTrigger);
    osMessageQueueDelete(comp_event_queue);
}

bool CyfralReaderComp::read(uint8_t* data, uint8_t count) {
    bool raw_data[capture_size];
    bool result = false;
    error = CyfralReaderCompError::NO_ERROR;

    if(ready_to_process == false) {
        error = CyfralReaderCompError::NOT_ENOUGH_DATA;
    } else {
        memset(raw_data, 0, sizeof(bool) * capture_size);
        if(capture_data(raw_data, capture_size)) {
            if(parse_data(raw_data, capture_size, data, count)) {
                result = true;
            }
        }

        ready_to_process = false;
    }

    return result;
}