PROJECT			= bootloader

SRC_DIR			= src
OBJ_DIR			= .obj

ASM_SOURCES		= $(wildcard $(SRC_DIR)/*.s)
C_SOURCES		= $(wildcard $(SRC_DIR)/*.c)
CPP_SOURCES		= $(wildcard $(SRC_DIR)/*.cpp)

# 
TARGET			?= f2
TARGET_DIR		= targets/$(TARGET)
include			$(TARGET_DIR)/target.mk
CFLAGS			+= -Itargets/include
C_SOURCES		+= $(wildcard $(TARGET_DIR)/*.c)

DEBUG ?= 1
ifeq ($(DEBUG), 1)
CFLAGS += -DDEBUG -g
else
CFLAGS += -DNDEBUG -Os
endif

PREFIX = arm-none-eabi-
ifdef GCC_PATH
CC	= $(GCC_PATH)/$(PREFIX)gcc
CPP	= $(GCC_PATH)/$(PREFIX)g++
AS	= $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP	= $(GCC_PATH)/$(PREFIX)objcopy
SZ	= $(GCC_PATH)/$(PREFIX)size
else
CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(PREFIX)gcc -x assembler-with-cpp
CP	= $(PREFIX)objcopy
SZ	= $(PREFIX)size
endif
HEX	= $(CP) -O ihex
BIN	= $(CP) -O binary -S

$(shell mkdir -p $(OBJ_DIR))

OBJECTS = $(addprefix $(OBJ_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

OBJECTS += $(addprefix $(OBJ_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

OBJECTS += $(addprefix $(OBJ_DIR)/,$(notdir $(CPP_SOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))

DEPS = $(OBJECTS:.o=.d)

CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"
CPPFLAGS = -fno-threadsafe-statics

all: $(OBJ_DIR)/$(PROJECT).elf $(OBJ_DIR)/$(PROJECT).hex $(OBJ_DIR)/$(PROJECT).bin

$(OBJ_DIR)/$(PROJECT).elf: $(OBJECTS)
	@echo "\tLD\t" $@
	@$(CC) $(LDFLAGS) $(OBJECTS) -o $@
	$(SZ) $@

$(OBJ_DIR)/$(PROJECT).hex: $(OBJ_DIR)/$(PROJECT).elf
	@echo "\tHEX\t" $@
	@$(HEX) $< $@
	
$(OBJ_DIR)/$(PROJECT).bin: $(OBJ_DIR)/$(PROJECT).elf
	@echo "\tBIN\t" $@
	@$(BIN) $< $@

$(OBJ_DIR)/%.o: %.c
	@echo "\tCC\t" $@
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.s
	@echo "\tASM\t" $@
	@$(AS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp
	@echo "\tCPP\t" $@
	@$(CPP) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

flash: $(OBJ_DIR)/$(PROJECT).bin
	st-flash --reset write $(OBJ_DIR)/$(PROJECT).bin $(BOOT_ADDRESS)

debug:
	st-util & arm-none-eabi-gdb -ex "PROJECT extended-remote 127.0.0.1:4242" $(OBJ_DIR)/$(PROJECT).elf

clean:
	$(RM) $(OBJ_DIR)/*

zz: | clean flash

zzz: | clean flash debug

-include $(DEPS)
