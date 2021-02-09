#include <furi.h>
#include "irda_nec.h"
#include "irda_protocols.h"

void ir_nec_preambula(void) {
    // 9ms carrier + 4.5ms pause
    irda_pwm_set(NEC_DUTY_CYCLE, NEC_CARRIER_FREQUENCY);
    delay_us(9000);
    irda_pwm_stop();
    delay_us(4500);
}

void ir_nec_send_bit(bool bit) {
    // 0 is 562.5us carrier + 1687.5us pause
    // 1 is 562.5us carrier + 562.5us pause
    irda_pwm_set(NEC_DUTY_CYCLE, NEC_CARRIER_FREQUENCY);
    delay_us(562.5);
    irda_pwm_stop();
    if(bit) {
        delay_us(562.5);
    } else {
        delay_us(1687.5);
    }
}

void ir_nec_send_byte(uint8_t data) {
    for(uint8_t i = 0; i < 8; i++) {
        ir_nec_send_bit((data & (1 << (i))) != 0);
    }
}

void ir_nec_send(uint16_t addr, uint8_t data) {
    // nec protocol is:
    // preambula + addr + inverse addr + command + inverse command + bit pulse
    //
    // oddly enough, my analyzer (https://github.com/ukw100/IRMP) displays the reverse command
    // and I don’t know if this is my fault or a feature of the analyzer
    // TODO: check the dictionary and check with a known remote
    uint8_t nec_packet[4] = {~(uint8_t)addr, ~(uint8_t)(addr >> 8), ~(uint8_t)data, data};

    osKernelLock();
    __disable_irq();

    ir_nec_preambula();
    ir_nec_send_byte(nec_packet[0]);
    ir_nec_send_byte(nec_packet[1]);
    ir_nec_send_byte(nec_packet[2]);
    ir_nec_send_byte(nec_packet[3]);
    ir_nec_send_bit(1);

    __enable_irq();
    osKernelUnlock();
}