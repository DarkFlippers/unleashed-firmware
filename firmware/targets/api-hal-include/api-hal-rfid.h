#pragma once
#include <stdint.h>

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

#ifdef __cplusplus
}
#endif