#include "app_common.h"
#include <interface/patterns/ble_thread/tl/mbox_def.h>
#include <interface/patterns/ble_thread/hw.h>
#include <furi_hal.h>

#include <stm32wbxx_ll_ipcc.h>
#include <stm32wbxx_ll_pwr.h>

#include <hsem_map.h>

#define HW_IPCC_TX_PENDING(channel)                     \
    ((!(LL_C1_IPCC_IsActiveFlag_CHx(IPCC, channel))) && \
     LL_C1_IPCC_IsEnabledTransmitChannel(IPCC, channel))
#define HW_IPCC_RX_PENDING(channel)                \
    (LL_C2_IPCC_IsActiveFlag_CHx(IPCC, channel) && \
     LL_C1_IPCC_IsEnabledReceiveChannel(IPCC, channel))

#define IPCC_SEND_CMD_TIMEOUT_US (33UL * 1000UL * 1000UL)

static void (*FreeBufCb)(void);

static void HW_IPCC_BLE_EvtHandler(void);
static void HW_IPCC_BLE_AclDataEvtHandler(void);
static void HW_IPCC_MM_FreeBufHandler(void);
static void HW_IPCC_SYS_CmdEvtHandler(void);
static void HW_IPCC_SYS_EvtHandler(void);
static void HW_IPCC_TRACES_EvtHandler(void);

void HW_IPCC_Rx_Handler(void) {
    if(HW_IPCC_RX_PENDING(HW_IPCC_SYSTEM_EVENT_CHANNEL)) {
        HW_IPCC_SYS_EvtHandler();
    } else if(HW_IPCC_RX_PENDING(HW_IPCC_BLE_EVENT_CHANNEL)) {
        HW_IPCC_BLE_EvtHandler();
    } else if(HW_IPCC_RX_PENDING(HW_IPCC_TRACES_CHANNEL)) {
        HW_IPCC_TRACES_EvtHandler();
    }
}

void HW_IPCC_Tx_Handler(void) {
    if(HW_IPCC_TX_PENDING(HW_IPCC_SYSTEM_CMD_RSP_CHANNEL)) {
        HW_IPCC_SYS_CmdEvtHandler();
    } else if(HW_IPCC_TX_PENDING(HW_IPCC_SYSTEM_CMD_RSP_CHANNEL)) {
        HW_IPCC_SYS_CmdEvtHandler();
    } else if(HW_IPCC_TX_PENDING(HW_IPCC_MM_RELEASE_BUFFER_CHANNEL)) {
        HW_IPCC_MM_FreeBufHandler();
    } else if(HW_IPCC_TX_PENDING(HW_IPCC_HCI_ACL_DATA_CHANNEL)) {
        HW_IPCC_BLE_AclDataEvtHandler();
    }
}

void HW_IPCC_Enable(void) {
    /**
  * Such as IPCC IP available to the CPU2, it is required to keep the IPCC clock running
    when FUS is running on CPU2 and CPU1 enters deep sleep mode
  */
    /**
   * When the device is out of standby, it is required to use the EXTI mechanism to wakeup CPU2
   */
    LL_C2_EXTI_EnableEvent_32_63(LL_EXTI_LINE_41);
    LL_EXTI_EnableRisingTrig_32_63(LL_EXTI_LINE_41);

    /**
   * In case the SBSFU is implemented, it may have already set the C2BOOT bit to startup the CPU2.
   * In that case, to keep the mechanism transparent to the user application, it shall call the system command
   * SHCI_C2_Reinit( ) before jumping to the application.
   * When the CPU2 receives that command, it waits for its event input to be set to restart the CPU2 firmware.
   * This is required because once C2BOOT has been set once, a clear/set on C2BOOT has no effect.
   * When SHCI_C2_Reinit( ) is not called, generating an event to the CPU2 does not have any effect
   * So, by default, the application shall both set the event flag and set the C2BOOT bit.
   */
    __SEV(); /* Set the internal event flag and send an event to the CPU2 */
    __WFE(); /* Clear the internal event flag */
    LL_PWR_EnableBootC2();
}

void HW_IPCC_Init(void) {
    LL_C1_IPCC_EnableIT_RXO(IPCC);
    LL_C1_IPCC_EnableIT_TXF(IPCC);

    NVIC_SetPriority(IPCC_C1_RX_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 6, 0));
    NVIC_EnableIRQ(IPCC_C1_RX_IRQn);
    NVIC_SetPriority(IPCC_C1_TX_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 6, 0));
    NVIC_EnableIRQ(IPCC_C1_TX_IRQn);
}

void HW_IPCC_BLE_Init(void) {
    LL_C1_IPCC_EnableReceiveChannel(IPCC, HW_IPCC_BLE_EVENT_CHANNEL);
}

void HW_IPCC_BLE_SendCmd(void) {
    LL_C1_IPCC_SetFlag_CHx(IPCC, HW_IPCC_BLE_CMD_CHANNEL);
}

static void HW_IPCC_BLE_EvtHandler(void) {
    HW_IPCC_BLE_RxEvtNot();

    LL_C1_IPCC_ClearFlag_CHx(IPCC, HW_IPCC_BLE_EVENT_CHANNEL);
}

void HW_IPCC_BLE_SendAclData(void) {
    LL_C1_IPCC_SetFlag_CHx(IPCC, HW_IPCC_HCI_ACL_DATA_CHANNEL);
    LL_C1_IPCC_EnableTransmitChannel(IPCC, HW_IPCC_HCI_ACL_DATA_CHANNEL);
}

static void HW_IPCC_BLE_AclDataEvtHandler(void) {
    LL_C1_IPCC_DisableTransmitChannel(IPCC, HW_IPCC_HCI_ACL_DATA_CHANNEL);

    HW_IPCC_BLE_AclDataAckNot();
}

void HW_IPCC_SYS_Init(void) {
    LL_C1_IPCC_EnableReceiveChannel(IPCC, HW_IPCC_SYSTEM_EVENT_CHANNEL);
}

void HW_IPCC_SYS_SendCmd(void) {
    LL_C1_IPCC_SetFlag_CHx(IPCC, HW_IPCC_SYSTEM_CMD_RSP_CHANNEL);

    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(IPCC_SEND_CMD_TIMEOUT_US);

    while(LL_C1_IPCC_IsActiveFlag_CHx(IPCC, HW_IPCC_SYSTEM_CMD_RSP_CHANNEL)) {
        furi_check(!furi_hal_cortex_timer_is_expired(timer), "HW_IPCC_SYS_SendCmd timeout");
    }

    HW_IPCC_SYS_CmdEvtHandler();
}

static void HW_IPCC_SYS_CmdEvtHandler(void) {
    LL_C1_IPCC_DisableTransmitChannel(IPCC, HW_IPCC_SYSTEM_CMD_RSP_CHANNEL);

    HW_IPCC_SYS_CmdEvtNot();
}

static void HW_IPCC_SYS_EvtHandler(void) {
    HW_IPCC_SYS_EvtNot();

    LL_C1_IPCC_ClearFlag_CHx(IPCC, HW_IPCC_SYSTEM_EVENT_CHANNEL);
}

void HW_IPCC_MM_SendFreeBuf(void (*cb)(void)) {
    if(LL_C1_IPCC_IsActiveFlag_CHx(IPCC, HW_IPCC_MM_RELEASE_BUFFER_CHANNEL)) {
        FreeBufCb = cb;
        LL_C1_IPCC_EnableTransmitChannel(IPCC, HW_IPCC_MM_RELEASE_BUFFER_CHANNEL);
    } else {
        cb();

        LL_C1_IPCC_SetFlag_CHx(IPCC, HW_IPCC_MM_RELEASE_BUFFER_CHANNEL);
    }
}

static void HW_IPCC_MM_FreeBufHandler(void) {
    LL_C1_IPCC_DisableTransmitChannel(IPCC, HW_IPCC_MM_RELEASE_BUFFER_CHANNEL);

    FreeBufCb();

    LL_C1_IPCC_SetFlag_CHx(IPCC, HW_IPCC_MM_RELEASE_BUFFER_CHANNEL);
}

void HW_IPCC_TRACES_Init(void) {
    LL_C1_IPCC_EnableReceiveChannel(IPCC, HW_IPCC_TRACES_CHANNEL);
}

static void HW_IPCC_TRACES_EvtHandler(void) {
    HW_IPCC_TRACES_EvtNot();

    LL_C1_IPCC_ClearFlag_CHx(IPCC, HW_IPCC_TRACES_CHANNEL);
}
