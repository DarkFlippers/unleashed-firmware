TOOLCHAIN = arm

BOOT_ADDRESS	= 0x08000000
FW_ADDRESS		= 0x08008000
OS_OFFSET		= 0x00008000
FLASH_ADDRESS	= 0x08008000

NO_BOOTLOADER ?= 0
ifeq ($(NO_BOOTLOADER), 1)
BOOT_ADDRESS	= 0x08000000
FW_ADDRESS		= 0x08000000
OS_OFFSET		= 0x00000000
FLASH_ADDRESS	= 0x08000000
CFLAGS			+= -DNO_BOOTLOADER
endif

BOOT_CFLAGS		= -DBOOT_ADDRESS=$(BOOT_ADDRESS) -DFW_ADDRESS=$(FW_ADDRESS) -DOS_OFFSET=$(OS_OFFSET)
MCU_FLAGS		= -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

CFLAGS			+= $(MCU_FLAGS) $(BOOT_CFLAGS) -DSTM32L476xx -Wall -fdata-sections -ffunction-sections
LDFLAGS			+= $(MCU_FLAGS) -specs=nosys.specs -specs=nano.specs 

CUBE_DIR		= ../lib/STM32CubeL4
C_SOURCES		+= \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pcd.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pcd_ex.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_usb.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c_ex.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ramfunc.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma_ex.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_exti.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc_ex.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_comp.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi_ex.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim_ex.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart.c \
	$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart_ex.c \
	$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/croutine.c \
	$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/event_groups.c \
	$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/list.c \
	$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/queue.c \
	$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c \
	$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/tasks.c \
	$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/timers.c \
	$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c \
	$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_1.c \
	$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c \
	$(CUBE_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c \
	$(CUBE_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c \
	$(CUBE_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c \
	$(CUBE_DIR)/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c \
	$(wildcard $(TARGET_DIR)/Src/*.c) \
	$(wildcard $(TARGET_DIR)/Src/fatfs/*.c)

ASM_SOURCES += $(TARGET_DIR)/startup_stm32l476xx.s

# Common
CFLAGS			+= \
	-DUSE_HAL_DRIVER \
	-DHAVE_FREERTOS \
	-DBUTON_INVERT=false \
	-DDEBUG_UART=huart1

ifeq ($(NO_BOOTLOADER), 1)
LDFLAGS			+= -T$(TARGET_DIR)/STM32L476RGTx_FLASH_NO_BOOT.ld
else
LDFLAGS			+= -T$(TARGET_DIR)/STM32L476RGTx_FLASH.ld
endif

CFLAGS += \
	-I$(TARGET_DIR)/Inc \
	-I$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Inc \
	-I$(CUBE_DIR)/Drivers/STM32L4xx_HAL_Driver/Inc/Legacy \
	-I$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/include \
	-I$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 \
	-I$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F \
	-I$(CUBE_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Inc \
	-I$(CUBE_DIR)/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc \
	-I$(CUBE_DIR)/Drivers/CMSIS/Device/ST/STM32L4xx/Include \
	-I$(CUBE_DIR)/Drivers/CMSIS/Include \
	-I$(CUBE_DIR)/Drivers/CMSIS/Include \
	-I$(TARGET_DIR)/Src/fatfs

