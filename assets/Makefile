PROJECT_ROOT		= $(abspath $(dir $(abspath $(firstword $(MAKEFILE_LIST))))..)

include				$(PROJECT_ROOT)/assets/assets.mk

$(ASSETS): $(ASSETS_SOURCES) $(ASSETS_COMPILLER)
	@echo "\tASSETS\t" $@
	@$(ASSETS_COMPILLER) icons "$(ASSETS_SOURCE_DIR)" "$(ASSETS_COMPILED_DIR)"

clean:
	@echo "\tCLEAN\t"
	@$(RM) $(ASSETS)
