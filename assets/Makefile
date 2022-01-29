PROJECT_ROOT		= $(abspath $(dir $(abspath $(firstword $(MAKEFILE_LIST))))..)

include				$(PROJECT_ROOT)/assets/assets.mk

.PHONY: all
all: icons protobuf dolphin

$(ASSETS): $(ASSETS_SOURCES) $(ASSETS_COMPILLER)
	@echo "\tASSETS\t\t" $@
	@$(ASSETS_COMPILLER) icons "$(ASSETS_SOURCE_DIR)" "$(ASSETS_COMPILED_DIR)"

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
	@$(ASSETS_COMPILLER) dolphin -s dolphin_blocking "$(DOLPHIN_SOURCE_DIR)/blocking" "$(DOLPHIN_INTERNAL_OUTPUT_DIR)"
	@echo "\tDOLPHIN internal"
	@$(ASSETS_COMPILLER) dolphin -s dolphin_internal "$(DOLPHIN_SOURCE_DIR)/internal" "$(DOLPHIN_INTERNAL_OUTPUT_DIR)"
	@echo "\tDOLPHIN external"
	@$(ASSETS_COMPILLER) dolphin "$(DOLPHIN_SOURCE_DIR)/external" "$(DOLPHIN_EXTERNAL_OUTPUT_DIR)"

.PHONY: dolphin
dolphin: $(DOLPHIN_EXTERNAL_OUTPUT_DIR)

clean:
	@echo "\tCLEAN\t"
	@$(RM) $(ASSETS_COMPILED_DIR)/*
	@$(RM) -rf $(DOLPHIN_EXTERNAL_OUTPUT_DIR)
