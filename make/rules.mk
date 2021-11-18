OBJ_DIR := $(OBJ_DIR)/$(TARGET)

# Include source folder paths to virtual paths
VPATH = $(sort $(dir $(C_SOURCES)) $(dir $(ASM_SOURCES)) $(dir $(CPP_SOURCES)))

# Gather object
OBJECTS = $(addprefix $(OBJ_DIR)/, $(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(OBJ_DIR)/, $(notdir $(ASM_SOURCES:.s=.o)))
OBJECTS += $(addprefix $(OBJ_DIR)/, $(notdir $(CPP_SOURCES:.cpp=.o)))

# Generate dependencies
DEPS = $(OBJECTS:.o=.d)

ifdef DFU_SERIAL
	DFU_OPTIONS += -S $(DFU_SERIAL)
endif

$(shell test -d $(OBJ_DIR) || mkdir -p $(OBJ_DIR))

BUILD_FLAGS_SHELL=\
	echo "$(CFLAGS)" > $(OBJ_DIR)/BUILD_FLAGS.tmp; \
	diff -u $(OBJ_DIR)/BUILD_FLAGS $(OBJ_DIR)/BUILD_FLAGS.tmp 2>&1 > /dev/null \
		&& ( echo "CFLAGS ok"; rm $(OBJ_DIR)/BUILD_FLAGS.tmp) \
		|| ( echo "CFLAGS has been changed"; mv $(OBJ_DIR)/BUILD_FLAGS.tmp $(OBJ_DIR)/BUILD_FLAGS )
$(info $(shell $(BUILD_FLAGS_SHELL)))

CHECK_AND_REINIT_SUBMODULES_SHELL=\
	if git submodule status | egrep -q '^[-]|^[+]' ; then \
		echo "INFO: Need to reinitialize git submodules"; \
		git submodule sync; \
		git submodule update --init; \
	fi
$(info $(shell $(CHECK_AND_REINIT_SUBMODULES_SHELL)))

all: $(OBJ_DIR)/$(PROJECT).elf $(OBJ_DIR)/$(PROJECT).hex $(OBJ_DIR)/$(PROJECT).bin $(OBJ_DIR)/$(PROJECT).dfu $(OBJ_DIR)/$(PROJECT).json
	@:

$(OBJ_DIR)/$(PROJECT).elf: $(OBJECTS)
	@echo "\tLD\t" $@
	@$(LD) $(LDFLAGS) $(OBJECTS) -o $@
	@$(SZ) $@

$(OBJ_DIR)/$(PROJECT).hex: $(OBJ_DIR)/$(PROJECT).elf
	@echo "\tHEX\t" $@
	@$(HEX) $< $@
	
$(OBJ_DIR)/$(PROJECT).bin: $(OBJ_DIR)/$(PROJECT).elf
	@echo "\tBIN\t" $@
	@$(BIN) $< $@

$(OBJ_DIR)/$(PROJECT).dfu: $(OBJ_DIR)/$(PROJECT).bin
	@echo "\tDFU\t" $@
	@../scripts/bin2dfu.py \
		-i $(OBJ_DIR)/$(PROJECT).bin \
		-o $(OBJ_DIR)/$(PROJECT).dfu \
		-a $(FLASH_ADDRESS) \
		-l "Flipper Zero $(shell echo $(TARGET) | tr a-z A-Z)" > /dev/null

$(OBJ_DIR)/$(PROJECT).json: $(OBJ_DIR)/$(PROJECT).dfu
	@echo "\tJSON\t" $@
	@../scripts/meta.py generate -p $(PROJECT) $(CFLAGS) > $(OBJ_DIR)/$(PROJECT).json

$(OBJ_DIR)/%.o: %.c $(OBJ_DIR)/BUILD_FLAGS
	@echo "\tCC\t" $(subst $(PROJECT_ROOT)/,,$(realpath $<)) "->" $@
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.s $(OBJ_DIR)/BUILD_FLAGS
	@echo "\tASM\t" $(subst $(PROJECT_ROOT)/,,$(realpath $<)) "->" $@
	@$(AS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp $(OBJ_DIR)/BUILD_FLAGS
	@echo "\tCPP\t" $(subst $(PROJECT_ROOT)/,,$(realpath $<)) "->" $@
	@$(CPP) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJ_DIR)/flash: $(OBJ_DIR)/$(PROJECT).bin
	openocd $(OPENOCD_OPTS) -c "program $(OBJ_DIR)/$(PROJECT).bin reset exit $(FLASH_ADDRESS)" 
	touch $@

$(OBJ_DIR)/upload: $(OBJ_DIR)/$(PROJECT).bin
	dfu-util -D $(OBJ_DIR)/$(PROJECT).bin -a 0 -s $(FLASH_ADDRESS) $(DFU_OPTIONS)
	touch $@

flash: $(OBJ_DIR)/flash

upload: $(OBJ_DIR)/upload

debug: flash
	arm-none-eabi-gdb-py \
		-ex 'target extended-remote | openocd -c "gdb_port pipe" $(OPENOCD_OPTS)' \
		-ex "set confirm off" \
		-ex "source ../debug/FreeRTOS/FreeRTOS.py" \
		-ex "source ../debug/PyCortexMDebug/PyCortexMDebug.py" \
		-ex "svd_load $(SVD_FILE)" \
		-ex "compare-sections" \
		$(OBJ_DIR)/$(PROJECT).elf; \

debug_other:
	arm-none-eabi-gdb-py \
		-ex 'target extended-remote | openocd -c "gdb_port pipe" $(OPENOCD_OPTS)' \
		-ex "set confirm off" \
		-ex "source ../debug/PyCortexMDebug/PyCortexMDebug.py" \
		-ex "svd_load $(SVD_FILE)" \


blackmagic:
	arm-none-eabi-gdb-py \
		-ex 'target extended-remote $(BLACKMAGIC)' \
		-ex 'monitor swdp_scan' \
		-ex 'monitor debug_bmp enable' \
		-ex 'attach 1' \
		-ex "set confirm off" \
		-ex "source ../debug/FreeRTOS/FreeRTOS.py" \
		-ex "source ../debug/PyCortexMDebug/PyCortexMDebug.py" \
		-ex "svd_load $(SVD_FILE)" \
		-ex "compare-sections" \
		$(OBJ_DIR)/$(PROJECT).elf; \

openocd:
	openocd $(OPENOCD_OPTS)

clean:
	@echo "\tCLEAN\t"
	@$(RM) $(OBJ_DIR)/*

z: clean
	$(MAKE) all

zz: clean
	$(MAKE) flash

zzz: clean
	$(MAKE) debug

generate_cscope_db:
	@echo "$(C_SOURCES) $(CPP_SOURCES) $(ASM_SOURCES)" | tr ' ' '\n' > $(OBJ_DIR)/source.list.p
	@cat ~/headers.list >> $(OBJ_DIR)/source.list.p
	@cat $(OBJ_DIR)/source.list.p | sed -e "s|^[^//]|$$PWD/&|g" > $(OBJ_DIR)/source.list
	@cscope -b -k -i $(OBJ_DIR)/source.list -f $(OBJ_DIR)/cscope.out
	@rm -rf $(OBJ_DIR)/source.list $(OBJ_DIR)/source.list.p

# Prevent make from trying to find .d targets
%.d: ;

-include $(DEPS)
