# Compiller 
ifeq ($(TOOLCHAIN), arm)
PREFIX = arm-none-eabi-
ifdef GCC_PATH
PREFIX = $(GCC_PATH)/$(PREFIX)
endif
endif

CC	= $(PREFIX)gcc
CPP	= $(PREFIX)g++
AS	= $(PREFIX)gcc -x assembler-with-cpp
CP	= $(PREFIX)objcopy
SZ	= $(PREFIX)size
HEX	= $(CP) -O ihex
BIN	= $(CP) -O binary -S

DEBUG ?= 1
ifeq ($(DEBUG), 1)
CFLAGS += -DDEBUG -g
else
CFLAGS += -DNDEBUG -Os
endif

CFLAGS		+= -MMD -MP -MF"$(@:%.o=%.d)"
CPPFLAGS	+= -fno-threadsafe-statics
LDFLAGS		+= -Wl,-Map=$(OBJ_DIR)/$(PROJECT).map,--cref -Wl,--gc-sections -Wl,--undefined=uxTopUsedPriority
