#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "api-hal.h"
#include "cmsis_os.h"
#include "furi-deprecated.h"

#include "log.h"
#include "input/input.h"

#ifdef __cplusplus
}
#endif

#include <stdio.h>

void set_exitcode(uint32_t _exitcode);

#define FURI_LIB (const char*[])
