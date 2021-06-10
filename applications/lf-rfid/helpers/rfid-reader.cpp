#include "rfid-reader.h"
#include <furi.h>
#include <api-hal.h>
#include <stm32wbxx_ll_cortex.h>
#include <tim.h>

extern COMP_HandleTypeDef hcomp1;

/**
 * @brief private violation assistant for RfidReader
 */
struct RfidReaderAccessor {
    static void decode(RfidReader& rfid_reader, bool polarity) {
        rfid_reader.decode(polarity);
    }
};

void RfidReader::decode(bool polarity) {
    uint32_t current_dwt_value = DWT->CYCCNT;

    switch(type) {
    case Type::Normal:
        decoder_em.process_front(polarity, current_dwt_value - last_dwt_value);
        decoder_hid26.process_front(polarity, current_dwt_value - last_dwt_value);
        //decoder_indala.process_front(polarity, current_dwt_value - last_dwt_value);
        //decoder_analyzer.process_front(polarity, current_dwt_value - last_dwt_value);

        last_dwt_value = current_dwt_value;
        break;
    case Type::Indala:
        break;
    }
}

static void comparator_trigger_callback(void* hcomp, void* comp_ctx) {
    COMP_HandleTypeDef* _hcomp = static_cast<COMP_HandleTypeDef*>(hcomp);
    RfidReader* _this = static_cast<RfidReader*>(comp_ctx);

    if(hcomp == &hcomp1) {
        RfidReaderAccessor::decode(
            *_this, (HAL_COMP_GetOutputLevel(_hcomp) == COMP_OUTPUT_LEVEL_HIGH));
    }
}

RfidReader::RfidReader() {
}

void RfidReader::start(Type _type) {
    type = _type;

    start_gpio();
    switch(type) {
    case Type::Normal:
        start_timer();
        break;
    case Type::Indala:
        start_timer_indala();
        break;
    }

    start_comparator();
}

void RfidReader::stop() {
    stop_gpio();
    stop_timer();
    stop_comparator();
}

bool RfidReader::read(LfrfidKeyType* type, uint8_t* data, uint8_t data_size) {
    bool result = false;

    if(decoder_em.read(data, data_size)) {
        *type = LfrfidKeyType::KeyEM4100;
        result = true;
    }

    if(decoder_hid26.read(data, data_size)) {
        *type = LfrfidKeyType::KeyH10301;
        result = true;
    }

    //decoder_indala.read(NULL, 0);
    //decoder_analyzer.read(NULL, 0);

    return result;
}

void RfidReader::start_comparator(void) {
    api_interrupt_add(comparator_trigger_callback, InterruptTypeComparatorTrigger, this);
    last_dwt_value = DWT->CYCCNT;

    hcomp1.Init.InputMinus = COMP_INPUT_MINUS_1_2VREFINT;
    hcomp1.Init.InputPlus = COMP_INPUT_PLUS_IO1;
    hcomp1.Init.OutputPol = COMP_OUTPUTPOL_NONINVERTED;
    hcomp1.Init.Hysteresis = COMP_HYSTERESIS_LOW;
    hcomp1.Init.BlankingSrce = COMP_BLANKINGSRC_NONE;
    hcomp1.Init.Mode = COMP_POWERMODE_MEDIUMSPEED;
    hcomp1.Init.WindowMode = COMP_WINDOWMODE_DISABLE;
    hcomp1.Init.TriggerMode = COMP_TRIGGERMODE_IT_RISING_FALLING;
    if(HAL_COMP_Init(&hcomp1) != HAL_OK) {
        Error_Handler();
    }

    HAL_COMP_Start(&hcomp1);
}

void RfidReader::start_timer(void) {
    api_hal_rfid_tim_read(125000, 0.5);
    api_hal_rfid_tim_read_start();
}

void RfidReader::start_timer_indala(void) {
    api_hal_rfid_tim_read(62500, 0.25);
    api_hal_rfid_tim_read_start();
}

void RfidReader::start_gpio(void) {
    api_hal_rfid_pins_read();
}

void RfidReader::stop_comparator(void) {
    HAL_COMP_Stop(&hcomp1);
    api_interrupt_remove(comparator_trigger_callback, InterruptTypeComparatorTrigger);
}

void RfidReader::stop_timer(void) {
    api_hal_rfid_tim_read_stop();
    api_hal_rfid_tim_reset();
}

void RfidReader::stop_gpio(void) {
    api_hal_rfid_pins_reset();
}