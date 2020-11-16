#include <api-hal-uid.h>
#include <stm32l4xx.h>

size_t api_hal_uid_size() {
    return 96/8;
}

const uint8_t* api_hal_uid() {
    return (const uint8_t *)UID_BASE;
}
