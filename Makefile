PROJECT_ROOT := $(abspath $(dir $(abspath $(firstword $(MAKEFILE_LIST)))))

include			$(PROJECT_ROOT)/make/git.mk
include			$(PROJECT_ROOT)/assets/copro.mk
include			$(PROJECT_ROOT)/assets/splash.mk

PROJECT_SOURCE_DIRECTORIES := \
	$(PROJECT_ROOT)/applications \
	$(PROJECT_ROOT)/core \
	$(PROJECT_ROOT)/firmware/targets \
	$(PROJECT_ROOT)/lib/app-template \
	$(PROJECT_ROOT)/lib/app-scened-template \
	$(PROJECT_ROOT)/lib/common-api \
	$(PROJECT_ROOT)/lib/drivers \
	$(PROJECT_ROOT)/lib/flipper_file \
	$(PROJECT_ROOT)/lib/infrared \
	$(PROJECT_ROOT)/lib/nfc_protocols \
	$(PROJECT_ROOT)/lib/ST25RFAL002 \
	$(PROJECT_ROOT)/lib/onewire \
	$(PROJECT_ROOT)/lib/qrcode \
	$(PROJECT_ROOT)/lib/subghz \
	$(PROJECT_ROOT)/lib/toolbox \
	$(PROJECT_ROOT)/lib/u8g2

NPROCS := 3
OS := $(shell uname -s)

ifeq ($(OS), Linux)
NPROCS := $(shell grep -c ^processor /proc/cpuinfo)
else ifeq ($(OS), Darwin)
NPROCS := $(shell sysctl -n hw.ncpu)
endif

include	$(PROJECT_ROOT)/make/defaults.mk

.PHONY: all
all: firmware_all
	@$(PROJECT_ROOT)/scripts/dist.py copy -t $(TARGET) -p firmware -s $(DIST_SUFFIX)

.PHONY: whole
whole: flash_radio firmware_flash

.PHONY: clean
clean: firmware_clean updater_clean
	@rm -rf $(PROJECT_ROOT)/dist/$(TARGET)

.PHONY: flash
flash: firmware_flash

.PHONY: debug
debug:
	@$(MAKE) -C firmware -j$(NPROCS) debug

.PHONY: debug_other
debug_other:
	@$(MAKE) -C firmware -j$(NPROCS) debug_other

.PHONY: blackmagic
blackmagic:
	@$(MAKE) -C firmware -j$(NPROCS) blackmagic

.PHONY: wipe
wipe:
	@$(PROJECT_ROOT)/scripts/flash.py wipe
	@$(PROJECT_ROOT)/scripts/ob.py set

.PHONY: firmware_all
firmware_all:
	@$(MAKE) -C $(PROJECT_ROOT)/firmware -j$(NPROCS) all

.PHONY: firmware_clean
firmware_clean:
	@$(MAKE) -C $(PROJECT_ROOT)/firmware -j$(NPROCS) clean

.PHONY: firmware_flash
firmware_flash:
ifeq ($(FORCE), 1)
	@rm $(PROJECT_ROOT)/firmware/.obj/f*-firmware/flash || true
endif
	@$(MAKE) -C $(PROJECT_ROOT)/firmware -j$(NPROCS) flash

.PHONY: updater
updater:
	@$(MAKE) -C $(PROJECT_ROOT)/firmware -j$(NPROCS) RAM_EXEC=1 all

.PHONY: updater_clean
updater_clean:
	@$(MAKE) -C $(PROJECT_ROOT)/firmware -j$(NPROCS) RAM_EXEC=1 clean

.PHONY: updater_debug
updater_debug:
	@$(MAKE) -C $(PROJECT_ROOT)/firmware -j$(NPROCS) RAM_EXEC=1 debug

.PHONY: updater_package_bin
updater_package_bin: firmware_all updater
	@$(PROJECT_ROOT)/scripts/dist.py copy -t $(TARGET) -p firmware updater -s $(DIST_SUFFIX) --bundlever "$(VERSION_STRING)"

.PHONY: updater_package
updater_package: firmware_all updater assets_manifest
	@$(PROJECT_ROOT)/scripts/dist.py copy \
	-t $(TARGET) -p firmware updater \
	-s $(DIST_SUFFIX) -r $(PROJECT_ROOT)/assets/resources \
	--bundlever "$(VERSION_STRING)" \
	--radio $(COPRO_STACK_BIN_PATH) \
	--radiotype $(COPRO_STACK_TYPE) \
	$(COPRO_DISCLAIMER) \
	--obdata $(PROJECT_ROOT)/scripts/$(COPRO_OB_DATA) \
	--splash $(UPDATER_SPLASH_DIR)

.PHONY: assets_manifest
assets_manifest:
	@$(MAKE) -C $(PROJECT_ROOT)/assets manifest

.PHONY: assets_rebuild
assets_rebuild:
	@$(MAKE) -C $(PROJECT_ROOT)/assets clean all

.PHONY: flash_radio
flash_radio:
	@$(PROJECT_ROOT)/scripts/flash.py core2radio $(COPRO_STACK_BIN_PATH) --addr=$(COPRO_STACK_ADDR)
	@$(PROJECT_ROOT)/scripts/ob.py set

.PHONY: flash_radio_fus
flash_radio_fus:
	@echo
	@echo "================   DON'T DO IT    ================"
	@echo "= Flashing FUS is going to erase secure enclave  ="
	@echo "= You will lose ability to use encrypted assets  ="
	@echo "=       type 'find / -exec rm -rf {} \;'         ="
	@echo "=     In case if you still want to continue      ="
	@echo "================    JUST DON'T    ================"
	@echo

.PHONY: flash_radio_fus_please_i_m_not_going_to_complain
flash_radio_fus_please_i_m_not_going_to_complain:
	@$(PROJECT_ROOT)/scripts/flash.py core2fus 0x080EC000 --statement=AGREE_TO_LOSE_FLIPPER_FEATURES_THAT_USE_CRYPTO_ENCLAVE $(COPRO_FIRMWARE_DIR)/stm32wb5x_FUS_fw_for_fus_0_5_3.bin
	@$(PROJECT_ROOT)/scripts/flash.py core2fus 0x080EC000 --statement=AGREE_TO_LOSE_FLIPPER_FEATURES_THAT_USE_CRYPTO_ENCLAVE $(COPRO_FIRMWARE_DIR)/stm32wb5x_FUS_fw.bin
	@$(PROJECT_ROOT)/scripts/ob.py set

.PHONY: lint
lint:
	@echo "Checking source code formatting"
	@$(PROJECT_ROOT)/scripts/lint.py check $(PROJECT_SOURCE_DIRECTORIES)

.PHONY: format
format:
	@echo "Reformating sources code"
	@$(PROJECT_ROOT)/scripts/lint.py format $(PROJECT_SOURCE_DIRECTORIES)

.PHONY: guruguru
guruguru:
	@echo "ぐるぐる回る"
	@$(PROJECT_ROOT)/scripts/guruguru.py $(PROJECT_ROOT)

.PHONY: generate_compile_db
generate_compile_db:
	@$(MAKE) -C $(PROJECT_ROOT)/firmware -j$(NPROCS) generate_compile_db