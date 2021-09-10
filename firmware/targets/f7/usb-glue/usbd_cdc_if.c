#include "usbd_cdc_if.h"
#include <furi-hal-vcp_i.h>

extern USBD_HandleTypeDef hUsbDeviceFS;

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);
static int8_t CDC_TransmitCplt_FS(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
    CDC_Init_FS,
    CDC_DeInit_FS,
    CDC_Control_FS,
    CDC_Receive_FS,
    CDC_TransmitCplt_FS
};

uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/** Initializes the CDC media low layer over the FS USB IP
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CDC_Init_FS(void) {
    /* Set Application Buffers */
    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
    return (USBD_OK);
}

/**
 * @brief  DeInitializes the CDC media low layer
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CDC_DeInit_FS(void) {
    return (USBD_OK);
}

/** Manage the CDC class requests
 * @param  cmd: Command code
 * @param  pbuf: Buffer containing command data (request parameters)
 * @param  length: Number of data to be sent (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length) {
    if (cmd == CDC_SEND_ENCAPSULATED_COMMAND) {
    } else if (cmd == CDC_GET_ENCAPSULATED_RESPONSE) {
    } else if (cmd == CDC_SET_COMM_FEATURE) {
    } else if (cmd == CDC_GET_COMM_FEATURE) {
    } else if (cmd == CDC_CLEAR_COMM_FEATURE) {
    } else if (cmd == CDC_SET_LINE_CODING) {
        /*******************************************************************************/
        /* Line Coding Structure                                                       */
        /*-----------------------------------------------------------------------------*/
        /* Offset | Field       | Size | Value  | Description                          */
        /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
        /* 4      | bCharFormat |   1  | Number | Stop bits                            */
        /*                                        0 - 1 Stop bit                       */
        /*                                        1 - 1.5 Stop bits                    */
        /*                                        2 - 2 Stop bits                      */
        /* 5      | bParityType |  1   | Number | Parity                               */
        /*                                        0 - None                             */
        /*                                        1 - Odd                              */
        /*                                        2 - Even                             */
        /*                                        3 - Mark                             */
        /*                                        4 - Space                            */
        /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
        /*******************************************************************************/
    } else if (cmd == CDC_GET_LINE_CODING) {
    } else if (cmd == CDC_SET_CONTROL_LINE_STATE) {
        furi_hal_vcp_on_cdc_control_line(((USBD_SetupReqTypedef*)pbuf)->wValue);
    } else if (cmd == CDC_SEND_BREAK) {
    } else {
    }

    return (USBD_OK);
}

/** Data received over USB OUT endpoint are sent over CDC interface through this function.
 *
 * @note
 * This function will issue a NAK packet on any OUT packet received on
 * USB endpoint until exiting this function. If you exit this function
 * before transfer is complete on CDC interface (ie. using DMA controller)
 * it will result in receiving more data while previous ones are still
 * not sent.
 *
 * @param  Buf: Buffer of data to be received
 * @param  Len: Number of data received (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len) {
    if (*Len) {
        furi_hal_vcp_on_cdc_rx(Buf, *Len);
    } else {
        USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    }
    
    return (USBD_OK);
}

/** CDC_Transmit_FS Data to send over USB IN endpoint are sent over CDC interface
 * through this function.
 * @param  Buf: Buffer of data to be sent
 * @param  Len: Number of data to be sent (in bytes)
 * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
 */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
    uint8_t result = USBD_OK;

    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
    if (hcdc->TxState != 0){
        return USBD_BUSY;
    }
    memcpy(UserTxBufferFS, Buf, Len);
    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, Len);
    result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);

    return result;
}

/** CDC_TransmitCplt_FS Data transmited callback
 *
 * @note
 * This function is IN transfer complete callback used to inform user that
 * the submitted Data is successfully sent over USB.
 *
 * @param  Buf: Buffer of data to be received
 * @param  Len: Number of data received (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CDC_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum) {
    uint8_t result = USBD_OK;

    furi_hal_vcp_on_cdc_tx_complete(*Len);

    return result;
}
