#pragma once
#include "flipper.h"
#include "flipper_v2.h"

enum class CyfralReaderError : uint8_t {
    NO_ERROR = 0,
    UNABLE_TO_DETECT = 1,
    RAW_DATA_SIZE_ERROR = 2,
    UNKNOWN_NIBBLE_VALUE = 3,
    NO_START_NIBBLE = 4,
};

class CyfralReader {
private:
    ADC_HandleTypeDef adc_config;
    ADC_TypeDef* adc_instance;
    uint32_t adc_channel;

    void get_line_minmax(uint16_t times, uint32_t* min_level, uint32_t* max_level);
    void capture_data(bool* data, uint16_t capture_size, uint32_t line_min, uint32_t line_max);
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
    CyfralReaderError error;

public:
    CyfralReader(ADC_TypeDef* adc, uint32_t Channel);
    ~CyfralReader();
    void start(void);
    void stop(void);
    bool read(uint8_t* data, uint8_t count);
};

void CyfralReader::get_line_minmax(uint16_t times, uint32_t* min_level, uint32_t* max_level) {
    uint32_t in = 0;
    uint32_t min = UINT_MAX;
    uint32_t max = 0;

    for(uint32_t i = 0; i < 256; i++) {
        HAL_ADC_Start(&adc_config);
        HAL_ADC_PollForConversion(&adc_config, 100);
        in = HAL_ADC_GetValue(&adc_config);
        if(in < min) min = in;
        if(in > max) max = in;
    }

    *min_level = min;
    *max_level = max;
}

void CyfralReader::capture_data(
    bool* data,
    uint16_t capture_size,
    uint32_t line_min,
    uint32_t line_max) {
    uint32_t input_value = 0;
    bool last_input_value = 0;

    uint32_t diff = line_max - line_min;
    uint32_t mid = line_min + diff / 2;

    uint32_t low_threshold = mid - (diff / 4);
    uint32_t high_threshold = mid - (diff / 4);

    uint16_t capture_position = 0;
    uint32_t instructions_per_us = (SystemCoreClock / 1000000.0f);
    uint32_t time_threshold = 75 * instructions_per_us;
    uint32_t capture_max_time = 140 * (capture_size * 2) * instructions_per_us;

    uint32_t start = DWT->CYCCNT;
    uint32_t end = DWT->CYCCNT;

    memset(data, 0, capture_size);

    osKernelLock();

    uint32_t capture_start = DWT->CYCCNT;
    while((capture_position < capture_size) &&
          ((DWT->CYCCNT - capture_start) < capture_max_time)) {
        // read adc
        HAL_ADC_Start(&adc_config);
        HAL_ADC_PollForConversion(&adc_config, 100);
        input_value = HAL_ADC_GetValue(&adc_config);

        // low to high transition
        if((input_value > high_threshold) && last_input_value == 0) {
            last_input_value = 1;
            start = DWT->CYCCNT;
        }

        // high to low transition
        if((input_value < low_threshold) && last_input_value == 1) {
            last_input_value = 0;
            end = DWT->CYCCNT;

            // check transition time
            if(end - start < time_threshold) {
                data[capture_position] = 1;
                capture_position++;
            } else {
                data[capture_position] = 0;
                capture_position++;
            }
        }
    }

    osKernelUnlock();
}

uint32_t CyfralReader::search_array_in_array(
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

bool CyfralReader::parse_data(bool* raw_data, uint16_t capture_size, uint8_t* data, uint8_t count) {
    const bool start_nibble[bits_in_nibble] = {1, 1, 1, 0};
    uint32_t start_position =
        search_array_in_array(raw_data, capture_size, start_nibble, bits_in_nibble);
    uint32_t end_position = 0;

    memset(data, 0, count);

    if(start_position < capture_size) {
        start_position = start_position + bits_in_nibble;
        end_position = start_position + count * 2 * bits_in_nibble;

        if(end_position >= capture_size) {
            error = CyfralReaderError::RAW_DATA_SIZE_ERROR;
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
                error = CyfralReaderError::UNKNOWN_NIBBLE_VALUE;
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

        error = CyfralReaderError::NO_ERROR;
        return true;
    }

    error = CyfralReaderError::NO_START_NIBBLE;
    return false;
}

CyfralReader::CyfralReader(ADC_TypeDef* adc, uint32_t channel) {
    adc_instance = adc;
    adc_channel = channel;
}

CyfralReader::~CyfralReader() {
}

void CyfralReader::start(void) {
    ADC_ChannelConfTypeDef sConfig = {0};

    // init ADC
    adc_config.Instance = adc_instance;
    adc_config.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    adc_config.Init.Resolution = ADC_RESOLUTION_12B;
    adc_config.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    adc_config.Init.ScanConvMode = ADC_SCAN_DISABLE;
    adc_config.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    adc_config.Init.LowPowerAutoWait = DISABLE;
    adc_config.Init.ContinuousConvMode = DISABLE;
    adc_config.Init.NbrOfConversion = 1;
    adc_config.Init.DiscontinuousConvMode = DISABLE;
    adc_config.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    adc_config.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    adc_config.Init.DMAContinuousRequests = DISABLE;
    adc_config.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    adc_config.Init.OversamplingMode = DISABLE;
    if(HAL_ADC_Init(&adc_config) != HAL_OK) {
        Error_Handler();
    }

    // init channel
    sConfig.Channel = adc_channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    if(HAL_ADC_ConfigChannel(&adc_config, &sConfig) != HAL_OK) {
        Error_Handler();
    }
}

void CyfralReader::stop(void) {
    HAL_ADC_DeInit(&adc_config);
}

bool CyfralReader::read(uint8_t* data, uint8_t count) {
    uint32_t line_level_min, line_level_max;
    bool raw_data[capture_size];
    bool result = false;
    error = CyfralReaderError::NO_ERROR;

    // calibrate
    get_line_minmax(256, &line_level_min, &line_level_max);

    // TODO think about other detection method
    // key not on line
    if(line_level_max > 2000) {
        error = CyfralReaderError::UNABLE_TO_DETECT;
        return false;
    }

    // capturing raw data consisting of bits
    capture_data(raw_data, capture_size, line_level_min, line_level_max);

    // parse captured data
    if(parse_data(raw_data, capture_size, data, count)) {
        result = true;
    }

    return result;
}