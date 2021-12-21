PROJECT_ROOT := $(abspath $(dir $(abspath $(firstword $(MAKEFILE_LIST)))))
COPRO_DIR := $(PROJECT_ROOT)/lib/STM32CubeWB/Projects/STM32WB_Copro_Wireless_Binaries/STM32WB5x

NPROCS := 1
OS := $(shell uname -s)

ifeq ($(OS), Linux)
NPROCS := $(shell grep -c ^processor /proc/cpuinfo)
else ifeq ($(OS), Darwin)
NPROCS := $(shell sysctl -n hw.ncpu)
endif

include	$(PROJECT_ROOT)/make/defaults.mk

.PHONY: all
all: bootloader_all firmware_all
	@$(PROJECT_ROOT)/scripts/dist.sh

.PHONY: whole
whole: flash_radio bootloader_flash firmware_flash

.PHONY: clean
clean: bootloader_clean firmware_clean
	@rm -rf $(PROJECT_ROOT)/dist/$(TARGET)

.PHONY: flash
flash: bootloader_flash firmware_flash

.PHONY: debug
debug:
	@$(MAKE) -C firmware -j$(NPROCS) debug

.PHONY: blackmagic
blackmagic:
	@$(MAKE) -C firmware -j$(NPROCS) blackmagic

.PHONY: wipe
wipe:
	@$(PROJECT_ROOT)/scripts/flash.py wipe
	@$(PROJECT_ROOT)/scripts/ob.py set

.PHONY: bootloader_all
bootloader_all:
	@$(MAKE) -C $(PROJECT_ROOT)/bootloader -j$(NPROCS) all

.PHONY: firmware_all
firmware_all:
	@$(MAKE) -C $(PROJECT_ROOT)/firmware -j$(NPROCS) all

.PHONY: bootloader_clean
bootloader_clean:
	@$(MAKE) -C $(PROJECT_ROOT)/bootloader -j$(NPROCS) clean

.PHONY: firmware_clean
firmware_clean:
	@$(MAKE) -C $(PROJECT_ROOT)/firmware -j$(NPROCS) clean

.PHONY: bootloader_flash
bootloader_flash:
ifeq ($(FORCE), 1)
	@rm $(PROJECT_ROOT)/bootloader/.obj/f*/flash || true
endif
	@$(MAKE) -C $(PROJECT_ROOT)/bootloader -j$(NPROCS) flash

.PHONY: firmware_flash
firmware_flash:
ifeq ($(FORCE), 1)
	@rm $(PROJECT_ROOT)/firmware/.obj/f*/flash || true
endif
	@$(MAKE) -C $(PROJECT_ROOT)/firmware -j$(NPROCS) flash

.PHONY: flash_radio
flash_radio:
	@$(PROJECT_ROOT)/scripts/flash.py core2radio 0x080C7000 $(COPRO_DIR)/stm32wb5x_BLE_Stack_full_fw.bin
	@$(PROJECT_ROOT)/scripts/ob.py set

.PHONY: flash_radio_fus
flash_radio_fus:
	@echo
	@echo "================   DON'T DO IT    ================"
	@echo "= Flashing FUS is going to erase secure enclave  ="
	@echo "= You will loose ability to use encrypted assets ="
	@echo "=       type 'find / -exec rm -rf {} \;'         ="
	@echo "=     In case if you still want to continue      ="
	@echo "================    JUST DON'T    ================"
	@echo

.PHONY: flash_radio_fus_please_i_m_not_going_to_complain
flash_radio_fus_please_i_m_not_going_to_complain:
	@$(PROJECT_ROOT)/scripts/flash.py core2fus 0x080EC000 --statement=AGREE_TO_LOOSE_FLIPPER_FEATURES_THAT_USES_CRYPTO_ENCLAVE $(COPRO_DIR)/stm32wb5x_FUS_fw_for_fus_0_5_3.bin
	@$(PROJECT_ROOT)/scripts/flash.py core2fus 0x080EC000 --statement=AGREE_TO_LOOSE_FLIPPER_FEATURES_THAT_USES_CRYPTO_ENCLAVE $(COPRO_DIR)/stm32wb5x_FUS_fw.bin
	@$(PROJECT_ROOT)/scripts/ob.py set

FORMAT_SOURCES = $(shell find applications bootloader core -iname "*.h" -o -iname "*.c" -o -iname "*.cpp")

.PHONY: format
format:
	@echo "Formatting sources with clang-format"
	@clang-format -style=file -i $(FORMAT_SOURCES)

