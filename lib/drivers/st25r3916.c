#include "st25r3916.h"

#include <furi.h>

void st25r3916_mask_irq(FuriHalSpiBusHandle* handle, uint32_t mask) {
    furi_assert(handle);

    uint8_t irq_mask_regs[4] = {
        mask & 0xff,
        (mask >> 8) & 0xff,
        (mask >> 16) & 0xff,
        (mask >> 24) & 0xff,
    };
    st25r3916_write_burst_regs(handle, ST25R3916_REG_IRQ_MASK_MAIN, irq_mask_regs, 4);
}

uint32_t st25r3916_get_irq(FuriHalSpiBusHandle* handle) {
    furi_assert(handle);

    uint8_t irq_regs[4] = {};
    uint32_t irq = 0;
    st25r3916_read_burst_regs(handle, ST25R3916_REG_IRQ_MASK_MAIN, irq_regs, 4);
    // FURI_LOG_I(
    //     "Mask Irq", "%02X %02X %02X %02X", irq_regs[0], irq_regs[1], irq_regs[2], irq_regs[3]);
    st25r3916_read_burst_regs(handle, ST25R3916_REG_IRQ_MAIN, irq_regs, 4);
    irq = (uint32_t)irq_regs[0];
    irq |= (uint32_t)irq_regs[1] << 8;
    irq |= (uint32_t)irq_regs[2] << 16;
    irq |= (uint32_t)irq_regs[3] << 24;
    // FURI_LOG_I("iRQ", "%02X %02X %02X %02X", irq_regs[0], irq_regs[1], irq_regs[2], irq_regs[3]);

    return irq;
}

void st25r3916_write_fifo(FuriHalSpiBusHandle* handle, const uint8_t* buff, size_t bits) {
    furi_assert(handle);
    furi_assert(buff);

    size_t bytes = (bits + 7) / 8;

    st25r3916_write_reg(handle, ST25R3916_REG_NUM_TX_BYTES2, (uint8_t)(bits & 0xFFU));
    st25r3916_write_reg(handle, ST25R3916_REG_NUM_TX_BYTES1, (uint8_t)((bits >> 8) & 0xFFU));

    st25r3916_reg_write_fifo(handle, buff, bytes);
}

bool st25r3916_read_fifo(
    FuriHalSpiBusHandle* handle,
    uint8_t* buff,
    size_t buff_size,
    size_t* buff_bits) {
    furi_assert(handle);
    furi_assert(buff);

    bool read_success = false;

    do {
        uint8_t fifo_status[2] = {};
        st25r3916_read_burst_regs(handle, ST25R3916_REG_FIFO_STATUS1, fifo_status, 2);

        uint16_t fifo_status_b9_b8 =
            ((fifo_status[1] & ST25R3916_REG_FIFO_STATUS2_fifo_b_mask) >>
             ST25R3916_REG_FIFO_STATUS2_fifo_b_shift);
        size_t bytes = (fifo_status_b9_b8 << 8) | fifo_status[0];

        uint8_t bits =
            ((fifo_status[1] & ST25R3916_REG_FIFO_STATUS2_fifo_lb_mask) >>
             ST25R3916_REG_FIFO_STATUS2_fifo_lb_shift);

        if(bytes == 0) break;
        if(bytes > buff_size) break;

        st25r3916_reg_read_fifo(handle, buff, bytes);

        if(bits) {
            *buff_bits = (bytes - 1) * 8 + bits;
        } else {
            *buff_bits = bytes * 8;
        }
        read_success = true;
    } while(false);

    return read_success;
}
