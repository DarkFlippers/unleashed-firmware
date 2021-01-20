#include <furi.h>

void prepare_data(uint32_t ID, uint32_t VENDOR, uint8_t* data) {
    uint8_t value[10];

    // vendor rows (4 bit in a row)
    value[0] = (VENDOR >> 4) & 0xF;
    value[1] = VENDOR & 0xF;

    const uint8_t ROW_SIZE = 4;
    const uint8_t HEADER_SIZE = 9;

    // ID rows (4 bit in a row)
    for(int i = 0; i < 8; i++) {
        value[i + 2] = (ID >> (28 - i * ROW_SIZE)) & 0xF;
    }

    for(uint8_t i = 0; i < HEADER_SIZE; i++) {
        data[i] = 1; // header
    }

    for(uint8_t i = 0; i < 10; i++) { // data
        for(uint8_t j = 0; j < ROW_SIZE; j++) {
            data[HEADER_SIZE + i * (ROW_SIZE + 1) + j] = (value[i] >> ((ROW_SIZE - 1) - j)) & 1;
        }

        // row parity
        data[HEADER_SIZE + i * (ROW_SIZE + 1) + ROW_SIZE] =
            (data[HEADER_SIZE + i * (ROW_SIZE + 1) + 0] +
             data[HEADER_SIZE + i * (ROW_SIZE + 1) + 1] +
             data[HEADER_SIZE + i * (ROW_SIZE + 1) + 2] +
             data[HEADER_SIZE + i * (ROW_SIZE + 1) + 3]) %
            2;
    }

    for(uint8_t i = 0; i < ROW_SIZE; i++) { //checksum
        uint8_t checksum = 0;
        for(uint8_t j = 0; j < 10; j++) {
            checksum += data[HEADER_SIZE + i + j * (ROW_SIZE + 1)];
        }
        data[i + 59] = checksum % 2;
    }

    data[63] = 0; // stop bit

    /*
    printf("em data: ");
    for(uint8_t i = 0; i < 64; i++) {
        printf("%d ", data[i]);
    }
    printf("\n");
    */
}

void em4100_emulation(uint8_t* data, GpioPin* pin) {
    taskENTER_CRITICAL();
    gpio_write(pin, true);

    for(uint8_t i = 0; i < 8; i++) {
        for(uint8_t j = 0; j < 64; j++) {
            delay_us(270);
            gpio_write(pin, data[j]);
            delay_us(270);
            gpio_write(pin, !data[j]);
        }
    }

    gpio_write(pin, false);
    taskEXIT_CRITICAL();
}
