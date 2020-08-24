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
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =  \
Src/main.c

CPP_SOURCES = ../core/app.cpp

C_SOURCES += ../core/debug.c
C_SOURCES += ../core/furi.c
C_SOURCES += ../core/furi_ac.c
C_SOURCES += Src/flipper_hal.c
C_SOURCES += Src/lo_os.c
C_SOURCES += Src/lo_hal.c

C_SOURCES += ../applications/tests/furiac_test.c
C_SOURCES += ../applications/tests/furi_record_test.c
C_SOURCES += ../applications/tests/test_index.c

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
# CFLAGS
#######################################

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32L476xx \
-DBUTON_INVERT=false \
-DDEBUG_UART=huart1

# C includes
C_INCLUDES =  \
-IInc \
-I../applications \
-I../core

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
LIBS = -lc -lm
LIBDIR = 
LDFLAGS = $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -pthread

# default action: build all
all: $(BUILD_DIR)/$(TARGET)


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

$(BUILD_DIR)/$(TARGET): $(OBJECTS) Makefile
	$(CPP) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
