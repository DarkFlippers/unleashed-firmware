#pragma once

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc.h"

/* Define size for the receive and transmit buffer over CDC */
/* It's up to user to redefine and/or remove those define */
#define APP_RX_DATA_SIZE  CDC_DATA_FS_MAX_PACKET_SIZE
#define APP_TX_DATA_SIZE  CDC_DATA_FS_MAX_PACKET_SIZE

/** CDC Interface callback. */
extern USBD_CDC_ItfTypeDef USBD_Interface_fops_FS;

uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);

#ifdef __cplusplus
}
#endif
