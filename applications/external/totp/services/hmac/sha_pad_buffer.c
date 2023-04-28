#include "sha_pad_buffer.h"
#include <string.h>

void sha_pad_buffer(uint8_t* buffer, size_t size) {
    if(size > 0) {
        buffer[0] = 0x80;
        if(size > 1) {
            memset(&buffer[1], 0, size - 1);
        }
    }
}