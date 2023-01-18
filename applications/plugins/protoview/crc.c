#include <stdint.h>
#include <stddef.h>

/* CRC8 with the specified initialization value 'init' and
 * polynomial 'poly'. */
uint8_t crc8(const uint8_t* data, size_t len, uint8_t init, uint8_t poly) {
    uint8_t crc = init;
    size_t i, j;
    for(i = 0; i < len; i++) {
        crc ^= data[i];
        for(j = 0; j < 8; j++) {
            if((crc & 0x80) != 0)
                crc = (uint8_t)((crc << 1) ^ poly);
            else
                crc <<= 1;
        }
    }
    return crc;
}
