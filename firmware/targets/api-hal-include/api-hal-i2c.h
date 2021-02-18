#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <i2c.h>

#ifdef __cplusplus
extern "C" {
#endif

void api_hal_i2c_init();

void api_hal_i2c_lock();

void api_hal_i2c_unlock();

#define with_api_hal_i2c(type, pointer, function_body)          \
    {                                                           \
        api_hal_i2c_lock();                                     \
        *pointer = ({ type __fn__ function_body __fn__; })();   \
        api_hal_i2c_unlock();                                   \
    }

#ifdef __cplusplus
}
#endif
