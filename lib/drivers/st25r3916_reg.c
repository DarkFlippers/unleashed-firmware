#include "st25r3916_reg.h"

#include <furi.h>

#define ST25R3916_WRITE_MODE \
    (0U << 6) /*!< ST25R3916 Operation Mode: Write                                */
#define ST25R3916_READ_MODE \
    (1U << 6) /*!< ST25R3916 Operation Mode: Read                                 */
#define ST25R3916_CMD_MODE \
    (3U << 6) /*!< ST25R3916 Operation Mode: Direct Command                       */
#define ST25R3916_FIFO_LOAD \
    (0x80U) /*!< ST25R3916 Operation Mode: FIFO Load                            */
#define ST25R3916_FIFO_READ \
    (0x9FU) /*!< ST25R3916 Operation Mode: FIFO Read                            */
#define ST25R3916_PT_A_CONFIG_LOAD \
    (0xA0U) /*!< ST25R3916 Operation Mode: Passive Target Memory A-Config Load  */
#define ST25R3916_PT_F_CONFIG_LOAD \
    (0xA8U) /*!< ST25R3916 Operation Mode: Passive Target Memory F-Config Load  */
#define ST25R3916_PT_TSN_DATA_LOAD \
    (0xACU) /*!< ST25R3916 Operation Mode: Passive Target Memory TSN Load       */
#define ST25R3916_PT_MEM_READ \
    (0xBFU) /*!< ST25R3916 Operation Mode: Passive Target Memory Read           */

#define ST25R3916_CMD_LEN \
    (1U) /*!< ST25R3916 CMD length                                           */
#define ST25R3916_FIFO_DEPTH (512U)
#define ST25R3916_BUF_LEN \
    (ST25R3916_CMD_LEN +  \
     ST25R3916_FIFO_DEPTH) /*!< ST25R3916 communication buffer: CMD + FIFO length    */

static void st25r3916_reg_tx_byte(FuriHalSpiBusHandle* handle, uint8_t byte) {
    uint8_t val = byte;
    furi_hal_spi_bus_tx(handle, &val, 1, 5);
}

void st25r3916_read_reg(FuriHalSpiBusHandle* handle, uint8_t reg, uint8_t* val) {
    furi_check(handle);
    st25r3916_read_burst_regs(handle, reg, val, 1);
}

void st25r3916_read_burst_regs(
    FuriHalSpiBusHandle* handle,
    uint8_t reg_start,
    uint8_t* values,
    uint8_t length) {
    furi_check(handle);
    furi_check(values);
    furi_check(length);

    furi_hal_gpio_write(handle->cs, false);

    if(reg_start & ST25R3916_SPACE_B) {
        // Send direct command first
        st25r3916_reg_tx_byte(handle, ST25R3916_CMD_SPACE_B_ACCESS);
    }
    st25r3916_reg_tx_byte(handle, (reg_start & ~ST25R3916_SPACE_B) | ST25R3916_READ_MODE);
    furi_hal_spi_bus_rx(handle, values, length, 5);

    furi_hal_gpio_write(handle->cs, true);
}

void st25r3916_write_reg(FuriHalSpiBusHandle* handle, uint8_t reg, uint8_t val) {
    furi_check(handle);
    uint8_t reg_val = val;
    st25r3916_write_burst_regs(handle, reg, &reg_val, 1);
}

void st25r3916_write_burst_regs(
    FuriHalSpiBusHandle* handle,
    uint8_t reg_start,
    const uint8_t* values,
    uint8_t length) {
    furi_check(handle);
    furi_check(values);
    furi_check(length);

    furi_hal_gpio_write(handle->cs, false);

    if(reg_start & ST25R3916_SPACE_B) {
        // Send direct command first
        st25r3916_reg_tx_byte(handle, ST25R3916_CMD_SPACE_B_ACCESS);
    }
    st25r3916_reg_tx_byte(handle, (reg_start & ~ST25R3916_SPACE_B) | ST25R3916_WRITE_MODE);
    furi_hal_spi_bus_tx(handle, values, length, 5);

    furi_hal_gpio_write(handle->cs, true);
}

void st25r3916_reg_write_fifo(FuriHalSpiBusHandle* handle, const uint8_t* buff, size_t length) {
    furi_check(handle);
    furi_check(buff);
    furi_check(length);
    furi_check(length <= ST25R3916_FIFO_DEPTH);

    furi_hal_gpio_write(handle->cs, false);
    st25r3916_reg_tx_byte(handle, ST25R3916_FIFO_LOAD);
    furi_hal_spi_bus_tx(handle, buff, length, 200);
    furi_hal_gpio_write(handle->cs, true);
}

void st25r3916_reg_read_fifo(FuriHalSpiBusHandle* handle, uint8_t* buff, size_t length) {
    furi_check(handle);
    furi_check(buff);
    furi_check(length);
    furi_check(length <= ST25R3916_FIFO_DEPTH);

    furi_hal_gpio_write(handle->cs, false);
    st25r3916_reg_tx_byte(handle, ST25R3916_FIFO_READ);
    furi_hal_spi_bus_rx(handle, buff, length, 200);
    furi_hal_gpio_write(handle->cs, true);
}

void st25r3916_write_pta_mem(FuriHalSpiBusHandle* handle, const uint8_t* values, size_t length) {
    furi_check(handle);
    furi_check(values);
    furi_check(length);
    furi_check(length <= ST25R3916_PTM_LEN);

    furi_hal_gpio_write(handle->cs, false);
    st25r3916_reg_tx_byte(handle, ST25R3916_PT_A_CONFIG_LOAD);
    furi_hal_spi_bus_tx(handle, values, length, 200);
    furi_hal_gpio_write(handle->cs, true);
}

void st25r3916_read_pta_mem(FuriHalSpiBusHandle* handle, uint8_t* buff, size_t length) {
    furi_check(handle);
    furi_check(buff);
    furi_check(length);
    furi_check(length <= ST25R3916_PTM_LEN);

    uint8_t tmp_buff[ST25R3916_PTM_LEN + 1];
    furi_hal_gpio_write(handle->cs, false);
    st25r3916_reg_tx_byte(handle, ST25R3916_PT_MEM_READ);
    furi_hal_spi_bus_rx(handle, tmp_buff, length + 1, 200);
    furi_hal_gpio_write(handle->cs, true);
    memcpy(buff, tmp_buff + 1, length);
}

void st25r3916_write_ptf_mem(FuriHalSpiBusHandle* handle, const uint8_t* values, size_t length) {
    furi_check(handle);
    furi_check(values);

    furi_hal_gpio_write(handle->cs, false);
    st25r3916_reg_tx_byte(handle, ST25R3916_PT_F_CONFIG_LOAD);
    furi_hal_spi_bus_tx(handle, values, length, 200);
    furi_hal_gpio_write(handle->cs, true);
}

void st25r3916_write_pttsn_mem(FuriHalSpiBusHandle* handle, uint8_t* buff, size_t length) {
    furi_check(handle);
    furi_check(buff);

    furi_hal_gpio_write(handle->cs, false);
    st25r3916_reg_tx_byte(handle, ST25R3916_PT_TSN_DATA_LOAD);
    furi_hal_spi_bus_tx(handle, buff, length, 200);
    furi_hal_gpio_write(handle->cs, true);
}

void st25r3916_direct_cmd(FuriHalSpiBusHandle* handle, uint8_t cmd) {
    furi_check(handle);

    furi_hal_gpio_write(handle->cs, false);
    st25r3916_reg_tx_byte(handle, cmd | ST25R3916_CMD_MODE);
    furi_hal_gpio_write(handle->cs, true);
}

void st25r3916_read_test_reg(FuriHalSpiBusHandle* handle, uint8_t reg, uint8_t* val) {
    furi_check(handle);

    furi_hal_gpio_write(handle->cs, false);
    st25r3916_reg_tx_byte(handle, ST25R3916_CMD_TEST_ACCESS);
    st25r3916_reg_tx_byte(handle, reg | ST25R3916_READ_MODE);
    furi_hal_spi_bus_rx(handle, val, 1, 5);
    furi_hal_gpio_write(handle->cs, true);
}

void st25r3916_write_test_reg(FuriHalSpiBusHandle* handle, uint8_t reg, uint8_t val) {
    furi_check(handle);

    furi_hal_gpio_write(handle->cs, false);
    st25r3916_reg_tx_byte(handle, ST25R3916_CMD_TEST_ACCESS);
    st25r3916_reg_tx_byte(handle, reg | ST25R3916_WRITE_MODE);
    furi_hal_spi_bus_tx(handle, &val, 1, 5);
    furi_hal_gpio_write(handle->cs, true);
}

void st25r3916_clear_reg_bits(FuriHalSpiBusHandle* handle, uint8_t reg, uint8_t clr_mask) {
    furi_check(handle);

    uint8_t reg_val = 0;
    st25r3916_read_reg(handle, reg, &reg_val);
    if((reg_val & ~clr_mask) != reg_val) {
        reg_val &= ~clr_mask;
        st25r3916_write_reg(handle, reg, reg_val);
    }
}

void st25r3916_set_reg_bits(FuriHalSpiBusHandle* handle, uint8_t reg, uint8_t set_mask) {
    furi_check(handle);

    uint8_t reg_val = 0;
    st25r3916_read_reg(handle, reg, &reg_val);
    if((reg_val | set_mask) != reg_val) {
        reg_val |= set_mask;
        st25r3916_write_reg(handle, reg, reg_val);
    }
}

void st25r3916_change_reg_bits(
    FuriHalSpiBusHandle* handle,
    uint8_t reg,
    uint8_t mask,
    uint8_t value) {
    furi_check(handle);

    st25r3916_modify_reg(handle, reg, mask, (mask & value));
}

void st25r3916_modify_reg(
    FuriHalSpiBusHandle* handle,
    uint8_t reg,
    uint8_t clr_mask,
    uint8_t set_mask) {
    furi_check(handle);

    uint8_t reg_val = 0;
    uint8_t new_val = 0;
    st25r3916_read_reg(handle, reg, &reg_val);
    new_val = (reg_val & ~clr_mask) | set_mask;
    if(new_val != reg_val) {
        st25r3916_write_reg(handle, reg, new_val);
    }
}

void st25r3916_change_test_reg_bits(
    FuriHalSpiBusHandle* handle,
    uint8_t reg,
    uint8_t mask,
    uint8_t value) {
    furi_check(handle);

    uint8_t reg_val = 0;
    uint8_t new_val = 0;
    st25r3916_read_test_reg(handle, reg, &reg_val);
    new_val = (reg_val & ~mask) | (mask & value);
    if(new_val != reg_val) {
        st25r3916_write_test_reg(handle, reg, new_val);
    }
}

bool st25r3916_check_reg(FuriHalSpiBusHandle* handle, uint8_t reg, uint8_t mask, uint8_t val) {
    furi_check(handle);

    uint8_t reg_val = 0;
    st25r3916_read_reg(handle, reg, &reg_val);
    return (reg_val & mask) == val;
}
