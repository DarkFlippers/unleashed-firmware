PROJECT_ROOT		= $(abspath $(dir $(abspath $(firstword $(MAKEFILE_LIST))))..)

include				$(PROJECT_ROOT)/assets/assets.mk

all: icons protobuf

$(ASSETS): $(ASSETS_SOURCES) $(ASSETS_COMPILLER)
	@echo "\tASSETS\t\t" $@
	@$(ASSETS_COMPILLER) icons "$(ASSETS_SOURCE_DIR)" "$(ASSETS_COMPILED_DIR)"

icons: $(ASSETS)

$(PROTOBUF) &: $(PROTOBUF_SOURCES) $(PROTOBUF_COMPILER)
	@echo "\tPROTOBUF\t" $(PROTOBUF_FILENAMES)
	@$(PROJECT_ROOT)/lib/nanopb/generator/nanopb_generator.py -q -I$(PROTOBUF_SOURCE_DIR) -D$(PROTOBUF_COMPILED_DIR) $(PROTOBUF_SOURCES)

protobuf: $(PROTOBUF)

clean:
	@echo "\tCLEAN\t"
	@$(RM) $(ASSETS_COMPILED_DIR)/*
