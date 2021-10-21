PROJECT_ROOT := $(abspath $(dir $(abspath $(firstword $(MAKEFILE_LIST)))))
COPRO_DIR := $(PROJECT_ROOT)/lib/STM32CubeWB/Projects/STM32WB_Copro_Wireless_Binaries/STM32WB5x

.PHONY: all
all: bootloader_all firmware_all

.PHONY: whole
whole: flash_radio bootloader_flash firmware_flash

.PHONY: clean
clean: bootloader_clean firmware_clean

.PHONY: flash
flash: bootloader_flash firmware_flash

.PHONY: debug
debug:
	$(MAKE) -C firmware -j9 debug

.PHONY: wipe
wipe:
	$(PROJECT_ROOT)/scripts/flash.py wipe
	$(PROJECT_ROOT)/scripts/ob.py set

.PHONY: bootloader_all
bootloader_all:
	$(MAKE) -C $(PROJECT_ROOT)/bootloader -j9 all

.PHONY: firmware_all
firmware_all:
	$(MAKE) -C $(PROJECT_ROOT)/firmware -j9 all

.PHONY: bootloader_clean
bootloader_clean:
	$(MAKE) -C $(PROJECT_ROOT)/bootloader -j9 clean

.PHONY: firmware_clean
firmware_clean:
	$(MAKE) -C $(PROJECT_ROOT)/firmware -j9 clean

.PHONY: bootloader_flash
bootloader_flash:
	rm $(PROJECT_ROOT)/bootloader/.obj/f*/flash || true
	$(MAKE) -C $(PROJECT_ROOT)/bootloader -j9 flash

.PHONY: firmware_flash
firmware_flash:
	rm $(PROJECT_ROOT)/firmware/.obj/f*/flash || true
	$(MAKE) -C $(PROJECT_ROOT)/firmware -j9 flash

.PHONY: flash_radio
flash_radio:
	$(PROJECT_ROOT)/scripts/flash.py core2radio 0x080CA000 $(COPRO_DIR)/stm32wb5x_BLE_Stack_full_fw.bin
	$(PROJECT_ROOT)/scripts/ob.py set

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

.PHONY: 
flash_radio_fus_please_i_m_not_going_to_complain:
	$(PROJECT_ROOT)/scripts/flash.py core2fus 0x080EC000 --statement=AGREE_TO_LOOSE_FLIPPER_FEATURES_THAT_USES_CRYPTO_ENCLAVE $(COPRO_DIR)/stm32wb5x_FUS_fw_for_fus_0_5_3.bin
	$(PROJECT_ROOT)/scripts/flash.py core2fus 0x080EC000 --statement=AGREE_TO_LOOSE_FLIPPER_FEATURES_THAT_USES_CRYPTO_ENCLAVE $(COPRO_DIR)/stm32wb5x_FUS_fw.bin
	$(PROJECT_ROOT)/scripts/ob.py set
