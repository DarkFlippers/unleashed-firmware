#include "sd_spi_io.h"
#include "sector_cache.h"
#include <furi.h>
#include <furi_hal.h>
#include <furi/core/core_defines.h>

// #define SD_SPI_DEBUG 1
#define TAG "SdSpi"

#ifdef SD_SPI_DEBUG
#define sd_spi_debug(...) FURI_LOG_I(TAG, __VA_ARGS__)
#else
#define sd_spi_debug(...)
#endif

#define SD_CMD_LENGTH 6
#define SD_DUMMY_BYTE 0xFF
#define SD_ANSWER_RETRY_COUNT 8
#define SD_IDLE_RETRY_COUNT 100

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

static inline void sd_spi_select_card() {
    furi_hal_gpio_write(furi_hal_sd_spi_handle->cs, false);
    furi_delay_us(10); // Entry guard time for some SD cards
}

static inline void sd_spi_deselect_card() {
    furi_delay_us(10); // Exit guard time for some SD cards
    furi_hal_gpio_write(furi_hal_sd_spi_handle->cs, true);
}

static void sd_spi_bus_to_ground() {
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

static void sd_spi_bus_rise_up() {
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

static SdSpiStatus sd_spi_wait_for_data(uint8_t data, uint32_t timeout_ms) {
    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(timeout_ms * 1000);
    uint8_t byte;

    do {
        byte = sd_spi_read_byte();
        if(furi_hal_cortex_timer_is_expired(timer)) {
            return SdSpiStatusTimeout;
        }
    } while((byte != data));

    return SdSpiStatusOK;
}

static inline void sd_spi_deselect_card_and_purge() {
    sd_spi_deselect_card();
    sd_spi_read_byte();
}

static inline void sd_spi_purge_crc() {
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
        if(sd_spi_wait_for_data(0xFF, timeout_ms) == SdSpiStatusOK) {
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

static SdSpiStatus sd_spi_init_spi_mode_v1(void) {
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
            return SdSpiStatusError;
        }
    } while(response.r1 == SdSpi_R1_IN_IDLE_STATE);

    sd_spi_debug("Init SD card in SPI mode v1 done");

    return SdSpiStatusOK;
}

static SdSpiStatus sd_spi_init_spi_mode_v2(void) {
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
            return SdSpiStatusError;
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
                return SdSpiStatusError;
            }
            // ACMD41 (SD_APP_OP_COND) to initialize SDHC or SDXC cards: R1 response (0x00: no errors)
            response = sd_spi_send_cmd(SD_CMD41_SD_APP_OP_COND, 0, 0xFF, SdSpiCmdAnswerTypeR1);
            sd_spi_deselect_card_and_purge();

            if(retry_count >= SD_IDLE_RETRY_COUNT) {
                sd_spi_debug("ACMD41 failed");
                return SdSpiStatusError;
            }
        } while(response.r1 == SdSpi_R1_IN_IDLE_STATE);
    }

    sd_spi_debug("Init SD card in SPI mode v2 done");

    return SdSpiStatusOK;
}

static SdSpiStatus sd_spi_init_spi_mode(void) {
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
            return SdSpiStatusError;
        }
    } while(response.r1 != SdSpi_R1_IN_IDLE_STATE);

    // CMD8 (SEND_IF_COND) to check the power supply status
    // and wait until response (R7 Format) equal to 0xAA and
    response = sd_spi_send_cmd(SD_CMD8_SEND_IF_COND, 0x1AA, 0x87, SdSpiCmdAnswerTypeR7);
    sd_spi_deselect_card_and_purge();

    if(FLAG_SET(response.r1, SdSpi_R1_ILLEGAL_COMMAND)) {
        if(sd_spi_init_spi_mode_v1() != SdSpiStatusOK) {
            sd_spi_debug("Init mode v1 failed");
            return SdSpiStatusError;
        }
        sd_high_capacity = 0;
    } else if(response.r1 == SdSpi_R1_IN_IDLE_STATE) {
        if(sd_spi_init_spi_mode_v2() != SdSpiStatusOK) {
            sd_spi_debug("Init mode v2 failed");
            return SdSpiStatusError;
        }

        // CMD58 (READ_OCR) to initialize SDHC or SDXC cards: R3 response
        response = sd_spi_send_cmd(SD_CMD58_READ_OCR, 0, 0xFF, SdSpiCmdAnswerTypeR3);
        sd_spi_deselect_card_and_purge();

        if(response.r1 != SdSpi_R1_NO_ERROR) {
            sd_spi_debug("CMD58 failed");
            return SdSpiStatusError;
        }
        sd_high_capacity = (response.r2 & 0x40) >> 6;
    } else {
        return SdSpiStatusError;
    }

    sd_spi_debug("SD card is %s", sd_high_capacity ? "SDHC or SDXC" : "SDSC");
    return SdSpiStatusOK;
}

static SdSpiStatus sd_spi_get_csd(SD_CSD* csd) {
    uint16_t counter = 0;
    uint8_t csd_data[16];
    SdSpiStatus ret = SdSpiStatusError;
    SdSpiCmdAnswer response;

    // CMD9 (SEND_CSD): R1 format (0x00 is no errors)
    response = sd_spi_send_cmd(SD_CMD9_SEND_CSD, 0, 0xFF, SdSpiCmdAnswerTypeR1);

    if(response.r1 == SdSpi_R1_NO_ERROR) {
        if(sd_spi_wait_for_data(SD_TOKEN_START_DATA_SINGLE_BLOCK_READ, SD_TIMEOUT_MS) ==
           SdSpiStatusOK) {
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
            csd->MaxBusClkFrec = csd_data[3];
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

            ret = SdSpiStatusOK;
        }
    }

    sd_spi_deselect_card_and_purge();

    return ret;
}

static SdSpiStatus sd_spi_get_cid(SD_CID* Cid) {
    uint16_t counter = 0;
    uint8_t cid_data[16];
    SdSpiStatus ret = SdSpiStatusError;
    SdSpiCmdAnswer response;

    // CMD10 (SEND_CID): R1 format (0x00 is no errors)
    response = sd_spi_send_cmd(SD_CMD10_SEND_CID, 0, 0xFF, SdSpiCmdAnswerTypeR1);

    if(response.r1 == SdSpi_R1_NO_ERROR) {
        if(sd_spi_wait_for_data(SD_TOKEN_START_DATA_SINGLE_BLOCK_READ, SD_TIMEOUT_MS) ==
           SdSpiStatusOK) {
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

            ret = SdSpiStatusOK;
        }
    }

    sd_spi_deselect_card_and_purge();

    return ret;
}

static SdSpiStatus
    sd_spi_cmd_read_blocks(uint32_t* data, uint32_t address, uint32_t blocks, uint32_t timeout_ms) {
    uint32_t block_address = address;
    uint32_t offset = 0;

    // CMD16 (SET_BLOCKLEN): R1 response (0x00: no errors)
    SdSpiCmdAnswer response =
        sd_spi_send_cmd(SD_CMD16_SET_BLOCKLEN, SD_BLOCK_SIZE, 0xFF, SdSpiCmdAnswerTypeR1);
    sd_spi_deselect_card_and_purge();

    if(response.r1 != SdSpi_R1_NO_ERROR) {
        return SdSpiStatusError;
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
            return SdSpiStatusError;
        }

        // Wait for the data start token
        if(sd_spi_wait_for_data(SD_TOKEN_START_DATA_SINGLE_BLOCK_READ, timeout_ms) ==
           SdSpiStatusOK) {
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
            return SdSpiStatusError;
        }

        sd_spi_deselect_card_and_purge();
    }

    return SdSpiStatusOK;
}

static SdSpiStatus sd_spi_cmd_write_blocks(
    uint32_t* data,
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
        return SdSpiStatusError;
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
            return SdSpiStatusError;
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
            return SdSpiStatusError;
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

    return SdSpiStatusOK;
}

uint8_t sd_max_mount_retry_count() {
    return 10;
}

SdSpiStatus sd_init(bool power_reset) {
    // Slow speed init
    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_sd_slow);
    furi_hal_sd_spi_handle = &furi_hal_spi_bus_handle_sd_slow;

    // We reset card in spi_lock context, so it is safe to disturb spi bus
    if(power_reset) {
        sd_spi_debug("Power reset");

        // disable power and set low on all bus pins
        furi_hal_power_disable_external_3_3v();
        sd_spi_bus_to_ground();
        hal_sd_detect_set_low();
        furi_delay_ms(250);

        // reinit bus and enable power
        sd_spi_bus_rise_up();
        hal_sd_detect_init();
        furi_hal_power_enable_external_3_3v();
        furi_delay_ms(100);
    }

    SdSpiStatus status = SdSpiStatusError;

    // Send 80 dummy clocks with CS high
    sd_spi_deselect_card();
    for(uint8_t i = 0; i < 80; i++) {
        sd_spi_write_byte(SD_DUMMY_BYTE);
    }

    for(uint8_t i = 0; i < 128; i++) {
        status = sd_spi_init_spi_mode();
        if(status == SdSpiStatusOK) {
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

SdSpiStatus sd_get_card_state(void) {
    SdSpiCmdAnswer response;

    // Send CMD13 (SEND_STATUS) to get SD status
    response = sd_spi_send_cmd(SD_CMD13_SEND_STATUS, 0, 0xFF, SdSpiCmdAnswerTypeR2);
    sd_spi_deselect_card_and_purge();

    // Return status OK if response is valid
    if((response.r1 == SdSpi_R1_NO_ERROR) && (response.r2 == SdSpi_R2_NO_ERROR)) {
        return SdSpiStatusOK;
    }

    return SdSpiStatusError;
}

SdSpiStatus sd_get_card_info(SD_CardInfo* card_info) {
    SdSpiStatus status;

    status = sd_spi_get_csd(&(card_info->Csd));

    if(status != SdSpiStatusOK) {
        return status;
    }

    status = sd_spi_get_cid(&(card_info->Cid));

    if(status != SdSpiStatusOK) {
        return status;
    }

    if(sd_high_capacity == 1) {
        card_info->LogBlockSize = 512;
        card_info->CardBlockSize = 512;
        card_info->CardCapacity = ((uint64_t)card_info->Csd.version.v2.DeviceSize + 1UL) * 1024UL *
                                  (uint64_t)card_info->LogBlockSize;
        card_info->LogBlockNbr = (card_info->CardCapacity) / (card_info->LogBlockSize);
    } else {
        card_info->CardCapacity = (card_info->Csd.version.v1.DeviceSize + 1);
        card_info->CardCapacity *= (1UL << (card_info->Csd.version.v1.DeviceSizeMul + 2));
        card_info->LogBlockSize = 512;
        card_info->CardBlockSize = 1UL << (card_info->Csd.RdBlockLen);
        card_info->CardCapacity *= card_info->CardBlockSize;
        card_info->LogBlockNbr = (card_info->CardCapacity) / (card_info->LogBlockSize);
    }

    return status;
}

SdSpiStatus
    sd_read_blocks(uint32_t* data, uint32_t address, uint32_t blocks, uint32_t timeout_ms) {
    SdSpiStatus status = sd_spi_cmd_read_blocks(data, address, blocks, timeout_ms);
    return status;
}

SdSpiStatus
    sd_write_blocks(uint32_t* data, uint32_t address, uint32_t blocks, uint32_t timeout_ms) {
    SdSpiStatus status = sd_spi_cmd_write_blocks(data, address, blocks, timeout_ms);
    return status;
}

SdSpiStatus sd_get_cid(SD_CID* cid) {
    SdSpiStatus status;

    furi_hal_spi_acquire(&furi_hal_spi_bus_handle_sd_fast);
    furi_hal_sd_spi_handle = &furi_hal_spi_bus_handle_sd_fast;

    memset(cid, 0, sizeof(SD_CID));
    status = sd_spi_get_cid(cid);

    furi_hal_sd_spi_handle = NULL;
    furi_hal_spi_release(&furi_hal_spi_bus_handle_sd_fast);

    return status;
}