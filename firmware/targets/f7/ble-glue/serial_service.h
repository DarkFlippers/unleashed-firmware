#pragma once

#include <stdint.h>
#include <stdbool.h>

#define SERIAL_SVC_DATA_LEN_MAX (245)

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t(*SerialSvcDataReceivedCallback)(uint8_t* buff, uint16_t size, void* context);
typedef void(*SerialSvcDataSentCallback)(void* context);

void serial_svc_start();

void serial_svc_set_callbacks(uint16_t buff_size, SerialSvcDataReceivedCallback on_received_cb, SerialSvcDataSentCallback on_sent_cb, void* context);

void serial_svc_notify_buffer_is_empty();

void serial_svc_stop();

bool serial_svc_update_tx(uint8_t* data, uint8_t data_len);

#ifdef __cplusplus
}
#endif
