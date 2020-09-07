TARGET = target_lo


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og


#######################################
# paths
#######################################
PROJECT_DIR = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
# Build path
BUILD_DIR = build

######################################
# source
######################################
C_SOURCES =
CPP_SOURCES =
C_DEFS =

# Target 

C_SOURCES += Src/main.c
C_SOURCES += Src/flipper_hal.c
C_SOURCES += Src/lo_os.c
C_SOURCES += Src/lo_hal.c

# C_DEFS += -DFURI_DEBUG

# Core

CPP_SOURCES += ../core/app.cpp

C_SOURCES += ../core/log.c
C_SOURCES += ../core/tty_uart.c
C_SOURCES += ../core/furi.c
C_SOURCES += ../core/furi_ac.c

# System applications

ifeq ($(TEST), 1)
C_SOURCES += ../applications/tests/furiac_test.c
C_SOURCES += ../applications/tests/furi_record_test.c
C_SOURCES += ../applications/tests/test_index.c
C_DEFS += -DTEST
endif

# Examples

ifeq ($(EXAMPLE_BLINK), 1)
C_SOURCES += ../applications/examples/blink.c
C_DEFS += -DEXAMPLE_BLINK
endif

ifeq ($(EXAMPLE_UART_WRITE), 1)
C_SOURCES += ../applications/examples/uart_write.c
C_DEFS += -DEXAMPLE_UART_WRITE
endif

ifeq ($(EXAMPLE_IPC), 1)
C_SOURCES += ../applications/examples/ipc.c
C_DEFS += -DEXAMPLE_IPC
endif

# User application

# Add C_SOURCES +=, C_DEFS += or CPP_SOURCES += here

#######################################
# binaries
#######################################

CC = gcc
CPP = g++
AS =
CP = objcopy
SZ = size
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S


#######################################
# Rust library
#######################################

RUST_LIB_SRC = $(realpath $(PROJECT_DIR)/../core-rs)
RUST_LIB_NAME = flipper_core
RUST_LIB_TARGET = x86_64-unknown-linux-gnu
RUST_LIB_FLAGS = --target=$(RUST_LIB_TARGET)

ifeq ($(DEBUG), 1)
    RUST_LIB_PATH = $(RUST_LIB_SRC)/target/$(RUST_LIB_TARGET)/debug
else
	RUST_LIB_FLAGS += --release
    RUST_LIB_PATH = $(RUST_LIB_SRC)/target/$(RUST_LIB_TARGET)/release
endif

RUST_LIB_CMD = cd $(RUST_LIB_SRC) && cargo build -p flipper-core $(RUST_LIB_FLAGS)

#######################################
# CFLAGS
#######################################


# C includes
C_INCLUDES =  \
-IInc \
-I../applications \
-I../core \
-I../core-rs/flipper-core/bindings

# compile gcc flags
CFLAGS = $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -pthread

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

CPPFLAGS = -fno-threadsafe-statics

#######################################
# LDFLAGS
#######################################

# libraries
LIBS = -lc -lm -l$(RUST_LIB_NAME)
LIBDIR = -L$(RUST_LIB_PATH)
LDFLAGS = $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -pthread

# default action: build all
all: $(BUILD_DIR)/$(TARGET)

rust_lib:
	$(RUST_LIB_CMD)

example_blink:
	rm -f $(BUILD_DIR)/app.o
	EXAMPLE_BLINK=1 make
	$(BUILD_DIR)/$(TARGET)


example_uart_write:
	rm -f $(BUILD_DIR)/app.o
	EXAMPLE_UART_WRITE=1 make
	$(BUILD_DIR)/$(TARGET)

example_ipc:
	rm -f $(BUILD_DIR)/app.o
	EXAMPLE_IPC=1 make
	$(BUILD_DIR)/$(TARGET)

test:
	rm -f $(BUILD_DIR)/app.o
	TEST=1 make
	timeout --signal=9 5 $(BUILD_DIR)/$(TARGET)

.PHONY: all rust_lib example_blink example_uart_write test

#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(CPP_SOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR)
	$(CPP) -c $(CFLAGS) $(CPPFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(RUST_LIB_PATH)/lib$(RUST_LIB_NAME).a: rust_lib

$(BUILD_DIR)/$(TARGET): $(RUST_LIB_PATH)/lib$(RUST_LIB_NAME).a $(OBJECTS) Makefile
	$(CPP) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
	cd $(RUST_LIB_SRC) && cargo clean

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
