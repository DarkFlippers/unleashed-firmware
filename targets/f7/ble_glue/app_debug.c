#include "app_common.h"
#include "app_debug.h"
#include <interface/patterns/ble_thread/tl/tl.h>
#include <interface/patterns/ble_thread/tl/mbox_def.h>
#include <interface/patterns/ble_thread/shci/shci.h>
#include <utilities/dbg_trace.h>
#include <utilities/utilities_common.h>

#include "stm32wbxx_ll_bus.h"
#include "stm32wbxx_ll_pwr.h"

#include <furi_hal.h>

typedef PACKED_STRUCT {
    GPIO_TypeDef* port;
    uint16_t pin;
    uint8_t enable;
    uint8_t reserved;
}
APPD_GpioConfig_t;

#define GPIO_NBR_OF_RF_SIGNALS           9
#define GPIO_CFG_NBR_OF_FEATURES         34
#define NBR_OF_TRACES_CONFIG_PARAMETERS  4
#define NBR_OF_GENERAL_CONFIG_PARAMETERS 4

/**
 * THIS SHALL BE SET TO A VALUE DIFFERENT FROM 0 ONLY ON REQUEST FROM ST SUPPORT
 */
#define BLE_DTB_CFG  0
// #define BLE_DTB_CFG 7
#define SYS_DBG_CFG1 (SHCI_C2_DEBUG_OPTIONS_IPCORE_LP | SHCI_C2_DEBUG_OPTIONS_CPU2_STOP_EN)

/* Private variables ---------------------------------------------------------*/
PLACE_IN_SECTION("MB_MEM2")
ALIGN(4) static SHCI_C2_DEBUG_TracesConfig_t APPD_TracesConfig = {0, 0, 0, 0};
PLACE_IN_SECTION("MB_MEM2")
ALIGN(4)
static SHCI_C2_DEBUG_GeneralConfig_t APPD_GeneralConfig =
    {BLE_DTB_CFG, SYS_DBG_CFG1, {0, 0}, 0, 0, 0, 0, 0};

/**
 * THE DEBUG ON GPIO FOR CPU2 IS INTENDED TO BE USED ONLY ON REQUEST FROM ST SUPPORT
 * It provides timing information on the CPU2 activity.
 * All configuration of (port, pin) is supported for each features and can be selected by the user
 * depending on the availability
 */
static const APPD_GpioConfig_t aGpioConfigList[GPIO_CFG_NBR_OF_FEATURES] = {
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* BLE_ISR - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_7, 1, 0}, /* BLE_STACK_TICK - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* BLE_CMD_PROCESS - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* BLE_ACL_DATA_PROCESS - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* SYS_CMD_PROCESS - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* RNG_PROCESS - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* NVM_PROCESS - Set on Entry / Reset on Exit */
    {GPIOB, LL_GPIO_PIN_3, 1, 0}, /* IPCC_GENERAL - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* IPCC_BLE_CMD_RX - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* IPCC_BLE_EVT_TX - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* IPCC_BLE_ACL_DATA_RX - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* IPCC_SYS_CMD_RX - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* IPCC_SYS_EVT_TX - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* IPCC_CLI_CMD_RX - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* IPCC_OT_CMD_RX - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* IPCC_OT_ACK_TX - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* IPCC_CLI_ACK_TX - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* IPCC_MEM_MANAGER_RX - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* IPCC_TRACES_TX - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_6, 1, 0}, /* HARD_FAULT - Set on Entry / Reset on Exit */
    /* From v1.1.1 */
    {GPIOC, LL_GPIO_PIN_1, 1, 0}, /* IP_CORE_LP_STATUS - Set on Entry / Reset on Exit */
    /* From v1.2.0 */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* END_OF_CONNECTION_EVENT - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* TIMER_SERVER_CALLBACK - Toggle on Entry */
    {GPIOA, LL_GPIO_PIN_4, 1, 0}, /* PES_ACTIVITY - Set on Entry / Reset on Exit */
    {GPIOC, LL_GPIO_PIN_0, 1, 0}, /* MB_BLE_SEND_EVT - Set on Entry / Reset on Exit */
    /* From v1.3.0 */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* BLE_NO_DELAY - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* BLE_STACK_STORE_NVM_CB - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* NVMA_WRITE_ONGOING - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* NVMA_WRITE_COMPLETE - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* NVMA_CLEANUP - Set on Entry / Reset on Exit */
    /* From v1.4.0 */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* NVMA_START - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* FLASH_EOP - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* FLASH_WRITE - Set on Entry / Reset on Exit */
    {GPIOA, LL_GPIO_PIN_0, 0, 0}, /* FLASH_ERASE - Set on Entry / Reset on Exit */
};

/**
 * THE DEBUG ON GPIO FOR CPU2 IS INTENDED TO BE USED ONLY ON REQUEST FROM ST SUPPORT
 * This table is relevant only for BLE
 * It provides timing information on BLE RF activity.
 * New signals may be allocated at any location when requested by ST
 * The GPIO allocated to each signal depend on the BLE_DTB_CFG value and cannot be changed
 */
#if(BLE_DTB_CFG == 7)
static const APPD_GpioConfig_t aRfConfigList[GPIO_NBR_OF_RF_SIGNALS] = {
    {GPIOB, LL_GPIO_PIN_2, 0, 0}, /* DTB10 - Tx/Rx SPI */
    {GPIOB, LL_GPIO_PIN_7, 0, 0}, /* DTB11 - Tx/Tx SPI Clk */
    {GPIOA, LL_GPIO_PIN_8, 0, 0}, /* DTB12 - Tx/Rx Ready & SPI Select */
    {GPIOA, LL_GPIO_PIN_9, 0, 0}, /* DTB13 - Tx/Rx Start */
    {GPIOA, LL_GPIO_PIN_10, 0, 0}, /* DTB14 - FSM0 */
    {GPIOA, LL_GPIO_PIN_11, 0, 0}, /* DTB15 - FSM1 */
    {GPIOB, LL_GPIO_PIN_8, 0, 0}, /* DTB16 - FSM2 */
    {GPIOB, LL_GPIO_PIN_11, 0, 0}, /* DTB17 - FSM3 */
    {GPIOB, LL_GPIO_PIN_10, 0, 0}, /* DTB18 - FSM4 */
};
#endif

static void APPD_SetCPU2GpioConfig(void);
static void APPD_BleDtbCfg(void);

void APPD_Init(void) {
    APPD_SetCPU2GpioConfig();
    APPD_BleDtbCfg();
}

void APPD_EnableCPU2(void) {
    SHCI_C2_DEBUG_Init_Cmd_Packet_t DebugCmdPacket = {
        {{0, 0, 0}}, /**< Does not need to be initialized */
        {(uint8_t*)aGpioConfigList,
         (uint8_t*)&APPD_TracesConfig,
         (uint8_t*)&APPD_GeneralConfig,
         GPIO_CFG_NBR_OF_FEATURES,
         NBR_OF_TRACES_CONFIG_PARAMETERS,
         NBR_OF_GENERAL_CONFIG_PARAMETERS}};

    /**< Traces channel initialization */
    TL_TRACES_Init();

    /** GPIO DEBUG Initialization */
    SHCI_C2_DEBUG_Init(&DebugCmdPacket);

    // We don't need External Power Amplifier
    // LL_GPIO_InitTypeDef  gpio_config;
    // gpio_config.Pull = GPIO_NOPULL;
    // gpio_config.Mode = GPIO_MODE_OUTPUT_PP;
    // gpio_config.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    // gpio_config.Pin = LL_GPIO_PIN_3;
    // HAL_GPIO_Init(GPIOC, &gpio_config);
    // SHCI_C2_ExtpaConfig((uint32_t)GPIOC, LL_GPIO_PIN_3, EXT_PA_ENABLED_LOW, EXT_PA_ENABLED);

    return;
}

static void APPD_SetCPU2GpioConfig(void) {
    LL_GPIO_InitTypeDef gpio_config = {0};
    uint8_t local_loop;
    uint16_t gpioa_pin_list;
    uint16_t gpiob_pin_list;
    uint16_t gpioc_pin_list;

    gpioa_pin_list = 0;
    gpiob_pin_list = 0;
    gpioc_pin_list = 0;

    for(local_loop = 0; local_loop < GPIO_CFG_NBR_OF_FEATURES; local_loop++) {
        if(aGpioConfigList[local_loop].enable != 0) {
            switch((uint32_t)aGpioConfigList[local_loop].port) {
            case(uint32_t)GPIOA:
                gpioa_pin_list |= aGpioConfigList[local_loop].pin;
                break;

            case(uint32_t)GPIOB:
                gpiob_pin_list |= aGpioConfigList[local_loop].pin;
                break;

            case(uint32_t)GPIOC:
                gpioc_pin_list |= aGpioConfigList[local_loop].pin;
                break;

            default:
                break;
            }
        }
    }

    gpio_config.Mode = LL_GPIO_MODE_OUTPUT;
    gpio_config.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_config.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio_config.Pull = LL_GPIO_PULL_NO;

    // Never disable SWD, why would you?
    // gpio_config.Pin = LL_GPIO_PIN_15 | LL_GPIO_PIN_14 | LL_GPIO_PIN_13;
    // LL_GPIO_Init(GPIOA, &gpio_config);

    if(gpioa_pin_list != 0) {
        gpio_config.Pin = gpioa_pin_list;
        LL_C2_AHB2_GRP1_EnableClock(LL_C2_AHB2_GRP1_PERIPH_GPIOA);
        LL_GPIO_Init(GPIOA, &gpio_config);
        LL_GPIO_ResetOutputPin(GPIOA, gpioa_pin_list);
    }

    if(gpiob_pin_list != 0) {
        gpio_config.Pin = gpiob_pin_list;
        LL_C2_AHB2_GRP1_EnableClock(LL_C2_AHB2_GRP1_PERIPH_GPIOB);
        LL_GPIO_Init(GPIOB, &gpio_config);
        LL_GPIO_ResetOutputPin(GPIOB, gpiob_pin_list);
    }

    if(gpioc_pin_list != 0) {
        gpio_config.Pin = gpioc_pin_list;
        LL_C2_AHB2_GRP1_EnableClock(LL_C2_AHB2_GRP1_PERIPH_GPIOC);
        LL_GPIO_Init(GPIOC, &gpio_config);
        LL_GPIO_ResetOutputPin(GPIOC, gpioc_pin_list);
    }
}

static void APPD_BleDtbCfg(void) {
#if(BLE_DTB_CFG != 0)
    LL_GPIO_InitTypeDef gpio_config = {0};
    uint8_t local_loop;
    uint16_t gpioa_pin_list;
    uint16_t gpiob_pin_list;

    gpioa_pin_list = 0;
    gpiob_pin_list = 0;

    for(local_loop = 0; local_loop < GPIO_NBR_OF_RF_SIGNALS; local_loop++) {
        if(aRfConfigList[local_loop].enable != 0) {
            switch((uint32_t)aRfConfigList[local_loop].port) {
            case(uint32_t)GPIOA:
                gpioa_pin_list |= aRfConfigList[local_loop].pin;
                break;
            case(uint32_t)GPIOB:
                gpiob_pin_list |= aRfConfigList[local_loop].pin;
                break;
            default:
                break;
            }
        }
    }

    gpio_config.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio_config.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_config.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio_config.Pull = LL_GPIO_PULL_NO;
    gpio_config.Alternate = LL_GPIO_AF_6;
    gpio_config.Pin = LL_GPIO_PIN_15 | LL_GPIO_PIN_14 | LL_GPIO_PIN_13;

    if(gpioa_pin_list != 0) {
        gpio_config.Pin = gpioa_pin_list;
        LL_C2_AHB2_GRP1_EnableClock(LL_C2_AHB2_GRP1_PERIPH_GPIOA);
        LL_GPIO_Init(GPIOA, &gpio_config);
    }

    if(gpiob_pin_list != 0) {
        gpio_config.Pin = gpiob_pin_list;
        LL_C2_AHB2_GRP1_EnableClock(LL_C2_AHB2_GRP1_PERIPH_GPIOB);
        LL_GPIO_Init(GPIOB, &gpio_config);
    }
#endif
}
