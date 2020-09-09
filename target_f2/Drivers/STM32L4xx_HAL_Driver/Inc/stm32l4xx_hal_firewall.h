/**
  ******************************************************************************
  * @file    stm32l4xx_hal_firewall.h
  * @author  MCD Application Team
  * @brief   Header file of FIREWALL HAL module.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32L4xx_HAL_FIREWALL_H
#define __STM32L4xx_HAL_FIREWALL_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal_def.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @addtogroup FIREWALL  FIREWALL
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup FIREWALL_Exported_Types FIREWALL Exported Types
  * @{
  */

/**
  * @brief FIREWALL Initialization Structure definition
  */
typedef struct
{
  uint32_t CodeSegmentStartAddress;        /*!< Protected code segment start address. This value is 24-bit long, the 8 LSB bits are
                                                reserved and forced to 0 in order to allow a 256-byte granularity. */

  uint32_t CodeSegmentLength;              /*!< Protected code segment length in bytes. This value is 22-bit long, the 8 LSB bits are
                                                reserved and forced to 0 for the length to be a multiple of 256 bytes. */

  uint32_t NonVDataSegmentStartAddress;    /*!< Protected non-volatile data segment start address. This value is 24-bit long, the 8 LSB
                                                bits are reserved and forced to 0 in order to allow a 256-byte granularity. */

  uint32_t NonVDataSegmentLength;          /*!< Protected non-volatile data segment length in bytes. This value is 22-bit long, the 8 LSB
                                                bits are reserved and forced to 0 for the length to be a multiple of 256 bytes. */

  uint32_t VDataSegmentStartAddress;       /*!< Protected volatile data segment start address. This value is 17-bit long, the 6 LSB bits
                                                are reserved and forced to 0 in order to allow a 64-byte granularity. */

  uint32_t VDataSegmentLength;             /*!< Protected volatile data segment length in bytes. This value is 17-bit long, the 6 LSB
                                                bits are reserved and forced to 0 for the length to be a multiple of 64 bytes. */

  uint32_t VolatileDataExecution;          /*!< Set VDE bit specifying whether or not the volatile data segment can be executed.
                                                 When VDS = 1 (set by parameter VolatileDataShared), VDE bit has no meaning.
                                                This parameter can be a value of @ref FIREWALL_VolatileData_Executable */

  uint32_t VolatileDataShared;             /*!< Set VDS bit in specifying whether or not the volatile data segment can be shared with a
                                                non-protected application code.
                                                This parameter can be a value of @ref FIREWALL_VolatileData_Shared */

}FIREWALL_InitTypeDef;


/**
  * @}
  */


/* Exported constants --------------------------------------------------------*/
/** @defgroup FIREWALL_Exported_Constants FIREWALL Exported Constants
  * @{
  */

/** @defgroup FIREWALL_VolatileData_Executable   FIREWALL volatile data segment execution status
  * @{
  */
#define FIREWALL_VOLATILEDATA_NOT_EXECUTABLE                 ((uint32_t)0x0000)
#define FIREWALL_VOLATILEDATA_EXECUTABLE                     ((uint32_t)FW_CR_VDE)
/**
  * @}
  */

/** @defgroup FIREWALL_VolatileData_Shared  FIREWALL volatile data segment share status
  * @{
  */
#define FIREWALL_VOLATILEDATA_NOT_SHARED                ((uint32_t)0x0000)
#define FIREWALL_VOLATILEDATA_SHARED                    ((uint32_t)FW_CR_VDS)
/**
  * @}
  */

/** @defgroup FIREWALL_Pre_Arm FIREWALL pre arm status
  * @{
  */
#define FIREWALL_PRE_ARM_RESET                 ((uint32_t)0x0000)
#define FIREWALL_PRE_ARM_SET                   ((uint32_t)FW_CR_FPA)

/**
  * @}
  */

/**
  * @}
  */

/* Private macros --------------------------------------------------------*/
/** @defgroup FIREWALL_Private_Macros   FIREWALL Private Macros
  * @{
  */
#define IS_FIREWALL_CODE_SEGMENT_ADDRESS(ADDRESS)        (((ADDRESS) >= FLASH_BASE) && ((ADDRESS) < (FLASH_BASE + FLASH_SIZE)))
#define IS_FIREWALL_CODE_SEGMENT_LENGTH(ADDRESS, LENGTH) (((ADDRESS) + (LENGTH)) <= (FLASH_BASE + FLASH_SIZE))

#define IS_FIREWALL_NONVOLATILEDATA_SEGMENT_ADDRESS(ADDRESS)        (((ADDRESS) >= FLASH_BASE) && ((ADDRESS) < (FLASH_BASE + FLASH_SIZE)))
#define IS_FIREWALL_NONVOLATILEDATA_SEGMENT_LENGTH(ADDRESS, LENGTH) (((ADDRESS) + (LENGTH)) <= (FLASH_BASE + FLASH_SIZE))

#define IS_FIREWALL_VOLATILEDATA_SEGMENT_ADDRESS(ADDRESS)        (((ADDRESS) >= SRAM1_BASE) && ((ADDRESS) < (SRAM1_BASE + SRAM1_SIZE_MAX)))
#define IS_FIREWALL_VOLATILEDATA_SEGMENT_LENGTH(ADDRESS, LENGTH) (((ADDRESS) + (LENGTH)) <= (SRAM1_BASE + SRAM1_SIZE_MAX))


#define IS_FIREWALL_VOLATILEDATA_SHARE(SHARE) (((SHARE) == FIREWALL_VOLATILEDATA_NOT_SHARED) || \
                                               ((SHARE) == FIREWALL_VOLATILEDATA_SHARED))

#define IS_FIREWALL_VOLATILEDATA_EXECUTE(EXECUTE) (((EXECUTE) == FIREWALL_VOLATILEDATA_NOT_EXECUTABLE) || \
                                                   ((EXECUTE) == FIREWALL_VOLATILEDATA_EXECUTABLE))
/**
  * @}
  */


/* Exported macros -----------------------------------------------------------*/
/** @defgroup FIREWALL_Exported_Macros FIREWALL Exported Macros
  * @{
  */

/** @brief  Check whether the FIREWALL is enabled or not.
  * @retval FIREWALL enabling status (TRUE or FALSE).
  */
#define  __HAL_FIREWALL_IS_ENABLED()  HAL_IS_BIT_CLR(SYSCFG->CFGR1, SYSCFG_CFGR1_FWDIS)


/** @brief Enable FIREWALL pre arm.
  * @note When FPA bit is set, any code executed outside the protected segment
  *       closes the Firewall, otherwise it generates a system reset.
  * @note This macro provides the same service as HAL_FIREWALL_EnablePreArmFlag() API
  *       but can be executed inside a code area protected by the Firewall.
  * @note This macro can be executed whatever the Firewall state (opened or closed) when
  *       NVDSL register is equal to 0. Otherwise (when NVDSL register is different from
  *       0, that is, when the non volatile data segment is defined), the macro can be
  *       executed only when the Firewall is opened.
  */
#define __HAL_FIREWALL_PREARM_ENABLE()                                                 \
             do {                                                                      \
                  __IO uint32_t tmpreg;                                                \
                  SET_BIT(FIREWALL->CR, FW_CR_FPA) ;                                   \
                  /* Read bit back to ensure it is taken into account by Peripheral */ \
                  /* (introduce proper delay inside macro execution) */                \
                  tmpreg = READ_BIT(FIREWALL->CR, FW_CR_FPA) ;                         \
                  UNUSED(tmpreg);                                                      \
                } while(0)



/** @brief Disable FIREWALL pre arm.
  * @note When FPA bit is set, any code executed outside the protected segment
  *       closes the Firewall, otherwise, it generates a system reset.
  * @note This macro provides the same service as HAL_FIREWALL_DisablePreArmFlag() API
  *       but can be executed inside a code area protected by the Firewall.
  * @note This macro can be executed whatever the Firewall state (opened or closed) when
  *       NVDSL register is equal to 0. Otherwise (when NVDSL register is different from
  *       0, that is, when the non volatile data segment is defined), the macro can be
  *       executed only when the Firewall is opened.
  */
#define __HAL_FIREWALL_PREARM_DISABLE()                                                \
             do {                                                                      \
                  __IO uint32_t tmpreg;                                                \
                  CLEAR_BIT(FIREWALL->CR, FW_CR_FPA) ;                                 \
                  /* Read bit back to ensure it is taken into account by Peripheral */ \
                  /* (introduce proper delay inside macro execution) */                \
                  tmpreg = READ_BIT(FIREWALL->CR, FW_CR_FPA) ;                         \
                  UNUSED(tmpreg);                                                      \
                } while(0)

/** @brief Enable volatile data sharing in setting VDS bit.
  * @note When VDS bit is set, the volatile data segment is shared with non-protected
  *       application code. It can be accessed whatever the Firewall state (opened or closed).
  * @note This macro can be executed inside a code area protected by the Firewall.
  * @note This macro can be executed whatever the Firewall state (opened or closed) when
  *       NVDSL register is equal to 0. Otherwise (when NVDSL register is different from
  *       0, that is, when the non volatile data segment is defined), the macro can be
  *       executed only when the Firewall is opened.
  */
#define __HAL_FIREWALL_VOLATILEDATA_SHARED_ENABLE()                                    \
             do {                                                                      \
                  __IO uint32_t tmpreg;                                                \
                  SET_BIT(FIREWALL->CR, FW_CR_VDS) ;                                   \
                  /* Read bit back to ensure it is taken into account by Peripheral */ \
                  /* (introduce proper delay inside macro execution) */                \
                  tmpreg = READ_BIT(FIREWALL->CR, FW_CR_VDS) ;                         \
                  UNUSED(tmpreg);                                                      \
                } while(0)

/** @brief Disable volatile data sharing in resetting VDS bit.
  * @note When VDS bit is reset, the volatile data segment is not shared and cannot be
  *       hit by a non protected executable code when the Firewall is closed. If it is
  *       accessed in such a condition, a system reset is generated by the Firewall.
  * @note This macro can be executed inside a code area protected by the Firewall.
  * @note This macro can be executed whatever the Firewall state (opened or closed) when
  *       NVDSL register is equal to 0. Otherwise (when NVDSL register is different from
  *       0, that is, when the non volatile data segment is defined), the macro can be
  *       executed only when the Firewall is opened.
  */
#define __HAL_FIREWALL_VOLATILEDATA_SHARED_DISABLE()                                   \
             do {                                                                      \
                  __IO uint32_t tmpreg;                                                \
                  CLEAR_BIT(FIREWALL->CR, FW_CR_VDS) ;                                 \
                  /* Read bit back to ensure it is taken into account by Peripheral */ \
                  /* (introduce proper delay inside macro execution) */                \
                  tmpreg = READ_BIT(FIREWALL->CR, FW_CR_VDS) ;                         \
                  UNUSED(tmpreg);                                                      \
                } while(0)

/** @brief Enable volatile data execution in setting VDE bit.
  * @note VDE bit is ignored when VDS is set. IF VDS = 1, the Volatile data segment can be
  *       executed whatever the VDE bit value.
  * @note When VDE bit is set (with VDS = 0), the volatile data segment is executable. When
  *       the Firewall call is closed, a "call gate" entry procedure is required to open
  *       first the Firewall.
  * @note This macro can be executed inside a code area protected by the Firewall.
  * @note This macro can be executed whatever the Firewall state (opened or closed) when
  *       NVDSL register is equal to 0. Otherwise (when NVDSL register is different from
  *       0, that is, when the non volatile data segment is defined), the macro can be
  *       executed only when the Firewall is opened.
  */
#define __HAL_FIREWALL_VOLATILEDATA_EXECUTION_ENABLE()                                 \
             do {                                                                      \
                  __IO uint32_t tmpreg;                                                \
                  SET_BIT(FIREWALL->CR, FW_CR_VDE) ;                                   \
                  /* Read bit back to ensure it is taken into account by Peripheral */ \
                  /* (introduce proper delay inside macro execution) */                \
                  tmpreg = READ_BIT(FIREWALL->CR, FW_CR_VDE) ;                         \
                  UNUSED(tmpreg);                                                      \
                } while(0)

/** @brief Disable volatile data execution in resetting VDE bit.
  * @note VDE bit is ignored when VDS is set. IF VDS = 1, the Volatile data segment can be
  *       executed whatever the VDE bit value.
  * @note When VDE bit is reset (with VDS = 0), the volatile data segment cannot  be executed.
  * @note This macro can be executed inside a code area protected by the Firewall.
  * @note This macro can be executed whatever the Firewall state (opened or closed) when
  *       NVDSL register is equal to 0. Otherwise (when NVDSL register is different from
  *       0, that is, when the non volatile data segment is defined), the macro can be
  *       executed only when the Firewall is opened.
  */
#define __HAL_FIREWALL_VOLATILEDATA_EXECUTION_DISABLE()                                \
             do {                                                                      \
                  __IO uint32_t tmpreg;                                                \
                  CLEAR_BIT(FIREWALL->CR, FW_CR_VDE) ;                                 \
                  /* Read bit back to ensure it is taken into account by Peripheral */ \
                  /* (introduce proper delay inside macro execution) */                \
                  tmpreg = READ_BIT(FIREWALL->CR, FW_CR_VDE) ;                         \
                  UNUSED(tmpreg);                                                      \
                } while(0)


/** @brief Check whether or not the volatile data segment is shared.
  * @note This macro can be executed inside a code area protected by the Firewall.
  * @note This macro can be executed whatever the Firewall state (opened or closed) when
  *       NVDSL register is equal to 0. Otherwise (when NVDSL register is different from
  *       0, that is, when the non volatile data segment is defined), the macro can be
  *       executed only when the Firewall is opened.
  * @retval VDS bit setting status (TRUE or FALSE).
  */
#define __HAL_FIREWALL_GET_VOLATILEDATA_SHARED() ((FIREWALL->CR & FW_CR_VDS) == FW_CR_VDS)

/** @brief Check whether or not the volatile data segment is declared executable.
  * @note This macro can be executed inside a code area protected by the Firewall.
  * @note This macro can be executed whatever the Firewall state (opened or closed) when
  *       NVDSL register is equal to 0. Otherwise (when NVDSL register is different from
  *       0, that is, when the non volatile data segment is defined), the macro can be
  *       executed only when the Firewall is opened.
  * @retval VDE bit setting status (TRUE or FALSE).
  */
#define __HAL_FIREWALL_GET_VOLATILEDATA_EXECUTION() ((FIREWALL->CR & FW_CR_VDE) == FW_CR_VDE)

/** @brief Check whether or not the Firewall pre arm bit is set.
  * @note This macro can be executed inside a code area protected by the Firewall.
  * @note This macro can be executed whatever the Firewall state (opened or closed) when
  *       NVDSL register is equal to 0. Otherwise (when NVDSL register is different from
  *       0, that is, when the non volatile data segment is defined), the macro can be
  *       executed only when the Firewall is opened.
  * @retval FPA bit setting status (TRUE or FALSE).
  */
#define __HAL_FIREWALL_GET_PREARM() ((FIREWALL->CR & FW_CR_FPA) == FW_CR_FPA)


/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/

/** @addtogroup FIREWALL_Exported_Functions FIREWALL Exported Functions
  * @{
  */

/** @addtogroup FIREWALL_Exported_Functions_Group1 Initialization Functions
  * @brief    Initialization and Configuration Functions
  * @{
  */

/* Initialization functions  ********************************/
HAL_StatusTypeDef HAL_FIREWALL_Config(FIREWALL_InitTypeDef * fw_init);
void HAL_FIREWALL_GetConfig(FIREWALL_InitTypeDef * fw_config);
void HAL_FIREWALL_EnableFirewall(void);
void HAL_FIREWALL_EnablePreArmFlag(void);
void HAL_FIREWALL_DisablePreArmFlag(void);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __STM32L4xx_HAL_FIREWALL_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
