TOOLCHAIN = x86

# Sources
C_SOURCES += $(TARGET_DIR)/Src/main.c
C_SOURCES += $(TARGET_DIR)/Src/flipper_hal.c
C_SOURCES += $(TARGET_DIR)/Src/lo_os.c
C_SOURCES += $(TARGET_DIR)/Src/lo_hal.c

# CFLAGS += -DFURI_DEBUG
CFLAGS += -I$(TARGET_DIR)/Inc
CFLAGS += -Wall -fdata-sections -ffunction-sections -pthread
LDFLAGS += -pthread

run: all
	$(OBJ_DIR)/$(PROJECT).elf