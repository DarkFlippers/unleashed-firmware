PROJECT_ROOT		= $(abspath $(dir $(abspath $(firstword $(MAKEFILE_LIST))))..)

include				$(PROJECT_ROOT)/assets/assets.mk
include				$(PROJECT_ROOT)/assets/copro.mk

.PHONY: all
all: icons protobuf dolphin manifest

$(ASSETS): $(ASSETS_SOURCES) $(ASSETS_COMPILER)
	@echo "\tASSETS\t\t" $@
	@$(ASSETS_COMPILER) icons "$(ASSETS_SOURCE_DIR)" "$(ASSETS_COMPILED_DIR)"

.PHONY: icons
icons: $(ASSETS)

$(PROTOBUF) &: $(PROTOBUF_SOURCES) $(PROTOBUF_COMPILER)
	@echo "\tPROTOBUF\t" $(PROTOBUF_FILENAMES)
	@$(PROTOBUF_COMPILER) -q -I$(PROTOBUF_SOURCE_DIR) -D$(PROTOBUF_COMPILED_DIR) $(PROTOBUF_SOURCES)
	@printf "#pragma once\n#define PROTOBUF_MAJOR_VERSION $(PROTOBUF_MAJOR_VERSION)\n#define PROTOBUF_MINOR_VERSION $(PROTOBUF_MINOR_VERSION)\n" > $(PROTOBUF_COMPILED_DIR)/protobuf_version.h

.PHONY: protobuf
protobuf: $(PROTOBUF)

$(DOLPHIN_EXTERNAL_OUTPUT_DIR): $(DOLPHIN_SOURCE_DIR)
	@echo "\tDOLPHIN blocking"
	@$(ASSETS_COMPILER) dolphin -s dolphin_blocking "$(DOLPHIN_SOURCE_DIR)/blocking" "$(DOLPHIN_INTERNAL_OUTPUT_DIR)"
	@echo "\tDOLPHIN internal"
	@$(ASSETS_COMPILER) dolphin -s dolphin_internal "$(DOLPHIN_SOURCE_DIR)/internal" "$(DOLPHIN_INTERNAL_OUTPUT_DIR)"
	@echo "\tDOLPHIN external"
	@$(ASSETS_COMPILER) dolphin "$(DOLPHIN_SOURCE_DIR)/external" "$(DOLPHIN_EXTERNAL_OUTPUT_DIR)"

.PHONY: manifest
manifest:
	$(ASSETS_COMPILER) manifest $(RESOURCES_DIR)

.PHONY: dolphin
dolphin: $(DOLPHIN_EXTERNAL_OUTPUT_DIR)

.PHONY: copro_bundle
copro_bundle:
	@mkdir -p $(COPRO_BUNDLE_DIR)
	@$(ASSETS_COMPILER) copro $(COPRO_CUBE_DIR) $(COPRO_BUNDLE_DIR) $(COPRO_MCU_FAMILY) --cube_ver=$(COPRO_CUBE_VERSION) --stack_type=$(COPRO_STACK_TYPE) --stack_file=$(COPRO_STACK_BIN) --stack_addr=$(COPRO_STACK_ADDR)

clean:
	@echo "\tCLEAN\t"
	@$(RM) $(ASSETS_COMPILED_DIR)/*
	@$(RM) -rf $(COPRO_BUNDLE_DIR)
	@$(RM) -rf $(DOLPHIN_EXTERNAL_OUTPUT_DIR)
