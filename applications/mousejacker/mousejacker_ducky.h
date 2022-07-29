#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <furi_hal_spi.h>
#include <stdio.h>
#include <string.h>
#include <nrf24.h>
#include <furi.h>
#include <furi_hal.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char* name;
    uint8_t hid;
    uint8_t mod;
} MJDuckyKey;

void mj_process_ducky_script(
    FuriHalSpiBusHandle* handle,
    uint8_t* addr,
    uint8_t addr_size,
    uint8_t rate,
    char* script);

#ifdef __cplusplus
}
#endif