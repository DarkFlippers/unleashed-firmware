#include "main.h"

#define SD_DUMMY_BYTE 0xFF
#define SD_CS_LOW() HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET)
#define SD_CS_HIGH() HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET)

#define SPI_SD_INSTANCE SPI3
#define SPI_SD_HANDLE hspi3

const uint32_t SpiTimeout = 1000;
extern SPI_HandleTypeDef SPI_SD_HANDLE;
uint8_t SD_IO_WriteByte(uint8_t Data);

/******************************************************************************
                            BUS OPERATIONS
 *******************************************************************************/

/**
 * @brief  SPI error treatment function
 * @retval None
 */
static void SPIx_Error(void) {
    /* De-initialize the SPI communication BUS */
    HAL_SPI_DeInit(&SPI_SD_HANDLE);

    /* Re-Initiaize the SPI communication BUS */
    HAL_SPI_Init(&SPI_SD_HANDLE);
}

/**
 * @brief  SPI Write byte(s) to device
 * @param  DataIn: Pointer to data buffer to write
 * @param  DataOut: Pointer to data buffer for read data
 * @param  DataLength: number of bytes to write
 * @retval None
 */
static void SPIx_WriteReadData(const uint8_t* DataIn, uint8_t* DataOut, uint16_t DataLength) {
    HAL_StatusTypeDef status = HAL_OK;
    status =
        HAL_SPI_TransmitReceive(&SPI_SD_HANDLE, (uint8_t*)DataIn, DataOut, DataLength, SpiTimeout);

    /* Check the communication status */
    if(status != HAL_OK) {
        /* Execute user timeout callback */
        SPIx_Error();
    }
}

/**
 * @brief  SPI Write a byte to device
 * @param  Value: value to be written
 * @retval None
 */
static void SPIx_Write(uint8_t Value) {
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t data;

    status = HAL_SPI_TransmitReceive(&SPI_SD_HANDLE, (uint8_t*)&Value, &data, 1, SpiTimeout);

    /* Check the communication status */
    if(status != HAL_OK) {
        /* Execute user timeout callback */
        SPIx_Error();
    }
}

void SD_SPI_Slow_Init(void) {
    SPI_SD_HANDLE.Instance = SPI_SD_INSTANCE;
    SPI_SD_HANDLE.Init.Mode = SPI_MODE_MASTER;
    SPI_SD_HANDLE.Init.Direction = SPI_DIRECTION_2LINES;
    SPI_SD_HANDLE.Init.DataSize = SPI_DATASIZE_8BIT;
    SPI_SD_HANDLE.Init.CLKPolarity = SPI_POLARITY_LOW;
    SPI_SD_HANDLE.Init.CLKPhase = SPI_PHASE_1EDGE;
    SPI_SD_HANDLE.Init.NSS = SPI_NSS_SOFT;
    SPI_SD_HANDLE.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    SPI_SD_HANDLE.Init.FirstBit = SPI_FIRSTBIT_MSB;
    SPI_SD_HANDLE.Init.TIMode = SPI_TIMODE_DISABLE;
    SPI_SD_HANDLE.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    SPI_SD_HANDLE.Init.CRCPolynomial = 7;
    SPI_SD_HANDLE.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    SPI_SD_HANDLE.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
    if(HAL_SPI_Init(&SPI_SD_HANDLE) != HAL_OK) {
        Error_Handler();
    }
}

void SD_SPI_Fast_Init(void) {
    SPI_SD_HANDLE.Instance = SPI_SD_INSTANCE;
    SPI_SD_HANDLE.Init.Mode = SPI_MODE_MASTER;
    SPI_SD_HANDLE.Init.Direction = SPI_DIRECTION_2LINES;
    SPI_SD_HANDLE.Init.DataSize = SPI_DATASIZE_8BIT;
    SPI_SD_HANDLE.Init.CLKPolarity = SPI_POLARITY_LOW;
    SPI_SD_HANDLE.Init.CLKPhase = SPI_PHASE_1EDGE;
    SPI_SD_HANDLE.Init.NSS = SPI_NSS_SOFT;
    SPI_SD_HANDLE.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    SPI_SD_HANDLE.Init.FirstBit = SPI_FIRSTBIT_MSB;
    SPI_SD_HANDLE.Init.TIMode = SPI_TIMODE_DISABLE;
    SPI_SD_HANDLE.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    SPI_SD_HANDLE.Init.CRCPolynomial = 7;
    SPI_SD_HANDLE.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    SPI_SD_HANDLE.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
    if(HAL_SPI_Init(&SPI_SD_HANDLE) != HAL_OK) {
        Error_Handler();
    }
}

/******************************************************************************
                            LINK OPERATIONS
 *******************************************************************************/

/********************************* LINK SD ************************************/
/**
 * @brief  Initialize the SD Card and put it into StandBy State (Ready for
 *         data transfer).
 * @retval None
 */
void SD_IO_Init(void) {
    uint8_t counter = 0;

    /* SD chip select high */
    SD_CS_HIGH();

    /* Send dummy byte 0xFF, 10 times with CS high */
    /* Rise CS and MOSI for 80 clocks cycles */
    for(counter = 0; counter <= 200; counter++) {
        /* Send dummy byte 0xFF */
        SD_IO_WriteByte(SD_DUMMY_BYTE);
    }
}

/**
 * @brief  Set SD interface Chip Select state
 * @param  val: 0 (low) or 1 (high) state
 * @retval None
 */
void SD_IO_CSState(uint8_t val) {
    if(val == 1) {
        SD_CS_HIGH();
    } else {
        SD_CS_LOW();
    }
}

/**
 * @brief  Write byte(s) on the SD
 * @param  DataIn: Pointer to data buffer to write
 * @param  DataOut: Pointer to data buffer for read data
 * @param  DataLength: number of bytes to write
 * @retval None
 */
void SD_IO_WriteReadData(const uint8_t* DataIn, uint8_t* DataOut, uint16_t DataLength) {
    /* Send the byte */
    SPIx_WriteReadData(DataIn, DataOut, DataLength);
}

/**
 * @brief  Write a byte on the SD.
 * @param  Data: byte to send.
 * @retval Data written
 */
uint8_t SD_IO_WriteByte(uint8_t Data) {
    uint8_t tmp;

    /* Send the byte */
    SPIx_WriteReadData(&Data, &tmp, 1);
    return tmp;
}
