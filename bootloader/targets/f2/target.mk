TOOLCHAIN = arm

BOOT_ADDRESS	= 0x08000000
FW_ADDRESS		= 0x08008000
OS_OFFSET		= 0x00008000
FLASH_ADDRESS	= 0x08000000

BOOT_CFLAGS		= -DBOOT_ADDRESS=$(BOOT_ADDRESS) -DFW_ADDRESS=$(FW_ADDRESS) -DOS_OFFSET=$(OS_OFFSET)
MCU_FLAGS		= -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

CFLAGS			+= $(MCU_FLAGS) $(BOOT_CFLAGS) -DSTM32L476xx -Wall -fdata-sections -ffunction-sections
LDFLAGS			+= $(MCU_FLAGS) -specs=nosys.specs -specs=nano.specs

CUBE_DIR		= ../lib/STM32CubeL4
CUBE_CMSIS_DIR	= $(CUBE_DIR)/Drivers/CMSIS
CUBE_HAL_DIR	= $(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver

ASM_SOURCES		+= $(CUBE_CMSIS_DIR)/Device/ST/STM32L4xx/Source/Templates/gcc/startup_stm32l476xx.s
C_SOURCES		+= $(CUBE_CMSIS_DIR)/Device/ST/STM32L4xx/Source/Templates/system_stm32l4xx.c
C_SOURCES		+= $(CUBE_HAL_DIR)/Src/stm32l4xx_ll_utils.c

CFLAGS			+= -I$(CUBE_CMSIS_DIR)/Include
CFLAGS			+= -I$(CUBE_CMSIS_DIR)/Device/ST/STM32L4xx/Include
CFLAGS			+= -I$(CUBE_HAL_DIR)/Inc
LDFLAGS			+= -Ttargets/f2/STM32L476RGTx_FLASH.ld

ASM_SOURCES		+= $(wildcard $(TARGET_DIR)/*.s)
C_SOURCES		+= $(wildcard $(TARGET_DIR)/*.c)
CPP_SOURCES		+= $(wildcard $(TARGET_DIR)/*.cpp)
