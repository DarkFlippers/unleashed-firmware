TOOLCHAIN = arm

BOOT_ADDRESS	= 0x08000000
FW_ADDRESS		= 0x08008000
OS_OFFSET		= 0x00008000
FLASH_ADDRESS	= 0x08000000

OPENOCD_OPTS	= -f interface/stlink.cfg -c "transport select hla_swd" -f ../debug/stm32wbx.cfg -c "init"
BOOT_CFLAGS		= -DBOOT_ADDRESS=$(BOOT_ADDRESS) -DFW_ADDRESS=$(FW_ADDRESS) -DOS_OFFSET=$(OS_OFFSET)
MCU_FLAGS		= -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

CFLAGS			+= $(MCU_FLAGS) $(BOOT_CFLAGS) -DSTM32WB55xx -Wall -fdata-sections -ffunction-sections
LDFLAGS			+= $(MCU_FLAGS) -specs=nosys.specs -specs=nano.specs 

HARDWARE_TARGET = 7

CUBE_DIR		= $(PROJECT_ROOT)/lib/STM32CubeWB

# ST HAL
CFLAGS			+=  -DUSE_FULL_LL_DRIVER
ASM_SOURCES		+= $(CUBE_DIR)/Drivers/CMSIS/Device/ST/STM32WBxx/Source/Templates/gcc/startup_stm32wb55xx_cm4.s
C_SOURCES		+= $(CUBE_DIR)/Drivers/CMSIS/Device/ST/STM32WBxx/Source/Templates/system_stm32wbxx.c
C_SOURCES		+= $(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_utils.c
C_SOURCES		+= $(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_gpio.c
C_SOURCES		+= $(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_i2c.c
C_SOURCES		+= $(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_spi.c

CFLAGS			+= -I$(CUBE_DIR)/Drivers/CMSIS/Include
CFLAGS			+= -I$(CUBE_DIR)/Drivers/CMSIS/Device/ST/STM32WBxx/Include
CFLAGS			+= -I$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Inc

LDFLAGS			+= -T$(TARGET_DIR)/stm32wb55xx_flash_cm4.ld

# Drivers
DRIVERS_DIR		= $(PROJECT_ROOT)//lib/drivers
CFLAGS			+= -I$(DRIVERS_DIR)
C_SOURCES		+= $(DRIVERS_DIR)/lp5562.c

# API-HAL
CFLAGS			+= -I$(TARGET_DIR)/furi-hal
C_SOURCES		+= $(wildcard $(TARGET_DIR)/furi-hal/*.c)

# Version generation
C_SOURCES		+= $(PROJECT_ROOT)/lib/toolbox/version.c

ASM_SOURCES		+= $(wildcard $(TARGET_DIR)/*.s)
C_SOURCES		+= $(wildcard $(TARGET_DIR)/*.c)
CPP_SOURCES		+= $(wildcard $(TARGET_DIR)/*.cpp)

SVD_FILE		= $(PROJECT_ROOT)/debug/STM32WB55_CM4.svd
