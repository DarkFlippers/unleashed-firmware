#pragma once

#include "avr_isp_spi_sw.h"
#include <furi_hal.h>

typedef struct AvrIspProg AvrIspProg;
typedef void (*AvrIspProgCallback)(void* context);

AvrIspProg* avr_isp_prog_init(void);
void avr_isp_prog_free(AvrIspProg* instance);
size_t avr_isp_prog_spaces_rx(AvrIspProg* instance) ;
bool avr_isp_prog_rx(AvrIspProg* instance, uint8_t* data, size_t len);
size_t avr_isp_prog_tx(AvrIspProg* instance, uint8_t* data, size_t max_len);
void avr_isp_prog_avrisp(AvrIspProg* instance);
void avr_isp_prog_exit(AvrIspProg* instance);
void avr_isp_prog_set_tx_callback(AvrIspProg* instance, AvrIspProgCallback callback, void* context);
