#include <api-hal-uid.h>
#include <stm32wbxx.h>

size_t api_hal_uid_size() {
    return 64/8;
}

const uint8_t* api_hal_uid() {
    return (const uint8_t *)UID64_BASE;
}
