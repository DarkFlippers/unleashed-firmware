COPRO_CUBE_VERSION	:= 1.13.3
COPRO_MCU_FAMILY	:= STM32WB5x
COPRO_STACK_BIN 	?= stm32wb5x_BLE_Stack_light_fw.bin
#  See __STACK_TYPE_CODES in scripts/flipper/assets/coprobin.py
COPRO_STACK_TYPE	?= ble_light
COPRO_DISCLAIMER	?=
COPRO_OB_DATA		?= ob.data
#  Keep 0 for auto, or put a value from release_notes for chosen stack
COPRO_STACK_ADDR	:= 0

COPRO_BUNDLE_DIR	:= $(ASSETS_DIR)/core2_firmware
COPRO_CUBE_DIR		:= $(PROJECT_ROOT)/lib/STM32CubeWB
COPRO_FIRMWARE_DIR	:= $(COPRO_CUBE_DIR)/Projects/STM32WB_Copro_Wireless_Binaries/$(COPRO_MCU_FAMILY)
COPRO_STACK_BIN_PATH	:= $(COPRO_FIRMWARE_DIR)/$(COPRO_STACK_BIN)
