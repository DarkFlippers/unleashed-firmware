OS := $(shell uname -s)

# Compiller 
ifeq ($(TOOLCHAIN), arm)
PREFIX = arm-none-eabi-
ifdef GCC_PATH
PREFIX := $(GCC_PATH)/$(PREFIX)
endif
endif

CCACHE := $(shell which ccache)

CC	= $(CCACHE) $(PREFIX)gcc -std=gnu17
CPP	= $(CCACHE) $(PREFIX)g++ -std=gnu++17
LD	= $(PREFIX)g++
AS	= $(PREFIX)gcc -x assembler-with-cpp
CP	= $(PREFIX)objcopy
SZ	= $(PREFIX)size
HEX	= $(CP) -O ihex
BIN	= $(CP) -O binary -S


ifeq ($(OS), Darwin)
GDB	= gdb
else 
GDB	= $(PREFIX)gdb-py
endif

DEBUG ?= 1
COMPACT ?= 0
ifeq ($(DEBUG), 1)
CFLAGS += -DFURI_DEBUG -DNDEBUG -Og -g
else ifeq ($(COMPACT), 1)
CFLAGS += -DFURI_NDEBUG -DNDEBUG -Os -g
else
CFLAGS += -DFURI_NDEBUG -DNDEBUG -Og -g
endif

CFLAGS		+= -fdata-sections -ffunction-sections -fno-math-errno -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)"
CPPFLAGS	+= -fno-threadsafe-statics -fno-use-cxa-atexit -fno-exceptions -fno-rtti
LDFLAGS		+= -Wl,-Map=$(OBJ_DIR)/$(PROJECT).map,--cref -Wl,--gc-sections -Wl,--undefined=uxTopUsedPriority -n
