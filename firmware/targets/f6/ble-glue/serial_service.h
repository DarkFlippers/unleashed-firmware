#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif

void serial_svc_start();

void serial_svc_set_rpc_session(RpcSession* rpc_session);

void serial_svc_stop();

bool serial_svc_update_rx(uint8_t* data, uint8_t data_len);

#ifdef __cplusplus
}
#endif
