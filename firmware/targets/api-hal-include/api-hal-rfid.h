#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <main.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief config rfid pins to reset state
 * 
 */
void api_hal_rfid_pins_reset();

/**
 * @brief config rfid pins to emulate state
 * 
 */
void api_hal_rfid_pins_emulate();

/**
 * @brief config rfid pins to read state
 * 
 */
void api_hal_rfid_pins_read();

/**
 * @brief config rfid timer to read state
 * 
 * @param freq timer frequency
 * @param duty_cycle timer duty cycle, 0.0-1.0
 */
void api_hal_rfid_tim_read(float freq, float duty_cycle);

/**
 * @brief start read timer
 * 
 */
void api_hal_rfid_tim_read_start();

/**
 * @brief stop read timer
 * 
 */
void api_hal_rfid_tim_read_stop();

/**
 * @brief config rfid timer to emulate state
 * 
 * @param freq timer frequency
 */
void api_hal_rfid_tim_emulate(float freq);

/**
 * @brief start emulation timer
 * 
 */
void api_hal_rfid_tim_emulate_start();

/**
 * @brief stop emulation timer
 * 
 */
void api_hal_rfid_tim_emulate_stop();

/**
 * @brief config rfid timers to reset state
 * 
 */
void api_hal_rfid_tim_reset();

/**
 * @brief check that timer instance is emulation timer
 * 
 * @param hw timer instance
 */
bool api_hal_rfid_is_tim_emulate(TIM_HandleTypeDef* hw);

/**
 * @brief set emulation timer period
 * 
 * @param period overall duration
 */
void api_hal_rfid_set_emulate_period(uint32_t period);

/**
 * @brief set emulation timer pulse
 * 
 * @param pulse duration of high level
 */
void api_hal_rfid_set_emulate_pulse(uint32_t pulse);

/**
 * @brief set read timer period
 * 
 * @param period overall duration
 */
void api_hal_rfid_set_read_period(uint32_t period);

/**
 * @brief set read timer pulse
 * 
 * @param pulse duration of high level
 */
void api_hal_rfid_set_read_pulse(uint32_t pulse);

/**
 * Сhanges the configuration of the RFID timer "on a fly"
 * @param freq new frequency
 * @param duty_cycle new duty cycle
 */
void api_hal_rfid_change_read_config(float freq, float duty_cycle);

#ifdef __cplusplus
}
#endif