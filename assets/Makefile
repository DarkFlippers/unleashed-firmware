PROJECT_ROOT		= $(abspath $(dir $(abspath $(firstword $(MAKEFILE_LIST))))..)

include				$(PROJECT_ROOT)/assets/assets.mk

all_assets: $(ASSETS) $(PROTOBUF)

$(ASSETS): $(ASSETS_SOURCES) $(ASSETS_COMPILLER)
	@echo "\tASSETS\t\t" $@
	@$(ASSETS_COMPILLER) icons "$(ASSETS_SOURCE_DIR)" "$(ASSETS_COMPILED_DIR)"

$(PROTOBUF) &: $(PROTOBUF_SOURCES) $(PROTOBUF_COMPILER)
	@echo "\tPROTOBUF\t" $(PROTOBUF_FILENAMES)
	@$(PROJECT_ROOT)/lib/nanopb/generator/nanopb_generator.py -q -I$(PROTOBUF_SOURCE_DIR) -D$(PROTOBUF_COMPILED_DIR) $(PROTOBUF_SOURCES)

clean:
	@echo "\tCLEAN\t"
	@$(RM) $(ASSETS)
