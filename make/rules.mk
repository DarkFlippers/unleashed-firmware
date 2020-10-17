OBJ_DIR := $(OBJ_DIR)/$(TARGET)

# Include source folder paths to virtual paths
VPATH = $(sort $(dir $(C_SOURCES)) $(dir $(ASM_SOURCES)) $(dir $(CPP_SOURCES)))

# Gather object
OBJECTS = $(addprefix $(OBJ_DIR)/, $(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(OBJ_DIR)/, $(notdir $(ASM_SOURCES:.s=.o)))
OBJECTS += $(addprefix $(OBJ_DIR)/, $(notdir $(CPP_SOURCES:.cpp=.o)))

# Generate dependencies
DEPS = $(OBJECTS:.o=.d)

$(shell mkdir -p $(OBJ_DIR))

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
	@$(CC) $(LDFLAGS) $(OBJECTS) -o $@
	@$(SZ) $@

$(OBJ_DIR)/$(PROJECT).hex: $(OBJ_DIR)/$(PROJECT).elf
	@echo "\tHEX\t" $@
	@$(HEX) $< $@
	
$(OBJ_DIR)/$(PROJECT).bin: $(OBJ_DIR)/$(PROJECT).elf
	@echo "\tBIN\t" $@
	@$(BIN) $< $@

$(OBJ_DIR)/%.o: %.c $(OBJ_DIR)/BUILD_FLAGS
	@echo "\tCC\t" $@
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.s $(OBJ_DIR)/BUILD_FLAGS
	@echo "\tASM\t" $@
	@$(AS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp $(OBJ_DIR)/BUILD_FLAGS
	@echo "\tCPP\t" $@
	@$(CPP) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJ_DIR)/flash: $(OBJ_DIR)/$(PROJECT).bin
	st-flash --reset write $(OBJ_DIR)/$(PROJECT).bin $(FLASH_ADDRESS)
	touch $@

$(OBJ_DIR)/upload: $(OBJ_DIR)/$(PROJECT).bin
	dfu-util -D $(OBJ_DIR)/$(PROJECT).bin -a 0 -s $(FLASH_ADDRESS)
	touch $@

flash: $(OBJ_DIR)/flash

upload: $(OBJ_DIR)/upload

debug: flash
	set -m; st-util -n --semihosting & echo $$! > $(OBJ_DIR)/st-util.PID
	arm-none-eabi-gdb-py \
		-ex "target extended-remote 127.0.0.1:4242" \
		-ex "set confirm off" \
		-ex "source ../debug/FreeRTOS/FreeRTOS.py" \
		$(OBJ_DIR)/$(PROJECT).elf; \
	kill `cat $(OBJ_DIR)/st-util.PID`; \
	rm $(OBJ_DIR)/st-util.PID

clean:
	@echo "\tCLEAN\t"
	@$(RM) $(OBJ_DIR)/*

z: clean
	$(MAKE) all

zz: clean
	$(MAKE) flash

zzz: clean
	$(MAKE) debug

FORMAT_SOURCES := $(shell find ../applications -iname "*.h" -o -iname "*.c" -o -iname "*.cpp")
FORMAT_SOURCES += $(shell find ../core -iname "*.h" -o -iname "*.c" -o -iname "*.cpp")

format:
	@echo "Formatting sources with clang-format"
	@clang-format -style=file -i $(FORMAT_SOURCES)

-include $(DEPS)
