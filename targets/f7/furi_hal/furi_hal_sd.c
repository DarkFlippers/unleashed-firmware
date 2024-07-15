#include <furi_hal_sd.h>
#include <stm32wbxx_ll_gpio.h>
#include <furi.h>
#include <furi_hal.h>
#include "../fatfs/sector_cache.h"
#define TAG "SdSpi"

#ifdef FURI_HAL_SD_SPI_DEBUG
#define sd_spi_debug(...) FURI_LOG_I(TAG, __VA_ARGS__)
#else
#define sd_spi_debug(...)
#endif

#define SD_CMD_LENGTH         (6)
#define SD_DUMMY_BYTE         (0xFF)
#define SD_ANSWER_RETRY_COUNT (8)
#define SD_IDLE_RETRY_COUNT   (100)
#define SD_TIMEOUT_MS         (1000)
#define SD_BLOCK_SIZE         (512)

#define FLAG_SET(x, y) (((x) & (y)) == (y))

static bool sd_high_capacity = false;

typedef enum {
    SdSpiDataResponceOK = 0x05,
    SdSpiDataResponceCRCError = 0x0B,
    SdSpiDataResponceWriteError = 0x0D,
    SdSpiDataResponceOtherError = 0xFF,
} SdSpiDataResponce;

typedef struct {
    uint8_t r1;
    uint8_t r2;
    uint8_t r3;
    uint8_t r4;
    uint8_t r5;
} SdSpiCmdAnswer;

typedef enum {
    SdSpiCmdAnswerTypeR1,
    SdSpiCmdAnswerTypeR1B,
    SdSpiCmdAnswerTypeR2,
    SdSpiCmdAnswerTypeR3,
    SdSpiCmdAnswerTypeR4R5,
    SdSpiCmdAnswerTypeR7,
} SdSpiCmdAnswerType;

/*
    SdSpiCmd and SdSpiToken use non-standard enum value names convention,
    because it is more convenient to look for documentation on a specific command.
    For example, to find out what the SD_CMD23_SET_BLOCK_COUNT command does, you need to look for
    SET_BLOCK_COUNT or CMD23 in the "Part 1 Physical Layer Simplified Specification".

    Do not use that naming convention in other places.
*/

typedef enum {
    SD_CMD0_GO_IDLE_STATE = 0,
    SD_CMD1_SEND_OP_COND = 1,
    SD_CMD8_SEND_IF_COND = 8,
    SD_CMD9_SEND_CSD = 9,
    SD_CMD10_SEND_CID = 10,
    SD_CMD12_STOP_TRANSMISSION = 12,
    SD_CMD13_SEND_STATUS = 13,
    SD_CMD16_SET_BLOCKLEN = 16,
    SD_CMD17_READ_SINGLE_BLOCK = 17,
    SD_CMD18_READ_MULT_BLOCK = 18,
    SD_CMD23_SET_BLOCK_COUNT = 23,
    SD_CMD24_WRITE_SINGLE_BLOCK = 24,
    SD_CMD25_WRITE_MULT_BLOCK = 25,
    SD_CMD27_PROG_CSD = 27,
    SD_CMD28_SET_WRITE_PROT = 28,
    SD_CMD29_CLR_WRITE_PROT = 29,
    SD_CMD30_SEND_WRITE_PROT = 30,
    SD_CMD32_SD_ERASE_GRP_START = 32,
    SD_CMD33_SD_ERASE_GRP_END = 33,
    SD_CMD34_UNTAG_SECTOR = 34,
    SD_CMD35_ERASE_GRP_START = 35,
    SD_CMD36_ERASE_GRP_END = 36,
    SD_CMD37_UNTAG_ERASE_GROUP = 37,
    SD_CMD38_ERASE = 38,
    SD_CMD41_SD_APP_OP_COND = 41,
    SD_CMD55_APP_CMD = 55,
    SD_CMD58_READ_OCR = 58,
} SdSpiCmd;

/** Data tokens */
typedef enum {
    SD_TOKEN_START_DATA_SINGLE_BLOCK_READ = 0xFE,
    SD_TOKEN_START_DATA_MULTIPLE_BLOCK_READ = 0xFE,
    SD_TOKEN_START_DATA_SINGLE_BLOCK_WRITE = 0xFE,
    SD_TOKEN_START_DATA_MULTIPLE_BLOCK_WRITE = 0xFC,
    SD_TOKEN_STOP_DATA_MULTIPLE_BLOCK_WRITE = 0xFD,
} SdSpiToken;

/** R1 answer value */
typedef enum {
    SdSpi_R1_NO_ERROR = 0x00,
    SdSpi_R1_IN_IDLE_STATE = 0x01,
    SdSpi_R1_ERASE_RESET = 0x02,
    SdSpi_R1_ILLEGAL_COMMAND = 0x04,
    SdSpi_R1_COM_CRC_ERROR = 0x08,
    SdSpi_R1_ERASE_SEQUENCE_ERROR = 0x10,
    SdSpi_R1_ADDRESS_ERROR = 0x20,
    SdSpi_R1_PARAMETER_ERROR = 0x40,
} SdSpiR1;

/** R2 answer value */
typedef enum {
    /* R2 answer value */
    SdSpi_R2_NO_ERROR = 0x00,
    SdSpi_R2_CARD_LOCKED = 0x01,
    SdSpi_R2_LOCKUNLOCK_ERROR = 0x02,
    SdSpi_R2_ERROR = 0x04,
    SdSpi_R2_CC_ERROR = 0x08,
    SdSpi_R2_CARD_ECC_FAILED = 0x10,
    SdSpi_R2_WP_VIOLATION = 0x20,
    SdSpi_R2_ERASE_PARAM = 0x40,
    SdSpi_R2_OUTOFRANGE = 0x80,
} SdSpiR2;

/**
 * @brief Card Specific Data: CSD Register
 */
typedef struct {
    /* Header part */
    uint8_t CSDStruct        : 2; /* CSD structure */
    uint8_t Reserved1        : 6; /* Reserved */
    uint8_t TAAC             : 8; /* Data read access-time 1 */
    uint8_t NSAC             : 8; /* Data read access-time 2 in CLK cycles */
    uint8_t MaxBusClkFreq    : 8; /* Max. bus clock frequency */
    uint16_t CardComdClasses : 12; /* Card command classes */
    uint8_t RdBlockLen       : 4; /* Max. read data block length */
    uint8_t PartBlockRead    : 1; /* Partial blocks for read allowed */
    uint8_t WrBlockMisalign  : 1; /* Write block misalignment */
    uint8_t RdBlockMisalign  : 1; /* Read block misalignment */
    uint8_t DSRImpl          : 1; /* DSR implemented */

    /* v1 or v2 struct */
    union csd_version {
        struct {
            uint8_t Reserved1          : 2; /* Reserved */
            uint16_t DeviceSize        : 12; /* Device Size */
            uint8_t MaxRdCurrentVDDMin : 3; /* Max. read current @ VDD min */
            uint8_t MaxRdCurrentVDDMax : 3; /* Max. read current @ VDD max */
            uint8_t MaxWrCurrentVDDMin : 3; /* Max. write current @ VDD min */
            uint8_t MaxWrCurrentVDDMax : 3; /* Max. write current @ VDD max */
            uint8_t DeviceSizeMul      : 3; /* Device size multiplier */
        } v1;
        struct {
            uint8_t Reserved1   : 6; /* Reserved */
            uint32_t DeviceSize : 22; /* Device Size */
            uint8_t Reserved2   : 1; /* Reserved */
        } v2;
    } version;

    uint8_t EraseSingleBlockEnable : 1; /* Erase single block enable */
    uint8_t EraseSectorSize        : 7; /* Erase group size multiplier */
    uint8_t WrProtectGrSize        : 7; /* Write protect group size */
    uint8_t WrProtectGrEnable      : 1; /* Write protect group enable */
    uint8_t Reserved2              : 2; /* Reserved */
    uint8_t WrSpeedFact            : 3; /* Write speed factor */
    uint8_t MaxWrBlockLen          : 4; /* Max. write data block length */
    uint8_t WriteBlockPartial      : 1; /* Partial blocks for write allowed */
    uint8_t Reserved3              : 5; /* Reserved */
    uint8_t FileFormatGrouop       : 1; /* File format group */
    uint8_t CopyFlag               : 1; /* Copy flag (OTP) */
    uint8_t PermWrProtect          : 1; /* Permanent write protection */
    uint8_t TempWrProtect          : 1; /* Temporary write protection */
    uint8_t FileFormat             : 2; /* File Format */
    uint8_t Reserved4              : 2; /* Reserved */
    uint8_t crc                    : 7; /* Reserved */
    uint8_t Reserved5              : 1; /* always 1*/

} SD_CSD;

/**
 * @brief Card Identification Data: CID Register
 */
typedef struct {
    uint8_t ManufacturerID; /* ManufacturerID */
    char OEM_AppliID[2]; /* OEM/Application ID */
    char ProdName[5]; /* Product Name */
    uint8_t ProdRev; /* Product Revision */
    uint32_t ProdSN; /* Product Serial Number */
    uint8_t Reserved1; /* Reserved1 */
    uint8_t ManufactYear; /* Manufacturing Year */
    uint8_t ManufactMonth; /* Manufacturing Month */
    uint8_t CID_CRC; /* CID CRC */
    uint8_t Reserved2; /* always 1 */
} SD_CID;

/**
 * @brief SD Card information structure
 */
typedef struct {
    SD_CSD Csd;
    SD_CID Cid;
    uint64_t CardCapacity; /*!< Card Capacity */
    uint32_t CardBlockSize; /*!< Card Block Size */
    uint32_t LogBlockNbr; /*!< Specifies the Card logical Capacity in blocks   */
    uint32_t LogBlockSize; /*!< Specifies logical block size in bytes           */
} SD_CardInfo;

/** Pointer to currently used SPI Handle */
FuriHalSpiBusHandle* furi_hal_sd_spi_handle = NULL;

static inline void sd_spi_select_card(void) {
    furi_hal_gpio_write(furi_hal_sd_spi_handle->cs, false);
    furi_delay_us(10); // Entry guard time for some SD cards
}

static inline void sd_spi_deselect_card(void) {
    furi_delay_us(10); // Exit guard time for some SD cards
    furi_hal_gpio_write(furi_hal_sd_spi_handle->cs, true);
}

static void sd_spi_bus_to_ground(void) {
    furi_hal_gpio_init_ex(
        furi_hal_sd_spi_handle->miso,
        GpioModeOutputPushPull,
        GpioPullNo,
        GpioSpeedVeryHigh,
        GpioAltFnUnused);
    furi_hal_gpio_init_ex(
        furi_hal_sd_spi_handle->mosi,
        GpioModeOutputPushPull,
        GpioPullNo,
        GpioSpeedVeryHigh,
        GpioAltFnUnused);
    furi_hal_gpio_init_ex(
        furi_hal_sd_spi_handle->sck,
        GpioModeOutputPushPull,
        GpioPullNo,
        GpioSpeedVeryHigh,
        GpioAltFnUnused);

    sd_spi_select_card();
    furi_hal_gpio_write(furi_hal_sd_spi_handle->miso, false);
    furi_hal_gpio_write(furi_hal_sd_spi_handle->mosi, false);
    furi_hal_gpio_write(furi_hal_sd_spi_handle->sck, false);
}

static void sd_spi_bus_rise_up(void) {
    sd_spi_deselect_card();

    furi_hal_gpio_init_ex(
        furi_hal_sd_spi_handle->miso,
        GpioModeAltFunctionPushPull,
        GpioPullUp,
        GpioSpeedVeryHigh,
        GpioAltFn5SPI2);
    furi_hal_gpio_init_ex(
        furi_hal_sd_spi_handle->mosi,
        GpioModeAltFunctionPushPull,
        GpioPullUp,
        GpioSpeedVeryHigh,
        GpioAltFn5SPI2);
    furi_hal_gpio_init_ex(
        furi_hal_sd_spi_handle->sck,
        GpioModeAltFunctionPushPull,
        GpioPullUp,
        GpioSpeedVeryHigh,
        GpioAltFn5SPI2);
}

static inline uint8_t sd_spi_read_byte(void) {
    uint8_t responce;
    furi_check(furi_hal_spi_bus_trx(furi_hal_sd_spi_handle, NULL, &responce, 1, SD_TIMEOUT_MS));
    return responce;
}

static inline void sd_spi_write_byte(uint8_t data) {
    furi_check(furi_hal_spi_bus_trx(furi_hal_sd_spi_handle, &data, NULL, 1, SD_TIMEOUT_MS));
}

static inline uint8_t sd_spi_write_and_read_byte(uint8_t data) {
    uint8_t responce;
    furi_check(furi_hal_spi_bus_trx(furi_hal_sd_spi_handle, &data, &responce, 1, SD_TIMEOUT_MS));
    return responce;
}

static inline void sd_spi_write_bytes(uint8_t* data, uint32_t size) {
    furi_check(furi_hal_spi_bus_trx(furi_hal_sd_spi_handle, data, NULL, size, SD_TIMEOUT_MS));
}

static inline void sd_spi_read_bytes(uint8_t* data, uint32_t size) {
    furi_check(furi_hal_spi_bus_trx(furi_hal_sd_spi_handle, NULL, data, size, SD_TIMEOUT_MS));
}

static inline void sd_spi_write_bytes_dma(uint8_t* data, uint32_t size) {
    uint32_t timeout_mul = (size / 512) + 1;
    furi_check(furi_hal_spi_bus_trx_dma(
        furi_hal_sd_spi_handle, data, NULL, size, SD_TIMEOUT_MS * timeout_mul));
}

static inline void sd_spi_read_bytes_dma(uint8_t* data, uint32_t size) {
    uint32_t timeout_mul = (size / 512) + 1;
    furi_check(furi_hal_spi_bus_trx_dma(
        furi_hal_sd_spi_handle, NULL, data, size, SD_TIMEOUT_MS * timeout_mul));
}

static uint8_t sd_spi_wait_for_data_and_read(void) {
    uint8_t retry_count = SD_ANSWER_RETRY_COUNT;
    uint8_t responce;

    // Wait until we get a valid data
    do {
        responce = sd_spi_read_byte();
        retry_count--;

    } while((responce == SD_DUMMY_BYTE) && retry_count);

    return responce;
}

static FuriStatus sd_spi_wait_for_data(uint8_t data, uint32_t timeout_ms) {
    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(timeout_ms * 1000);
    uint8_t byte;

    do {
        byte = sd_spi_read_byte();
        if(furi_hal_cortex_timer_is_expired(timer)) {
            return FuriStatusErrorTimeout;
        }
    } while(byte != data);

    return FuriStatusOk;
}

static inline void sd_spi_deselect_card_and_purge(void) {
    sd_spi_deselect_card();
    sd_spi_read_byte();
}

static inline void sd_spi_purge_crc(void) {
    sd_spi_read_byte();
    sd_spi_read_byte();
}

static SdSpiCmdAnswer
    sd_spi_send_cmd(SdSpiCmd cmd, uint32_t arg, uint8_t crc, SdSpiCmdAnswerType answer_type) {
    uint8_t frame[SD_CMD_LENGTH];
    SdSpiCmdAnswer cmd_answer = {
        .r1 = SD_DUMMY_BYTE,
        .r2 = SD_DUMMY_BYTE,
        .r3 = SD_DUMMY_BYTE,
        .r4 = SD_DUMMY_BYTE,
        .r5 = SD_DUMMY_BYTE,
    };

    // R1 Length = NCS(0)+ 6 Bytes command + NCR(min1 max8) + 1 Bytes answer + NEC(0) = 15bytes
    // R1b identical to R1 + Busy information
    // R2 Length = NCS(0)+ 6 Bytes command + NCR(min1 max8) + 2 Bytes answer + NEC(0) = 16bytes

    frame[0] = ((uint8_t)cmd | 0x40);
    frame[1] = (uint8_t)(arg >> 24);
    frame[2] = (uint8_t)(arg >> 16);
    frame[3] = (uint8_t)(arg >> 8);
    frame[4] = (uint8_t)(arg);
    frame[5] = (crc | 0x01);

    sd_spi_select_card();
    sd_spi_write_bytes(frame, sizeof(frame));

    switch(answer_type) {
    case SdSpiCmdAnswerTypeR1:
        cmd_answer.r1 = sd_spi_wait_for_data_and_read();
        break;
    case SdSpiCmdAnswerTypeR1B:
        // TODO FL-3507: can be wrong, at least for SD_CMD12_STOP_TRANSMISSION you need to purge one byte before reading R1
        cmd_answer.r1 = sd_spi_wait_for_data_and_read();

        // In general this shenenigans seems suspicious, please double check SD specs if you are using SdSpiCmdAnswerTypeR1B
        // reassert card
        sd_spi_deselect_card();
        furi_delay_us(1000);
        sd_spi_deselect_card();

        // and wait for it to be ready
        while(sd_spi_read_byte() != 0xFF) {
        };

        break;
    case SdSpiCmdAnswerTypeR2:
        cmd_answer.r1 = sd_spi_wait_for_data_and_read();
        cmd_answer.r2 = sd_spi_read_byte();
        break;
    case SdSpiCmdAnswerTypeR3:
    case SdSpiCmdAnswerTypeR7:
        cmd_answer.r1 = sd_spi_wait_for_data_and_read();
        cmd_answer.r2 = sd_spi_read_byte();
        cmd_answer.r3 = sd_spi_read_byte();
        cmd_answer.r4 = sd_spi_read_byte();
        cmd_answer.r5 = sd_spi_read_byte();
        break;
    default:
        break;
    }
    return cmd_answer;
}

static SdSpiDataResponce sd_spi_get_data_response(uint32_t timeout_ms) {
    SdSpiDataResponce responce = sd_spi_read_byte();
    // read busy response byte
    sd_spi_read_byte();

    switch(responce & 0x1F) {
    case SdSpiDataResponceOK:
        // TODO FL-3508: check timings
        sd_spi_deselect_card();
        sd_spi_select_card();

        // wait for 0xFF
        if(sd_spi_wait_for_data(0xFF, timeout_ms) == FuriStatusOk) {
            return SdSpiDataResponceOK;
        } else {
            return SdSpiDataResponceOtherError;
        }
    case SdSpiDataResponceCRCError:
        return SdSpiDataResponceCRCError;
    case SdSpiDataResponceWriteError:
        return SdSpiDataResponceWriteError;
    default:
        return SdSpiDataResponceOtherError;
    }
}

static FuriStatus sd_spi_init_spi_mode_v1(void) {
    SdSpiCmdAnswer response;
    uint8_t retry_count = 0;

    sd_spi_debug("Init SD card in SPI mode v1");

    do {
        retry_count++;

        // CMD55 (APP_CMD) before any ACMD command: R1 response (0x00: no errors)
        sd_spi_send_cmd(SD_CMD55_APP_CMD, 0, 0xFF, SdSpiCmdAnswerTypeR1);
        sd_spi_deselect_card_and_purge();

        // ACMD41 (SD_APP_OP_COND) to initialize SDHC or SDXC cards: R1 response (0x00: no errors)
        response = sd_spi_send_cmd(SD_CMD41_SD_APP_OP_COND, 0, 0xFF, SdSpiCmdAnswerTypeR1);
        sd_spi_deselect_card_and_purge();

        if(retry_count >= SD_IDLE_RETRY_COUNT) {
            return FuriStatusError;
        }
    } while(response.r1 == SdSpi_R1_IN_IDLE_STATE);

    sd_spi_debug("Init SD card in SPI mode v1 done");

    return FuriStatusOk;
}

static FuriStatus sd_spi_init_spi_mode_v2(void) {
    SdSpiCmdAnswer response;
    uint8_t retry_count = 0;

    sd_spi_debug("Init SD card in SPI mode v2");

    do {
        retry_count++;
        // CMD55 (APP_CMD) before any ACMD command: R1 response (0x00: no errors)
        sd_spi_send_cmd(SD_CMD55_APP_CMD, 0, 0xFF, SdSpiCmdAnswerTypeR1);
        sd_spi_deselect_card_and_purge();

        // ACMD41 (APP_OP_COND) to initialize SDHC or SDXC cards: R1 response (0x00: no errors)
        response =
            sd_spi_send_cmd(SD_CMD41_SD_APP_OP_COND, 0x40000000, 0xFF, SdSpiCmdAnswerTypeR1);
        sd_spi_deselect_card_and_purge();

        if(retry_count >= SD_IDLE_RETRY_COUNT) {
            sd_spi_debug("ACMD41 failed");
            return FuriStatusError;
        }
    } while(response.r1 == SdSpi_R1_IN_IDLE_STATE);

    if(FLAG_SET(response.r1, SdSpi_R1_ILLEGAL_COMMAND)) {
        sd_spi_debug("ACMD41 is illegal command");
        retry_count = 0;
        do {
            retry_count++;
            // CMD55 (APP_CMD) before any ACMD command: R1 response (0x00: no errors)
            response = sd_spi_send_cmd(SD_CMD55_APP_CMD, 0, 0xFF, SdSpiCmdAnswerTypeR1);
            sd_spi_deselect_card_and_purge();

            if(response.r1 != SdSpi_R1_IN_IDLE_STATE) {
                sd_spi_debug("CMD55 failed");
                return FuriStatusError;
            }
            // ACMD41 (SD_APP_OP_COND) to initialize SDHC or SDXC cards: R1 response (0x00: no errors)
            response = sd_spi_send_cmd(SD_CMD41_SD_APP_OP_COND, 0, 0xFF, SdSpiCmdAnswerTypeR1);
            sd_spi_deselect_card_and_purge();

            if(retry_count >= SD_IDLE_RETRY_COUNT) {
                sd_spi_debug("ACMD41 failed");
                return FuriStatusError;
            }
        } while(response.r1 == SdSpi_R1_IN_IDLE_STATE);
    }

    sd_spi_debug("Init SD card in SPI mode v2 done");

    return FuriStatusOk;
}

static FuriStatus sd_spi_init_spi_mode(void) {
    SdSpiCmdAnswer response;
    uint8_t retry_count;

    // CMD0 (GO_IDLE_STATE) to put SD in SPI mode and
    // wait for In Idle State Response (R1 Format) equal to 0x01
    retry_count = 0;
    do {
        retry_count++;
        response = sd_spi_send_cmd(SD_CMD0_GO_IDLE_STATE, 0, 0x95, SdSpiCmdAnswerTypeR1);
        sd_spi_deselect_card_and_purge();

        if(retry_count >= SD_IDLE_RETRY_COUNT) {
            sd_spi_debug("CMD0 failed");
            return FuriStatusError;
        }
    } while(response.r1 != SdSpi_R1_IN_IDLE_STATE);

    // CMD8 (SEND_IF_COND) to check the power supply status
    // and wait until response (R7 Format) equal to 0xAA and
    response = sd_spi_send_cmd(SD_CMD8_SEND_IF_COND, 0x1AA, 0x87, SdSpiCmdAnswerTypeR7);
    sd_spi_deselect_card_and_purge();

    if(FLAG_SET(response.r1, SdSpi_R1_ILLEGAL_COMMAND)) {
        if(sd_spi_init_spi_mode_v1() != FuriStatusOk) {
            sd_spi_debug("Init mode v1 failed");
            return FuriStatusError;
        }
        sd_high_capacity = 0;
    } else if(response.r1 == SdSpi_R1_IN_IDLE_STATE) {
        if(sd_spi_init_spi_mode_v2() != FuriStatusOk) {
            sd_spi_debug("Init mode v2 failed");
            return FuriStatusError;
        }

        // CMD58 (READ_OCR) to initialize SDHC or SDXC cards: R3 response
        response = sd_spi_send_cmd(SD_CMD58_READ_OCR, 0, 0xFF, SdSpiCmdAnswerTypeR3);
        sd_spi_deselect_card_and_purge();

        if(response.r1 != SdSpi_R1_NO_ERROR) {
            sd_spi_debug("CMD58 failed");
            return FuriStatusError;
        }
        sd_high_capacity = (response.r2 & 0x40) >> 6;
    } else {
        return FuriStatusError;
    }

    sd_spi_debug("SD card is %s", sd_high_capacity ? "SDHC or SDXC" : "SDSC");
    return FuriStatusOk;
}

static FuriStatus sd_spi_get_csd(SD_CSD* csd) {
    uint16_t counter = 0;
    uint8_t csd_data[16];
    FuriStatus ret = FuriStatusError;
    SdSpiCmdAnswer response;

    // CMD9 (SEND_CSD): R1 format (0x00 is no errors)
    response = sd_spi_send_cmd(SD_CMD9_SEND_CSD, 0, 0xFF, SdSpiCmdAnswerTypeR1);

    if(response.r1 == SdSpi_R1_NO_ERROR) {
        if(sd_spi_wait_for_data(SD_TOKEN_START_DATA_SINGLE_BLOCK_READ, SD_TIMEOUT_MS) ==
           FuriStatusOk) {
            // read CSD data
            for(counter = 0; counter < 16; counter++) {
                csd_data[counter] = sd_spi_read_byte();
            }

            sd_spi_purge_crc();

            /*************************************************************************
            CSD header decoding 
            *************************************************************************/

            csd->CSDStruct = (csd_data[0] & 0xC0) >> 6;
            csd->Reserved1 = csd_data[0] & 0x3F;
            csd->TAAC = csd_data[1];
            csd->NSAC = csd_data[2];
            csd->MaxBusClkFreq = csd_data[3];
            csd->CardComdClasses = (csd_data[4] << 4) | ((csd_data[5] & 0xF0) >> 4);
            csd->RdBlockLen = csd_data[5] & 0x0F;
            csd->PartBlockRead = (csd_data[6] & 0x80) >> 7;
            csd->WrBlockMisalign = (csd_data[6] & 0x40) >> 6;
            csd->RdBlockMisalign = (csd_data[6] & 0x20) >> 5;
            csd->DSRImpl = (csd_data[6] & 0x10) >> 4;

            /*************************************************************************
            CSD v1/v2 decoding  
            *************************************************************************/

            if(sd_high_capacity == 0) {
                csd->version.v1.Reserved1 = ((csd_data[6] & 0x0C) >> 2);
                csd->version.v1.DeviceSize = ((csd_data[6] & 0x03) << 10) | (csd_data[7] << 2) |
                                             ((csd_data[8] & 0xC0) >> 6);
                csd->version.v1.MaxRdCurrentVDDMin = (csd_data[8] & 0x38) >> 3;
                csd->version.v1.MaxRdCurrentVDDMax = (csd_data[8] & 0x07);
                csd->version.v1.MaxWrCurrentVDDMin = (csd_data[9] & 0xE0) >> 5;
                csd->version.v1.MaxWrCurrentVDDMax = (csd_data[9] & 0x1C) >> 2;
                csd->version.v1.DeviceSizeMul = ((csd_data[9] & 0x03) << 1) |
                                                ((csd_data[10] & 0x80) >> 7);
            } else {
                csd->version.v2.Reserved1 = ((csd_data[6] & 0x0F) << 2) |
                                            ((csd_data[7] & 0xC0) >> 6);
                csd->version.v2.DeviceSize = ((csd_data[7] & 0x3F) << 16) | (csd_data[8] << 8) |
                                             csd_data[9];
                csd->version.v2.Reserved2 = ((csd_data[10] & 0x80) >> 8);
            }

            csd->EraseSingleBlockEnable = (csd_data[10] & 0x40) >> 6;
            csd->EraseSectorSize = ((csd_data[10] & 0x3F) << 1) | ((csd_data[11] & 0x80) >> 7);
            csd->WrProtectGrSize = (csd_data[11] & 0x7F);
            csd->WrProtectGrEnable = (csd_data[12] & 0x80) >> 7;
            csd->Reserved2 = (csd_data[12] & 0x60) >> 5;
            csd->WrSpeedFact = (csd_data[12] & 0x1C) >> 2;
            csd->MaxWrBlockLen = ((csd_data[12] & 0x03) << 2) | ((csd_data[13] & 0xC0) >> 6);
            csd->WriteBlockPartial = (csd_data[13] & 0x20) >> 5;
            csd->Reserved3 = (csd_data[13] & 0x1F);
            csd->FileFormatGrouop = (csd_data[14] & 0x80) >> 7;
            csd->CopyFlag = (csd_data[14] & 0x40) >> 6;
            csd->PermWrProtect = (csd_data[14] & 0x20) >> 5;
            csd->TempWrProtect = (csd_data[14] & 0x10) >> 4;
            csd->FileFormat = (csd_data[14] & 0x0C) >> 2;
            csd->Reserved4 = (csd_data[14] & 0x03);
            csd->crc = (csd_data[15] & 0xFE) >> 1;
            csd->Reserved5 = (csd_data[15] & 0x01);

            ret = FuriStatusOk;
        }
    }

    sd_spi_deselect_card_and_purge();

    return ret;
}

static FuriStatus sd_spi_get_cid(SD_CID* Cid) {
    uint16_t counter = 0;
    uint8_t cid_data[16];
    FuriStatus ret = FuriStatusError;
    SdSpiCmdAnswer response;

    // CMD10 (SEND_CID): R1 format (0x00 is no errors)
    response = sd_spi_send_cmd(SD_CMD10_SEND_CID, 0, 0xFF, SdSpiCmdAnswerTypeR1);

    if(response.r1 == SdSpi_R1_NO_ERROR) {
        if(sd_spi_wait_for_data(SD_TOKEN_START_DATA_SINGLE_BLOCK_READ, SD_TIMEOUT_MS) ==
           FuriStatusOk) {
            // read CID data
            for(counter = 0; counter < 16; counter++) {
                cid_data[counter] = sd_spi_read_byte();
            }

            sd_spi_purge_crc();

            Cid->ManufacturerID = cid_data[0];
            memcpy(Cid->OEM_AppliID, cid_data + 1, 2);
            memcpy(Cid->ProdName, cid_data + 3, 5);
            Cid->ProdRev = cid_data[8];
            Cid->ProdSN = cid_data[9] << 24;
            Cid->ProdSN |= cid_data[10] << 16;
            Cid->ProdSN |= cid_data[11] << 8;
            Cid->ProdSN |= cid_data[12];
            Cid->Reserved1 = (cid_data[13] & 0xF0) >> 4;
            Cid->ManufactYear = (cid_data[13] & 0x0F) << 4;
            Cid->ManufactYear |= (cid_data[14] & 0xF0) >> 4;
            Cid->ManufactMonth = (cid_data[14] & 0x0F);
            Cid->CID_CRC = (cid_data[15] & 0xFE) >> 1;
            Cid->Reserved2 = 1;

            ret = FuriStatusOk;
        }
    }

    sd_spi_deselect_card_and_purge();

    return ret;
}

static FuriStatus
    sd_spi_cmd_read_blocks(uint32_t* data, uint32_t address, uint32_t blocks, uint32_t timeout_ms) {
    uint32_t block_address = address;
    uint32_t offset = 0;

    // CMD16 (SET_BLOCKLEN): R1 response (0x00: no errors)
    SdSpiCmdAnswer response =
        sd_spi_send_cmd(SD_CMD16_SET_BLOCKLEN, SD_BLOCK_SIZE, 0xFF, SdSpiCmdAnswerTypeR1);
    sd_spi_deselect_card_and_purge();

    if(response.r1 != SdSpi_R1_NO_ERROR) {
        return FuriStatusError;
    }

    if(!sd_high_capacity) {
        block_address = address * SD_BLOCK_SIZE;
    }

    while(blocks--) {
        // CMD17 (READ_SINGLE_BLOCK): R1 response (0x00: no errors)
        response =
            sd_spi_send_cmd(SD_CMD17_READ_SINGLE_BLOCK, block_address, 0xFF, SdSpiCmdAnswerTypeR1);
        if(response.r1 != SdSpi_R1_NO_ERROR) {
            sd_spi_deselect_card_and_purge();
            return FuriStatusError;
        }

        // Wait for the data start token
        if(sd_spi_wait_for_data(SD_TOKEN_START_DATA_SINGLE_BLOCK_READ, timeout_ms) ==
           FuriStatusOk) {
            // Read the data block
            sd_spi_read_bytes_dma((uint8_t*)data + offset, SD_BLOCK_SIZE);
            sd_spi_purge_crc();

            // increase offset
            offset += SD_BLOCK_SIZE;

            // increase block address
            if(sd_high_capacity) {
                block_address += 1;
            } else {
                block_address += SD_BLOCK_SIZE;
            }
        } else {
            sd_spi_deselect_card_and_purge();
            return FuriStatusError;
        }

        sd_spi_deselect_card_and_purge();
    }

    return FuriStatusOk;
}

static FuriStatus sd_spi_cmd_write_blocks(
    const uint32_t* data,
    uint32_t address,
    uint32_t blocks,
    uint32_t timeout_ms) {
    uint32_t block_address = address;
    uint32_t offset = 0;

    // CMD16 (SET_BLOCKLEN): R1 response (0x00: no errors)
    SdSpiCmdAnswer response =
        sd_spi_send_cmd(SD_CMD16_SET_BLOCKLEN, SD_BLOCK_SIZE, 0xFF, SdSpiCmdAnswerTypeR1);
    sd_spi_deselect_card_and_purge();

    if(response.r1 != SdSpi_R1_NO_ERROR) {
        return FuriStatusError;
    }

    if(!sd_high_capacity) {
        block_address = address * SD_BLOCK_SIZE;
    }

    while(blocks--) {
        // CMD24 (WRITE_SINGLE_BLOCK): R1 response (0x00: no errors)
        response = sd_spi_send_cmd(
            SD_CMD24_WRITE_SINGLE_BLOCK, block_address, 0xFF, SdSpiCmdAnswerTypeR1);
        if(response.r1 != SdSpi_R1_NO_ERROR) {
            sd_spi_deselect_card_and_purge();
            return FuriStatusError;
        }

        // Send dummy byte for NWR timing : one byte between CMD_WRITE and TOKEN
        // TODO FL-3509: check bytes count
        sd_spi_write_byte(SD_DUMMY_BYTE);
        sd_spi_write_byte(SD_DUMMY_BYTE);

        // Send the data start token
        sd_spi_write_byte(SD_TOKEN_START_DATA_SINGLE_BLOCK_WRITE);
        sd_spi_write_bytes_dma((uint8_t*)data + offset, SD_BLOCK_SIZE);
        sd_spi_purge_crc();

        // Read data response
        SdSpiDataResponce data_responce = sd_spi_get_data_response(timeout_ms);
        sd_spi_deselect_card_and_purge();

        if(data_responce != SdSpiDataResponceOK) {
            return FuriStatusError;
        }

        // increase offset
        offset += SD_BLOCK_SIZE;

        // increase block address
        if(sd_high_capacity) {
            block_address += 1;
        } else {
            block_address += SD_BLOCK_SIZE;
        }
    }

    return FuriStatusOk;
}

static FuriStatus sd_spi_get_card_state(void) {
    SdSpiCmdAnswer response;

    // Send CMD13 (SEND_STATUS) to get SD status
    response = sd_spi_send_cmd(SD_CMD13_SEND_STATUS, 0, 0xFF, SdSpiCmdAnswerTypeR2);
    sd_spi_deselect_card_and_purge();

    // Return status OK if response is valid
    if((response.r1 == SdSpi_R1_NO_ERROR) && (response.r2 == SdSpi_R2_NO_ERROR)) {
        return FuriStatusOk;
    }

    return FuriStatusError;
}

static inline bool sd_cache_get(uint32_t address, uint32_t* data) {
    uint8_t* cached_data = sector_cache_get(address);
    if(cached_data) {
        memcpy(data, cached_data, SD_BLOCK_SIZE);
        return true;
    }
    return false;
}

static inline void sd_cache_put(uint32_t address, uint32_t* data) {
    sector_cache_put(address, (uint8_t*)data);
}

static inline void sd_cache_invalidate_range(uint32_t start_sector, uint32_t end_sector) {
    sector_cache_invalidate_range(start_sector, end_sector);
}

static inline void sd_cache_invalidate_all(void) {
    sector_cache_init();
}

static FuriStatus sd_device_read(uint32_t* buff, uint32_t sector, uint32_t count) {
    FuriStatus status = FuriStatusError;

    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_sd_fast);
    furi_hal_sd_spi_handle = &furi_hal_spi_bus_handle_sd_fast;

    if(sd_spi_cmd_read_blocks(buff, sector, count, SD_TIMEOUT_MS) == FuriStatusOk) {
        FuriHalCortexTimer timer = furi_hal_cortex_timer_get(SD_TIMEOUT_MS * 1000);

        /* wait until the read operation is finished */
        do {
            status = sd_spi_get_card_state();

            if(furi_hal_cortex_timer_is_expired(timer)) {
                status = FuriStatusErrorTimeout;
                break;
            }
        } while(status != FuriStatusOk);
    }

    furi_hal_sd_spi_handle = NULL;
    furi_hal_spi_release(&furi_hal_spi_bus_handle_sd_fast);

    return status;
}

static FuriStatus sd_device_write(const uint32_t* buff, uint32_t sector, uint32_t count) {
    FuriStatus status = FuriStatusError;

    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_sd_fast);
    furi_hal_sd_spi_handle = &furi_hal_spi_bus_handle_sd_fast;

    if(sd_spi_cmd_write_blocks(buff, sector, count, SD_TIMEOUT_MS) == FuriStatusOk) {
        FuriHalCortexTimer timer = furi_hal_cortex_timer_get(SD_TIMEOUT_MS * 1000);

        /* wait until the Write operation is finished */
        do {
            status = sd_spi_get_card_state();

            if(furi_hal_cortex_timer_is_expired(timer)) {
                sd_cache_invalidate_all();

                status = FuriStatusErrorTimeout;
                break;
            }
        } while(status != FuriStatusOk);
    }

    furi_hal_sd_spi_handle = NULL;
    furi_hal_spi_release(&furi_hal_spi_bus_handle_sd_fast);

    return status;
}

void furi_hal_sd_presence_init(void) {
    // low speed input with pullup
    furi_hal_gpio_init(&gpio_sdcard_cd, GpioModeInput, GpioPullUp, GpioSpeedLow);
}

static void furi_hal_sd_present_pin_set_low(void) {
    // low speed input with pullup
    furi_hal_gpio_init_simple(&gpio_sdcard_cd, GpioModeOutputOpenDrain);
    furi_hal_gpio_write(&gpio_sdcard_cd, 0);
}

bool furi_hal_sd_is_present(void) {
    bool result = !furi_hal_gpio_read(&gpio_sdcard_cd);
    return result;
}

uint8_t furi_hal_sd_max_mount_retry_count(void) {
    return 10;
}

FuriStatus furi_hal_sd_init(bool power_reset) {
    // Slow speed init
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_sd_slow);
    furi_hal_sd_spi_handle = &furi_hal_spi_bus_handle_sd_slow;

    // We reset card in spi_lock context, so it is safe to disturb spi bus
    if(power_reset) {
        sd_spi_debug("Power reset");

        // disable power and set low on all bus pins
        furi_hal_power_disable_external_3_3v();
        sd_spi_bus_to_ground();
        furi_hal_sd_present_pin_set_low();
        furi_delay_ms(250);

        // reinit bus and enable power
        sd_spi_bus_rise_up();
        furi_hal_sd_presence_init();
        furi_hal_power_enable_external_3_3v();
        furi_delay_ms(100);
    }

    FuriStatus status = FuriStatusError;

    // Send 80 dummy clocks with CS high
    sd_spi_deselect_card();
    for(uint8_t i = 0; i < 80; i++) {
        sd_spi_write_byte(SD_DUMMY_BYTE);
    }

    for(uint8_t i = 0; i < 128; i++) {
        status = sd_spi_init_spi_mode();
        if(status == FuriStatusOk) {
            // SD initialized and init to SPI mode properly
            sd_spi_debug("SD init OK after %d retries", i);
            break;
        }
    }

    furi_hal_sd_spi_handle = NULL;
    furi_hal_spi_release(&furi_hal_spi_bus_handle_sd_slow);

    // Init sector cache
    sector_cache_init();

    return status;
}

FuriStatus furi_hal_sd_get_card_state(void) {
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_sd_fast);
    furi_hal_sd_spi_handle = &furi_hal_spi_bus_handle_sd_fast;

    FuriStatus status = sd_spi_get_card_state();

    furi_hal_sd_spi_handle = NULL;
    furi_hal_spi_release(&furi_hal_spi_bus_handle_sd_fast);

    return status;
}

FuriStatus furi_hal_sd_read_blocks(uint32_t* buff, uint32_t sector, uint32_t count) {
    furi_check(buff);

    FuriStatus status;
    bool single_sector = count == 1;

    if(single_sector) {
        if(sd_cache_get(sector, buff)) {
            return FuriStatusOk;
        }
    }

    status = sd_device_read(buff, sector, count);

    if(status != FuriStatusOk) {
        uint8_t counter = furi_hal_sd_max_mount_retry_count();

        while(status != FuriStatusOk && counter > 0 && furi_hal_sd_is_present()) {
            if((counter % 2) == 0) {
                // power reset sd card
                status = furi_hal_sd_init(true);
            } else {
                status = furi_hal_sd_init(false);
            }

            if(status == FuriStatusOk) {
                status = sd_device_read(buff, sector, count);
            }
            counter--;
        }
    }

    if(single_sector && status == FuriStatusOk) {
        sd_cache_put(sector, buff);
    }

    return status;
}

FuriStatus furi_hal_sd_write_blocks(const uint32_t* buff, uint32_t sector, uint32_t count) {
    furi_check(buff);

    FuriStatus status;

    sd_cache_invalidate_range(sector, sector + count);

    status = sd_device_write(buff, sector, count);

    if(status != FuriStatusOk) {
        uint8_t counter = furi_hal_sd_max_mount_retry_count();

        while(status != FuriStatusOk && counter > 0 && furi_hal_sd_is_present()) {
            if((counter % 2) == 0) {
                // power reset sd card
                status = furi_hal_sd_init(true);
            } else {
                status = furi_hal_sd_init(false);
            }

            if(status == FuriStatusOk) {
                status = sd_device_write(buff, sector, count);
            }
            counter--;
        }
    }

    return status;
}

FuriStatus furi_hal_sd_info(FuriHalSdInfo* info) {
    furi_check(info);

    FuriStatus status;
    SD_CSD csd;
    SD_CID cid;

    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_sd_fast);
    furi_hal_sd_spi_handle = &furi_hal_spi_bus_handle_sd_fast;

    do {
        status = sd_spi_get_csd(&csd);

        if(status != FuriStatusOk) {
            break;
        }

        status = sd_spi_get_cid(&cid);

        if(status != FuriStatusOk) {
            break;
        }

        if(sd_high_capacity == 1) {
            info->logical_block_size = 512;
            info->block_size = 512;
            info->capacity = ((uint64_t)csd.version.v2.DeviceSize + 1UL) * 1024UL *
                             (uint64_t)info->logical_block_size;
            info->logical_block_count = (info->capacity) / (info->logical_block_size);
        } else {
            info->capacity = (csd.version.v1.DeviceSize + 1);
            info->capacity *= (1UL << (csd.version.v1.DeviceSizeMul + 2));
            info->logical_block_size = 512;
            info->block_size = 1UL << (csd.RdBlockLen);
            info->capacity *= info->block_size;
            info->logical_block_count = (info->capacity) / (info->logical_block_size);
        }

        info->manufacturer_id = cid.ManufacturerID;

        memcpy(info->oem_id, cid.OEM_AppliID, sizeof(info->oem_id) - 1);
        info->oem_id[sizeof(info->oem_id) - 1] = '\0';

        memcpy(info->product_name, cid.ProdName, sizeof(info->product_name) - 1);
        info->product_name[sizeof(info->product_name) - 1] = '\0';

        info->product_revision_major = cid.ProdRev >> 4;
        info->product_revision_minor = cid.ProdRev & 0x0F;
        info->product_serial_number = cid.ProdSN;
        info->manufacturing_year = 2000 + cid.ManufactYear;
        info->manufacturing_month = cid.ManufactMonth;

    } while(false);

    furi_hal_sd_spi_handle = NULL;
    furi_hal_spi_release(&furi_hal_spi_bus_handle_sd_fast);

    return status;
}
