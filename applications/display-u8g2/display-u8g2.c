#include "u8g2/u8g2.h"
#include "flipper.h"

extern SPI_HandleTypeDef hspi1;

// #define DEBUG 1

// TODO rewrite u8g2 to pass thread-local context in this handlers

static uint8_t
u8g2_gpio_and_delay_stm32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr) {
    switch(msg) {
    //Initialize SPI peripheral
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
        /* HAL initialization contains all what we need so we can skip this part. */
        break;

    //Function which implements a delay, arg_int contains the amount of ms
    case U8X8_MSG_DELAY_MILLI:
        osDelay(arg_int);
        break;

    //Function which delays 10us
    case U8X8_MSG_DELAY_10MICRO:
        delay_us(10);
        break;

    //Function which delays 100ns
    case U8X8_MSG_DELAY_100NANO:
        asm("nop");
        break;

    // Function to define the logic level of the RESET line
    case U8X8_MSG_GPIO_RESET:
#ifdef DEBUG
        fuprintf(log, "[u8g2] rst %d\n", arg_int);
#endif

        // TODO change it to FuriRecord pin
        HAL_GPIO_WritePin(
            DISPLAY_RST_GPIO_Port, DISPLAY_RST_Pin, arg_int ? GPIO_PIN_SET : GPIO_PIN_RESET);
        break;

    default:
#ifdef DEBUG
        fufuprintf(log, "[u8g2] unknown io %d\n", msg);
#endif

        return 0; //A message was received which is not implemented, return 0 to indicate an error
    }

    return 1; // command processed successfully.
}

static uint8_t u8x8_hw_spi_stm32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr) {
    switch(msg) {
    case U8X8_MSG_BYTE_SEND:
#ifdef DEBUG
        fuprintf(log, "[u8g2] send %d bytes %02X\n", arg_int, ((uint8_t*)arg_ptr)[0]);
#endif

        // TODO change it to FuriRecord SPI
        HAL_SPI_Transmit(&hspi1, (uint8_t*)arg_ptr, arg_int, 10000);
        break;

    case U8X8_MSG_BYTE_SET_DC:
#ifdef DEBUG
        fuprintf(log, "[u8g2] dc %d\n", arg_int);
#endif

        // TODO change it to FuriRecord pin
        HAL_GPIO_WritePin(
            DISPLAY_DI_GPIO_Port, DISPLAY_DI_Pin, arg_int ? GPIO_PIN_SET : GPIO_PIN_RESET);
        break;

    case U8X8_MSG_BYTE_INIT:
#ifdef DEBUG
        fuprintf(log, "[u8g2] init\n");
#endif

        // TODO change it to FuriRecord pin
        HAL_GPIO_WritePin(DISPLAY_CS_GPIO_Port, DISPLAY_CS_Pin, GPIO_PIN_RESET);
        break;

    case U8X8_MSG_BYTE_START_TRANSFER:
#ifdef DEBUG
        fuprintf(log, "[u8g2] start\n");
#endif

        // TODO change it to FuriRecord pin
        HAL_GPIO_WritePin(DISPLAY_CS_GPIO_Port, DISPLAY_CS_Pin, GPIO_PIN_RESET);
        asm("nop");
        break;

    case U8X8_MSG_BYTE_END_TRANSFER:
#ifdef DEBUG
        fuprintf(log, "[u8g2] end\n");
#endif

        asm("nop");
        // TODO change it to FuriRecord pin
        HAL_GPIO_WritePin(DISPLAY_CS_GPIO_Port, DISPLAY_CS_Pin, GPIO_PIN_SET);
        break;

    default:
#ifdef DEBUG
        fuprintf(log, "[u8g2] unknown xfer %d\n", msg);
#endif

        return 0;
    }

    return 1;
}

typedef struct {
    SemaphoreHandle_t update; // queue to pass events from callback to app thread
    FuriRecordSubscriber* log; // app logger
} DisplayCtx;

static void handle_fb_change(const void* fb, size_t fb_size, void* raw_ctx) {
    DisplayCtx* ctx = (DisplayCtx*)raw_ctx; // make right type

    fuprintf(ctx->log, "[display_u8g2] change fb\n");

    // send update to app thread
    xSemaphoreGive(ctx->update);
}

void display_u8g2(void* p) {
    FuriRecordSubscriber* log = get_default_log();

    // TODO we need different app to contol backlight
    HAL_GPIO_WritePin(DISPLAY_BACKLIGHT_GPIO_Port, DISPLAY_BACKLIGHT_Pin, GPIO_PIN_SET);

    u8g2_t _u8g2;
    u8g2_Setup_st7565_erc12864_alt_f(
        &_u8g2, U8G2_R0, u8x8_hw_spi_stm32, u8g2_gpio_and_delay_stm32);
    u8g2_InitDisplay(
        &_u8g2); // send init sequence to the display, display is in sleep mode after this
    u8g2_SetContrast(&_u8g2, 36);

    if(!furi_create("u8g2_fb", (void*)&_u8g2, sizeof(_u8g2))) {
        fuprintf(log, "[display_u8g2] cannot create fb record\n");
        furiac_exit(NULL);
    }

    StaticSemaphore_t event_descriptor;
    // create stack-based counting semaphore
    SemaphoreHandle_t update = xSemaphoreCreateCountingStatic(255, 0, &event_descriptor);

    if(update == NULL) {
        fuprintf(log, "[display_u8g2] cannot create update semaphore\n");
        furiac_exit(NULL);
    }

    // save log and event queue in context structure
    DisplayCtx ctx = {.update = update, .log = log};

    // subscribe to record. ctx will be passed to handle_fb_change
    FuriRecordSubscriber* fb_record =
        furi_open("u8g2_fb", false, false, handle_fb_change, NULL, &ctx);

    if(fb_record == NULL) {
        fuprintf(log, "[display] cannot open fb record\n");
        furiac_exit(NULL);
    }

    u8g2_t* u8g2 = (u8g2_t*)furi_take(fb_record);
    u8g2_SetPowerSave(u8g2, 0); // wake up display
    u8g2_SendBuffer(u8g2);
    furi_give(fb_record);

    while(1) {
        // wait for event
        if(xSemaphoreTake(update, 10000) == pdTRUE) {
            HAL_GPIO_WritePin(DISPLAY_BACKLIGHT_GPIO_Port, DISPLAY_BACKLIGHT_Pin, GPIO_PIN_SET);

            u8g2_t* u8g2 = (u8g2_t*)furi_take(fb_record);
            u8g2_SetPowerSave(u8g2, 0); // wake up display
            u8g2_SendBuffer(u8g2);
            furi_give(fb_record);
        } else {
            // TODO we need different app to contol backlight
            HAL_GPIO_WritePin(DISPLAY_BACKLIGHT_GPIO_Port, DISPLAY_BACKLIGHT_Pin, GPIO_PIN_RESET);
        }
    }
}