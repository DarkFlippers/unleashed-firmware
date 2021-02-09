#include <furi.h>
#include "irda_samsung.h"
#include "irda_protocols.h"

void ir_samsung_preambula(void) {
    irda_pwm_set(SAMSUNG_DUTY_CYCLE, SAMSUNG_CARRIER_FREQUENCY);
    delay_us(4500);
    irda_pwm_stop();
    delay_us(4500);
}

void ir_samsung_send_bit(bool bit) {
    irda_pwm_set(SAMSUNG_DUTY_CYCLE, SAMSUNG_CARRIER_FREQUENCY);
    delay_us(560);
    irda_pwm_stop();
    if(bit) {
        delay_us(1590);
    } else {
        delay_us(560);
    }
}

void ir_samsung_send_byte(uint8_t data) {
    for(uint8_t i = 0; i < 8; i++) {
        ir_samsung_send_bit((data & (1 << (i))) != 0);
    }
}

void ir_samsung_send(uint16_t addr, uint16_t data) {
    uint8_t samsung_packet[4] = {
        (uint8_t)addr, (uint8_t)(addr >> 8), (uint8_t)data, (uint8_t)(data >> 8)};

    osKernelLock();
    __disable_irq();

    ir_samsung_preambula();
    ir_samsung_send_byte(samsung_packet[0]);
    ir_samsung_send_byte(samsung_packet[1]);
    ir_samsung_send_byte(samsung_packet[2]);
    ir_samsung_send_byte(samsung_packet[3]);
    ir_samsung_send_bit(0);

    __enable_irq();
    osKernelUnlock();
}