#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void dev_info_svc_start();

void dev_info_svc_stop();

bool dev_info_svc_is_started();

#ifdef __cplusplus
}
#endif
