OBJ_DIR := $(OBJ_DIR)/$(TARGET)

# Include source folder paths to virtual paths
VPATH = $(sort $(dir $(C_SOURCES)) $(dir $(ASM_SOURCES)) $(dir $(CPP_SOURCES)))

# Gather object
OBJECTS = $(addprefix $(OBJ_DIR)/, $(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(OBJ_DIR)/, $(notdir $(ASM_SOURCES:.s=.o)))
OBJECTS += $(addprefix $(OBJ_DIR)/, $(notdir $(CPP_SOURCES:.cpp=.o)))

# Generate dependencies
DEPS = $(OBJECTS:.o=.d)

$(shell test -d $(OBJ_DIR) || mkdir -p $(OBJ_DIR))

BUILD_FLAGS_SHELL=\
	echo "$(CFLAGS)" > $(OBJ_DIR)/BUILD_FLAGS.tmp; \
	diff $(OBJ_DIR)/BUILD_FLAGS $(OBJ_DIR)/BUILD_FLAGS.tmp 2>/dev/null \
		&& ( echo "CFLAGS ok"; rm $(OBJ_DIR)/BUILD_FLAGS.tmp) \
		|| ( echo "CFLAGS has been changed"; mv $(OBJ_DIR)/BUILD_FLAGS.tmp $(OBJ_DIR)/BUILD_FLAGS )
$(info $(shell $(BUILD_FLAGS_SHELL)))

CHECK_AND_REINIT_SUBMODULES_SHELL=\
	if git submodule status | egrep -q '^[-]|^[+]' ; then \
		echo "INFO: Need to reinitialize git submodules"; \
		git submodule update --init; \
	fi
$(info $(shell $(CHECK_AND_REINIT_SUBMODULES_SHELL)))

all: $(OBJ_DIR)/$(PROJECT).elf $(OBJ_DIR)/$(PROJECT).hex $(OBJ_DIR)/$(PROJECT).bin

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

$(OBJ_DIR)/%.o: %.c $(OBJ_DIR)/BUILD_FLAGS $(ASSETS)
	@echo "\tCC\t" $< "->" $@
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.s $(OBJ_DIR)/BUILD_FLAGS $(ASSETS)
	@echo "\tASM\t" $< "->" $@
	@$(AS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp $(OBJ_DIR)/BUILD_FLAGS $(ASSETS)
	@echo "\tCPP\t" $< "->" $@
	@$(CPP) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJ_DIR)/flash: $(OBJ_DIR)/$(PROJECT).bin
	openocd $(OPENOCD_OPTS) -c "program $(OBJ_DIR)/$(PROJECT).bin reset exit $(FLASH_ADDRESS)" 
	touch $@

$(OBJ_DIR)/upload: $(OBJ_DIR)/$(PROJECT).bin
	dfu-util -D $(OBJ_DIR)/$(PROJECT).bin -a 0 -s $(FLASH_ADDRESS)
	touch $@

$(ASSETS): $(ASSETS_SOURCES)
	@echo "\tASSETS\t" $@
	@$(ASSETS_COMPILLER) icons -s $(ASSETS_SOURCE_DIR) -o $(ASSETS_OUTPUT_DIR)

flash: $(OBJ_DIR)/flash

upload: $(OBJ_DIR)/upload

debug: flash
	arm-none-eabi-gdb \
		-ex 'target extended-remote | openocd -c "gdb_port pipe" $(OPENOCD_OPTS)' \
		-ex "set confirm off" \
		-ex "source ../debug/FreeRTOS/FreeRTOS.py" \
		-ex "source ../debug/PyCortexMDebug/scripts/gdb.py" \
		-ex "svd_load $(SVD_FILE)" \
		-ex "compare-sections" \
		$(OBJ_DIR)/$(PROJECT).elf; \

bm_debug: flash
	set -m; blackmagic & echo $$! > $(OBJ_DIR)/agent.PID
	arm-none-eabi-gdb \
		-ex "target extended-remote 127.0.0.1:2000" \
		-ex "set confirm off" \
		-ex "monitor debug_bmp enable"\
		-ex "monitor swdp_scan"\
		-ex "attach 1"\
		-ex "source ../debug/FreeRTOS/FreeRTOS.py" \
		$(OBJ_DIR)/$(PROJECT).elf; \
	kill `cat $(OBJ_DIR)/agent.PID`; \
	rm $(OBJ_DIR)/agent.PID

clean:
	@echo "\tCLEAN\t"
	@$(RM) $(OBJ_DIR)/*
	@$(RM) $(ASSETS)

z: clean
	$(MAKE) all

zz: clean
	$(MAKE) flash

zzz: clean
	$(MAKE) debug

FORMAT_SOURCES := $(shell find ../applications -iname "*.h" -o -iname "*.c" -o -iname "*.cpp")
FORMAT_SOURCES += $(shell find ../bootloader -iname "*.h" -o -iname "*.c" -o -iname "*.cpp")
FORMAT_SOURCES += $(shell find ../core -iname "*.h" -o -iname "*.c" -o -iname "*.cpp")

format:
	@echo "Formatting sources with clang-format"
	@clang-format -style=file -i $(FORMAT_SOURCES)

-include $(DEPS)
