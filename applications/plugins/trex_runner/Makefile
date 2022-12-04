FLIPPER_FIRMWARE_PATH ?= /home/gelin/work/github/flipperzero-firmware/

.PHONY: build
build: $(FLIPPER_FIRMWARE_PATH)/applications_user/t-rex-runner
	cd $(FLIPPER_FIRMWARE_PATH) && ./fbt fap_t-rex-runner

.PHONY: launch
launch: $(FLIPPER_FIRMWARE_PATH)/applications_user/t-rex-runner
	cd $(FLIPPER_FIRMWARE_PATH) && ./fbt launch_app APPSRC=applications_user/t-rex-runner

.PHONY: assets
assets:
	rm assets_icons.*
	$(MAKE) assets_icons.c

assets_icons.c: $(FLIPPER_FIRMWARE_PATH)/applications_user/t-rex-runner
	cd $(FLIPPER_FIRMWARE_PATH) && ./scripts/assets.py icons applications_user/t-rex-runner/assets/ applications_user/t-rex-runner/

$(FLIPPER_FIRMWARE_PATH)/applications_user/t-rex-runner:
	ln -s $(PWD) $(FLIPPER_FIRMWARE_PATH)/applications_user/t-rex-runner
