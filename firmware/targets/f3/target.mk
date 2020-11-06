TOOLCHAIN = arm

DEBUG_AGENT		= openocd -f interface/stlink-v2.cfg -c "transport select hla_swd" -f ../debug/stm32wbx.cfg -c "init" -c "reset halt"

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

CFLAGS			+= $(MCU_FLAGS) $(BOOT_CFLAGS) -DSTM32WB55xx -Wall -fdata-sections -ffunction-sections
LDFLAGS			+= $(MCU_FLAGS) -specs=nosys.specs -specs=nano.specs 

CUBE_DIR		= ../lib/STM32CubeWB
C_SOURCES		+= \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_gpio.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_pcd.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_pcd_ex.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_usb.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_rcc.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_rcc_ex.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_flash.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_flash_ex.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_hsem.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_dma.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_dma_ex.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_pwr.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_pwr_ex.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_cortex.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_exti.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_adc.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_adc_ex.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_adc.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_i2c.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_i2c_ex.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_rtc.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_rtc_ex.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_spi.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_spi_ex.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_tim.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_tim_ex.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_uart.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_uart_ex.c \
	$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_comp.c \
	$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/croutine.c \
	$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/event_groups.c \
	$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/list.c \
	$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/queue.c \
	$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c \
	$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/tasks.c \
	$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/timers.c \
	$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c \
	$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c \
	$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c \
	$(CUBE_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c \
	$(CUBE_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c \
	$(CUBE_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c \
	$(CUBE_DIR)/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c \
	$(wildcard $(TARGET_DIR)/Src/*.c) \
	$(wildcard $(TARGET_DIR)/Src/fatfs/*.c) \
	$(wildcard $(TARGET_DIR)/api-hal/*.c)

ASM_SOURCES += $(TARGET_DIR)/startup_stm32wb55xx_cm4.s

# Common
CFLAGS			+= \
	-DUSE_HAL_DRIVER \
	-DHAVE_FREERTOS \
	-DBUTON_INVERT=true \
	-DDEBUG_UART=huart1

ifeq ($(NO_BOOTLOADER), 1)
LDFLAGS			+= -T$(TARGET_DIR)/stm32wb55xx_flash_cm4_no_boot.ld
else
LDFLAGS			+= -T$(TARGET_DIR)/stm32wb55xx_flash_cm4.ld
endif

CFLAGS += \
	-I$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Inc \
	-I$(CUBE_DIR)/Drivers/STM32WBxx_HAL_Driver/Inc/Legacy \
	-I$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/include \
	-I$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 \
	-I$(CUBE_DIR)/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F \
	-I$(CUBE_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Inc \
	-I$(CUBE_DIR)/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc \
	-I$(CUBE_DIR)/Drivers/CMSIS/Device/ST/STM32WBxx/Include \
	-I$(CUBE_DIR)/Drivers/CMSIS/Include \
	-I$(TARGET_DIR)/Inc \
	-I$(TARGET_DIR)/Src/fatfs \
	-I$(TARGET_DIR)/api-hal

