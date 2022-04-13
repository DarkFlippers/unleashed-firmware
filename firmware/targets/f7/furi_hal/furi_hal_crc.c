#include <furi_hal.h>
#include <stm32wbxx_ll_crc.h>

typedef enum {
    CRC_State_Reset,
    CRC_State_Ready,
    CRC_State_Busy,
} CRC_State;

typedef struct {
    CRC_State state;
    osMutexId_t mtx;
} HAL_CRC_Control;

static volatile HAL_CRC_Control hal_crc_control = {
    .state = CRC_State_Reset,
    .mtx = NULL,
};

void furi_hal_crc_init(bool synchronize) {
    /* initialize peripheral with default generating polynomial */
    LL_CRC_SetInputDataReverseMode(CRC, LL_CRC_INDATA_REVERSE_BYTE);
    LL_CRC_SetOutputDataReverseMode(CRC, LL_CRC_OUTDATA_REVERSE_BIT);
    LL_CRC_SetPolynomialCoef(CRC, LL_CRC_DEFAULT_CRC32_POLY);
    LL_CRC_SetPolynomialSize(CRC, LL_CRC_POLYLENGTH_32B);
    LL_CRC_SetInitialData(CRC, LL_CRC_DEFAULT_CRC_INITVALUE);

    if(synchronize) {
        hal_crc_control.mtx = osMutexNew(NULL);
    }
    hal_crc_control.state = CRC_State_Ready;
}

void furi_hal_crc_reset() {
    furi_check(hal_crc_control.state == CRC_State_Ready);
    if(hal_crc_control.mtx) {
        osMutexRelease(hal_crc_control.mtx);
    }
    LL_CRC_ResetCRCCalculationUnit(CRC);
}

static uint32_t furi_hal_crc_handle_8(uint8_t pBuffer[], uint32_t BufferLength) {
    uint32_t i; /* input data buffer index */
    hal_crc_control.state = CRC_State_Busy;
    /* Processing time optimization: 4 bytes are entered in a row with a single word write,
     * last bytes must be carefully fed to the CRC calculator to ensure a correct type
     * handling by the peripheral */
    for(i = 0U; i < (BufferLength / 4U); i++) {
        LL_CRC_FeedData32(
            CRC,
            ((uint32_t)pBuffer[4U * i] << 24U) | ((uint32_t)pBuffer[(4U * i) + 1U] << 16U) |
                ((uint32_t)pBuffer[(4U * i) + 2U] << 8U) | (uint32_t)pBuffer[(4U * i) + 3U]);
    }
    /* last bytes specific handling */
    if((BufferLength % 4U) != 0U) {
        if((BufferLength % 4U) == 1U) {
            LL_CRC_FeedData8(CRC, pBuffer[4U * i]);
        } else if((BufferLength % 4U) == 2U) {
            LL_CRC_FeedData16(
                CRC, ((uint16_t)(pBuffer[4U * i]) << 8U) | (uint16_t)pBuffer[(4U * i) + 1U]);
        } else if((BufferLength % 4U) == 3U) {
            LL_CRC_FeedData16(
                CRC, ((uint16_t)(pBuffer[4U * i]) << 8U) | (uint16_t)pBuffer[(4U * i) + 1U]);
            LL_CRC_FeedData8(CRC, pBuffer[(4U * i) + 2U]);
        }
    }

    hal_crc_control.state = CRC_State_Ready;
    /* Return the CRC computed value */
    return LL_CRC_ReadData32(CRC);
}

static uint32_t furi_hal_crc_accumulate(uint32_t pBuffer[], uint32_t BufferLength) {
    furi_check(hal_crc_control.state == CRC_State_Ready);
    if(hal_crc_control.mtx) {
        furi_check(osMutexGetOwner(hal_crc_control.mtx) != NULL);
    }
    return furi_hal_crc_handle_8((uint8_t*)pBuffer, BufferLength);
}

uint32_t furi_hal_crc_feed(void* data, uint16_t length) {
    return ~furi_hal_crc_accumulate(data, length);
}

bool furi_hal_crc_acquire(uint32_t timeout) {
    furi_assert(hal_crc_control.mtx);
    return osMutexAcquire(hal_crc_control.mtx, timeout) == osOK;
}
